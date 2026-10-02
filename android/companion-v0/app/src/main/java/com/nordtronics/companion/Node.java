package com.nordtronics.companion;

import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

/**
 * One sensor node, read from either API dialect, plus the presentation rules
 * the v0.1 screens need (tasks 0049, 0069).
 *
 * <p>The production {@code GET /v1/nodes} entry is
 * {@code {node_id, last_seen_utc, status, age_seconds, latest:{pm25,
 * temperature_c, humidity_pct, battery_v}}}; the mock's flat
 * {@code GET /api/nodes} entry is {@code {id, pm25, temp_c, humidity,
 * battery_v, last_seen}}. This constructor reads the nested form when
 * {@code latest} is present and the flat form otherwise, so both backends
 * render identically without a second model class.
 *
 * <p>Nothing here invents a reading: every display value is either a field from
 * the API response or a documented transformation of one (C to F, volts to a
 * percentage of a 4.2 V cell). Values neither payload carries — LoRa RSSI and
 * packet-delivery percentage, which the mockup shows — are deliberately not
 * fabricated; the detail screen's health panel is built from packet recency,
 * PM2.5 headroom and battery instead.
 */
public class Node {

    /** Consensus watch level: at or above this PM2.5 the node needs attention. */
    public static final double PM25_WATCH = 35.0;

    /** Single-cell Li-ion thresholds used for the battery bar. */
    public static final double BATTERY_EMPTY_V = 3.00;
    public static final double BATTERY_FULL_V = 4.20;

    public final String id;
    public final double pm25;
    public final double tempC;
    public final double humidity;
    public final double batteryV;
    public final String lastSeen;

    public Node(JSONObject o) {
        this.id = firstNonEmpty(o.optString("node_id", ""), o.optString("id", ""), "?");

        // The production payload nests the newest reading; the mock flattens it
        // onto the node object itself.
        JSONObject r = o.optJSONObject("latest");
        if (r == null) {
            r = o;
        }
        this.pm25 = num(r, "pm25");
        this.tempC = firstNum(r, "temperature_c", "temp_c");
        this.humidity = firstNum(r, "humidity_pct", "humidity");
        this.batteryV = num(r, "battery_v");

        this.lastSeen = firstNonEmpty(
                o.optString("last_seen_utc", ""), o.optString("last_seen", ""), "");
    }

    private static double num(JSONObject o, String key) {
        return o.has(key) && !o.isNull(key) ? o.optDouble(key, Double.NaN) : Double.NaN;
    }

    private static double firstNum(JSONObject o, String a, String b) {
        double v = num(o, a);
        return Double.isNaN(v) ? num(o, b) : v;
    }

    private static String firstNonEmpty(String a, String b, String fallback) {
        if (a != null && !a.isEmpty()) {
            return a;
        }
        if (b != null && !b.isEmpty()) {
            return b;
        }
        return fallback;
    }

    /** "node-01" -> "Node 01", so cards read like the mockup's field names. */
    public String displayName() {
        String[] parts = id.split("[-_]");
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
        return b.length() == 0 ? id : b.toString();
    }

    /** True when this node crosses the consensus watch level or is low on charge. */
    public boolean attention() {
        return pm25 >= PM25_WATCH || lowBattery();
    }

    public boolean lowBattery() {
        return !Double.isNaN(batteryV) && batteryV < 3.60;
    }

    public String statusWord() {
        return attention() ? "Watch" : "Healthy";
    }

    /** "Healthy · received 2 h ago" — the card's second line. */
    public String statusLine() {
        return statusWord() + " \u00B7 " + lastSeenPhrase();
    }

    /** "received 18 sec ago", from the API's own last-seen timestamp. */
    public String lastSeenPhrase() {
        long age = ageSeconds();
        if (age < 0) {
            return "last seen " + (lastSeen.isEmpty() ? "unknown" : lastSeen);
        }
        if (age < 90) {
            return "received " + age + " sec ago";
        }
        if (age < 5400) {
            return "received " + Math.round(age / 60.0) + " min ago";
        }
        if (age < 172800) {
            return "received " + Math.round(age / 3600.0) + " h ago";
        }
        return "received " + Math.round(age / 86400.0) + " d ago";
    }

    /** Seconds since last-seen, or -1 when the timestamp cannot be read. */
    public long ageSeconds() {
        if (lastSeen == null || lastSeen.isEmpty()) {
            return -1;
        }
        SimpleDateFormat f = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        f.setTimeZone(TimeZone.getTimeZone("UTC"));
        try {
            Date d = f.parse(lastSeen);
            return d == null ? -1 : Math.max(0, (System.currentTimeMillis() - d.getTime()) / 1000);
        } catch (Exception e) {
            return -1;
        }
    }

    /** "SEP 23 · 14:00" (UTC), or the raw string if it is not a timestamp. */
    public String lastSeenStamp() {
        if (lastSeen == null || lastSeen.isEmpty()) {
            return "unknown";
        }
        SimpleDateFormat in = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        in.setTimeZone(TimeZone.getTimeZone("UTC"));
        try {
            Date d = in.parse(lastSeen);
            if (d == null) {
                return lastSeen;
            }
            SimpleDateFormat out = new SimpleDateFormat("MMM d \u00B7 HH:mm", Locale.US);
            out.setTimeZone(TimeZone.getTimeZone("UTC"));
            return out.format(d).toUpperCase(Locale.US);
        } catch (Exception e) {
            return lastSeen;
        }
    }

    public double tempF() {
        return Double.isNaN(tempC) ? Double.NaN : tempC * 9.0 / 5.0 + 32.0;
    }

    /** Battery as a percentage of a full 4.2 V cell (3.00 V = 0 %). */
    public int batteryPercent() {
        if (Double.isNaN(batteryV)) {
            return 0;
        }
        double pct = (batteryV - BATTERY_EMPTY_V) / (BATTERY_FULL_V - BATTERY_EMPTY_V) * 100.0;
        return (int) Math.max(0, Math.min(100, Math.round(pct)));
    }

    /** 100 % inside a minute, decaying to 5 % by the time a packet is an hour old. */
    public int recencyPercent() {
        long age = ageSeconds();
        if (age < 0) {
            return 0;
        }
        if (age <= 60) {
            return 100;
        }
        if (age >= 3600) {
            return 5;
        }
        return (int) Math.max(5, 100 - (age - 60) * 95 / 3540);
    }

    /** How much PM2.5 room is left before the watch level: 100 % = clean air. */
    public int pmHeadroomPercent() {
        if (Double.isNaN(pm25)) {
            return 0;
        }
        double pct = 100.0 - (pm25 / PM25_WATCH) * 100.0;
        return (int) Math.max(0, Math.min(100, Math.round(pct)));
    }
}
