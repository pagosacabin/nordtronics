package com.nordtronics.companion;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * The real REST client for contract v1 — wired, but not yet the app's data
 * source (task 0090).
 *
 * <p>Every path it names is a path the contract defines. It is deliberately
 * <b>not</b> wired into {@link ApiSource} yet: the live backend answers
 * {@code /healthz} and {@code /v1/nodes} only, and {@code /v1/alerts} is still a
 * 404, so pointing the screens here today would blank the Alerts screen. The
 * switch is one line in {@code ApiSource} once the backend implements the three
 * new routes and the extended node fields.
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
        return new NetworkStatus(new JSONObject(ApiClient.get(PATH_NETWORK_STATUS)));
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
        return new NodeInfo(new JSONObject(
                ApiClient.get(String.format(Locale.US, PATH_NODE, nodeId))));
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
        JSONArray arr = new JSONObject(ApiClient.get(path)).optJSONArray("points");
        if (arr != null) {
            for (int i = 0; i < arr.length(); i++) {
                out.add(new TrendPoint(arr.optJSONObject(i)));
            }
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
