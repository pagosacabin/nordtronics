package com.nordtronics.companion;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * The property-wide consensus summary — {@code GET /v1/network/status} in the
 * contract v1. This is what the Property-overview watch banner reads: how many
 * nodes are reporting, which (if any) are outside their limits, and the
 * base station's own one-line note.
 */
public class NetworkStatus {

    public final String state;
    public final int nodesReporting;
    public final int nodesTotal;
    public final List<String> outsideLimits = new ArrayList<>();
    public final String note;
    public final String lastPacketUtc;

    public NetworkStatus(JSONObject o) {
        this.state = o.optString("state", "healthy");
        this.nodesReporting = o.optInt("nodes_reporting", 0);
        this.nodesTotal = o.optInt("nodes_total", 0);
        this.note = o.optString("note", "");
        this.lastPacketUtc = o.optString("last_packet_utc", "");
        JSONArray arr = o.optJSONArray("outside_limits");
        if (arr != null) {
            for (int i = 0; i < arr.length(); i++) {
                outsideLimits.add(arr.optString(i, ""));
            }
        }
    }

    public boolean isWatch() {
        return "watch".equals(state);
    }

    public boolean isAlert() {
        return "alert".equals(state);
    }

    /** "2 of 2 nodes reporting". */
    public String reportingLine() {
        return nodesReporting + " of " + nodesTotal + " nodes reporting";
    }

    /** The banner title, without the leading state word. */
    public String bannerBody() {
        return note == null || note.isEmpty()
                ? "Rest of network at baseline."
                : note;
    }
}
