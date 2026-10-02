package com.nordtronics.companion;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

/**
 * Screen 1 — Property overview ("Property line"), UI spec v2 (task 0090).
 *
 * <p>The screen's content is built by {@link Screens#propertyOverview} from
 * {@link WildfireApi} data: the watch banner and reporting line come from
 * {@code /v1/network/status}, the 2×2 metric medians from the node list, and the
 * field-node cards from the node objects. Nothing on it is a literal.
 *
 * <p>It is still the launcher activity and it still owns the shared bottom nav;
 * the v0.1 network-consensus hero and stat grid it used to draw are superseded
 * by the v2 watch banner and metric card grid.
 */
public class NodesActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_nodes);
        Screens.shell(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Screens.renderPropertyOverview(this);
    }
}
