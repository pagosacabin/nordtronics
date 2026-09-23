package com.nordtronics.companion;

import android.os.Bundle;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;

/** Screen 3 — full readings for one node, plus the ping round-trip. */
public class NodeDetailActivity extends AppCompatActivity {

    public static final String EXTRA_NODE_ID = "node_id";

    private LinearLayout detail;
    private TextView pingResult;
    private TextView status;
    private String nodeId = "node-01";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_node_detail);
        WindowInsetsHelper.applySystemBarInsets(this);

        detail = findViewById(R.id.detail);
        pingResult = findViewById(R.id.ping_result);
        status = findViewById(R.id.status);

        String extra = getIntent().getStringExtra(EXTRA_NODE_ID);
        if (extra != null && !extra.isEmpty()) {
            nodeId = extra;
        }
        // The node id used to be the action bar title; it is now the header bar.
        ((TextView) findViewById(R.id.header)).setText(getString(R.string.detail_title) + " — " + nodeId);

        findViewById(R.id.btn_ping).setOnClickListener(v -> sendPing());
        loadNode();
    }

    private void loadNode() {
        status.setText("GET " + ApiClient.BASE_URL + "/api/nodes …");
        new Thread(() -> {
            try {
                JSONArray arr = new JSONArray(ApiClient.get("/api/nodes"));
                Node found = null;
                for (int i = 0; i < arr.length(); i++) {
                    Node n = new Node(arr.getJSONObject(i));
                    if (nodeId.equals(n.id)) {
                        found = n;
                        break;
                    }
                }
                final Node node = found;
                runOnUiThread(() -> {
                    if (node == null) {
                        detail.removeAllViews();
                        status.setText("Node " + nodeId + " not in /api/nodes");
                    } else {
                        renderDetail(node);
                    }
                });
            } catch (final Exception e) {
                runOnUiThread(() -> status.setText("Request failed: " + e.getMessage()));
            }
        }, "node-fetch").start();
    }

    private void renderDetail(Node n) {
        detail.removeAllViews();
        detail.addView(Ui.line(this, "Node", n.id));
        detail.addView(Ui.line(this, "PM2.5", String.format("%.1f µg/m³", n.pm25)));
        detail.addView(Ui.line(this, "Temperature", String.format("%.1f °C", n.tempC)));
        detail.addView(Ui.line(this, "Humidity", String.format("%.0f %%RH", n.humidity)));
        detail.addView(Ui.line(this, "Battery", String.format("%.2f V", n.batteryV)));
        detail.addView(Ui.line(this, "Last seen", n.lastSeen));
        status.setText("Readings from " + ApiClient.BASE_URL + "/api/nodes");
    }

    private void sendPing() {
        final String url = ApiClient.BASE_URL + "/api/nodes/" + nodeId + "/ping";
        pingResult.setText("POST " + url + " …");
        status.setText("POST " + url + " …");
        new Thread(() -> {
            try {
                final String body = ApiClient.post("/api/nodes/" + nodeId + "/ping");
                runOnUiThread(() -> {
                    pingResult.setText("Ping confirmed by server:\n" + body);
                    status.setText("POST " + url + " → 200 OK");
                });
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    pingResult.setText("Ping failed: " + e.getMessage());
                    status.setText("POST " + url + " failed");
                });
            }
        }, "ping-post").start();
    }
}
