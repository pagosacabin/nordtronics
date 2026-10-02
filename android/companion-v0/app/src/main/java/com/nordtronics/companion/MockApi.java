package com.nordtronics.companion;

import org.json.JSONArray;
import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Random;
import java.util.TimeZone;

/**
 * The mock implementation of {@link WildfireApi} (task 0090).
 *
 * <p>It serves the exact mock state from UI spec v2 — Node 01 healthy, Node 02
 * on watch, the four alerts with their states, badge 2 — using the payload
 * shapes of contract v1, so the app is built against the real wire format and
 * the backend only has to implement the contract later. Every screen reads this
 * through {@link WildfireApi}; nothing in {@code res/} or in an activity names
 * this class.
 *
 * <p>Two deliberate properties:
 * <ul>
 *   <li><b>Times are relative to now.</b> The spec says "Updated 4 min ago" and
 *       "Last packet received 4 min ago"; a frozen absolute timestamp would read
 *       as days-old staleness the moment the build is a day old, so the packet
 *       instants are generated at {@code now - 4 min} per call and the contract's
 *       own {@code age_seconds} carries the 240 s.</li>
 *   <li><b>Alerts are mutable state.</b> {@code acknowledgeAlert} moves an
 *       {@code active} item to {@code acknowledged} in memory, which is what the
 *       nav badge and the Acknowledge button read. The state machine is the
 *       contract's; the base station owns the real one.</li>
 * </ul>
 */
public class MockApi implements WildfireApi {

    /** UI spec v2: every screen shows the same 4-minute-old packet. */
    private static final long PACKET_AGE_SECONDS = 240;

    private static final String STATE_ACTIVE = "active";
    private static final String STATE_ACKNOWLEDGED = "acknowledged";
    private static final String STATE_CLEARED = "cleared";

    /** The four alerts of UI spec v2 / contract v1, newest first. */
    private final List<JSONObject> alertState = new ArrayList<>();

    public MockApi() {
        alertState.add(alert(
                "al-004", "watch", "node-02", "watch", STATE_ACTIVE,
                "PM2.5 47.9 \u00B5g/m\u00B3 on one node, awaiting confirmation.",
                "Elevated on a single node. Escalates only if a second node confirms.",
                "2026-10-02T09:40:00Z"));
        alertState.add(alert(
                "al-003", "battery", "node-02", "notice", STATE_ACTIVE,
                "Battery below 3.8 V.",
                "3.71 V at last report.",
                "2026-10-02T08:12:00Z"));
        alertState.add(alert(
                "al-002", "heat", "node-02", "warning", STATE_ACKNOWLEDGED,
                "Temperature above 27 \u00B0C.",
                "28.1 \u00B0C at last report.",
                "2026-10-02T07:06:00Z"));
        alertState.add(alert(
                "al-001", "smoke", "node-01", "warning", STATE_CLEARED,
                "PM2.5 rising fast.",
                "Cleared after node returned to baseline.",
                "2026-09-23T13:58:00Z"));
    }

    private static JSONObject alert(String id, String type, String nodeId, String severity,
                                    String state, String title, String detail, String created) {
        JSONObject o = new JSONObject();
        try {
            o.put("alert_id", id);
            o.put("type", type);
            o.put("node_id", nodeId);
            o.put("severity", severity);
            o.put("state", state);
            o.put("title", title);
            o.put("detail", detail);
            o.put("created_utc", created);
            o.put("updated_utc", created);
        } catch (Exception e) {
            throw new IllegalStateException("mock alert " + id, e);
        }
        return o;
    }

    // ------------------------------------------------------------ WildfireApi

    @Override
    public NetworkStatus networkStatus() {
        JSONObject o = new JSONObject();
        try {
            o.put("state", "watch");
            o.put("nodes_reporting", 2);
            o.put("nodes_total", 2);
            o.put("outside_limits", new JSONArray().put("node-02"));
            o.put("note", "1 of 2 nodes outside limits. Rest of network at baseline.");
            o.put("last_packet_utc", isoNow(-PACKET_AGE_SECONDS));
            o.put("generated_utc", isoNow(0));
        } catch (Exception e) {
            throw new IllegalStateException("mock network status", e);
        }
        return new NetworkStatus(o);
    }

    @Override
    public List<NodeInfo> nodes() {
        List<NodeInfo> out = new ArrayList<>();
        out.add(new NodeInfo(nodePayload("node-01")));
        out.add(new NodeInfo(nodePayload("node-02")));
        return out;
    }

    @Override
    public NodeInfo node(String nodeId) {
        return new NodeInfo(nodePayload(nodeId));
    }

    @Override
    public List<AlertItem> alerts() {
        List<AlertItem> out = new ArrayList<>();
        for (JSONObject o : alertState) {
            out.add(new AlertItem(o));
        }
        return out;
    }

    @Override
    public List<TrendPoint> readings(String nodeId, String metric, int hours) {
        int count = Math.max(2, hours * 4);          // contract: <= 1 point per 15 min
        long stepSeconds = Math.max(900, hours * 3600L / count);
        double base = "temperature_c".equals(metric) ? 21.3
                : "humidity_pct".equals(metric) ? 38.0
                : "battery_v".equals(metric) ? 4.05 : 12.4;
        double amplitude = "temperature_c".equals(metric) ? 2.4
                : "humidity_pct".equals(metric) ? 6.0
                : "battery_v".equals(metric) ? 0.05 : 3.6;
        List<Double> values = synth(seed(nodeId, metric, hours), count, base, amplitude, true);
        List<TrendPoint> out = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            long secondsAgo = (count - 1 - i) * stepSeconds;
            out.add(new TrendPoint(isoNow(-secondsAgo), values.get(i)));
        }
        return out;
    }

    @Override
    public void acknowledgeAlert(String alertId) {
        for (JSONObject o : alertState) {
            if (alertId.equals(o.optString("alert_id"))) {
                if (STATE_ACTIVE.equals(o.optString("state"))) {
                    try {
                        o.put("state", STATE_ACKNOWLEDGED);
                        o.put("updated_utc", isoNow(0));
                    } catch (Exception e) {
                        throw new IllegalStateException("acknowledge " + alertId, e);
                    }
                }
                return;
            }
        }
    }

    @Override
    public String sourceLabel() {
        return "local mock backend";
    }

    // ---------------------------------------------------------------- payloads

    /**
     * The v1 node object. Node 01 and Node 02 carry exactly the values UI spec v2
     * pins: 12.4 µg/m³ / 70 °F / 38 % RH / 87 % charging, and 47.9 µg/m³ / 82 °F /
     * 22 % RH / 3.71 V.
     */
    private JSONObject nodePayload(String nodeId) {
        boolean one = !"node-02".equals(nodeId);
        JSONObject o = new JSONObject();
        try {
            o.put("node_id", nodeId);
            o.put("display_name", one ? "Node 01" : "Node 02");
            // Spec-literal display identifier ("Node ID: NT-01-7A3F"). The contract
            // lists `serial` (I-01-7A3F-22) but no short display id, so the mock
            // carries it as one more additive field.
            o.put("display_id", one ? "NT-01-7A3F" : "NT-02-4C1B");
            o.put("zone", one ? "Zone 7" : "Zone 5");
            o.put("location_label", one ? "Sierra Ridge Trail" : "Creek Bench");
            o.put("firmware", "v2.3.1");
            o.put("hw_rev", "HW Rev 4");
            o.put("serial", one ? "I-01-7A3F-22" : "I-02-4C1B-19");
            o.put("status", "live");
            o.put("last_seen_utc", isoNow(-PACKET_AGE_SECONDS));
            o.put("age_seconds", PACKET_AGE_SECONDS);
            o.put("rssi_dbm", one ? -68 : -79);
            o.put("link_margin_db", one ? 32 : 18);
            o.put("gateway_hops", 1);
            // Node 01 reports a pack percentage; node 02's row shows its cell
            // voltage instead (UI spec v2), so it carries no `battery_pct` and
            // the card falls back to `battery_v` — never a derived percentage.
            if (one) {
                o.put("battery_pct", 87);
            }
            o.put("charging", one);
            o.put("charge_today_pct", one ? 6 : 0);

            JSONObject latest = new JSONObject();
            latest.put("pm25", one ? 12.4 : 47.9);
            latest.put("temperature_c", one ? 21.3 : 27.8);
            latest.put("humidity_pct", one ? 38.0 : 22.0);
            latest.put("battery_v", one ? 4.05 : 3.71);
            o.put("latest", latest);

            JSONObject trends = new JSONObject();
            trends.put("pm25", array(synth(seed(nodeId, "pm25", 12), 25,
                    one ? 12.1 : 44.0, one ? 1.1 : 6.0, false)));
            trends.put("temperature_c", array(synth(seed(nodeId, "temperature_c", 12), 25,
                    one ? 21.0 : 27.4, 1.6, false)));
            trends.put("humidity_pct", array(synth(seed(nodeId, "humidity_pct", 12), 25,
                    one ? 38.5 : 22.5, 3.0, false)));
            o.put("trends_12h", trends);
        } catch (Exception e) {
            throw new IllegalStateException("mock node " + nodeId, e);
        }
        return o;
    }

    /** JSONArray.put throws a checked JSONException; this keeps the call sites clean. */
    private static JSONArray array(List<Double> values) {
        JSONArray a = new JSONArray();
        for (Double v : values) {
            try {
                a.put(v.doubleValue());
            } catch (Exception e) {
                throw new IllegalStateException("mock trend array", e);
            }
        }
        return a;
    }
    // -------------------------------------------------------------- synthesis

    private static long seed(String nodeId, String metric, int hours) {
        long h = 1125899906842597L;
        String key = nodeId + "/" + metric + "/" + hours;
        for (int i = 0; i < key.length(); i++) {
            h = 31 * h + key.charAt(i);
        }
        return h;
    }

    /**
     * A deterministic, documented trace: a slow rise/fall (a diurnal shape when
     * {@code diurnal}) plus bounded jitter, seeded per node+metric so the same
     * build always draws the same line. Values are clamped at zero.
     */
    private static List<Double> synth(long seed, int count, double base, double amplitude,
                                       boolean diurnal) {
        Random r = new Random(seed);
        List<Double> out = new ArrayList<>(count);
        for (int i = 0; i < count; i++) {
            double shape = diurnal
                    ? Math.sin(Math.PI * 2.0 * i / Math.max(2, count)) * amplitude * 0.45
                    : 0.0;
            double drift = amplitude * 0.35 * (r.nextDouble() - 0.5) * 2.0;
            out.add(Math.max(0.0, base + shape + drift));
        }
        return out;
    }

    // ---------------------------------------------------------------- helpers

    private static String isoNow(long offsetSeconds) {
        SimpleDateFormat f = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        f.setTimeZone(TimeZone.getTimeZone("UTC"));
        return f.format(new Date(System.currentTimeMillis() + offsetSeconds * 1000L));
    }
}
