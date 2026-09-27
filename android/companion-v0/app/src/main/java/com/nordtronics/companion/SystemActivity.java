package com.nordtronics.companion;

import android.os.Bundle;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

/**
 * System / privacy — the v0.1 mockup's system view (task 0049).
 *
 * <p>Backend connection status (the result of a real request to the app's API
 * base URL, not a constant) — since task 0069 the reachability probe is the
 * live {@code GET /healthz} in release, falling back to the node-list path
 * where the backend has no health route — the privacy statement (only
 * environmental telemetry leaves the property) — the prototype/mock-mode
 * indicator, and About with the app version.
 */
public class SystemActivity extends AppCompatActivity {

    private TextView status;
    private TextView baseStatus;
    private TextView baseAddress;
    private TextView baseHeartbeat;
    private TextView protoSource;
    private TextView aboutVersion;
    private TextView aboutBuild;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_system);
        WindowInsetsHelper.applySystemBarInsets(this);

        status = findViewById(R.id.status);
        baseStatus = findViewById(R.id.base_status);
        baseAddress = findViewById(R.id.base_address);
        baseHeartbeat = findViewById(R.id.base_heartbeat);
        protoSource = findViewById(R.id.proto_source);
        aboutVersion = findViewById(R.id.about_version);
        aboutBuild = findViewById(R.id.about_build);

        baseAddress.setText(ApiClient.BASE_URL);
        protoSource.setText(ApiClient.BASE_URL + ApiClient.healthPath());
        aboutVersion.setText("v" + BuildConfig.VERSION_NAME);
        aboutBuild.setText(BuildConfig.VERSION_CODE + " \u00B7 " + BuildConfig.BUILD_TYPE.toUpperCase(Locale.US));

        Ui.bindNav(this, R.id.nav_system);

        // The "Prototype data" badge re-checks the connection.
        findViewById(R.id.demo_badge).setOnClickListener(v -> checkBackend());
        checkBackend();
    }

    /** A live read of the backend, so the status line on screen is earned. */
    private void checkBackend() {
        status.setText("GET " + ApiClient.BASE_URL + ApiClient.healthPath() + " \u2026");
        new Thread(() -> {
            try {
                ApiClient.getHealth();
                JSONArray arr = ApiClient.getNodes();
                final int count = arr.length();
                runOnUiThread(() -> {
                    baseStatus.setText(R.string.value_online);
                    baseStatus.setTextColor(Ui.col(this, R.color.nt_ok));
                    baseHeartbeat.setText(count + " NODES \u00B7 " + utcNow());
                    status.setText("Backend reachable at " + ApiClient.BASE_URL
                            + " \u2014 " + ApiClient.healthPath() + " ok \u00B7 " + count
                            + " nodes in " + ApiClient.nodesPath());
                });
            } catch (final Exception e) {
                runOnUiThread(() -> {
                    baseStatus.setText(R.string.value_unreachable);
                    baseStatus.setTextColor(Ui.col(this, R.color.nt_warn));
                    baseHeartbeat.setText("—");
                    status.setText("Backend unreachable: " + e.getMessage());
                });
            }
        }, "system-check").start();
    }

    private static String utcNow() {
        SimpleDateFormat f = new SimpleDateFormat("HH:mm:ss'Z'", Locale.US);
        f.setTimeZone(TimeZone.getTimeZone("UTC"));
        return f.format(new Date());
    }
}
