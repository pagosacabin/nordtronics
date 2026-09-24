package com.nordtronics.companion;

import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

/**
 * One sensor node, exactly as {@code GET /api/nodes} returns it, plus the
 * presentation rules the v0.1 screens need (task 0049).
 *
 * <p>Nothing here invents a reading: every display value is either a field from
 * the API response or a documented transformation of one (C to F, volts to a
 * percentage of a 4.2 V cell). Values the mock payload does not carry — LoRa RSSI
 * and packet-delivery percentage, which the mockup shows — are deliberately not
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
        this.id = o.optString("id", "?");
        this.pm25 = o.optDouble("pm25", Double.NaN);
        this.tempC = o.optDouble("temp_c", Double.NaN);
        this.humidity = o.optDouble("humidity", Double.NaN);
        this.batteryV = o.optDouble("battery_v", Double.NaN);
        this.lastSeen = o.optString("last_seen", "");
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

    /** "received 18 sec ago", from the API's own last_seen timestamp. */
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

    /** Seconds since last_seen, or -1 when the timestamp cannot be read. */
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
