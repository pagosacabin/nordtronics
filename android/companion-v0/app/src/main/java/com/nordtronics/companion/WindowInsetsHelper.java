package com.nordtronics.companion;

import android.app.Activity;
import android.view.View;

import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

/**
 * Edge-to-edge window insets for every screen (task 0047).
 *
 * <p>The app targets SDK 35, so on Android 15+ the system lays the window out
 * edge-to-edge and no longer reserves space for the status bar, the navigation
 * bar or a display cutout. The AVD used for the 0045 screenshots was an Android
 * 13 image, where the framework still padded the window, so nothing looked
 * wrong there — on a physical Android 15/16 device the decor instead draws the
 * action bar over the top of the content and the screen's first row ended up
 * underneath it, untappable.
 *
 * <p>Rather than assume any particular framework behaviour, each activity calls
 * {@link #applySystemBarInsets(Activity)} and its content is padded by the
 * insets the device actually reports: status bar and cutout on top, navigation
 * bar (or gesture handle) at the bottom, cutout/bar width on the sides. Calling
 * {@code setDecorFitsSystemWindows(false)} makes that contract explicit and
 * identical on every API level from 24 up, so a device that pads the window for
 * us cannot double-pad it.
 */
final class WindowInsetsHelper {

    private WindowInsetsHelper() {
    }

    /** Pads the activity's content area by the device's real system-bar insets. */
    static void applySystemBarInsets(final Activity activity) {
        WindowCompat.setDecorFitsSystemWindows(activity.getWindow(), false);

        final View content = activity.findViewById(android.R.id.content);
        ViewCompat.setOnApplyWindowInsetsListener(content, (view, windowInsets) -> {
            Insets bars = windowInsets.getInsets(
                    WindowInsetsCompat.Type.systemBars()
                            | WindowInsetsCompat.Type.displayCutout());
            view.setPadding(bars.left, bars.top, bars.right, bars.bottom);
            // Consumed: the child views are laid out inside the padded area and
            // must not apply the same insets a second time.
            return WindowInsetsCompat.CONSUMED;
        });

        // Edge-to-edge makes the system bars transparent, so the app decides
        // whether their icons are drawn light or dark. Since 0049 the screens are
        // dark-first (the v0.1 mockup's dark scheme, #121718), so the bar icons
        // must be LIGHT — with the previous dark-icon setting the status bar and
        // the gesture pill were invisible against the dark background.
        WindowInsetsControllerCompat controller =
                WindowCompat.getInsetsController(activity.getWindow(), content);
        controller.setAppearanceLightStatusBars(false);
        controller.setAppearanceLightNavigationBars(false);

        ViewCompat.requestApplyInsets(content);
    }
}
