package com.nordtronics.factorydemo;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

/**
 * Task 0153 — the factory demo screen.
 *
 * One screen, built from platform widgets only: a greeting, a tap counter and a
 * build stamp so the installed APK is visibly the build CI produced.
 */
public class MainActivity extends Activity {

    private int count = 0;
    private TextView counterView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final int pad = dp(24);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER_HORIZONTAL);
        root.setPadding(pad, pad * 2, pad, pad);
        root.setBackgroundColor(Color.WHITE);

        TextView greeting = new TextView(this);
        greeting.setText("Hello, Stephen \u2014 built by the factory");
        greeting.setTextSize(TypedValue.COMPLEX_UNIT_SP, 26);
        greeting.setTextColor(Color.rgb(0x11, 0x2A, 0x22));
        greeting.setGravity(Gravity.CENTER);
        root.addView(greeting, matchWrap());

        TextView subtitle = new TextView(this);
        subtitle.setText("Juno\u2013Hermes software factory \u00b7 demo 0153");
        subtitle.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        subtitle.setTextColor(Color.rgb(0x55, 0x6B, 0x62));
        subtitle.setGravity(Gravity.CENTER);
        subtitle.setPadding(0, dp(8), 0, dp(32));
        root.addView(subtitle, matchWrap());

        counterView = new TextView(this);
        counterView.setText("0");
        counterView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 72);
        counterView.setTextColor(Color.rgb(0x0B, 0x6E, 0x4F));
        counterView.setGravity(Gravity.CENTER);
        root.addView(counterView, matchWrap());

        TextView taps = new TextView(this);
        taps.setText("taps");
        taps.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        taps.setTextColor(Color.rgb(0x55, 0x6B, 0x62));
        taps.setGravity(Gravity.CENTER);
        taps.setPadding(0, 0, 0, dp(24));
        root.addView(taps, matchWrap());

        Button button = new Button(this);
        button.setText("Tap to count");
        button.setAllCaps(false);
        button.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
        button.setOnClickListener(v -> {
            count++;
            counterView.setText(String.valueOf(count));
        });
        root.addView(button, matchWrap());

        TextView stamp = new TextView(this);
        stamp.setText("Build: " + BuildConfig.BUILD_STAMP + " \u00b7 " + BuildConfig.BUILD_SHA);
        stamp.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        stamp.setTextColor(Color.rgb(0x77, 0x88, 0x80));
        stamp.setGravity(Gravity.CENTER);
        stamp.setPadding(0, dp(40), 0, 0);
        root.addView(stamp, matchWrap());

        ScrollView scroll = new ScrollView(this);
        scroll.addView(root);
        setContentView(scroll);
    }

    private LinearLayout.LayoutParams matchWrap() {
        return new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }
}
