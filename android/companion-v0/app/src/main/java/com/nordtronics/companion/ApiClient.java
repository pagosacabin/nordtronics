package com.nordtronics.companion;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * Tiny HTTP client for the Companion API.
 *
 * <p>Every server address and every request path the app uses is a {@code
 * BuildConfig} field declared in {@code app/build.gradle} — every build type
 * declares the production API on {@code nordtronics.io} DNS since task 0096
 * (before that, debug overrode it to the local mock). Nothing here (or anywhere
 * else) names a host or an endpoint path itself, so which backend the app talks
 * to is a build-type decision made in exactly one place.
 *
 * <p>The data source is still swappable: {@link ApiSource} prefers the live
 * {@link HttpApiClient} and falls back to the bundled {@link MockApi} when the
 * backend cannot be reached, which is what the local mock server used to
 * provide over HTTP. The two dialects are still both readable — the production
 * API answers with the wrapped payload shapes its OpenAPI document declares
 * ({@code {"nodes":[...]}}, nested {@code latest} readings, {@code temperature_c}
 * field names) and the local mock server still answers the flat shapes in
 * {@code mock-server/server.py} (a bare array, flattened readings, {@code temp_c}
 * field names). {@link #getNodes()} accepts either shape, and {@link Node}
 * reads either set of field names, so one set of screens serves both.
 */
public final class ApiClient {

    /** The single API base URL constant, read from the BuildConfig field. */
    public static final String BASE_URL = BuildConfig.API_BASE_URL;

    /**
     * A route this backend does not serve, answered 404. Subclasses
     * {@link IOException}, so callers that treat every failure the same keep
     * working; callers that can degrade for an absent route catch this one
     * (see {@link #getIfPresent(String)}).
     */
    public static class NotFoundException extends IOException {
        public final int code;
        public final String path;

        NotFoundException(int code, String path, String body) {
            super("HTTP " + code + " from " + path + ": " + body);
            this.code = code;
            this.path = path;
        }
    }

    /** Shown wherever a feature has no endpoint on this build's backend. */
    public static final String ALERTS_UNAVAILABLE =
            "Alert history is not served by the production API yet.";
    public static final String PING_UNAVAILABLE =
            "Ping is not served by the production API yet.";

    private static final int TIMEOUT_MS = 8000;

    private ApiClient() {
    }

    // ------------------------------------------------------------- endpoints

    /** The node-list path this build talks to ({@code /v1/nodes} in release). */
    public static String nodesPath() {
        return BuildConfig.PATH_NODES;
    }

    /**
     * The reachability probe path: {@code /healthz} in release. The mock has no
     * dedicated health route, so in debug this falls back to the node list,
     * which is the only endpoint it answers.
     */
    public static String healthPath() {
        return BuildConfig.PATH_HEALTH != null ? BuildConfig.PATH_HEALTH : BuildConfig.PATH_NODES;
    }

    /** True when this build's backend serves an alerts feed. */
    public static boolean alertsAvailable() {
        return BuildConfig.PATH_ALERTS != null;
    }

    /** True when this build's backend serves the node ping endpoint. */
    public static boolean pingAvailable() {
        return BuildConfig.PATH_NODE_PING != null;
    }

    /** The ping path for {@code nodeId}; only valid when {@link #pingAvailable()}. */
    public static String pingPath(String nodeId) {
        return String.format(BuildConfig.PATH_NODE_PING, nodeId);
    }

    // ------------------------------------------------------------ requests

    /** GET the node list, in either dialect. */
    public static JSONArray getNodes() throws IOException {
        return arrayFrom(get(nodesPath()), "nodes");
    }

    /** GET the alerts feed; throws when this build's backend has no such route. */
    public static JSONArray getAlerts() throws IOException {
        if (!alertsAvailable()) {
            throw new IOException(ALERTS_UNAVAILABLE);
        }
        return arrayFrom(get(BuildConfig.PATH_ALERTS), "alerts");
    }

    /** GET the reachability probe; returns the raw body (its fields are unused). */
    public static String getHealth() throws IOException {
        return get(healthPath());
    }

    /** POST the node ping; throws when this build's backend has no such route. */
    public static String postPing(String nodeId) throws IOException {
        if (!pingAvailable()) {
            throw new IOException(PING_UNAVAILABLE);
        }
        return post(pingPath(nodeId));
    }

    /** GET {@code path}; returns the raw JSON response body. */
    public static String get(String path) throws IOException {
        return request("GET", path);
    }

    /**
     * GET {@code path}, but treat a 404 as an answer rather than a failure:
     * returns {@code null} when the backend does not serve the route, the body
     * otherwise. The live backend answers 404 for contract routes it has not
     * implemented yet ({@code /v1/network/status}, {@code /v1/nodes/{id}}), and
     * a screen should degrade for that, not blank out.
     */
    public static String getIfPresent(String path) throws IOException {
        try {
            return request("GET", path);
        } catch (NotFoundException e) {
            return null;
        }
    }

    /**
     * POST {@code path} with an empty JSON object body; returns the raw JSON
     * response body (e.g. {@code {"ok":true,"node_id":"node-01"}}).
     */
    public static String post(String path) throws IOException {
        return request("POST", path);
    }

    /**
     * Reads a list response in either dialect: the array itself (mock) or an
     * object wrapping it under {@code key} (production).
     */
    private static JSONArray arrayFrom(String body, String key) throws IOException {
        String trimmed = body == null ? "" : body.trim();
        try {
            if (trimmed.startsWith("[")) {
                return new JSONArray(trimmed);
            }
            JSONArray arr = new JSONObject(trimmed).optJSONArray(key);
            return arr == null ? new JSONArray() : arr;
        } catch (JSONException e) {
            throw new IOException("Unrecognised response shape (no \"" + key + "\" list)");
        }
    }

    private static String request(String method, String path) throws IOException {
        HttpURLConnection conn = (HttpURLConnection) new URL(BASE_URL + path).openConnection();
        try {
            conn.setRequestMethod(method);
            conn.setConnectTimeout(TIMEOUT_MS);
            conn.setReadTimeout(TIMEOUT_MS);
            conn.setRequestProperty("Accept", "application/json");
            if ("POST".equals(method)) {
                conn.setDoOutput(true);
                conn.setRequestProperty("Content-Type", "application/json");
                conn.getOutputStream().write("{}".getBytes("UTF-8"));
            }
            int code = conn.getResponseCode();
            InputStream in = (code >= 200 && code < 300) ? conn.getInputStream() : conn.getErrorStream();
            String body = in == null ? "" : readAll(in);
            if (code == 404) {
                throw new NotFoundException(code, path, body);
            }
            if (code < 200 || code >= 300) {
                throw new IOException("HTTP " + code + " from " + path + ": " + body);
            }
            return body;
        } finally {
            conn.disconnect();
        }
    }

    private static String readAll(InputStream in) throws IOException {
        try {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            byte[] buf = new byte[4096];
            int n;
            while ((n = in.read(buf)) > 0) {
                out.write(buf, 0, n);
            }
            return out.toString("UTF-8");
        } finally {
            in.close();
        }
    }
}
