package com.nordtronics.companion;

import android.content.Context;
import android.graphics.Typeface;
import android.util.TypedValue;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * Small helpers for building list rows in code. The app deliberately avoids
 * RecyclerView: two nodes / three alerts do not need an adapter, and this keeps
 * the dependency list to appcompat alone.
 */
final class Ui {

    private Ui() {
    }

    /** A three-line list row: bold title, reading line, muted third line. */
    static View row(Context c, String title, String line2, String line3, View.OnClickListener onTap) {
        LinearLayout box = new LinearLayout(c);
        box.setOrientation(LinearLayout.VERTICAL);
        box.setPadding(0, dp(c, 14), 0, dp(c, 14));

        TextView t1 = new TextView(c);
        t1.setText(title);
        t1.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
        t1.setTypeface(Typeface.DEFAULT_BOLD);
        box.addView(t1);

        TextView t2 = new TextView(c);
        t2.setText(line2);
        t2.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        box.addView(t2);

        TextView t3 = new TextView(c);
        t3.setText(line3);
        t3.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        t3.setAlpha(0.65f);
        box.addView(t3);

        View divider = new View(c);
        divider.setBackgroundColor(0x22000000);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, dp(c, 1));
        lp.topMargin = dp(c, 12);
        divider.setLayoutParams(lp);
        box.addView(divider);

        if (onTap != null) {
            box.setClickable(true);
            box.setOnClickListener(onTap);
        }
        return box;
    }

    /** A label/value line for the node-detail screen. */
    static View line(Context c, String label, String value) {
        LinearLayout row = new LinearLayout(c);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setPadding(0, dp(c, 6), 0, dp(c, 6));

        TextView l = new TextView(c);
        l.setText(label);
        l.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        l.setAlpha(0.65f);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(dp(c, 130),
                LinearLayout.LayoutParams.WRAP_CONTENT);
        l.setLayoutParams(lp);
        row.addView(l);

        TextView v = new TextView(c);
        v.setText(value);
        v.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
        v.setTypeface(Typeface.DEFAULT_BOLD);
        row.addView(v);
        return row;
    }

    static int dp(Context c, int value) {
        return (int) (value * c.getResources().getDisplayMetrics().density + 0.5f);
    }
}
