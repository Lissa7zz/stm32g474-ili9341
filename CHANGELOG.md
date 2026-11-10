# Changelog

## [0.2.0] - 2026-11-10

- Extracted the ILI9341 implementation into a reusable `ili9341.c/.h` driver.
- Added explicit width and height parameters for RGB565 image drawing.
- Unified PB10 as a GPIO backlight output in CubeMX and generated code.
- Added a CMake cross-build and GitHub Actions firmware build.
- Added documented front and back photos of the ILI9341 module.

## [0.1.0] - 2026-11-10

- Added the STM32CubeIDE project for NUCLEO-G474RE and ILI9341.
- Added SPI, GPIO, reset, command/data, chip-select, and backlight configuration.
- Added a generated 320 x 240 RGB565 demo image.
- Documented wiring, CubeMX settings, power notes, and build steps.
- Included the ST UM2505 NUCLEO-G474RE board manual with attribution.
