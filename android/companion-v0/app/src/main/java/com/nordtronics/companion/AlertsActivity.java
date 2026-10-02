package com.nordtronics.companion;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

/**
 * Screen 3 — Alerts, UI spec v2 (task 0090).
 *
 * <p>The consensus hero, the filter chips (All / Watch / Smoke / Battery / Heat
 * with their live counts), and the recent-activity feed with each item's
 * severity and state. The only interactive behaviour is the contract's state
 * machine: Acknowledge is offered on {@code active} items only, and
 * acknowledging one drops the nav badge, because the badge counts the
 * unacknowledged items.
 */
public class AlertsActivity extends AppCompatActivity {

    /** Optional starting filter ("all" | "watch" | "smoke" | "battery" | "heat"). */
    public static final String EXTRA_FILTER = "filter";

    private static final String FILTER_ALL = "all";

    private String filter = FILTER_ALL;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_alerts);
        Screens.shell(this);

        String extra = getIntent() == null ? null : getIntent().getStringExtra(EXTRA_FILTER);
        filter = extra == null || extra.isEmpty() ? FILTER_ALL : extra;
    }

    @Override
    protected void onResume() {
        super.onResume();
        Screens.renderAlerts(this, filter);
    }

    /** Filter-chip handler: repaint the feed for one alert type. */
    void setFilter(String key) {
        filter = key == null || key.isEmpty() ? FILTER_ALL : key;
        Screens.renderAlerts(this, filter);
    }

    /** The filter this screen is currently showing. */
    String currentFilter() {
        return filter;
    }
}
