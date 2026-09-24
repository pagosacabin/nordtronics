package com.nordtronics.companion;

import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

/**
 * One alert-history entry, exactly as {@code GET /api/alerts} returns it, plus
 * the severity/type presentation the v0.1 Alerts screen needs (task 0049).
 *
 * <p>The mock payload carries no explicit severity, so the screen derives one
 * from the alert type (smoke and heat are warnings, battery and everything else
 * are notices) and says so on the System screen's prototype panel rather than
 * presenting the derivation as field data.
 */
public class Alert {

    /** An alert this fresh counts as "active" on the Alerts screen. */
    public static final long ACTIVE_WINDOW_SECONDS = 6 * 3600;

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

    public String typeKey() {
        return type == null ? "" : type.toLowerCase(Locale.US);
    }

    public String title() {
        return type.toUpperCase(Locale.US) + "  \u00B7  " + nodeId;
    }

    public String detailLine() {
        return message;
    }

    /** True for the tones the mockup paints as warnings (smoke, heat). */
    public boolean warning() {
        return "smoke".equals(typeKey()) || "heat".equals(typeKey());
    }

    public String severityLabel() {
        return warning() ? "SEVERITY: WARNING" : "SEVERITY: NOTICE";
    }

    /** True while this alert is inside the active window. */
    public boolean active() {
        long age = ageSeconds();
        return age >= 0 && age <= ACTIVE_WINDOW_SECONDS;
    }

    public long ageSeconds() {
        if (at == null || at.isEmpty()) {
            return -1;
        }
        SimpleDateFormat f = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        f.setTimeZone(TimeZone.getTimeZone("UTC"));
        try {
            Date d = f.parse(at);
            return d == null ? -1 : Math.max(0, (System.currentTimeMillis() - d.getTime()) / 1000);
        } catch (Exception e) {
            return -1;
        }
    }

    /** "SEP 23 · 13:58" (UTC), or the raw string when it is not a timestamp. */
    public String timeLabel() {
        if (at == null || at.isEmpty()) {
            return "";
        }
        SimpleDateFormat in = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        in.setTimeZone(TimeZone.getTimeZone("UTC"));
        try {
            Date d = in.parse(at);
            if (d == null) {
                return at;
            }
            SimpleDateFormat out = new SimpleDateFormat("MMM d \u00B7 HH:mm", Locale.US);
            out.setTimeZone(TimeZone.getTimeZone("UTC"));
            return out.format(d).toUpperCase(Locale.US);
        } catch (Exception e) {
            return at;
        }
    }
}
