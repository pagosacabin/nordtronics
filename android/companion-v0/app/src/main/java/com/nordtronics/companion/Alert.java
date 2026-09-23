package com.nordtronics.companion;

import org.json.JSONObject;

/** One alert-history entry, exactly as {@code GET /api/alerts} returns it. */
public class Alert {

    public final String id;
    public final String nodeId;
    public final String type;
    public final String message;
    public final String at;

    public Alert(JSONObject o) {
        this.id = o.optString("id", "?");
        this.nodeId = o.optString("node_id", "?");
        this.type = o.optString("type", "?");
        this.message = o.optString("message", "");
        this.at = o.optString("at", "");
    }

    public String title() {
        return type.toUpperCase() + "  ·  " + nodeId;
    }

    public String detailLine() {
        return message;
    }

    public String atLine() {
        return at;
    }
}
