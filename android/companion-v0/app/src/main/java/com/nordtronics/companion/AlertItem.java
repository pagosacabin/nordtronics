package com.nordtronics.companion;

import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

/**
 * One entry in the alerts feed, exactly as contract v1's {@code GET /v1/alerts}
 * returns it: {@code type} (watch|smoke|battery|heat), {@code severity}
 * (watch|notice|warning|critical) and {@code state}
 * (active|acknowledged|cleared|snoozed).
 *
 * <p>The alert state machine is the contract's, run locally against the mock:
 * Acknowledge is offered only on {@code active} items, and the nav badge counts
 * {@code active} items only — never acknowledged, cleared or snoozed ones.
 */
public class AlertItem {

    public final String alertId;
    public final String type;
    public final String nodeId;
    public final String severity;
    public final String state;
    public final String title;
    public final String detail;
    public final String createdUtc;
    public final String updatedUtc;

    public AlertItem(JSONObject o) {
        this.alertId = o.optString("alert_id", "?");
        this.type = o.optString("type", "");
        this.nodeId = o.optString("node_id", "");
        this.severity = o.optString("severity", "");
        this.state = o.optString("state", "active");
        this.title = o.optString("title", "");
        this.detail = o.optString("detail", "");
        this.createdUtc = o.optString("created_utc", "");
        this.updatedUtc = o.optString("updated_utc", "");
    }

    // ------------------------------------------------------------- state

    public boolean isActive() {
        return "active".equals(state);
    }

    public boolean isAcknowledged() {
        return "acknowledged".equals(state);
    }

    public boolean isCleared() {
        return "cleared".equals(state);
    }

    public boolean isSnoozed() {
        return "snoozed".equals(state);
    }

    /** Contract v1: Acknowledge is offered only on active items. */
    public boolean canAcknowledge() {
        return isActive();
    }

    /**
     * The badge rule, verbatim from the contract: active items of any severity.
     * Watch alerts are active items, so they are already counted here.
     */
    public static int badgeCount(List<AlertItem> alerts) {
        int n = 0;
        for (AlertItem a : alerts) {
            if (a.isActive()) {
                n++;
            }
        }
        return n;
    }

    public static int countType(List<AlertItem> alerts, String type) {
        int n = 0;
        for (AlertItem a : alerts) {
            if (type.equalsIgnoreCase(a.type)) {
                n++;
            }
        }
        return n;
    }

    // -------------------------------------------------------- presentation

    /** "WATCH", "BATTERY", "HEAT", "SMOKE". */
    public String typeLabel() {
        return type.toUpperCase(Locale.US);
    }

    /** The row's first line: "WATCH · node-02". */
    public String heading() {
        return typeLabel() + " \u00B7 " + nodeId;
    }

    /** "Watch" | "Notice" | "Warning" | "Critical" — the severity chip word. */
    public String severityWord() {
        if (severity == null || severity.isEmpty()) {
            return "";
        }
        return Character.toUpperCase(severity.charAt(0)) + severity.substring(1);
    }

    /** "Active" | "Acknowledged" | "Cleared" | "Snoozed" — the state chip word. */
    public String stateWord() {
        if (state == null || state.isEmpty()) {
            return "";
        }
        return Character.toUpperCase(state.charAt(0)) + state.substring(1);
    }

    /** True when the row carries a state chip (everything except a plain active item). */
    public boolean showsState() {
        return isAcknowledged() || isCleared() || isSnoozed();
    }

    /** "SEP 23 \u00B7 13:58" (UTC) for the cleared entry; "" otherwise. */
    public String stamp() {
        String iso = isCleared() && createdUtc != null && !createdUtc.isEmpty()
                ? createdUtc
                : "";
        if (iso.isEmpty()) {
            return "";
        }
        SimpleDateFormat in = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        in.setTimeZone(TimeZone.getTimeZone("UTC"));
        try {
            Date d = in.parse(iso);
            if (d == null) {
                return "";
            }
            SimpleDateFormat out = new SimpleDateFormat("MMM d \u00B7 HH:mm", Locale.US);
            out.setTimeZone(TimeZone.getTimeZone("UTC"));
            return out.format(d);
        } catch (Exception e) {
            return "";
        }
    }
}
