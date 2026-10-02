package com.nordtronics.companion;

/**
 * Where the app gets its {@link WildfireApi} (task 0090; flipped to the live
 * API by task 0096).
 *
 * <p>This is the <b>only</b> file that names an implementation. Screens ask for
 * {@code ApiSource.get()} and never import {@link MockApi},
 * {@link HttpApiClient} or {@link FallbackApi}, so the data source stays a
 * one-line change here and no screen code.
 *
 * <p>Since 0096 the app runs against the live backend: {@code get()} returns a
 * {@link FallbackApi} that prefers {@link HttpApiClient} and serves any failed
 * read from the bundled {@link MockApi}. {@link #usingMock()} reports which one
 * answered last, and the shell uses it to show or hide the "PROTOTYPE – MOCK
 * DATA" banner.
 *
 * <p>The instance is process-wide and, for the mock, stateful: acknowledging an
 * alert must survive a screen switch, which is exactly the behaviour the badge
 * count depends on.
 */
public final class ApiSource {

    private static volatile FallbackApi instance;

    private ApiSource() {
    }

    public static WildfireApi get() {
        FallbackApi local = instance;
        if (local == null) {
            synchronized (ApiSource.class) {
                local = instance;
                if (local == null) {
                    // Swap point (task 0096): the live client, with the bundled
                    // mock as the offline fallback. Before 0096 this line read
                    // `local = new MockApi()` — the app was mock-only.
                    local = new FallbackApi();
                    instance = local;
                }
            }
        }
        return local;
    }

    /**
     * True when the most recent read was served by the bundled mock, i.e. the
     * backend was unreachable. False before the first read, so a live build
     * never flashes the mock banner.
     */
    public static boolean usingMock() {
        FallbackApi local = instance;
        return local != null && local.mockInUse();
    }
}
