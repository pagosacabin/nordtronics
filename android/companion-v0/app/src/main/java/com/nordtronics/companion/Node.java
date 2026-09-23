package com.nordtronics.companion;

import org.json.JSONObject;

/** One sensor node, exactly as {@code GET /api/nodes} returns it. */
public class Node {

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

    /** Title line for a list row. */
    public String title() {
        return id;
    }

    /** One-line reading summary for a list row. */
    public String readingLine() {
        return String.format("PM2.5 %.1f µg/m³   %.1f °C   %.0f %%RH   %.2f V",
                pm25, tempC, humidity, batteryV);
    }

    public String lastSeenLine() {
        return "last seen " + lastSeen;
    }
}
