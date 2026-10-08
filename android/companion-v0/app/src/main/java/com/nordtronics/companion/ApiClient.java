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
 * BuildConfig} field declared in {@code app/build.gradle} — the release build
 * type declares the production API on {@code nordtronics.io} DNS, the debug
 * build type declares the local mock. Nothing here (or anywhere else) names a
 * host or an endpoint path itself, so which backend the app talks to is a
 * build-type decision made in exactly one place.
 *
 * <p>The two backends speak two dialects. The production API answers with the
 * wrapped payload shapes its OpenAPI document declares ({@code {"nodes":[...]}},
 * nested {@code latest} readings, {@code temperature_c} field names); the local
 * mock answers with the flat shapes in {@code mock-server/server.py} (a bare
 * array, flattened readings, {@code temp_c} field names). {@link #getNodes()}
 * accepts either shape, and {@link Node} reads either set of field names, so
 * one set of screens serves both.
 */
public final class ApiClient {

    /** The single API base URL constant, read from the BuildConfig field. */
    public static final String BASE_URL = BuildConfig.API_BASE_URL;

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
            // Auth for the production API: the release build bakes the shared
            // key in as BuildConfig.API_KEY. Debug builds (local mock) carry
            // an empty key and skip the header.
            String apiKey = BuildConfig.API_KEY;
            if (apiKey != null && !apiKey.isEmpty()) {
                conn.setRequestProperty("X-API-Key", apiKey);
            }
            if ("POST".equals(method)) {
                conn.setDoOutput(true);
                conn.setRequestProperty("Content-Type", "application/json");
                conn.getOutputStream().write("{}".getBytes("UTF-8"));
            }
            int code = conn.getResponseCode();
            InputStream in = (code >= 200 && code < 300) ? conn.getInputStream() : conn.getErrorStream();
            String body = in == null ? "" : readAll(in);
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
