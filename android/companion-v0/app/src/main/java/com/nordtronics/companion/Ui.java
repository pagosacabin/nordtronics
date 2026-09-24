package com.nordtronics.companion;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.Locale;

/**
 * Building blocks for the v0.1 screens (task 0049).
 *
 * <p>The screens in {@code res/layout} carry the static structure — brand row,
 * hero, stat grid, panel chrome — and this class builds everything that depends
 * on API data: node cards, alert rows, metric tiles, health bars, the node glyph
 * and the bottom-nav selection. No RecyclerView is used (the app's dependency
 * list stays appcompat-only) and no new artwork is added: the glyph is drawn
 * with a Paint, so the repo's branding assets remain the only images shipped.
 */
final class Ui {

    private Ui() {
    }

    // ---------------------------------------------------------------- sizing

    static int dp(Context c, float value) {
        return Math.round(value * c.getResources().getDisplayMetrics().density);
    }

    static int col(Context c, int colorRes) {
        return c.getColor(colorRes);
    }

    // --------------------------------------------------------- view builders

    static LinearLayout column(Context c) {
        LinearLayout box = new LinearLayout(c);
        box.setOrientation(LinearLayout.VERTICAL);
        return box;
    }

    static LinearLayout row(Context c) {
        LinearLayout box = new LinearLayout(c);
        box.setOrientation(LinearLayout.HORIZONTAL);
        return box;
    }

    /** A text view with the app's three type treatments: body, label, mono. */
    static TextView text(Context c, CharSequence value, float sp, int color, boolean bold) {
        TextView t = new TextView(c);
        t.setText(value);
        t.setTextSize(TypedValue.COMPLEX_UNIT_SP, sp);
        t.setTextColor(color);
        if (bold) {
            t.setTypeface(Typeface.DEFAULT_BOLD);
        }
        return t;
    }

    /** Monospaced figures — every measurement on these screens uses them. */
    static TextView mono(Context c, CharSequence value, float sp, int color, boolean bold) {
        TextView t = text(c, value, sp, color, bold);
        t.setTypeface(Typeface.MONOSPACE, bold ? Typeface.BOLD : Typeface.NORMAL);
        return t;
    }

    /** A rounded backing in a flat colour (mockup: rounded cards/panels). */
    static GradientDrawable round(Context c, int color, float radiusDp) {
        GradientDrawable d = new GradientDrawable();
        d.setShape(GradientDrawable.RECTANGLE);
        d.setColor(color);
        d.setCornerRadius(dp(c, radiusDp));
        return d;
    }

    static GradientDrawable oval(Context c, int color) {
        GradientDrawable d = new GradientDrawable();
        d.setShape(GradientDrawable.OVAL);
        d.setColor(color);
        return d;
    }

    static LinearLayout.LayoutParams weight(float w) {
        LinearLayout.LayoutParams lp =
                new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.weight = w;
        return lp;
    }

    static LinearLayout.LayoutParams margins(int l, int t, int r, int b) {
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.setMargins(l, t, r, b);
        return lp;
    }

    // ----------------------------------------------------------- node pieces

    /**
     * The node glyph — the same three shrinking arcs over a dot the mockup uses
     * as its sensor mark. Drawn in code so no image asset is added.
     */
    static final class Glyph extends View {

        private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final int size;

        Glyph(Context c, int color, float sizeDp) {
            super(c);
            this.size = dp(c, sizeDp);
            paint.setColor(color);
            paint.setStyle(Paint.Style.FILL);
        }

        @Override
        protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
            setMeasuredDimension(size, size);
        }

        @Override
        protected void onDraw(Canvas canvas) {
            float w = getWidth();
            float h = getHeight();
            float barH = Math.max(1.5f, h * 0.10f);
            float cy = h * 0.14f;
            float[] widths = {0.94f, 0.68f, 0.42f};
            for (float f : widths) {
                float half = w * f / 2f;
                canvas.drawRoundRect(w / 2f - half, cy, w / 2f + half, cy + barH,
                        barH / 2f, barH / 2f, paint);
                cy += barH * 2.1f;
            }
            canvas.drawCircle(w / 2f, h * 0.82f, Math.max(2f, w * 0.11f), paint);
        }
    }

    /** The 42dp rounded glyph tile that heads a node card / the detail header. */
    static FrameLayout iconBox(Context c, boolean attention) {
        FrameLayout box = new FrameLayout(c);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(dp(c, 42), dp(c, 42));
        box.setLayoutParams(lp);
        box.setBackground(round(c, col(c, attention ? R.color.nt_warn_soft : R.color.nt_ok_soft), 13));
        box.addView(new Glyph(c, col(c, attention ? R.color.nt_warn : R.color.nt_ok), 21));
        FrameLayout.LayoutParams glyphLp = new FrameLayout.LayoutParams(dp(c, 21), dp(c, 21));
        glyphLp.gravity = Gravity.CENTER;
        box.getChildAt(0).setLayoutParams(glyphLp);
        return box;
    }

    /** One metric column: mono value over a muted label. */
    private static LinearLayout metric(Context c, String value, String label, int valueColor) {
        LinearLayout box = column(c);
        box.setLayoutParams(weight(1));
        TextView v = mono(c, value, 13, valueColor, true);
        box.addView(v);
        box.addView(text(c, label, 9.5f, col(c, R.color.nt_muted), false));
        return box;
    }

    /**
     * A field-node card: status glyph, name, last-seen line, then PM2.5,
     * temperature, humidity and battery voltage side by side, plus the chevron
     * that opens the drill-down.
     */
    static View nodeCard(Context c, Node n, View.OnClickListener onTap) {
        boolean attention = n.attention();

        LinearLayout card = column(c);
        card.setBackground(round(c, col(c, R.color.nt_surface_2), 16));
        card.setPadding(dp(c, 12), dp(c, 12), dp(c, 12), dp(c, 12));
        card.setLayoutParams(margins(0, 0, 0, dp(c, 10)));

        LinearLayout head = row(c);
        head.setGravity(Gravity.CENTER_VERTICAL);
        head.addView(iconBox(c, attention));

        LinearLayout nameBox = column(c);
        nameBox.setLayoutParams(weight(1));
        nameBox.setPadding(dp(c, 12), 0, dp(c, 8), 0);
        nameBox.addView(text(c, n.displayName(), 15, col(c, R.color.nt_ink), true));
        nameBox.addView(text(c, n.statusLine(), 11, col(c, R.color.nt_muted), false));
        head.addView(nameBox);

        head.addView(mono(c, "\u203A", 20, col(c, R.color.nt_muted), false));
        card.addView(head);

        LinearLayout metrics = row(c);
        metrics.setPadding(0, dp(c, 10), 0, 0);
        metrics.addView(metric(c, fmt1(n.pm25) + " \u00B5g/m\u00B3", "PM2.5",
                col(c, attention ? R.color.nt_warn : R.color.nt_ink)));
        metrics.addView(metric(c, fmt0(n.tempF()) + "\u00B0F", "Temperature",
                col(c, R.color.nt_ink)));
        metrics.addView(metric(c, fmt0(n.humidity) + "% RH", "Humidity",
                col(c, R.color.nt_ink)));
        metrics.addView(metric(c, fmt2(n.batteryV) + " V", "Battery",
                col(c, n.lowBattery() ? R.color.nt_warn : R.color.nt_ink)));
        card.addView(metrics);

        card.setClickable(true);
        card.setFocusable(true);
        card.setOnClickListener(onTap);
        return card;
    }

    /**
     * An alert-history row: tone disc, type + node, message, and the timestamp
     * (mockup: .history-row).
     */
    static View alertRow(Context c, Alert a) {
        boolean warn = a.warning();

        LinearLayout box = column(c);
        box.setBackground(round(c, col(c, R.color.nt_surface_2), 14));
        box.setPadding(dp(c, 14), dp(c, 14), dp(c, 14), dp(c, 14));
        box.setLayoutParams(margins(0, 0, 0, dp(c, 10)));

        LinearLayout top = row(c);
        top.setGravity(Gravity.CENTER_VERTICAL);

        FrameLayout disc = new FrameLayout(c);
        disc.setLayoutParams(new LinearLayout.LayoutParams(dp(c, 38), dp(c, 38)));
        disc.setBackground(oval(c, col(c, warn ? R.color.nt_warn_soft : R.color.nt_blue_soft)));
        TextView mark = text(c, warn ? "!" : "\u2713", 18,
                col(c, warn ? R.color.nt_warn : R.color.nt_blue), true);
        FrameLayout.LayoutParams markLp = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT);
        markLp.gravity = Gravity.CENTER;
        mark.setLayoutParams(markLp);
        disc.addView(mark);
        top.addView(disc);

        LinearLayout copy = column(c);
        copy.setLayoutParams(weight(1));
        copy.setPadding(dp(c, 12), 0, dp(c, 8), 0);
        copy.addView(text(c, a.title(), 14, col(c, R.color.nt_ink), true));
        copy.addView(text(c, a.detailLine(), 12, col(c, R.color.nt_muted), false));
        top.addView(copy);

        top.addView(mono(c, a.timeLabel(), 10, col(c, R.color.nt_muted), false));
        box.addView(top);

        String severity = a.severityLabel();
        TextView sev = text(c, severity, 10, col(c, warn ? R.color.nt_warn : R.color.nt_blue), true);
        sev.setPadding(0, dp(c, 10), 0, 0);
        box.addView(sev);
        return box;
    }

    /** A health bar and the views that drive it once the activity knows values. */
    static final class Bar {
        View root;
        View fill;
        View spacer;
        TextView value;
    }

    /**
     * A health row: label, proportional bar, mono value. The bar is a two-view
     * weight split, so setting {@code setBar()} needs no measurement pass.
     */
    static Bar healthBar(Context c, String label, String valuePlaceholder) {
        Bar bar = new Bar();

        LinearLayout box = row(c);
        box.setGravity(Gravity.CENTER_VERTICAL);
        box.setPadding(0, dp(c, 7), 0, dp(c, 7));

        TextView l = text(c, label, 11, col(c, R.color.nt_muted), false);
        LinearLayout.LayoutParams labelLp = new LinearLayout.LayoutParams(dp(c, 104),
                LinearLayout.LayoutParams.WRAP_CONTENT);
        l.setLayoutParams(labelLp);
        box.addView(l);

        LinearLayout track = row(c);
        track.setLayoutParams(weight(1));
        track.setBackground(round(c, col(c, R.color.nt_surface_3), 9));

        View fill = new View(c);
        fill.setBackground(round(c, col(c, R.color.nt_ok), 9));
        View spacer = new View(c);
        track.addView(fill);
        track.addView(spacer);
        box.addView(track);

        TextView value = mono(c, valuePlaceholder, 11, col(c, R.color.nt_ink), false);
        LinearLayout.LayoutParams valueLp = new LinearLayout.LayoutParams(dp(c, 84),
                LinearLayout.LayoutParams.WRAP_CONTENT);
        value.setLayoutParams(valueLp);
        value.setGravity(Gravity.END);
        box.addView(value);

        bar.root = box;
        bar.fill = fill;
        bar.spacer = spacer;
        bar.value = value;
        setBar(bar, 0, valuePlaceholder);
        return bar;
    }

    /** Sets the bar's fill proportion (0..100) and its right-hand value text. */
    static void setBar(Bar bar, int percent, String value) {
        setBar(bar.fill, bar.spacer, bar.value, percent, value);
    }

    /** Same, for bars whose views come from a layout instead of {@link #healthBar}. */
    static void setBar(View fill, View spacer, TextView value, int percent, String text) {
        int clamped = Math.max(0, Math.min(100, percent));
        Context c = fill.getContext();
        LinearLayout.LayoutParams fillLp = new LinearLayout.LayoutParams(0, dp(c, 7));
        fillLp.weight = Math.max(0.01f, clamped);
        fill.setLayoutParams(fillLp);
        LinearLayout.LayoutParams spacerLp = new LinearLayout.LayoutParams(0, dp(c, 7));
        spacerLp.weight = Math.max(0.01f, 100 - clamped);
        spacer.setLayoutParams(spacerLp);
        value.setText(text);
    }

    // ------------------------------------------------------------ bottom nav

    private static final int[] NAV_ITEMS = {R.id.nav_nodes, R.id.nav_alerts, R.id.nav_system};
    private static final int[] NAV_BARS = {R.id.nav_nodes_bar, R.id.nav_alerts_bar, R.id.nav_system_bar};
    private static final int[] NAV_LABELS = {R.id.nav_nodes_label, R.id.nav_alerts_label, R.id.nav_system_label};

    /**
     * Wires the shared bottom nav for one screen: marks {@code activeId} and
     * makes the other two switch screens (finishing this one, since the nav is
     * a top-level destination switch, not a stack push).
     */
    static void bindNav(final Activity a, final int activeId) {
        final Class<?>[] targets = {NodesActivity.class, AlertsActivity.class, SystemActivity.class};
        for (int i = 0; i < NAV_ITEMS.length; i++) {
            markNav(a, activeId, i, NAV_ITEMS[i] == activeId);
            final Class<?> target = targets[i];
            final int id = NAV_ITEMS[i];
            a.findViewById(id).setOnClickListener(v -> {
                if (id == activeId) {
                    return;
                }
                a.startActivity(new Intent(a, target));
                a.finish();
            });
        }
    }

    private static void markNav(Activity a, int activeId, int index, boolean active) {
        View bar = a.findViewById(NAV_BARS[index]);
        bar.setBackground(round(a, active ? col(a, R.color.nt_accent) : 0x00000000, 2));
        TextView label = a.findViewById(NAV_LABELS[index]);
        label.setTextColor(col(a, active ? R.color.nt_accent_strong : R.color.nt_muted));
    }

    // ------------------------------------------------------------- formatting

    /** "12.4" — PM2.5 keeps one decimal, as in the mockup. */
    static String fmt1(double v) {
        return Double.isNaN(v) ? "—" : String.format(Locale.US, "%.1f", v);
    }

    /** "21" — temperatures and humidities read as whole numbers. */
    static String fmt0(double v) {
        return Double.isNaN(v) ? "—" : String.format(Locale.US, "%.0f", v);
    }

    /** "18 sec ago" / "11 h ago" — a compact age for the health bar. */
    static String ageLabel(long seconds) {
        if (seconds < 90) {
            return seconds + " sec ago";
        }
        if (seconds < 5400) {
            return Math.round(seconds / 60.0) + " min ago";
        }
        if (seconds < 172800) {
            return Math.round(seconds / 3600.0) + " h ago";
        }
        return Math.round(seconds / 86400.0) + " d ago";
    }

    /** "4.05" — battery voltage to two decimals. */
    static String fmt2(double v) {
        return Double.isNaN(v) ? "—" : String.format(Locale.US, "%.2f", v);
    }
}
