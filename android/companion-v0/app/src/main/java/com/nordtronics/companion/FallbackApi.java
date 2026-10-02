package com.nordtronics.companion;

import java.util.List;

/**
 * The data source {@link ApiSource} hands to the screens (task 0096): the live
 * REST client first, the bundled mock only when the backend cannot be reached.
 *
 * <p>This is the "mock still available as the offline fallback" rule, and it is
 * deliberately per call, not a mode switch: each read tries {@link HttpApiClient}
 * and, on any failure (no network, DNS, TLS, a non-2xx the client does not
 * degrade for), serves the same call from {@link MockApi}. The screens are
 * written against {@link WildfireApi}, so they neither know nor care which
 * answered — but {@link #mockInUse()} records it, and {@code Screens} shows the
 * "PROTOTYPE – MOCK DATA" banner while it is true, so mock data is always
 * labelled and never passed off as live.
 *
 * <p>An absent route is not a failure: {@code HttpApiClient.networkStatus()}
 * returns {@code null} for the 404 the live backend answers on
 * {@code /v1/network/status}, and a null there is a successful live read (the
 * screen omits the watch banner) rather than a reason to fall back.
 *
 * <p>The mock instance is process-wide and stateful, which is what the badge
 * count needs: acknowledging an alert offline must survive a screen switch.
 */
class FallbackApi implements WildfireApi {

    private final WildfireApi live = new HttpApiClient();
    private final WildfireApi mock = new MockApi();

    /** True when the most recent read was served by the bundled mock. */
    private volatile boolean mockInUse = false;

    boolean mockInUse() {
        return mockInUse;
    }

    @Override
    public NetworkStatus networkStatus() throws Exception {
        try {
            NetworkStatus status = live.networkStatus();
            mockInUse = false;
            return status;
        } catch (Exception e) {
            mockInUse = true;
            return mock.networkStatus();
        }
    }

    @Override
    public List<NodeInfo> nodes() throws Exception {
        try {
            List<NodeInfo> nodes = live.nodes();
            mockInUse = false;
            return nodes;
        } catch (Exception e) {
            mockInUse = true;
            return mock.nodes();
        }
    }

    @Override
    public NodeInfo node(String nodeId) throws Exception {
        try {
            NodeInfo node = live.node(nodeId);
            mockInUse = false;
            return node;
        } catch (Exception e) {
            mockInUse = true;
            return mock.node(nodeId);
        }
    }

    @Override
    public List<AlertItem> alerts() throws Exception {
        try {
            List<AlertItem> alerts = live.alerts();
            mockInUse = false;
            return alerts;
        } catch (Exception e) {
            mockInUse = true;
            return mock.alerts();
        }
    }

    @Override
    public List<TrendPoint> readings(String nodeId, String metric, int hours) throws Exception {
        try {
            List<TrendPoint> points = live.readings(nodeId, metric, hours);
            mockInUse = false;
            return points;
        } catch (Exception e) {
            mockInUse = true;
            return mock.readings(nodeId, metric, hours);
        }
    }

    @Override
    public void acknowledgeAlert(String alertId) throws Exception {
        try {
            live.acknowledgeAlert(alertId);
            mockInUse = false;
        } catch (Exception e) {
            // Offline: the mock runs the same state machine, so the badge still
            // follows an acknowledge. The base station owns the real state.
            mockInUse = true;
            mock.acknowledgeAlert(alertId);
        }
    }

    @Override
    public String sourceLabel() {
        return mockInUse ? "MOCK (bundled sample data)"
                : live.sourceLabel();
    }
}
