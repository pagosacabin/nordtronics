package com.nordtronics.companion;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.view.View;

/**
 * A thin sparkline (task 0090): one polyline through the 12 h trend arrays the
 * contract serves, with a dot on the newest point. Drawn with a Paint — like the
 * node glyph in {@link Ui}, it adds no image asset and no chart dependency.
 */
public class SparklineView extends View {

    private final Paint stroke = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint dot = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Path path = new Path();
    private final float[] values;
    private final int heightDp;

    public SparklineView(Context c, double[] series, int color, int heightDp) {
        super(c);
        this.values = series == null ? new float[0] : toFloats(series);
        this.heightDp = heightDp;
        stroke.setStyle(Paint.Style.STROKE);
        stroke.setStrokeWidth(Ui.dp(c, 1.6f));
        stroke.setStrokeCap(Paint.Cap.ROUND);
        stroke.setStrokeJoin(Paint.Join.ROUND);
        stroke.setColor(color);
        dot.setStyle(Paint.Style.FILL);
        dot.setColor(color);
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
        setMeasuredDimension(resolveSize(Ui.dp(getContext(), 120), widthMeasureSpec),
                Ui.dp(getContext(), heightDp));
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (values.length < 2) {
            return;
        }
        float pad = Ui.dp(getContext(), 3);
        float w = getWidth() - pad * 2;
        float h = getHeight() - pad * 2;
        float min = Float.MAX_VALUE;
        float max = -Float.MAX_VALUE;
        for (float v : values) {
            min = Math.min(min, v);
            max = Math.max(max, v);
        }
        if (max - min < 0.0001f) {
            max = min + 1f;                       // a flat line still reads as a line
        }
        path.reset();
        for (int i = 0; i < values.length; i++) {
            float x = pad + w * i / (values.length - 1);
            float y = pad + h - h * (values[i] - min) / (max - min);
            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        canvas.drawPath(path, stroke);

        float lastX = pad + w;
        float lastY = pad + h - h * (values[values.length - 1] - min) / (max - min);
        canvas.drawCircle(lastX, lastY, Ui.dp(getContext(), 2.1f), dot);
    }
}
