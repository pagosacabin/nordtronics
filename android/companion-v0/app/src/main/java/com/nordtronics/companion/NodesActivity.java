package com.nordtronics.companion;

import android.content.Intent;
import android.os.Bundle;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;

import java.util.ArrayList;
import java.util.List;

/** Screen 1 — the node list: PM2.5, temperature, humidity, battery, last seen. */
public class NodesActivity extends AppCompatActivity {

    private LinearLayout list;
    private TextView status;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_nodes);

        list = findViewById(R.id.list);
        status = findViewById(R.id.status);

        findViewById(R.id.btn_refresh).setOnClickListener(v -> load());
        findViewById(R.id.btn_alerts).setOnClickListener(
                v -> startActivity(new Intent(this, AlertsActivity.class)));
    }

    @Override
    protected void onResume() {
        super.onResume();
        load();
    }

    private void load() {
        status.setText("GET " + ApiClient.BASE_URL + "/api/nodes …");
        new Thread(() -> {
            try {
                JSONArray arr = new JSONArray(ApiClient.get("/api/nodes"));
                final List<Node> nodes = new ArrayList<>();
                for (int i = 0; i < arr.length(); i++) {
                    nodes.add(new Node(arr.getJSONObject(i)));
                }
                runOnUiThread(() -> render(nodes));
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    list.removeAllViews();
                    status.setText("Request failed: " + e.getMessage());
                });
            }
        }, "nodes-fetch").start();
    }

    private void render(List<Node> nodes) {
        list.removeAllViews();
        status.setText(nodes.size() + " nodes from " + ApiClient.BASE_URL + "/api/nodes");
        for (final Node n : nodes) {
            list.addView(Ui.row(this, n.title(), n.readingLine(), n.lastSeenLine(), v -> {
                Intent detail = new Intent(this, NodeDetailActivity.class);
                detail.putExtra(NodeDetailActivity.EXTRA_NODE_ID, n.id);
                startActivity(detail);
            }));
        }
    }
}
