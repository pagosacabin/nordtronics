package com.nordtronics.companion;

import org.json.JSONObject;

/** One point of a metric series: {@code {"t":"...Z","v":9.8}} (contract v1). */
public class TrendPoint {

    public final String t;
    public final double v;

    public TrendPoint(String t, double v) {
        this.t = t;
        this.v = v;
    }

    public TrendPoint(JSONObject o) {
        this.t = o.optString("t", "");
        this.v = o.optDouble("v", Double.NaN);
    }
}
