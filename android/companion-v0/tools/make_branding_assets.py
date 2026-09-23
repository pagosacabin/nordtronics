#!/usr/bin/env python3
"""Generate the Companion v0 branding drawables from the repo logo assets (task 0048).

Source artwork (no new artwork is drawn here, only resampled/masked):

  branding/nordtronics-logo/nordtronics-logo-mark-only.png     1600x1600
  branding/nordtronics-logo/nordtronics-logo-full-lockup.png   1920x1280

The mark-only file is an opaque dark plate (#222629) carrying the orange emblem
(rows 381..1041) above the grey wordmark (rows 1075..1218). This script separates
the emblem from the plate and the wordmark into an alpha channel, then emits:

  res/mipmap-{mdpi..xxxhdpi}/ic_launcher.png             legacy icon (pre-API-26)
  res/mipmap-{mdpi..xxxhdpi}/ic_launcher_foreground.png  adaptive-icon foreground
  res/drawable-{mdpi..xxxhdpi}/brand_mark.png            in-app app-bar mark (24dp)
  res/drawable-{mdpi..xxxhdpi}/splash_mark.png           splash-screen mark (288dp)

Run from android/companion-v0:  python3 tools/make_branding_assets.py
Requires Pillow only (a host tool, not an app dependency).
"""

import os

import numpy as np
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
APP = os.path.dirname(HERE)                       # android/companion-v0
REPO = os.path.dirname(os.path.dirname(APP))      # repo root
SRC_MARK = os.path.join(REPO, "branding/nordtronics-logo/nordtronics-logo-mark-only.png")
RES = os.path.join(APP, "app/src/main/res")

# Plate colour of the source artwork; also the brand dark used by the icon
# background layer and the splash background (see res/values/colors.xml).
PLATE = (34, 38, 41)     # #222629
ORANGE = (247, 165, 67)  # #F7A543

# Emblem bounding box inside the 1600x1600 source (measured, see module docstring).
EMBLEM_BOX = (307, 381, 1294, 1042)

DENSITIES = {"mdpi": 1.0, "hdpi": 1.5, "xhdpi": 2.0, "xxhdpi": 3.0, "xxxhdpi": 4.0}


def emblem_rgba():
    """The emblem as premultiplied-clean RGBA: orange, alpha ramp over the plate."""
    src = np.asarray(Image.open(SRC_MARK).convert("RGB")).astype(float)
    crop = src[EMBLEM_BOX[1]:EMBLEM_BOX[3], EMBLEM_BOX[0]:EMBLEM_BOX[2]]
    # Distance from the plate colour, normalised against the emblem colour, gives
    # a clean alpha ramp through the antialiased edges.
    plate = np.array(PLATE, dtype=float)
    dist = np.abs(crop - plate).sum(axis=2)
    # The plate is uniform (per-pixel distance <= ~6), the emblem is 366 away,
    # so a ramp above the plate noise keeps the mark solid instead of ghosting
    # its dimmer interior, while still antialiasing the true edges.
    alpha = np.clip((dist - 40.0) / 80.0, 0.0, 1.0)
    out = np.zeros(crop.shape[:2] + (4,), dtype=np.uint8)
    out[..., 0], out[..., 1], out[..., 2] = ORANGE
    out[..., 3] = (alpha * 255).round().astype(np.uint8)
    return Image.fromarray(out, "RGBA")


def fit(mark, box_w, box_h, canvas_w, canvas_h, scale=1.0):
    """Centre `mark` in a canvas, scaled to occupy scale * the given box.

    Only the alpha channel is resampled and it is laid over a flat emblem-colour
    plate, so downscaling cannot fringe the edges with the transparent black or
    shift the brand orange.
    """
    w = max(1, int(round(box_w * scale)))
    h = max(1, int(round(w * mark.height / mark.width)))
    if h > box_h * scale:
        h = max(1, int(round(box_h * scale)))
        w = max(1, int(round(h * mark.width / mark.height)))
    a = mark.getchannel("A").resize((w, h), Image.LANCZOS)
    m = Image.new("RGBA", (w, h), ORANGE + (0,))
    m.putalpha(a)
    canvas = Image.new("RGBA", (canvas_w, canvas_h), (0, 0, 0, 0))
    canvas.paste(m, ((canvas_w - w) // 2, (canvas_h - h) // 2), m)
    return canvas


def rounded_bg(size, radius_frac=0.22):
    bg = Image.new("RGBA", (size, size), PLATE + (255,))
    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle(
        [0, 0, size - 1, size - 1], radius=int(size * radius_frac), fill=255)
    bg.putalpha(mask)
    return bg


def save(img, *path):
    dest = os.path.join(RES, *path)
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    img.save(dest)
    print("wrote", os.path.relpath(dest, APP), img.size)


def main():
    mark = emblem_rgba()

    for dens, f in DENSITIES.items():
        # --- legacy launcher icon: mark on the brand-dark plate ---------------
        # 48dp at mdpi; the mark keeps a ~1/4 margin so launcher masks and
        # shadows never clip it.
        size = int(round(48 * f))
        bg = rounded_bg(size)
        bg.alpha_composite(
            fit(mark, size, size, size, size, scale=0.60),
            (0, 0))
        save(bg, f"mipmap-{dens}", "ic_launcher.png")

        # --- adaptive icon foreground: 108dp canvas, inner 66dp safe zone -----
        # The emblem is ~1.5:1, so the width is what has to fit: 60dp of the
        # 66dp safe circle keeps the horizontal extremes inside any mask.
        canvas = int(round(108 * f))
        save(fit(mark, canvas, canvas, canvas, canvas, scale=0.60 / 1.08),
             f"mipmap-{dens}", "ic_launcher_foreground.png")

        # --- in-app app-bar mark: 24dp tall --------------------------------
        save(fit(mark, int(round(72 * f)), int(round(24 * f)),
                 int(round(72 * f)), int(round(24 * f))),
             f"drawable-{dens}", "brand_mark.png")

        # --- splash mark: 288dp canvas, mark filling the inner 176dp --------
        # Android 12+ draws the splash icon inside a 192dp circle; a padded
        # canvas means neither a scale-to-fit nor an intrinsic-size draw can
        # clip the emblem.
        c = int(round(288 * f))
        save(fit(mark, c, c, c, c, scale=176.0 / 288.0),
             f"drawable-{dens}", "splash_mark.png")


if __name__ == "__main__":
    main()
