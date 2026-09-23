package com.nordtronics.companion;

import android.content.Intent;
import android.os.Bundle;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;

import java.util.ArrayList;
import java.util.List;

/** Screen 2 — alert history: type, node, message, timestamp. */
public class AlertsActivity extends AppCompatActivity {

    private LinearLayout list;
    private TextView status;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_alerts);
        WindowInsetsHelper.applySystemBarInsets(this);

        list = findViewById(R.id.list);
        status = findViewById(R.id.status);

        findViewById(R.id.btn_refresh).setOnClickListener(v -> load());
        findViewById(R.id.btn_nodes).setOnClickListener(v -> {
            startActivity(new Intent(this, NodesActivity.class));
            finish();
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        load();
    }

    private void load() {
        status.setText("GET " + ApiClient.BASE_URL + "/api/alerts …");
        new Thread(() -> {
            try {
                JSONArray arr = new JSONArray(ApiClient.get("/api/alerts"));
                final List<Alert> alerts = new ArrayList<>();
                for (int i = 0; i < arr.length(); i++) {
                    alerts.add(new Alert(arr.getJSONObject(i)));
                }
                runOnUiThread(() -> render(alerts));
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    list.removeAllViews();
                    status.setText("Request failed: " + e.getMessage());
                });
            }
        }, "alerts-fetch").start();
    }

    private void render(List<Alert> alerts) {
        list.removeAllViews();
        status.setText(alerts.size() + " alerts from " + ApiClient.BASE_URL + "/api/alerts");
        for (Alert a : alerts) {
            list.addView(Ui.row(this, a.title(), a.detailLine(), a.atLine(), null));
        }
    }
}
