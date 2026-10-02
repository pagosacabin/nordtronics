package com.nordtronics.companion;

import java.util.List;

/**
 * The single interface every screen talks to (task 0090).
 *
 * <p>It mirrors the Wildfire Companion API contract v1
 * ({@code docs/wildfire/app-api-contract-v1-2026-10-02.md}): SI units on the
 * wire, UTC ISO-8601 timestamps, no coordinates. Two implementations exist:
 *
 * <ul>
 *   <li>{@link HttpApiClient} — the real REST client, and since task 0096 the
 *       app's primary data source ({@code https://api.nordtronics.io}).</li>
 *   <li>{@link MockApi} — the bundled sample state from UI spec v2, now the
 *       offline fallback {@link FallbackApi} serves when the backend cannot be
 *       reached.</li>
 * </ul>
 *
 * <p>Screens only ever name this interface, via {@link ApiSource}; swapping the
 * data source is a one-line change in {@code ApiSource} and no screen code. The
 * app is display + acknowledge only — the consensus rules run on the base
 * station, never here.
 */
public interface WildfireApi {

    /**
     * {@code GET /v1/network/status} — the property-wide consensus summary.
     *
     * @return the summary, or {@code null} when this backend does not serve the
     *         route (the live backend answers 404 today); the property screen
     *         then draws no watch banner rather than inventing a state.
     */
    NetworkStatus networkStatus() throws Exception;

    /** {@code GET /v1/nodes} — the field-node list. */
    List<NodeInfo> nodes() throws Exception;

    /** {@code GET /v1/nodes/{id}} — one node, with its 12 h trend arrays. */
    NodeInfo node(String nodeId) throws Exception;

    /** {@code GET /v1/alerts} — the alert feed. */
    List<AlertItem> alerts() throws Exception;

    /**
     * {@code GET /v1/nodes/{id}/readings?metric=&amp;hours=} — one metric time
     * series, oldest to newest. {@code metric} is one of {@code pm25},
     * {@code temperature_c}, {@code humidity_pct}, {@code battery_v}.
     */
    List<TrendPoint> readings(String nodeId, String metric, int hours) throws Exception;

    /** {@code POST /v1/alerts/{id}/acknowledge} — state moves to {@code acknowledged}. */
    void acknowledgeAlert(String alertId) throws Exception;

    /** Human-readable provenance of this implementation, for the mock banner. */
    String sourceLabel();
}
