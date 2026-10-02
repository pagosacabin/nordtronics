package com.nordtronics.companion;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.view.View;

/**
 * The 24-hour PM2.5 trend chart (task 0090), drawn as three shaded threshold
 * bands with the series over them. The bands are the contract's display bands:
 * Clean &lt; 12 · Elevated 12–35 · High &gt; 35 µg/m³. Their labels live as text
 * views beside the chart (see {@link Screens#bandLegend}), so the exact wording
 * is readable and checkable without OCR-ing a canvas.
 *
 * <p>The y axis is pinned to 0–50 µg/m³ rather than auto-scaled to the data, so
 * all three bands stay visible and a clean day's line sits low where it belongs.
 */
public class TrendChartView extends View {

    /** Top of the y axis; the three bands must all be visible on a clean day. */
    public static final double Y_MAX = 50.0;

    private final Paint band = new Paint();
    private final Paint stroke = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint grid = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Path path = new Path();
    private final float[] values;
    private final int cleanColor;
    private final int elevatedColor;
    private final int highColor;
    private final int lineColor;
    private final int gridColor;
    private final int heightDp;

    public TrendChartView(Context c, double[] series, int lineColor, int cleanColor,
                          int elevatedColor, int highColor, int gridColor, int heightDp) {
        super(c);
        this.values = series == null ? new float[0] : toFloats(series);
        this.lineColor = lineColor;
        this.cleanColor = cleanColor;
        this.elevatedColor = elevatedColor;
        this.highColor = highColor;
        this.gridColor = gridColor;
        this.heightDp = heightDp;
        stroke.setStyle(Paint.Style.STROKE);
        stroke.setStrokeWidth(Ui.dp(c, 1.8f));
        stroke.setStrokeJoin(Paint.Join.ROUND);
        stroke.setColor(lineColor);
        grid.setStyle(Paint.Style.STROKE);
        grid.setStrokeWidth(Ui.dp(c, 1f));
        grid.setColor(gridColor);
    }

    private static float[] toFloats(double[] in) {
        float[] out = new float[in.length];
        for (int i = 0; i < in.length; i++) {
            out[i] = (float) in[i];
        }
        return out;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(resolveSize(Ui.dp(getContext(), 300), widthMeasureSpec),
                Ui.dp(getContext(), heightDp));
    }

    private float yFor(double v, float top, float h) {
        double clamped = Math.max(0, Math.min(Y_MAX, v));
        return top + (float) (h * (1.0 - clamped / Y_MAX));
    }

    @Override
    protected void onDraw(Canvas canvas) {
        float pad = Ui.dp(getContext(), 2);
        float w = getWidth() - pad * 2;
        float h = getHeight() - pad * 2;
        if (w <= 0 || h <= 0) {
            return;
        }

        // Bands, bottom-up: Clean < 12, Elevated 12-35, High > 35.
        float y12 = yFor(NodeInfo.PM25_ELEVATED, pad, h);
        float y35 = yFor(NodeInfo.PM25_HIGH, pad, h);
        band.setColor(highColor);
        canvas.drawRect(pad, pad, pad + w, y35, band);
        band.setColor(elevatedColor);
        canvas.drawRect(pad, y35, pad + w, y12, band);
        band.setColor(cleanColor);
        canvas.drawRect(pad, y12, pad + w, pad + h, band);

        // Band boundary lines.
        canvas.drawLine(pad, y12, pad + w, y12, grid);
        canvas.drawLine(pad, y35, pad + w, y35, grid);

        if (values.length < 2) {
            return;
        }
        path.reset();
        for (int i = 0; i < values.length; i++) {
            float x = pad + w * i / (values.length - 1);
            float y = yFor(values[i], pad, h);
            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        canvas.drawPath(path, stroke);
    }
}
