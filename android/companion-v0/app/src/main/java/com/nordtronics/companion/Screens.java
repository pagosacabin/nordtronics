package com.nordtronics.companion;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

/**
 * The three UI-spec-v2 screens (task 0090).
 *
 * <p>Everything here is built from {@link WildfireApi} data — no screen holds a
 * literal reading. The static labels are the spec's own wording, kept next to
 * the layout that uses them (the same convention {@link Ui} already follows for
 * its chips and severity words).
 *
 * <p>Spec: {@code docs/wildfire/app-ui-spec-v2-2026-10-02.md}. Where the spec
 * describes an element the app cannot own — the Android phone frame, the 7:31 /
 * 5G status bar — the device draws it, and that is the only deliberate
 * deviation, recorded in the staged reply.
 */
final class Screens {

    /**
     * UI spec v2: the thin banner on every screen. Since task 0096 it is shown
     * only while the bundled mock is answering — a live read leaves the banner
     * hidden, so mock data is never passed off as real.
     */
    static final String PROTOTYPE_BANNER = "PROTOTYPE \u2013 MOCK DATA";

    /** UI spec v2: the footer on every screen, for the mock fallback. */
    static final String FOOTER_MOCK =
            "Readings from local mock backend \u00B7 Privacy-first \u00B7 No cameras \u00B7 "
                    + "Locations stay on your network";

    /** The same footer with the live backend named, when the API answered. */
    static final String FOOTER_LIVE =
            "Readings from " + ApiClient.BASE_URL + " \u00B7 Privacy-first \u00B7 No cameras "
                    + "\u00B7 Locations stay on your network";

    /** UI spec v2 chart bands, labelled exactly as the spec writes them. */
    static final String BAND_CLEAN = "Clean < 12";
    static final String BAND_ELEVATED = "Elevated 12 \u2013 35";
    static final String BAND_HIGH = "> 35";

    private static final String NODE_ONE = "node-01";

    private Screens() {
    }

    // ------------------------------------------------------------------ shell

    /** Insets + the data-source banner; call right after setContentView. */
    static void shell(Activity a) {
        WindowInsetsHelper.applySystemBarInsets(a);
        sourceBanner(a);
    }

    /**
     * The "PROTOTYPE – MOCK DATA" banner, shown only while the bundled mock is
     * answering (task 0096). Called by {@link #shell(Activity)} and again after
     * each screen's read, so it follows a fallback that happens mid-session.
     */
    static void sourceBanner(Activity a) {
        TextView banner = a.findViewById(R.id.prototype_banner);
        if (banner != null) {
            boolean mock = ApiSource.usingMock();
            banner.setVisibility(mock ? View.VISIBLE : View.GONE);
            banner.setText(mock ? PROTOTYPE_BANNER : "");
        }
    }

    static LinearLayout content(Activity a) {
        return a.findViewById(R.id.screen_content);
    }

    static void clear(Activity a) {
        content(a).removeAllViews();
    }

    // ------------------------------------------------------- screen 1 render

    /**
     * Screen 1 — Property overview ("Property line"). Header, watch banner,
     * 2×2 metric cards, the property schematic and the field-node list.
     */
    static void propertyOverview(Activity a, List<NodeInfo> nodes, NetworkStatus status) {
        LinearLayout into = content(a);
        into.removeAllViews();

        into.addView(propertyHeader(a, "Property line", "Last packet received " + ageOf(status, nodes)));

        if (status != null) {
            into.addView(watchBanner(a, status, nodes));
        }

        NodeInfo one = find(nodes, NODE_ONE);
        List<Double> pm = new ArrayList<>();
        List<Double> tf = new ArrayList<>();
        List<Double> rh = new ArrayList<>();
        for (NodeInfo n : nodes) {
            if (!Double.isNaN(n.pm25)) {
                pm.add(n.pm25);
            }
            if (!Double.isNaN(n.tempF())) {
                tf.add(n.tempF());
            }
            if (!Double.isNaN(n.humidityPct)) {
                rh.add(n.humidityPct);
            }
        }

        // Property medians, computed across the reporting nodes exactly as the
        // spec's numbers read: median(12.4, 47.9) = 30 µg/m³,
        // median(70, 82) = 76 °F, median(38, 22) = 30 % RH.
        List<View> cards = new ArrayList<>();
        cards.add(metricCard(a, "PM2.5 median", Ui.fmt0(median(pm)), "\u00B5g/m\u00B3", null, 0,
                one == null ? null : NodeInfo.toArray(one.trendPm25), R.color.nt_blue));
        cards.add(metricCard(a, "Air temperature", Ui.fmt0(median(tf)), "\u00B0F", null, 0,
                one == null ? null : NodeInfo.toArray(fahrenheit(one.trendTemperatureC)),
                R.color.nt_blue));
        cards.add(metricCard(a, "Humidity", Ui.fmt0(median(rh)), "% RH", null, 0,
                one == null ? null : NodeInfo.toArray(one.trendHumidityPct), R.color.nt_blue));
        cards.add(metricCard(a, "Reporting", reportingOf(nodes), "nodes", null, 0,
                NodeInfo.toArray(flat(nodes.size(), 12)), R.color.nt_blue));
        into.addView(metricGrid(a, cards));

        // Property schematic.
        into.addView(sectionHead(a, "Property schematic", "Approximate layout"));
        LinearLayout scheme = card(a, R.color.nt_surface_2);
        scheme.addView(new PropertySchematicView(a,
                Ui.col(a, R.color.nt_ok), Ui.col(a, R.color.nt_warn), Ui.col(a, R.color.nt_text2)));
        into.addView(scheme);

        // Field nodes.
        into.addView(sectionHead(a, "Field nodes", null));
        for (NodeInfo n : nodes) {
            into.addView(fieldNodeCard(a, n));
        }

        into.addView(footer(a));
    }

    /** The alarm-orange-bordered watch banner: state word, note, reporting line. */
    private static View watchBanner(Activity a, NetworkStatus status, List<NodeInfo> nodes) {
        boolean alert = status.isAlert();
        int accent = Ui.col(a, alert ? R.color.nt_warn : R.color.nt_warn);

        LinearLayout box = card(a, R.color.nt_warn_soft);
        box.setBackground(bordered(a, R.color.nt_warn_soft, accent));

        LinearLayout top = Ui.row(a);
        top.setGravity(Gravity.CENTER_VERTICAL);
        TextView state = Ui.text(a, status.isWatch() ? "Watch" : "Alert", 21, accent, true);
        state.setLayoutParams(Ui.weight(1));
        top.addView(state);
        List<Double> rising = new ArrayList<>();
        for (int i = 0; i < 12; i++) {
            rising.add(80.0 + i * 1.6);
        }
        SparklineView spark = new SparklineView(a, NodeInfo.toArray(rising), accent, 22);
        top.addView(spark);
        box.addView(top);

        box.addView(Ui.text(a, status.bannerBody(), 13, Ui.col(a, R.color.nt_ink), false));
        TextView reporting = Ui.text(a, status.reportingLine(), 11, Ui.col(a, R.color.nt_ok), false);
        reporting.setPadding(0, Ui.dp(a, 6), 0, 0);
        box.addView(reporting);
        return box;
    }

    /** One field-node card: name, status line, then the four readings. */
    private static View fieldNodeCard(Activity a, NodeInfo n) {
        boolean attention = n.inWatch();
        int valueColor = Ui.col(a, attention ? R.color.nt_warn : R.color.nt_ink);

        LinearLayout box = card(a, R.color.nt_surface_2);

        LinearLayout head = Ui.row(a);
        head.setGravity(Gravity.CENTER_VERTICAL);
        head.addView(dot(a, attention ? R.color.nt_warn : R.color.nt_ok, 10));

        LinearLayout nameBox = Ui.column(a);
        nameBox.setLayoutParams(Ui.weight(1));
        nameBox.setPadding(Ui.dp(a, 12), 0, Ui.dp(a, 8), 0);
        nameBox.addView(Ui.text(a, n.displayName + " \u2014 " + n.statusLine(), 15,
                Ui.col(a, R.color.nt_ink), true));
        nameBox.addView(Ui.text(a, n.updatedPhrase(), 11, Ui.col(a, R.color.nt_text2), false));
        head.addView(nameBox);
        box.addView(head);

        LinearLayout metrics = Ui.row(a);
        metrics.setPadding(0, Ui.dp(a, 12), 0, 0);
        metrics.addView(metric(a, Ui.fmt1(n.pm25) + " \u00B5g/m\u00B3", "PM2.5", valueColor));
        metrics.addView(metric(a, Ui.fmt0(n.tempF()) + " \u00B0F", "Temperature",
                Ui.col(a, R.color.nt_ink)));
        metrics.addView(metric(a, Ui.fmt0(n.humidityPct) + " % RH", "Humidity",
                Ui.col(a, R.color.nt_ink)));
        // Battery reads as a percentage where the contract carries one, and as
        // the cell voltage where it does not (node-02 in the mock).
        String battery = Double.isNaN(n.batteryPct)
                ? Ui.fmt2(n.batteryV) + " V"
                : Ui.fmt0(n.batteryPct) + " %";
        metrics.addView(metric(a, battery, "Battery",
                Ui.col(a, attention ? R.color.nt_warn : R.color.nt_ok)));
        box.addView(metrics);

        box.setClickable(true);
        box.setFocusable(true);
        box.setOnClickListener(v -> openNode(a, n.nodeId));
        return box;
    }

    // ------------------------------------------------------- screen 2 render

    /** Screen 2 — Node 01 detail. */
    static void nodeDetail(Activity a, NodeInfo n, List<Double> pm24h) {
        LinearLayout into = content(a);
        into.removeAllViews();

        into.addView(brandHeader(a));

        // Live banner: recency on the left, "Live" with a sage pulse line right.
        LinearLayout live = card(a, R.color.nt_surface_2);
        LinearLayout liveRow = Ui.row(a);
        liveRow.setGravity(Gravity.CENTER_VERTICAL);
        TextView updated = Ui.text(a, n.updatedPhrase(), 13, Ui.col(a, R.color.nt_ink), false);
        updated.setLayoutParams(Ui.weight(1));
        liveRow.addView(updated);

        List<Double> pulse = new ArrayList<>();
        for (int i = 0; i < 14; i++) {
            pulse.add(i % 2 == 0 ? 30.0 : 70.0);
        }
        SparklineView pulseLine = new SparklineView(a, NodeInfo.toArray(pulse),
                Ui.col(a, R.color.nt_ok), 16);
        liveRow.addView(pulseLine);
        TextView liveLabel = Ui.text(a, "Live", 12, Ui.col(a, R.color.nt_ok), true);
        liveLabel.setPadding(Ui.dp(a, 8), 0, 0, 0);
        liveRow.addView(liveLabel);
        live.addView(liveRow);
        into.addView(live);

        // Title block.
        LinearLayout titleBox = Ui.column(a);
        titleBox.setPadding(0, Ui.dp(a, 14), 0, 0);
        titleBox.addView(Ui.text(a, n.displayName + " \u2014 Outdoor Air & Environmental Monitor",
                20, Ui.col(a, R.color.nt_ink), true));
        LinearLayout chipRow = Ui.row(a);
        chipRow.setGravity(Gravity.CENTER_VERTICAL);
        chipRow.setPadding(0, Ui.dp(a, 8), 0, 0);
        boolean watch = n.inWatch();
        chipRow.addView(chip(a, n.statusWord(), watch ? R.color.nt_warn : R.color.nt_ok,
                watch ? R.color.nt_warn_soft : R.color.nt_ok_soft));
        titleBox.addView(chipRow);
        TextView loc = Ui.text(a, n.locationLine(), 11.5f, Ui.col(a, R.color.nt_text2), false);
        loc.setPadding(0, Ui.dp(a, 8), 0, 0);
        titleBox.addView(loc);
        into.addView(titleBox);

        // Metric cards with 12 h sparklines.
        List<View> cards = new ArrayList<>();
        // The chip names the node's own air verdict ("Clean-air" while the node
        // is Healthy — UI spec v2); the numeric bands are the chart's labels.
        String airChip = n.inWatch() ? NodeInfo.pm25Band(n.pm25) + "-air" : "Clean-air";
        cards.add(metricCard(a, "PM2.5", Ui.fmt1(n.pm25), "\u00B5g/m\u00B3",
                airChip,
                n.inWatch() ? R.color.nt_warn : R.color.nt_ok,
                NodeInfo.toArray(n.trendPm25), R.color.nt_ok, true));
        cards.add(metricCard(a, "Temperature",
                Ui.fmt0(n.tempF()),
                "\u00B0F (" + Ui.fmt1(n.temperatureC) + " \u00B0C)", null, 0,
                NodeInfo.toArray(fahrenheit(n.trendTemperatureC)), R.color.nt_warn, true));
        cards.add(metricCard(a, "Humidity", Ui.fmt0(n.humidityPct), "% RH", null, 0,
                NodeInfo.toArray(n.trendHumidityPct), R.color.nt_blue, true));
        cards.add(batteryCard(a, n));
        into.addView(metricGrid(a, cards));

        // Link & hardware.
        into.addView(sectionHead(a, "Link & hardware", null));
        LinearLayout hw = card(a, R.color.nt_surface_2);
        LinearLayout connected = Ui.row(a);
        connected.setGravity(Gravity.CENTER_VERTICAL);
        connected.addView(dot(a, R.color.nt_ok, 9));
        TextView conn = Ui.text(a, "Connected", 13, Ui.col(a, R.color.nt_ok), true);
        conn.setPadding(Ui.dp(a, 9), 0, 0, 0);
        connected.addView(conn);
        hw.addView(connected);
        hw.addView(hwLine(a, "LoRa Link", "RSSI " + minus(n.rssiDbm) + " dBm"));
        hw.addView(marginBar(a, n.linkMarginDb));
        hw.addView(hwLine(a, "Gateway hops", Ui.fmt0(n.gatewayHops)));
        hw.addView(hwLine(a, "Hardware", n.firmware));
        hw.addView(hwLine(a, "HW Rev", n.hwRev));
        hw.addView(hwLine(a, "Serial", n.serial));
        hw.addView(hwLine(a, "Battery", Ui.fmt2(n.batteryV) + " V"));
        into.addView(hw);

        // Trends.
        into.addView(sectionHead(a, "Trends", null));
        LinearLayout trend = card(a, R.color.nt_surface_2);
        LinearLayout trendHead = Ui.row(a);
        trendHead.setGravity(Gravity.CENTER_VERTICAL);
        TextView legend = Ui.text(a, "PM2.5 \u00B7 Now: " + Ui.fmt1(n.pm25) + " \u00B5g/m\u00B3",
                11, Ui.col(a, R.color.nt_text2), false);
        legend.setLayoutParams(Ui.weight(1));
        trendHead.addView(legend);
        trendHead.addView(chip(a, "24-hour trend \u25BE", R.color.nt_text2, R.color.nt_surface_3));
        trend.addView(trendHead);

        TrendChartView chart = new TrendChartView(a,
                pm24h == null ? new double[0] : NodeInfo.toArray(pm24h),
                Ui.col(a, R.color.nt_ink),
                bandColor(a, R.color.nt_ok), bandColor(a, R.color.nt_accent),
                bandColor(a, R.color.nt_warn), Ui.col(a, R.color.nt_line), 150);
        LinearLayout.LayoutParams chartLp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, Ui.dp(a, 150));
        chartLp.setMargins(0, Ui.dp(a, 10), 0, Ui.dp(a, 8));
        chart.setLayoutParams(chartLp);
        trend.addView(chart);
        trend.addView(bandLegend(a));
        into.addView(trend);

        // Recent history: the Sep-23 smoke event, cleared, no Acknowledge CTA.
        into.addView(sectionHead(a, "Recent history", null));
        LinearLayout hist = card(a, R.color.nt_surface_2);
        LinearLayout histHead = Ui.row(a);
        histHead.setGravity(Gravity.CENTER_VERTICAL);
        TextView histTitle = Ui.text(a, "SMOKE \u00B7 node-01 \u00B7 Sep 23", 13,
                Ui.col(a, R.color.nt_ink), true);
        histTitle.setLayoutParams(Ui.weight(1));
        histHead.addView(histTitle);
        histHead.addView(chip(a, "Cleared", R.color.nt_ok, R.color.nt_ok_soft));
        hist.addView(histHead);
        hist.addView(Ui.text(a, "PM2.5 rising fast.", 12, Ui.col(a, R.color.nt_text2), false));
        LinearLayout histActions = Ui.row(a);
        histActions.setPadding(0, Ui.dp(a, 12), 0, 0);
        histActions.addView(button(a, "View trend", R.color.nt_surface_3, R.color.nt_ink, null));
        hist.addView(histActions);
        into.addView(hist);

        into.addView(footer(a));
    }

    // ------------------------------------------------------- screen 3 render

    /** Screen 3 — Alerts. */
    static void alerts(AlertsActivity a, List<AlertItem> alerts, String filter) {
        LinearLayout into = content(a);
        into.removeAllViews();

        LinearLayout titleBox = Ui.column(a);
        titleBox.addView(Ui.text(a, "Alerts", 26, Ui.col(a, R.color.nt_ink), true));
        titleBox.addView(Ui.text(a, "Consensus-based detection cuts down false alarms.", 12,
                Ui.col(a, R.color.nt_text2), false));
        into.addView(titleBox);

        // Hero card: the network is not escalating anything.
        LinearLayout hero = card(a, R.color.nt_ok_soft);
        LinearLayout heroRow = Ui.row(a);
        heroRow.setGravity(Gravity.TOP);
        TextView mark = Ui.text(a, "\u2713", 26, Ui.col(a, R.color.nt_ok), true);
        heroRow.addView(mark);
        LinearLayout heroCopy = Ui.column(a);
        heroCopy.setLayoutParams(Ui.weight(1));
        heroCopy.setPadding(Ui.dp(a, 12), 0, 0, 0);
        heroCopy.addView(Ui.text(a, "No active alerts", 18, Ui.col(a, R.color.nt_ink), true));
        heroCopy.addView(Ui.text(a,
                "The network needs a matching rise across nearby nodes before escalating "
                        + "a smoke event. Node 02 is elevated on its own \u2014 shown as Watch "
                        + "until a second node confirms.",
                12, Ui.col(a, R.color.nt_ink), false));
        heroRow.addView(heroCopy);
        hero.addView(heroRow);
        into.addView(hero);

        // Filter chips.
        LinearLayout chips = Ui.row(a);
        chips.setPadding(0, Ui.dp(a, 12), 0, Ui.dp(a, 4));
        chips.addView(filterChip(a, "All (" + alerts.size() + ")", "all", filter));
        chips.addView(filterChip(a, "Watch (" + AlertItem.countType(alerts, "watch") + ")",
                "watch", filter));
        chips.addView(filterChip(a, "Smoke (" + AlertItem.countType(alerts, "smoke") + ")",
                "smoke", filter));
        chips.addView(filterChip(a, "Battery (" + AlertItem.countType(alerts, "battery") + ")",
                "battery", filter));
        chips.addView(filterChip(a, "Heat (" + AlertItem.countType(alerts, "heat") + ")",
                "heat", filter));
        into.addView(chips);

        into.addView(sectionHead(a, "Recent activity", null));
        for (AlertItem item : filtered(alerts, filter)) {
            into.addView(alertCard(a, item));
        }

        into.addView(footer(a));
    }

    private static List<AlertItem> filtered(List<AlertItem> alerts, String filter) {
        if (filter == null || "all".equals(filter)) {
            return alerts;
        }
        List<AlertItem> out = new ArrayList<>();
        for (AlertItem item : alerts) {
            if (filter.equalsIgnoreCase(item.type)) {
                out.add(item);
            }
        }
        return out;
    }

    /** One alert row: heading, message, severity/state chips, its action buttons. */
    private static View alertCard(final AlertsActivity a, final AlertItem item) {
        boolean alarm = "watch".equals(item.severity) || "warning".equals(item.severity)
                || "critical".equals(item.severity);

        LinearLayout box = card(a, R.color.nt_surface_2);
        LinearLayout head = Ui.row(a);
        head.setGravity(Gravity.CENTER_VERTICAL);
        TextView title = Ui.text(a, item.heading(), 14, Ui.col(a, R.color.nt_ink), true);
        title.setLayoutParams(Ui.weight(1));
        head.addView(title);
        if (item.showsState()) {
            head.addView(chip(a, item.stateWord(),
                    item.isCleared() ? R.color.nt_ok : R.color.nt_text2,
                    item.isCleared() ? R.color.nt_ok_soft : R.color.nt_surface_3));
        }
        box.addView(head);

        TextView msg = Ui.text(a, item.title, 12.5f, Ui.col(a, R.color.nt_ink), false);
        msg.setPadding(0, Ui.dp(a, 6), 0, 0);
        box.addView(msg);

        LinearLayout meta = Ui.row(a);
        meta.setGravity(Gravity.CENTER_VERTICAL);
        meta.setPadding(0, Ui.dp(a, 8), 0, 0);
        meta.addView(chip(a, "Severity: " + item.severityWord(),
                alarm ? R.color.nt_warn : R.color.nt_blue,
                alarm ? R.color.nt_warn_soft : R.color.nt_blue_soft));
        String stamp = item.stamp();
        if (!stamp.isEmpty()) {
            TextView when = Ui.text(a, stamp, 10.5f, Ui.col(a, R.color.nt_text2), false);
            when.setPadding(Ui.dp(a, 10), 0, 0, 0);
            meta.addView(when);
        }
        box.addView(meta);

        LinearLayout actions = Ui.row(a);
        actions.setPadding(0, Ui.dp(a, 12), 0, 0);
        if ("watch".equals(item.type)) {
            actions.addView(button(a, "View nodes", R.color.nt_surface_3, R.color.nt_ink, null));
            actions.addView(spacer(a));
            actions.addView(button(a, "View trend", R.color.nt_surface_3, R.color.nt_ink, null));
        } else if ("battery".equals(item.type)) {
            actions.addView(button(a, "View node", R.color.nt_surface_3, R.color.nt_ink, null));
            if (item.canAcknowledge()) {
                actions.addView(spacer(a));
                actions.addView(button(a, "Acknowledge", R.color.nt_accent, R.color.nt_bg,
                        v -> acknowledge(a, item)));
            }
        } else if ("heat".equals(item.type)) {
            actions.addView(button(a, "View trend", R.color.nt_surface_3, R.color.nt_ink, null));
            actions.addView(spacer(a));
            actions.addView(button(a, "View node", R.color.nt_surface_3, R.color.nt_ink, null));
        } else {
            // Cleared/snoozed: contract v1 — "View trend" only, never Acknowledge.
            actions.addView(button(a, "View trend", R.color.nt_surface_3, R.color.nt_ink, null));
        }
        box.addView(actions);
        return box;
    }

    /**
     * The Acknowledge action. It runs the contract's state machine against the
     * API and re-renders this screen, so the badge follows: an active item
     * becomes acknowledged and the unacknowledged count drops.
     */
    private static void acknowledge(final AlertsActivity a, final AlertItem item) {
        new Thread(() -> {
            try {
                ApiSource.get().acknowledgeAlert(item.alertId);
            } catch (Exception ignored) {
                // A failed acknowledge leaves the feed unchanged and re-renders
                // the same state — never a locally-invented "acknowledged".
            }
            a.runOnUiThread(() -> renderAlerts(a, a.currentFilter()));
        }, "alert-ack").start();
    }

    // ----------------------------------------------------------- shared parts

    /** Header with the mountain mark, a title and a recency line. */
    static View propertyHeader(Context c, String title, String subtitle) {
        LinearLayout box = Ui.row(c);
        box.setGravity(Gravity.CENTER_VERTICAL);
        box.setPadding(0, Ui.dp(c, 4), 0, Ui.dp(c, 12));

        ImageView mark = new ImageView(c);
        mark.setImageResource(R.drawable.brand_mark);
        mark.setAdjustViewBounds(true);
        LinearLayout.LayoutParams markLp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, Ui.dp(c, 30));
        mark.setLayoutParams(markLp);
        box.addView(mark);

        LinearLayout col = Ui.column(c);
        col.setLayoutParams(Ui.weight(1));
        col.setPadding(Ui.dp(c, 10), 0, 0, 0);
        col.addView(Ui.text(c, title, 22, Ui.col(c, R.color.nt_ink), true));
        LinearLayout recency = Ui.row(c);
        recency.setGravity(Gravity.CENTER_VERTICAL);
        recency.setPadding(0, Ui.dp(c, 4), 0, 0);
        recency.addView(dot(c, R.color.nt_ok, 7));
        TextView sub = Ui.text(c, subtitle, 11.5f, Ui.col(c, R.color.nt_text2), false);
        sub.setPadding(Ui.dp(c, 7), 0, 0, 0);
        recency.addView(sub);
        col.addView(recency);
        box.addView(col);
        return box;
    }

    /** Screen-2 header: back arrow, brand line, overflow menu. */
    static View brandHeader(final Activity a) {
        LinearLayout box = Ui.row(a);
        box.setGravity(Gravity.CENTER_VERTICAL);
        box.setPadding(0, Ui.dp(a, 4), 0, Ui.dp(a, 10));

        TextView back = Ui.text(a, "\u2039", 24, Ui.col(a, R.color.nt_ink), false);
        back.setPadding(Ui.dp(a, 2), 0, Ui.dp(a, 12), 0);
        back.setClickable(true);
        back.setOnClickListener(v -> a.finish());
        box.addView(back);

        LinearLayout col = Ui.column(a);
        col.setLayoutParams(Ui.weight(1));
        col.addView(Ui.text(a, "Nordtronics", 14, Ui.col(a, R.color.nt_ink), true));
        col.addView(Ui.text(a, "Wildfire companion", 10, Ui.col(a, R.color.nt_text2), false));
        box.addView(col);

        TextView overflow = Ui.text(a, "\u22EE", 20, Ui.col(a, R.color.nt_text2), false);
        overflow.setPadding(Ui.dp(a, 8), 0, Ui.dp(a, 2), 0);
        box.addView(overflow);
        return box;
    }

    /** A section head, with an optional right-hand "Approximate layout" label. */
    static View sectionHead(Context c, String title, String note) {
        LinearLayout box = Ui.row(c);
        box.setGravity(Gravity.CENTER_VERTICAL);
        box.setPadding(0, Ui.dp(c, 18), 0, Ui.dp(c, 8));
        TextView t = Ui.text(c, title, 16, Ui.col(c, R.color.nt_ink), true);
        t.setLayoutParams(Ui.weight(1));
        box.addView(t);
        if (note != null) {
            box.addView(Ui.text(c, note, 10.5f, Ui.col(c, R.color.nt_text2), false));
        }
        return box;
    }

    /** Screen 1's 2×2 metric grid. */
    private static View metricGrid(Context c, List<View> cards) {
        LinearLayout grid = Ui.column(c);
        for (int i = 0; i < cards.size(); i += 2) {
            LinearLayout row = Ui.row(c);
            row.addView(cards.get(i), weighted());
            if (i + 1 < cards.size()) {
                row.addView(cards.get(i + 1), weightedSpaced(c));
            } else {
                View blank = new View(c);
                row.addView(blank, weightedSpaced(c));
            }
            LinearLayout.LayoutParams rowLp = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT);
            rowLp.bottomMargin = Ui.dp(c, 8);
            row.setLayoutParams(rowLp);
            grid.addView(row);
        }
        return grid;
    }

    private static LinearLayout.LayoutParams weighted() {
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                0, LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.weight = 1;
        return lp;
    }

    private static LinearLayout.LayoutParams weightedSpaced(Context c) {
        LinearLayout.LayoutParams lp = weighted();
        lp.leftMargin = Ui.dp(c, 8);
        return lp;
    }

    /** A metric card: label, big mono value, optional chip, optional sparkline. */
    private static View metricCard(Context c, String label, String value, String unit,
                                   String chipText, int chipColor, double[] spark,
                                   int sparkColor) {
        return metricCard(c, label, value, unit, chipText, chipColor, spark, sparkColor, false);
    }

    private static View metricCard(Context c, String label, String value, String unit,
                                   String chipText, int chipColor, double[] spark,
                                   int sparkColor, boolean axisLabels) {
        LinearLayout box = card(c, R.color.nt_elev);
        box.addView(Ui.text(c, label, 11, Ui.col(c, R.color.nt_text2), false));

        LinearLayout valueRow = Ui.row(c);
        valueRow.setGravity(Gravity.BOTTOM);
        valueRow.setPadding(0, Ui.dp(c, 6), 0, 0);
        valueRow.addView(Ui.mono(c, value, 22, Ui.col(c, R.color.nt_ink), true));
        if (unit != null && !unit.isEmpty()) {
            TextView u = Ui.mono(c, unit, 10, Ui.col(c, R.color.nt_text2), false);
            u.setPadding(Ui.dp(c, 4), 0, 0, Ui.dp(c, 3));
            valueRow.addView(u);
        }
        box.addView(valueRow);

        if (chipText != null) {
            LinearLayout chipRow = Ui.row(c);
            chipRow.setPadding(0, Ui.dp(c, 8), 0, 0);
            chipRow.addView(chip(c, chipText, chipColor,
                    chipColor == R.color.nt_ok ? R.color.nt_ok_soft : R.color.nt_warn_soft));
            box.addView(chipRow);
        }

        if (spark != null && spark.length > 1) {
            SparklineView line = new SparklineView(c, spark, Ui.col(c, sparkColor), 26);
            LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT, Ui.dp(c, 26));
            lp.topMargin = Ui.dp(c, 8);
            line.setLayoutParams(lp);
            box.addView(line);
            if (axisLabels) {
                LinearLayout axis = Ui.row(c);
                axis.setPadding(0, Ui.dp(c, 2), 0, 0);
                TextView a1 = Ui.mono(c, "-12h", 8.5f, Ui.col(c, R.color.nt_text2), false);
                a1.setLayoutParams(Ui.weight(1));
                axis.addView(a1);
                TextView a2 = Ui.mono(c, "-6h", 8.5f, Ui.col(c, R.color.nt_text2), false);
                a2.setLayoutParams(Ui.weight(1));
                a2.setGravity(Gravity.CENTER_HORIZONTAL);
                axis.addView(a2);
                TextView a3 = Ui.mono(c, "Now", 8.5f, Ui.col(c, R.color.nt_text2), false);
                a3.setLayoutParams(Ui.weight(1));
                a3.setGravity(Gravity.END);
                axis.addView(a3);
                box.addView(axis);
            }
        }
        return box;
    }

    /** The battery card: percentage, charging state, gained today, icon. */
    private static View batteryCard(Context c, NodeInfo n) {
        LinearLayout box = card(c, R.color.nt_elev);
        box.addView(Ui.text(c, "Battery", 11, Ui.col(c, R.color.nt_text2), false));

        LinearLayout valueRow = Ui.row(c);
        valueRow.setGravity(Gravity.BOTTOM);
        valueRow.setPadding(0, Ui.dp(c, 6), 0, 0);
        valueRow.addView(Ui.mono(c, Ui.fmt0(n.batteryPct), 22, Ui.col(c, R.color.nt_ok), true));
        TextView pct = Ui.mono(c, "%", 10, Ui.col(c, R.color.nt_text2), false);
        pct.setPadding(Ui.dp(c, 4), 0, 0, Ui.dp(c, 3));
        valueRow.addView(pct);
        box.addView(valueRow);

        TextView state = Ui.text(c, (n.charging ? "Solar charging" : "On battery")
                + " \u00B7 +" + Ui.fmt0(n.chargeTodayPct) + " % today",
                10.5f, Ui.col(c, R.color.nt_ok), false);
        state.setPadding(0, Ui.dp(c, 8), 0, 0);
        box.addView(state);
        box.addView(Ui.text(c, Ui.fmt2(n.batteryV) + " V in Link & hardware", 9,
                Ui.col(c, R.color.nt_text2), false));
        return box;
    }

    /** The LoRa margin bar: link margin in dB against a 40 dB design ceiling. */
    private static View marginBar(Context c, double marginDb) {
        LinearLayout box = Ui.row(c);
        box.setGravity(Gravity.CENTER_VERTICAL);
        box.setPadding(0, Ui.dp(c, 6), 0, Ui.dp(c, 6));

        TextView label = Ui.text(c, "Link margin", 11, Ui.col(c, R.color.nt_text2), false);
        LinearLayout.LayoutParams labelLp = new LinearLayout.LayoutParams(
                Ui.dp(c, 84), LinearLayout.LayoutParams.WRAP_CONTENT);
        label.setLayoutParams(labelLp);
        box.addView(label);

        int percent = (int) Math.max(0, Math.min(100, Math.round(marginDb / 40.0 * 100)));
        LinearLayout track = Ui.row(c);
        track.setLayoutParams(Ui.weight(1));
        track.setBackground(Ui.round(c, Ui.col(c, R.color.nt_surface_3), 9));
        View fill = new View(c);
        fill.setBackground(Ui.round(c, Ui.col(c, R.color.nt_ok), 9));
        View spacer = new View(c);
        track.addView(fill);
        track.addView(spacer);
        box.addView(track);

        TextView value = Ui.mono(c, Ui.fmt0(marginDb) + " dB", 11, Ui.col(c, R.color.nt_ink), false);
        value.setGravity(Gravity.END);
        LinearLayout.LayoutParams valueLp = new LinearLayout.LayoutParams(
                Ui.dp(c, 56), LinearLayout.LayoutParams.WRAP_CONTENT);
        value.setLayoutParams(valueLp);
        box.addView(value);

        Ui.setBar(fill, spacer, value, percent, Ui.fmt0(marginDb) + " dB");
        return box;
    }

    /** One label/value line in a panel. */
    private static View hwLine(Context c, String label, String value) {
        LinearLayout box = Ui.row(c);
        box.setGravity(Gravity.CENTER_VERTICAL);
        box.setPadding(0, Ui.dp(c, 5), 0, Ui.dp(c, 5));
        TextView l = Ui.text(c, label, 11, Ui.col(c, R.color.nt_text2), false);
        l.setLayoutParams(Ui.weight(1));
        box.addView(l);
        box.addView(Ui.mono(c, value, 11, Ui.col(c, R.color.nt_ink), false));
        return box;
    }

    /** The three labelled threshold bands, in the chart's own colours. */
    static View bandLegend(Context c) {
        LinearLayout box = Ui.column(c);
        box.addView(bandRow(c, BAND_CLEAN, R.color.nt_ok));
        box.addView(bandRow(c, BAND_ELEVATED, R.color.nt_accent));
        box.addView(bandRow(c, BAND_HIGH, R.color.nt_warn));
        return box;
    }

    private static View bandRow(Context c, String text, int color) {
        LinearLayout row = Ui.row(c);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(0, Ui.dp(c, 2), 0, Ui.dp(c, 2));
        View swatch = new View(c);
        swatch.setBackground(Ui.round(c, Ui.col(c, color), 2));
        LinearLayout.LayoutParams swatchLp = new LinearLayout.LayoutParams(
                Ui.dp(c, 14), Ui.dp(c, 9));
        swatchLp.rightMargin = Ui.dp(c, 8);
        swatch.setLayoutParams(swatchLp);
        row.addView(swatch);
        row.addView(Ui.mono(c, text, 10, Ui.col(c, R.color.nt_text2), false));
        return row;
    }

    /** A metric column: mono value over a label. */
    private static View metric(Context c, String value, String label, int valueColor) {
        LinearLayout box = Ui.column(c);
        box.setLayoutParams(Ui.weight(1));
        box.addView(Ui.mono(c, value, 12, valueColor, true));
        box.addView(Ui.text(c, label, 9.5f, Ui.col(c, R.color.nt_text2), false));
        return box;
    }

    static LinearLayout card(Context c, int bgColorRes) {
        LinearLayout box = Ui.column(c);
        box.setBackground(Ui.round(c, Ui.col(c, bgColorRes), 16));
        box.setPadding(Ui.dp(c, 14), Ui.dp(c, 14), Ui.dp(c, 14), Ui.dp(c, 14));
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.bottomMargin = Ui.dp(c, 8);
        box.setLayoutParams(lp);
        return box;
    }

    private static android.graphics.drawable.GradientDrawable bordered(Context c, int bg, int stroke) {
        android.graphics.drawable.GradientDrawable d = Ui.round(c, Ui.col(c, bg), 16);
        d.setStroke(Ui.dp(c, 1.3f), stroke);
        return d;
    }

    static View chip(Context c, String text, int textColor, int bgColor) {
        TextView t = Ui.text(c, text, 10, Ui.col(c, textColor), true);
        t.setBackground(Ui.round(c, Ui.col(c, bgColor), 9));
        t.setPadding(Ui.dp(c, 9), Ui.dp(c, 5), Ui.dp(c, 9), Ui.dp(c, 5));
        return t;
    }

    private static View filterChip(final AlertsActivity a, final String text, final String key,
                            final String active) {
        boolean on = key.equals(active);
        TextView t = Ui.text(a, text, 10.5f,
                Ui.col(a, on ? R.color.nt_ink : R.color.nt_text2), true);
        t.setBackground(Ui.round(a, Ui.col(a, on ? R.color.nt_surface_3 : R.color.nt_surface_2), 10));
        t.setPadding(Ui.dp(a, 10), Ui.dp(a, 6), Ui.dp(a, 10), Ui.dp(a, 6));
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.rightMargin = Ui.dp(a, 6);
        t.setLayoutParams(lp);
        t.setClickable(true);
        t.setFocusable(true);
        t.setOnClickListener(v -> a.setFilter(key));
        return t;
    }

    static View button(Context c, String text, int bgColor, int textColor,
                       View.OnClickListener onTap) {
        TextView t = Ui.text(c, text, 11.5f, Ui.col(c, textColor), true);
        t.setBackground(Ui.round(c, Ui.col(c, bgColor), 10));
        t.setPadding(Ui.dp(c, 14), Ui.dp(c, 8), Ui.dp(c, 14), Ui.dp(c, 8));
        t.setGravity(Gravity.CENTER);
        if (onTap != null) {
            t.setClickable(true);
            t.setFocusable(true);
            t.setOnClickListener(onTap);
        }
        return t;
    }

    private static View spacer(Context c) {
        View v = new View(c);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                Ui.dp(c, 8), 1);
        v.setLayoutParams(lp);
        return v;
    }

    static View dot(Context c, int colorRes, float sizeDp) {
        View v = new View(c);
        v.setBackground(Ui.oval(c, Ui.col(c, colorRes)));
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                Ui.dp(c, sizeDp), Ui.dp(c, sizeDp));
        v.setLayoutParams(lp);
        return v;
    }

    static View footer(Context c) {
        String text = ApiSource.usingMock() ? FOOTER_MOCK : FOOTER_LIVE;
        TextView t = Ui.text(c, text, 10, Ui.col(c, R.color.nt_text2), false);
        t.setGravity(Gravity.CENTER);
        t.setPadding(0, Ui.dp(c, 18), 0, Ui.dp(c, 6));
        return t;
    }

    // ------------------------------------------------------------- formatting

    static void openNode(Activity a, String nodeId) {
        Intent detail = new Intent(a, NodeDetailActivity.class);
        detail.putExtra(NodeDetailActivity.EXTRA_NODE_ID, nodeId);
        a.startActivity(detail);
    }

    static NodeInfo find(List<NodeInfo> nodes, String nodeId) {
        for (NodeInfo n : nodes) {
            if (nodeId.equals(n.nodeId)) {
                return n;
            }
        }
        return nodes.isEmpty() ? null : nodes.get(0);
    }

    /**
     * "Last packet received &lt;x&gt;": the summary's own {@code last_packet_utc}
     * where the backend serves one, otherwise the freshest packet in the node
     * list — the live backend has no {@code /v1/network/status} today (task
     * 0096), and "unknown" next to cards reading "Updated 2 h ago" reads as a
     * defect. Never a clock: the age is the payload's own {@code age_seconds},
     * or derived from the timestamp it carries.
     */
    static String ageOf(NetworkStatus status, List<NodeInfo> nodes) {
        if (status != null && status.lastPacketUtc != null && !status.lastPacketUtc.isEmpty()) {
            long age = secondsSince(status.lastPacketUtc);
            if (age >= 0) {
                return agePhrase(age);
            }
        }
        long freshest = -1;
        for (NodeInfo n : nodes) {
            if (n.ageSeconds >= 0 && (freshest < 0 || n.ageSeconds < freshest)) {
                freshest = Math.round(n.ageSeconds);
            }
        }
        return freshest < 0 ? "unknown" : agePhrase(freshest);
    }

    private static String agePhrase(long ageSeconds) {
        if (ageSeconds < 5400) {
            return Math.round(ageSeconds / 60.0) + " min ago";
        }
        return Math.round(ageSeconds / 3600.0) + " h ago";
    }

    private static long secondsSince(String isoUtc) {
        java.text.SimpleDateFormat f =
                new java.text.SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        f.setTimeZone(java.util.TimeZone.getTimeZone("UTC"));
        try {
            java.util.Date d = f.parse(isoUtc);
            return d == null ? -1
                    : Math.max(0, (System.currentTimeMillis() - d.getTime()) / 1000);
        } catch (Exception e) {
            return -1;
        }
    }

    /** "-68" -> "−68" (U+2212, the spec's minus sign). */
    static String minus(double v) {
        String plain = Ui.fmt0(v);
        return plain.startsWith("-") ? "\u2212" + plain.substring(1) : plain;
    }

    static String reportingOf(List<NodeInfo> nodes) {
        int reporting = 0;
        for (NodeInfo n : nodes) {
            if (n.ageSeconds >= 0 && n.ageSeconds <= 900) {
                reporting++;
            }
        }
        return reporting + "/" + nodes.size();
    }

    static double median(List<Double> values) {
        if (values.isEmpty()) {
            return Double.NaN;
        }
        List<Double> sorted = NodeInfo.sortedCopy(values);
        int mid = sorted.size() / 2;
        if (sorted.size() % 2 == 1) {
            return sorted.get(mid);
        }
        return (sorted.get(mid - 1) + sorted.get(mid)) / 2.0;
    }

    private static List<Double> fahrenheit(List<Double> celsius) {
        List<Double> out = new ArrayList<>();
        for (Double c : celsius) {
            out.add(c == null ? Double.NaN : c * 9.0 / 5.0 + 32.0);
        }
        return out;
    }

    private static int bandColor(Context c, int colorRes) {
        int rgb = Ui.col(c, colorRes) & 0x00FFFFFF;
        return 0x33 << 24 | rgb;                 // 20 % alpha, so the line stays legible
    }

    // ------------------------------------------------------------- rendering

    /** Screen 1 render, called by NodesActivity. */
    static void renderPropertyOverview(final Activity a) {
        new Thread(() -> {
            final WildfireApi api = ApiSource.get();
            try {
                final List<NodeInfo> nodes = api.nodes();
                final NetworkStatus status = api.networkStatus();
                final int badge = AlertItem.badgeCount(api.alerts());
                a.runOnUiThread(() -> {
                    propertyOverview(a, nodes, status);
                    sourceBanner(a);
                    Ui.bindNav(a, R.id.nav_nodes, badge);
                });
            } catch (final Exception e) {
                a.runOnUiThread(() -> {
                    clear(a);
                    LinearLayout into = content(a);
                    into.addView(propertyHeader(a, "Property line", "No packet"));
                    LinearLayout err = card(a, R.color.nt_warn_soft);
                    err.addView(Ui.text(a, "Could not read the API: " + e.getMessage(), 12,
                            Ui.col(a, R.color.nt_ink), false));
                    into.addView(err);
                    sourceBanner(a);
                    Ui.bindNav(a, R.id.nav_nodes, 0);
                });
            }
        }, "property-overview").start();
    }

    /** Screen 2 render, called by NodeDetailActivity. */
    static void renderNodeDetail(final Activity a, final String nodeId) {
        new Thread(() -> {
            final WildfireApi api = ApiSource.get();
            try {
                final NodeInfo n = api.node(nodeId);
                final List<TrendPoint> points = api.readings(nodeId, "pm25", 24);
                final List<Double> series = new ArrayList<>();
                for (TrendPoint p : points) {
                    series.add(p.v);
                }
                final int badge = AlertItem.badgeCount(api.alerts());
                a.runOnUiThread(() -> {
                    nodeDetail(a, n, series);
                    sourceBanner(a);
                    Ui.bindNav(a, R.id.nav_nodes, badge);
                });
            } catch (final Exception e) {
                a.runOnUiThread(() -> {
                    clear(a);
                    LinearLayout into = content(a);
                    into.addView(brandHeader(a));
                    LinearLayout err = card(a, R.color.nt_warn_soft);
                    err.addView(Ui.text(a, "Could not read the API: " + e.getMessage(), 12,
                            Ui.col(a, R.color.nt_ink), false));
                    into.addView(err);
                    sourceBanner(a);
                    Ui.bindNav(a, R.id.nav_nodes, 0);
                });
            }
        }, "node-detail").start();
    }

    /** Screen 3 render, called by AlertsActivity (and by its filter chips). */
    static void renderAlerts(final AlertsActivity a, final String filter) {
        new Thread(() -> {
            final WildfireApi api = ApiSource.get();
            try {
                final List<AlertItem> alerts = api.alerts();
                a.runOnUiThread(() -> {
                    alerts(a, alerts, filter);
                    sourceBanner(a);
                    Ui.bindNav(a, R.id.nav_alerts, AlertItem.badgeCount(alerts));
                });
            } catch (final Exception e) {
                a.runOnUiThread(() -> {
                    clear(a);
                    LinearLayout into = content(a);
                    into.addView(Ui.text(a, "Alerts", 26, Ui.col(a, R.color.nt_ink), true));
                    LinearLayout err = card(a, R.color.nt_warn_soft);
                    err.addView(Ui.text(a, "Could not read the alerts feed: " + e.getMessage(),
                            12, Ui.col(a, R.color.nt_ink), false));
                    into.addView(err);
                    sourceBanner(a);
                    Ui.bindNav(a, R.id.nav_alerts, 0);
                });
            }
        }, "alerts").start();
    }

    /** Ascending seed used by the property overview's reporting card. */
    static List<Double> flat(double value, int count) {
        List<Double> out = new ArrayList<>(Collections.nCopies(count, value));
        return out;
    }
}
