package com.nordtronics.companion;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.DashPathEffect;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.view.View;

/**
 * The property schematic (task 0090): a dashed property boundary, the two field
 * nodes with a connecting line, and a 0/50/100 ft scale bar. Positions are
 * client-side layout hints only — the contract is explicit that no coordinates
 * ever cross the wire, and this view is fed no location data at all.
 */
public class PropertySchematicView extends View {

    private final Paint boundary = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint link = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint dot = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint halo = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint label = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint scale = new Paint(Paint.ANTI_ALIAS_FLAG);

    private final int sage;
    private final int alarm;
    private final int muted;

    public PropertySchematicView(Context c, int sage, int alarm, int muted) {
        super(c);
        this.sage = sage;
        this.alarm = alarm;
        this.muted = muted;

        boundary.setStyle(Paint.Style.STROKE);
        boundary.setStrokeWidth(Ui.dp(c, 1.3f));
        boundary.setColor(0x66EDE6D6);
        boundary.setPathEffect(new DashPathEffect(
                new float[]{Ui.dp(c, 5), Ui.dp(c, 4)}, 0));

        link.setStyle(Paint.Style.STROKE);
        link.setStrokeWidth(Ui.dp(c, 1.2f));
        link.setColor(muted);

        halo.setStyle(Paint.Style.FILL);
        halo.setColor(0x33E76F51);

        dot.setStyle(Paint.Style.FILL);

        label.setColor(muted);
        label.setTextSize(Ui.dp(c, 9));
        label.setTypeface(android.graphics.Typeface.MONOSPACE);

        scale.setStyle(Paint.Style.STROKE);
        scale.setStrokeWidth(Ui.dp(c, 1.2f));
        scale.setColor(muted);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(resolveSize(Ui.dp(getContext(), 300), widthMeasureSpec),
                Ui.dp(getContext(), 150));
    }

    @Override
    protected void onDraw(Canvas canvas) {
        float pad = Ui.dp(getContext(), 10);
        float w = getWidth();
        float h = getHeight();

        RectF bounds = new RectF(pad, pad, w - pad, h - pad - Ui.dp(getContext(), 18));
        canvas.drawRoundRect(bounds, Ui.dp(getContext(), 12), Ui.dp(getContext(), 12), boundary);

        float n1x = bounds.left + bounds.width() * 0.28f;
        float n1y = bounds.top + bounds.height() * 0.66f;
        float n2x = bounds.left + bounds.width() * 0.72f;
        float n2y = bounds.top + bounds.height() * 0.34f;

        Path linkPath = new Path();
        linkPath.moveTo(n1x, n1y);
        linkPath.lineTo(n2x, n2y);
        canvas.drawPath(linkPath, link);

        // Node 02: Watch halo + alarm-orange dot.
        canvas.drawCircle(n2x, n2y, Ui.dp(getContext(), 11), halo);
        dot.setColor(alarm);
        canvas.drawCircle(n2x, n2y, Ui.dp(getContext(), 5), dot);
        canvas.drawText("Node 02", n2x + Ui.dp(getContext(), 14), n2y + Ui.dp(getContext(), 3), label);

        // Node 01: sage dot.
        dot.setColor(sage);
        canvas.drawCircle(n1x, n1y, Ui.dp(getContext(), 5), dot);
        canvas.drawText("Node 01", n1x + Ui.dp(getContext(), 14), n1y + Ui.dp(getContext(), 3), label);

        // Scale bar 0 / 50 / 100 ft.
        float barY = h - Ui.dp(getContext(), 8);
        float barLeft = pad;
        float barRight = w - pad;
        canvas.drawLine(barLeft, barY, barRight, barY, scale);
        float mid = (barLeft + barRight) / 2f;
        canvas.drawLine(barLeft, barY - Ui.dp(getContext(), 4), barLeft, barY + Ui.dp(getContext(), 4), scale);
        canvas.drawLine(mid, barY - Ui.dp(getContext(), 4), mid, barY + Ui.dp(getContext(), 4), scale);
        canvas.drawLine(barRight, barY - Ui.dp(getContext(), 4), barRight, barY + Ui.dp(getContext(), 4), scale);
        canvas.drawText("0 ft", barLeft, barY - Ui.dp(getContext(), 6), label);
        canvas.drawText("50 ft", mid - Ui.dp(getContext(), 9), barY - Ui.dp(getContext(), 6), label);
        canvas.drawText("100 ft", barRight - Ui.dp(getContext(), 22), barY - Ui.dp(getContext(), 6), label);
    }
}
