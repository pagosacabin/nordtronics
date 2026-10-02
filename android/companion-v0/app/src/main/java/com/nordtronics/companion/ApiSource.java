package com.nordtronics.companion;

/**
 * Where the app gets its {@link WildfireApi} (task 0090).
 *
 * <p>This is the <b>only</b> file that names an implementation. Screens ask for
 * {@code ApiSource.get()} and never import {@link MockApi} or
 * {@link HttpApiClient}, so swapping the data source when the backend ships the
 * v1 routes is a one-line change here and no screen code.
 *
 * <p>The instance is process-wide and, for the mock, stateful: acknowledging an
 * alert must survive a screen switch, which is exactly the behaviour the badge
 * count depends on.
 */
public final class ApiSource {

    private static volatile WildfireApi instance;

    private ApiSource() {
    }

    public static WildfireApi get() {
        WildfireApi local = instance;
        if (local == null) {
            synchronized (ApiSource.class) {
                local = instance;
                if (local == null) {
                    // Swap point: `new HttpApiClient()` once the backend serves
                    // /v1/alerts, /v1/network/status and the extended node fields.
                    local = new MockApi();
                    instance = local;
                }
            }
        }
        return local;
    }
}
