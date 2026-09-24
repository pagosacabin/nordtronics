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
 * Alerts — the v0.1 mockup's alerts view (task 0049).
 *
 * <p>Consensus statement hero ("no active alerts" unless something inside the
 * active window is firing), a severity/type filter, then the alert history from
 * {@code GET /api/alerts}. Filter chips are built from the types the payload
 * actually contains plus the two named tones, so an unknown type still lands in
 * "All".
 */
public class AlertsActivity extends AppCompatActivity {

    private LinearLayout alertList;
    private TextView status;
    private TextView heroTitle;
    private TextView heroBody;
    private TextView heroState;
    private TextView emptyAlerts;

    private final List<Alert> alerts = new ArrayList<>();
    private String filter = "all";

    private TextView chipAll;
    private TextView chipSmoke;
    private TextView chipBattery;
    private TextView chipHeat;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_alerts);
        WindowInsetsHelper.applySystemBarInsets(this);

        alertList = findViewById(R.id.alert_list);
        status = findViewById(R.id.status);
        heroTitle = findViewById(R.id.alert_hero_title);
        heroBody = findViewById(R.id.alert_hero_body);
        heroState = findViewById(R.id.alert_hero_state);
        emptyAlerts = findViewById(R.id.empty_alerts);

        chipAll = findViewById(R.id.filter_all);
        chipSmoke = findViewById(R.id.filter_smoke);
        chipBattery = findViewById(R.id.filter_battery);
        chipHeat = findViewById(R.id.filter_heat);

        chipAll.setOnClickListener(v -> select("all"));
        chipSmoke.setOnClickListener(v -> select("smoke"));
        chipBattery.setOnClickListener(v -> select("battery"));
        chipHeat.setOnClickListener(v -> select("heat"));

        Ui.bindNav(this, R.id.nav_alerts);
        paintChips();
    }

    @Override
    protected void onResume() {
        super.onResume();
        load();
    }

    private void select(String key) {
        filter = key;
        paintChips();
        applyFilter();
    }

    private void load() {
        status.setText("GET " + ApiClient.BASE_URL + "/api/alerts \u2026");
        new Thread(() -> {
            try {
                JSONArray arr = new JSONArray(ApiClient.get("/api/alerts"));
                final List<Alert> fetched = new ArrayList<>();
                for (int i = 0; i < arr.length(); i++) {
                    fetched.add(new Alert(arr.getJSONObject(i)));
                }
                runOnUiThread(() -> render(fetched));
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    alertList.removeAllViews();
                    emptyAlerts.setVisibility(View.GONE);
                    status.setText("Request failed: " + e.getMessage());
                });
            }
        }, "alerts-fetch").start();
    }

    private void render(List<Alert> fetched) {
        alerts.clear();
        alerts.addAll(fetched);
        status.setText(alerts.size() + " alerts from " + ApiClient.BASE_URL + "/api/alerts");

        int active = 0;
        for (Alert a : alerts) {
            if (a.active()) {
                active++;
            }
        }
        if (active == 0) {
            heroTitle.setText("No active alerts");
            heroBody.setText("The network needs a matching rise across nearby nodes before escalating a smoke event.");
            heroState.setText("\u2713");
            heroState.setTextColor(Ui.col(this, R.color.nt_ok));
            heroState.setBackgroundResource(R.drawable.bg_disc_ok);
        } else {
            heroTitle.setText(active == 1 ? "1 active alert" : active + " active alerts");
            heroBody.setText("Something crossed a threshold in the last few hours. Check the affected nodes before escalating.");
            heroState.setText("!");
            heroState.setTextColor(Ui.col(this, R.color.nt_warn));
            heroState.setBackgroundResource(R.drawable.bg_disc_warn);
        }

        alertList.removeAllViews();
        for (Alert a : alerts) {
            View row = Ui.alertRow(this, a);
            row.setTag(a.typeKey());
            alertList.addView(row);
        }
        applyFilter();
    }

    private void applyFilter() {
        int shown = 0;
        for (int i = 0; i < alertList.getChildCount(); i++) {
            View row = alertList.getChildAt(i);
            boolean visible = "all".equals(filter) || filter.equals(row.getTag());
            row.setVisibility(visible ? View.VISIBLE : View.GONE);
            if (visible) {
                shown++;
            }
        }
        emptyAlerts.setVisibility(shown == 0 ? View.VISIBLE : View.GONE);
        if (shown == 0 && alertList.getChildCount() > 0) {
            emptyAlerts.setText("No " + filter + " alerts in the history.");
        }
    }

    private void paintChips() {
        paintChip(chipAll, "all");
        paintChip(chipSmoke, "smoke");
        paintChip(chipBattery, "battery");
        paintChip(chipHeat, "heat");
    }

    private void paintChip(TextView chip, String key) {
        boolean active = key.equals(filter);
        chip.setBackgroundResource(active ? R.drawable.bg_chip_active : 0);
        chip.setTextColor(Ui.col(this, active ? R.color.nt_ink : R.color.nt_muted));
    }
}
