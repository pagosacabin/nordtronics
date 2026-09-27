package com.nordtronics.companion;

import android.os.Bundle;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;

import java.util.ArrayList;
import java.util.List;

/**
 * Node detail — the v0.1 mockup's drill-down view (task 0049).
 *
 * <p>Readings (PM2.5, temperature, humidity, battery), the link/hardware bars,
 * the node's own recent history and the ping-test button. The health bars are
 * built from values the API actually returns — packet recency, PM2.5 headroom
 * and battery voltage — because neither payload carries RSSI or
 * packet-delivery figures.
 *
 * <p>Since task 0069 the node list is read through {@code ApiClient.nodesPath()}.
 * The alert history and the ping button have no production endpoint (the live
 * API serves neither), so those two panels say so and make no request unless
 * the build's backend — the debug build's mock — actually serves them.
 */
public class NodeDetailActivity extends AppCompatActivity {

    public static final String EXTRA_NODE_ID = "node_id";

    private TextView status;
    private TextView pingResult;
    private TextView detailName;
    private TextView detailMeta;
    private TextView rdPm;
    private TextView rdPmNote;
    private TextView rdTemp;
    private TextView rdTempNote;
    private TextView rdHumidity;
    private TextView rdHumidityNote;
    private TextView rdBattery;
    private TextView rdBatteryNote;
    private LinearLayout historyList;
    private TextView historyEmpty;
    private View detailIcon;

    private String nodeId = "node-01";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_node_detail);
        WindowInsetsHelper.applySystemBarInsets(this);

        status = findViewById(R.id.status);
        pingResult = findViewById(R.id.ping_result);
        detailName = findViewById(R.id.detail_name);
        detailMeta = findViewById(R.id.detail_meta);
        rdPm = findViewById(R.id.rd_pm);
        rdPmNote = findViewById(R.id.rd_pm_note);
        rdTemp = findViewById(R.id.rd_temp);
        rdTempNote = findViewById(R.id.rd_temp_note);
        rdHumidity = findViewById(R.id.rd_humidity);
        rdHumidityNote = findViewById(R.id.rd_humidity_note);
        rdBattery = findViewById(R.id.rd_battery);
        rdBatteryNote = findViewById(R.id.rd_battery_note);
        historyList = findViewById(R.id.history_list);
        historyEmpty = findViewById(R.id.history_empty);
        detailIcon = findViewById(R.id.detail_icon);

        String extra = getIntent().getStringExtra(EXTRA_NODE_ID);
        if (extra != null && !extra.isEmpty()) {
            nodeId = extra;
        }
        detailName.setText(nodeId);

        findViewById(R.id.btn_back).setOnClickListener(v -> {
            startActivity(new android.content.Intent(this, NodesActivity.class));
            finish();
        });
        findViewById(R.id.btn_ping).setOnClickListener(v -> sendPing());
        Ui.bindNav(this, R.id.nav_nodes);

        loadNode();
        loadHistory();
    }

    // ------------------------------------------------------------ node data

    private void loadNode() {
        status.setText("GET " + ApiClient.BASE_URL + ApiClient.nodesPath() + " \u2026");
        new Thread(() -> {
            try {
                JSONArray arr = ApiClient.getNodes();
                Node found = null;
                for (int i = 0; i < arr.length(); i++) {
                    Node n = new Node(arr.getJSONObject(i));
                    if (nodeId.equals(n.id)) {
                        found = n;
                        break;
                    }
                }
                final Node node = found;
                runOnUiThread(() -> {
                    if (node == null) {
                        detailMeta.setText("Node " + nodeId + " is not in " + ApiClient.nodesPath());
                        status.setText("GET " + ApiClient.BASE_URL + ApiClient.nodesPath()
                                + " \u2192 no node " + nodeId);
                    } else {
                        renderDetail(node);
                    }
                });
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    detailMeta.setText("Request failed");
                    status.setText("Request failed: " + e.getMessage());
                });
            }
        }, "node-fetch").start();
    }

    private void renderDetail(Node n) {
        boolean attention = n.attention();
        detailName.setText(n.displayName());
        detailMeta.setText(n.statusWord() + " \u00B7 " + n.lastSeenPhrase()
                + " \u00B7 last packet " + n.lastSeenStamp());
        detailIcon.setBackground(Ui.round(this,
                Ui.col(this, attention ? R.color.nt_warn_soft : R.color.nt_ok_soft), 13));

        boolean pmKnown = !Double.isNaN(n.pm25);
        rdPm.setText(pmKnown ? Ui.fmt1(n.pm25) + " \u00B5g/m\u00B3" : "—");
        rdPm.setTextColor(Ui.col(this, pmKnown && n.pm25 >= Node.PM25_WATCH
                ? R.color.nt_warn : R.color.nt_ink));
        rdPmNote.setText(pmKnown && n.pm25 >= Node.PM25_WATCH
                ? "Above the " + Ui.fmt0(Node.PM25_WATCH) + " \u00B5g/m\u00B3 watch level"
                : "Clean-air range");

        rdTemp.setText(Ui.fmt0(n.tempF()) + "\u00B0F");
        rdTempNote.setText(Double.isNaN(n.tempC) ? "Ambient"
                : Ui.fmt1(n.tempC) + " \u00B0C ambient");
        rdHumidity.setText(Ui.fmt0(n.humidity) + "% RH");
        rdHumidityNote.setText("Fire-weather context");
        rdBattery.setText(Ui.fmt2(n.batteryV) + " V");
        rdBatteryNote.setText(n.lowBattery() ? "Below the 3.60 V floor" : "Solar charging");

        Ui.setBar(findViewById(R.id.bar_recency), findViewById(R.id.bar_recency_spacer),
                (TextView) findViewById(R.id.val_recency), n.recencyPercent(),
                n.ageSeconds() < 0 ? "unknown" : Ui.ageLabel(n.ageSeconds()));
        Ui.setBar(findViewById(R.id.bar_headroom), findViewById(R.id.bar_headroom_spacer),
                (TextView) findViewById(R.id.val_headroom), n.pmHeadroomPercent(),
                Ui.fmt1(Math.max(0, Node.PM25_WATCH - n.pm25)) + " \u00B5g/m\u00B3");
        Ui.setBar(findViewById(R.id.bar_battery), findViewById(R.id.bar_battery_spacer),
                (TextView) findViewById(R.id.val_battery), n.batteryPercent(),
                n.batteryPercent() + "% \u00B7 " + Ui.fmt2(n.batteryV) + " V");

        status.setText("Readings from " + ApiClient.BASE_URL + ApiClient.nodesPath());
    }

    // -------------------------------------------------------- alert history

    private void loadHistory() {
        if (!ApiClient.alertsAvailable()) {
            // The production API serves no alerts feed, so there is nothing to
            // request: say so rather than firing a call that must 404.
            historyList.removeAllViews();
            historyEmpty.setText(ApiClient.ALERTS_UNAVAILABLE);
            historyEmpty.setVisibility(View.VISIBLE);
            return;
        }
        new Thread(() -> {
            try {
                JSONArray arr = ApiClient.getAlerts();
                final List<Alert> mine = new ArrayList<>();
                for (int i = 0; i < arr.length(); i++) {
                    Alert a = new Alert(arr.getJSONObject(i));
                    if (nodeId.equals(a.nodeId)) {
                        mine.add(a);
                    }
                }
                runOnUiThread(() -> renderHistory(mine));
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    historyList.removeAllViews();
                    historyEmpty.setText("History unavailable: " + e.getMessage());
                    historyEmpty.setVisibility(View.VISIBLE);
                });
            }
        }, "node-history-fetch").start();
    }

    private void renderHistory(List<Alert> mine) {
        historyList.removeAllViews();
        for (Alert a : mine) {
            historyList.addView(Ui.alertRow(this, a));
        }
        historyEmpty.setText("No alerts recorded for this node.");
        historyEmpty.setVisibility(mine.isEmpty() ? View.VISIBLE : View.GONE);
    }

    // ----------------------------------------------------------------- ping

    private void sendPing() {
        if (!ApiClient.pingAvailable()) {
            // No production endpoint to POST to; do not invent one.
            pingResult.setText(ApiClient.PING_UNAVAILABLE);
            status.setText("Ping unavailable on " + ApiClient.BASE_URL);
            return;
        }
        final String url = ApiClient.BASE_URL + ApiClient.pingPath(nodeId);
        pingResult.setText("POST " + url + " \u2026");
        status.setText("POST " + url + " \u2026");
        new Thread(() -> {
            try {
                final String body = ApiClient.postPing(nodeId);
                runOnUiThread(() -> {
                    pingResult.setText("Ping confirmed by server:\n" + body);
                    status.setText("POST " + url + " \u2192 200 OK");
                });
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    pingResult.setText("Ping failed: " + e.getMessage());
                    status.setText("POST " + url + " failed");
                });
            }
        }, "ping-post").start();
    }
}
