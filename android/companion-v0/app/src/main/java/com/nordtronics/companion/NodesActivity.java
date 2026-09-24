package com.nordtronics.companion;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Nodes screen — the v0.1 mockup's "Property line" view (task 0049).
 *
 * <p>Everything on screen is derived from {@code GET /api/nodes}: the consensus
 * hero (how many nodes are reporting, how many need attention), the four stat
 * tiles (medians across the reporting nodes) and the node cards (status, PM2.5,
 * temperature, humidity, battery voltage, last seen). The All/Watch filter hides
 * cards that are inside their limits.
 *
 * <p>The data layer is the same mock backend 0045 wired up — the API base URL is
 * still the single {@code BuildConfig.API_BASE_URL} field and the payload shapes
 * are untouched. The screens label themselves "Prototype data" because that is
 * what they are.
 */
public class NodesActivity extends AppCompatActivity {

    public static final double PM25_WATCH = Node.PM25_WATCH;

    private LinearLayout nodeList;
    private TextView status;
    private TextView subtitle;
    private TextView heroEyebrow;
    private TextView heroTitle;
    private TextView heroBody;
    private TextView heroState;
    private TextView statPm;
    private TextView statTemp;
    private TextView statHumidity;
    private TextView statReporting;
    private TextView emptyNodes;
    private TextView filterAll;
    private TextView filterWatch;

    private final List<Node> nodes = new ArrayList<>();
    private boolean watchOnly = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_nodes);
        WindowInsetsHelper.applySystemBarInsets(this);

        nodeList = findViewById(R.id.node_list);
        status = findViewById(R.id.status);
        subtitle = findViewById(R.id.screen_subtitle);
        heroEyebrow = findViewById(R.id.hero_eyebrow);
        heroTitle = findViewById(R.id.hero_title);
        heroBody = findViewById(R.id.hero_body);
        heroState = findViewById(R.id.hero_state);
        statPm = findViewById(R.id.stat_pm);
        statTemp = findViewById(R.id.stat_temp);
        statHumidity = findViewById(R.id.stat_humidity);
        statReporting = findViewById(R.id.stat_reporting);
        emptyNodes = findViewById(R.id.empty_nodes);
        filterAll = findViewById(R.id.filter_all);
        filterWatch = findViewById(R.id.filter_watch);

        filterAll.setOnClickListener(v -> {
            watchOnly = false;
            paintChips();
            applyFilter();
        });
        filterWatch.setOnClickListener(v -> {
            watchOnly = true;
            paintChips();
            applyFilter();
        });

        // The "Prototype data" badge doubles as the refresh affordance; the
        // mockup's mobile layout has no toolbar to put a button in.
        findViewById(R.id.demo_badge).setOnClickListener(v -> load());

        Ui.bindNav(this, R.id.nav_nodes);
        paintChips();
    }

    @Override
    protected void onResume() {
        super.onResume();
        load();
    }

    private void load() {
        status.setText("GET " + ApiClient.BASE_URL + "/api/nodes \u2026");
        new Thread(() -> {
            try {
                JSONArray arr = new JSONArray(ApiClient.get("/api/nodes"));
                final List<Node> fetched = new ArrayList<>();
                for (int i = 0; i < arr.length(); i++) {
                    fetched.add(new Node(arr.getJSONObject(i)));
                }
                runOnUiThread(() -> render(fetched));
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    nodeList.removeAllViews();
                    emptyNodes.setVisibility(View.GONE);
                    status.setText("Request failed: " + e.getMessage());
                });
            }
        }, "nodes-fetch").start();
    }

    private void render(List<Node> fetched) {
        nodes.clear();
        nodes.addAll(fetched);
        status.setText(nodes.size() + " nodes from " + ApiClient.BASE_URL + "/api/nodes");

        int reporting = 0;
        int attention = 0;
        List<Double> pm = new ArrayList<>();
        List<Double> tempF = new ArrayList<>();
        List<Double> humidity = new ArrayList<>();
        long freshest = Long.MAX_VALUE;
        String freshestPhrase = "received at an unknown time";

        for (Node n : nodes) {
            if (n.ageSeconds() >= 0) {
                reporting++;
                if (n.ageSeconds() < freshest) {
                    freshest = n.ageSeconds();
                    freshestPhrase = n.lastSeenPhrase();
                }
            }
            if (n.attention()) {
                attention++;
            }
            if (!Double.isNaN(n.pm25)) {
                pm.add(n.pm25);
            }
            if (!Double.isNaN(n.tempF())) {
                tempF.add(n.tempF());
            }
            if (!Double.isNaN(n.humidity)) {
                humidity.add(n.humidity);
            }
        }

        subtitle.setText("Last packet " + freshestPhrase);
        statPm.setText(Ui.fmt0(median(pm)));
        statTemp.setText(Ui.fmt0(median(tempF)));
        statHumidity.setText(Ui.fmt0(median(humidity)));
        statReporting.setText(reporting + "/" + nodes.size());

        boolean clear = attention == 0;
        heroEyebrow.setText("Network consensus \u00B7 " + reporting + " of " + nodes.size());
        if (clear) {
            heroTitle.setText("All clear");
            heroBody.setText("Readings are stable across the property. No smoke pattern is forming.");
            heroState.setText("\u2713");
            heroState.setTextColor(Ui.col(this, R.color.nt_ok));
            heroState.setBackgroundResource(R.drawable.bg_disc_ok);
        } else {
            heroTitle.setText("Watch");
            heroBody.setText(attention == 1
                    ? "One node is outside its limits. The rest of the network is at baseline."
                    : attention + " nodes are outside their limits. Check them before escalating.");
            heroState.setText("!");
            heroState.setTextColor(Ui.col(this, R.color.nt_warn));
            heroState.setBackgroundResource(R.drawable.bg_disc_warn);
        }

        nodeList.removeAllViews();
        for (final Node n : nodes) {
            View card = Ui.nodeCard(this, n, v -> {
                Intent detail = new Intent(this, NodeDetailActivity.class);
                detail.putExtra(NodeDetailActivity.EXTRA_NODE_ID, n.id);
                startActivity(detail);
            });
            card.setTag(n.attention() ? "watch" : "ok");
            nodeList.addView(card);
        }
        applyFilter();
    }

    /** Shows or hides each card according to the All/Watch filter. */
    private void applyFilter() {
        int shown = 0;
        for (int i = 0; i < nodeList.getChildCount(); i++) {
            View card = nodeList.getChildAt(i);
            boolean visible = !watchOnly || "watch".equals(card.getTag());
            card.setVisibility(visible ? View.VISIBLE : View.GONE);
            if (visible) {
                shown++;
            }
        }
        emptyNodes.setVisibility(shown == 0 ? View.VISIBLE : View.GONE);
    }

    private void paintChips() {
        paintChip(filterAll, !watchOnly);
        paintChip(filterWatch, watchOnly);
    }

    private void paintChip(TextView chip, boolean active) {
        chip.setBackgroundResource(active ? R.drawable.bg_chip_active : 0);
        chip.setTextColor(Ui.col(this, active ? R.color.nt_ink : R.color.nt_muted));
    }

    private static double median(List<Double> values) {
        if (values.isEmpty()) {
            return Double.NaN;
        }
        Collections.sort(values);
        int mid = values.size() / 2;
        if (values.size() % 2 == 1) {
            return values.get(mid);
        }
        return (values.get(mid - 1) + values.get(mid)) / 2.0;
    }
}
