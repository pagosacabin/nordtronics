package com.nordtronics.companion;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * Tiny HTTP client for the Companion API.
 *
 * The base URL for the whole app lives in exactly ONE place: the
 * {@code API_BASE_URL} BuildConfig field declared in {@code app/build.gradle}
 * (see the "THE API BASE URL" comment there). Nothing else in the app hardcodes
 * a host, so repointing at the real VPS API is a one-line change.
 */
public final class ApiClient {

    /** The single API base URL constant, read from the BuildConfig field. */
    public static final String BASE_URL = BuildConfig.API_BASE_URL;

    private static final int TIMEOUT_MS = 8000;

    private ApiClient() {
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
