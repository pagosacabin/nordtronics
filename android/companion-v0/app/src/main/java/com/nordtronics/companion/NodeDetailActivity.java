package com.nordtronics.companion;

import android.os.Bundle;
import android.text.TextUtils;

import androidx.appcompat.app.AppCompatActivity;

/**
 * Screen 2 — Node detail, UI spec v2 (task 0090).
 *
 * <p>Shows the live banner, the 2×2 metric cards with their 12 h sparklines,
 * Link &amp; hardware (where the cell voltage lives), the 24-hour PM2.5 trend
 * with its three labelled threshold bands, and Recent history — the Sep-23
 * smoke event, shown as Cleared with a single "View trend" action.
 *
 * <p>The node is named by {@link #EXTRA_NODE_ID}; the screen defaults to
 * {@code node-01}, which is the node UI spec v2 documents.
 */
public class NodeDetailActivity extends AppCompatActivity {

    public static final String EXTRA_NODE_ID = "node_id";

    /** UI spec v2 documents Node 01; that is also the fallback. */
    private static final String DEFAULT_NODE = "node-01";

    private String nodeId = DEFAULT_NODE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_node_detail);
        Screens.shell(this);

        String extra = getIntent() == null ? null : getIntent().getStringExtra(EXTRA_NODE_ID);
        nodeId = TextUtils.isEmpty(extra) ? DEFAULT_NODE : extra;
    }

    @Override
    protected void onResume() {
        super.onResume();
        Screens.renderNodeDetail(this, nodeId);
    }
}
