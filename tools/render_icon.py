#!/usr/bin/env python3
"""Render the original Rhodes Daily Terminal application icon."""

from pathlib import Path
from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parent.parent
S = 1024
img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
d = ImageDraw.Draw(img)

# Compact industrial terminal body and inset screen.
d.rounded_rectangle((92, 92, 932, 932), 184, fill="#111A20", outline="#F5C451", width=36)
d.rounded_rectangle((158, 170, 866, 818), 116, fill="#0B1014", outline="#344A52", width=22)
d.rectangle((215, 230, 809, 252), fill="#73F4C7")

# Clock/focus arc behind the companion.
d.arc((252, 278, 772, 798), 205, 522, fill="#F5C451", width=42)
d.ellipse((482, 300, 542, 360), fill="#F5C451")
d.rounded_rectangle((495, 348, 529, 520), 15, fill="#F5C451")
d.polygon([(512, 518), (640, 594), (616, 632), (488, 555)], fill="#F5C451")

# Friendly crystalline terminal pet: broad silhouette remains clear at 48 px.
d.ellipse((316, 462, 708, 750), fill="#73F4C7")
d.polygon([(352, 502), (300, 376), (426, 454)], fill="#73F4C7")
d.polygon([(580, 454), (698, 362), (670, 520)], fill="#73F4C7")
d.polygon([(444, 470), (512, 326), (584, 470)], fill="#A8FFE8")
d.ellipse((402, 566, 456, 620), fill="#111A20")
d.ellipse((568, 566, 622, 620), fill="#111A20")
d.arc((446, 588, 578, 682), 12, 168, fill="#111A20", width=22)

# Four hardware-key ticks anchor the device identity without text.
for x in (316, 444, 572, 700):
    d.rounded_rectangle((x - 34, 838, x + 34, 878), 20, fill="#F5C451")

preview = ROOT / "assets" / "icon-preview.png"
preview.parent.mkdir(parents=True, exist_ok=True)
img.resize((512, 512), Image.Resampling.LANCZOS).save(preview, optimize=True)
img.resize((128, 128), Image.Resampling.LANCZOS).save(ROOT / "icon.png", optimize=True)
print(f"rendered {ROOT / 'icon.png'} and {preview}")
