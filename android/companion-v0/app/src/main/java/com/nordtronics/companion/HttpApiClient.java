package com.nordtronics.companion;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * The real REST client for contract v1 — the app's primary data source since
 * task 0096 ({@link ApiSource} prefers this and falls back to {@link MockApi}
 * only when the backend is unreachable).
 *
 * <p>Every path it names is a path the contract defines. The live backend is a
 * subset of the contract today (confirmed 2026-10-02, task 0096): it serves
 * {@code /healthz}, {@code /v1/nodes}, {@code /v1/nodes/{id}/readings} and
 * {@code /v1/alerts}, but <b>not</b> {@code /v1/network/status} or
 * {@code /v1/nodes/{id}}, and its readings payload carries one row per reading
 * (every metric on the row) rather than contract v1's {@code points:[{t,v}]}.
 * Three deliberate degradations keep the screens real without inventing data:
 *
 * <ul>
 *   <li>{@link #networkStatus()} returns {@code null} for a 404 — the property
 *       screen omits the watch banner when there is no summary to show
 *       ({@code Screens.propertyOverview} guards the null). The app never
 *       computes consensus itself: that rule belongs to the base station.</li>
 *   <li>{@link #node(String)} falls back to the node's entry in
 *       {@code GET /v1/nodes}, which carries the same node object (including
 *       {@code latest}); only a node the backend has never heard from throws.</li>
 *   <li>{@link #readings} accepts either payload: contract v1's
 *       {@code points}, or the deployed {@code readings} rows, selecting the
 *       requested metric off each row and reversing into the contract's
 *       oldest-to-newest order.</li>
 * </ul>
 *
 * <p>Requests go through {@link ApiClient}, so the base URL stays the single
 * BuildConfig field it has been since task 0069.
 */
public class HttpApiClient implements WildfireApi {

    // The contract's paths, in one place.
    static final String PATH_NETWORK_STATUS = "/v1/network/status";
    static final String PATH_NODES = "/v1/nodes";
    static final String PATH_NODE = "/v1/nodes/%s";
    static final String PATH_READINGS = "/v1/nodes/%s/readings?metric=%s&hours=%d";
    static final String PATH_ACK = "/v1/alerts/%s/acknowledge";
    // The alerts feed path is NOT declared here: it comes from the BuildConfig
    // field (`ApiClient.PATH_ALERTS`), which is where the build type decides
    // whether this backend serves one at all. A literal here would bypass that
    // gate and point at a route the build may not have (task 0095).

    @Override
    public NetworkStatus networkStatus() throws Exception {
        // 404 = this backend does not serve the consensus summary yet. That is
        // an answer, not a failure: return null and let the screen show the
        // node list without a banner, rather than falling back to mock data.
        String body = ApiClient.getIfPresent(PATH_NETWORK_STATUS);
        return body == null ? null : new NetworkStatus(new JSONObject(body));
    }

    @Override
    public List<NodeInfo> nodes() throws Exception {
        List<NodeInfo> out = new ArrayList<>();
        JSONArray arr = new JSONObject(ApiClient.get(PATH_NODES)).optJSONArray("nodes");
        if (arr != null) {
            for (int i = 0; i < arr.length(); i++) {
                out.add(new NodeInfo(arr.optJSONObject(i)));
            }
        }
        return out;
    }

    @Override
    public NodeInfo node(String nodeId) throws Exception {
        String body = ApiClient.getIfPresent(String.format(Locale.US, PATH_NODE, nodeId));
        if (body != null) {
            return new NodeInfo(new JSONObject(body));
        }
        // No GET /v1/nodes/{id} on this backend: the node list entry is the same
        // node object, `latest` reading included, so the detail screen is served
        // from it. Only an id the backend has no telemetry for throws.
        for (NodeInfo n : nodes()) {
            if (nodeId.equals(n.nodeId)) {
                return n;
            }
        }
        throw new IOException("unknown node: " + nodeId);
    }

    @Override
    public List<AlertItem> alerts() throws Exception {
        List<AlertItem> out = new ArrayList<>();
        // ApiClient.getAlerts() reads the BuildConfig path and throws the
        // explained IOException when this build's backend has no feed, so a
        // build without an alerts route fails with a sentence, not an HTTP 404.
        JSONArray arr = ApiClient.getAlerts();
        for (int i = 0; i < arr.length(); i++) {
            out.add(new AlertItem(arr.optJSONObject(i)));
        }
        return out;
    }

    @Override
    public List<TrendPoint> readings(String nodeId, String metric, int hours) throws Exception {
        List<TrendPoint> out = new ArrayList<>();
        String path = String.format(Locale.US, PATH_READINGS, nodeId, metric, hours);
        JSONObject payload = new JSONObject(ApiClient.get(path));

        JSONArray points = payload.optJSONArray("points");
        if (points != null) {
            // Contract v1 shape: {"points":[{"t":"...Z","v":9.8}, ...]}, already
            // oldest to newest and already the requested metric.
            for (int i = 0; i < points.length(); i++) {
                out.add(new TrendPoint(points.optJSONObject(i)));
            }
            return out;
        }

        // Deployed shape: {"readings":[{recorded_utc, observed_utc, pm25,
        // temperature_c, humidity_pct, battery_v}, ...]}, newest first and
        // carrying every metric on each row. Take the requested metric off each
        // row and reverse into the contract's oldest-to-newest order.
        JSONArray rows = payload.optJSONArray("readings");
        if (rows == null) {
            return out;
        }
        for (int i = rows.length() - 1; i >= 0; i--) {
            JSONObject row = rows.optJSONObject(i);
            if (row == null) {
                continue;
            }
            String stamp = row.optString("observed_utc", row.optString("recorded_utc", ""));
            out.add(new TrendPoint(stamp, row.optDouble(metric, Double.NaN)));
        }
        return out;
    }

    @Override
    public void acknowledgeAlert(String alertId) throws Exception {
        ApiClient.post(String.format(Locale.US, PATH_ACK, alertId));
    }

    @Override
    public String sourceLabel() {
        return ApiClient.BASE_URL;
    }
}
