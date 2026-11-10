from pathlib import Path

from PIL import Image, ImageOps


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets" / "demo-source.jpg"
HEADER = ROOT / "firmware" / "Core" / "Inc" / "ili9341_image.h"
SOURCE_FILE = ROOT / "firmware" / "Core" / "Src" / "ili9341_image.c"
PREVIEW = ROOT / "assets" / "demo-preview.png"
WIDTH = 320
HEIGHT = 240


def rgb565(red: int, green: int, blue: int) -> int:
    return ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3)


image = Image.open(SOURCE).convert("RGB")
fit = ImageOps.fit(
    image,
    (WIDTH, HEIGHT),
    method=Image.Resampling.LANCZOS,
    centering=(0.5, 0.5),
)
fit.save(PREVIEW, optimize=True)

data = bytearray()
for y in range(HEIGHT):
    for x in range(WIDTH):
        red, green, blue = fit.getpixel((x, y))
        pixel = rgb565(red, green, blue)
        data.extend((pixel >> 8, pixel & 0xFF))

if len(data) != WIDTH * HEIGHT * 2:
    raise RuntimeError(f"Unexpected data size: {len(data)}")

HEADER.write_text(
    "\n".join(
        [
            "#ifndef ILI9341_IMAGE_H",
            "#define ILI9341_IMAGE_H",
            "",
            "#include <stdint.h>",
            "",
            "#define ILI9341_IMAGE_WIDTH 320U",
            "#define ILI9341_IMAGE_HEIGHT 240U",
            "#define ILI9341_IMAGE_DATA_SIZE 153600U",
            "",
            "extern const uint8_t g_ili9341_image_rgb565[ILI9341_IMAGE_DATA_SIZE];",
            "",
            "#endif /* ILI9341_IMAGE_H */",
            "",
        ]
    ),
    encoding="ascii",
    newline="\n",
)

lines = [
    '#include "ili9341_image.h"',
    "",
    "/* Generated 320x240 RGB565 demo image, high byte first. */",
    "const uint8_t g_ili9341_image_rgb565[ILI9341_IMAGE_DATA_SIZE] =",
    "{",
]
for offset in range(0, len(data), 16):
    chunk = data[offset : offset + 16]
    lines.append("  " + ", ".join(f"0x{value:02X}U" for value in chunk) + ",")
lines.extend(("};", ""))
SOURCE_FILE.write_text(
    "\n".join(lines),
    encoding="ascii",
    newline="\n",
)

print(f"Generated {SOURCE_FILE} ({len(data)} bytes)")
