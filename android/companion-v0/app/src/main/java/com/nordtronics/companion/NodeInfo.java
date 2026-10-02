package com.nordtronics.companion;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * One sensor node, as the contract v1 node object
 * ({@code docs/wildfire/app-api-contract-v1-2026-10-02.md}). The newest reading
 * arrives nested under {@code latest}; the v1 display fields sit on the node
 * object itself. All values are SI (µg/m³, °C, %RH, V, dBm) — conversion to °F
 * happens here, for display only.
 */
public class NodeInfo {

    /** Display threshold band: at or above this PM2.5 a reading is "High". */
    public static final double PM25_ELEVATED = 12.0;
    public static final double PM25_HIGH = 35.0;

    /** Contract v1: a watch alert clears below this PM2.5. */
    public static final double PM25_WATCH = 25.0;

    public final String nodeId;
    public final String displayName;
    public final String displayId;
    public final String zone;
    public final String locationLabel;
    public final String firmware;
    public final String hwRev;
    public final String serial;
    public final String status;
    public final String lastSeenUtc;

    public final double pm25;
    public final double temperatureC;
    public final double humidityPct;
    public final double batteryV;
    public final double batteryPct;
    public final double chargeTodayPct;
    public final double rssiDbm;
    public final double linkMarginDb;
    public final double gatewayHops;
    public final double ageSeconds;

    public final boolean charging;

    /** 12 h sparkline arrays, oldest to newest (25 points = one per 30 min). */
    public final List<Double> trendPm25 = new ArrayList<>();
    public final List<Double> trendTemperatureC = new ArrayList<>();
    public final List<Double> trendHumidityPct = new ArrayList<>();

    public NodeInfo(JSONObject o) {
        this.nodeId = o.optString("node_id", "?");
        this.displayName = firstNonEmpty(o.optString("display_name", ""), displayNameOf(nodeId));
        this.displayId = o.optString("display_id", "");
        this.zone = o.optString("zone", "");
        this.locationLabel = o.optString("location_label", "");
        this.firmware = o.optString("firmware", "");
        this.hwRev = o.optString("hw_rev", "");
        this.serial = o.optString("serial", "");
        this.status = o.optString("status", "");
        this.lastSeenUtc = o.optString("last_seen_utc", "");
        this.rssiDbm = num(o, "rssi_dbm");
        this.linkMarginDb = num(o, "link_margin_db");
        this.gatewayHops = num(o, "gateway_hops");
        this.batteryPct = num(o, "battery_pct");
        this.chargeTodayPct = num(o, "charge_today_pct");
        this.charging = o.optBoolean("charging", false);
        this.ageSeconds = num(o, "age_seconds");

        JSONObject r = o.optJSONObject("latest");
        if (r == null) {
            r = o;
        }
        this.pm25 = num(r, "pm25");
        this.temperatureC = num(r, "temperature_c");
        this.humidityPct = num(r, "humidity_pct");
        this.batteryV = num(r, "battery_v");

        JSONObject t = o.optJSONObject("trends_12h");
        if (t != null) {
            fill(trendPm25, t.optJSONArray("pm25"));
            fill(trendTemperatureC, t.optJSONArray("temperature_c"));
            fill(trendHumidityPct, t.optJSONArray("humidity_pct"));
        }
    }

    private static void fill(List<Double> into, JSONArray arr) {
        if (arr == null) {
            return;
        }
        for (int i = 0; i < arr.length(); i++) {
            into.add(arr.optDouble(i, Double.NaN));
        }
    }

    private static double num(JSONObject o, String key) {
        return o.has(key) && !o.isNull(key) ? o.optDouble(key, Double.NaN) : Double.NaN;
    }

    private static String firstNonEmpty(String a, String b) {
        return a != null && !a.isEmpty() ? a : b;
    }

    private static String displayNameOf(String nodeId) {
        String[] parts = nodeId.split("[-_]");
        StringBuilder b = new StringBuilder();
        for (String p : parts) {
            if (p.isEmpty()) {
                continue;
            }
            if (b.length() > 0) {
                b.append(' ');
            }
            b.append(Character.toUpperCase(p.charAt(0))).append(p.substring(1));
        }
        return b.length() == 0 ? nodeId : b.toString();
    }

    /** The consensus word the property banner and the card use. */
    public String statusWord() {
        if ("stale".equals(status) || (ageSeconds >= 0 && ageSeconds > 900)) {
            return "Stale";
        }
        return pm25 >= PM25_WATCH ? "Watch" : "Healthy";
    }

    public boolean inWatch() {
        return "Watch".equals(statusWord());
    }

    /**
     * The node card's second line. Node 02 carries the extra qualifier from UI
     * spec v2 ("awaiting confirmation") because a single elevated node is not an
     * alert until a second node confirms.
     */
    public String statusLine() {
        if (inWatch()) {
            return "Watch \u00B7 awaiting confirmation";
        }
        return statusWord();
    }

    /** "Updated 4 min ago" — from the API's own age, never from a clock. */
    public String updatedPhrase() {
        return "Updated " + agePhrase();
    }

    public String agePhrase() {
        if (ageSeconds < 0) {
            return "at an unknown time";
        }
        long s = Math.round(ageSeconds);
        if (s < 90) {
            return s + " sec ago";
        }
        if (s < 5400) {
            return Math.round(s / 60.0) + " min ago";
        }
        if (s < 172800) {
            return Math.round(s / 3600.0) + " h ago";
        }
        return Math.round(s / 86400.0) + " d ago";
    }

    public double tempF() {
        return Double.isNaN(temperatureC) ? Double.NaN : temperatureC * 9.0 / 5.0 + 32.0;
    }

    /** "Sierra Ridge Trail · Zone 7 · Node ID: NT-01-7A3F". */
    public String locationLine() {
        StringBuilder b = new StringBuilder();
        if (locationLabel != null && !locationLabel.isEmpty()) {
            b.append(locationLabel);
        }
        if (zone != null && !zone.isEmpty()) {
            if (b.length() > 0) {
                b.append(" \u00B7 ");
            }
            b.append(zone);
        }
        if (displayId != null && !displayId.isEmpty()) {
            if (b.length() > 0) {
                b.append(" \u00B7 ");
            }
            b.append("Node ID: ").append(displayId);
        }
        return b.toString();
    }

    /** The band name for a PM2.5 value, matching the labelled chart bands. */
    public static String pm25Band(double v) {
        if (Double.isNaN(v)) {
            return "\u2014";
        }
        if (v < PM25_ELEVATED) {
            return "Clean";
        }
        return v <= PM25_HIGH ? "Elevated" : "High";
    }

    /** Ascending copy of one of the trend arrays, for the sparklines. */
    public List<Double> trend(String metric) {
        if ("temperature_c".equals(metric)) {
            return new ArrayList<>(trendTemperatureC);
        }
        if ("humidity_pct".equals(metric)) {
            return new ArrayList<>(trendHumidityPct);
        }
        return new ArrayList<>(trendPm25);
    }

    /** Convenience for the sparkline: min/max padded so a flat line still draws. */
    public static double[] toArray(List<Double> values) {
        double[] out = new double[values.size()];
        for (int i = 0; i < out.length; i++) {
            Double d = values.get(i);
            out[i] = d == null ? Double.NaN : d;
        }
        return out;
    }

    public static List<Double> sortedCopy(List<Double> values) {
        List<Double> copy = new ArrayList<>(values);
        Collections.sort(copy);
        return copy;
    }
}
