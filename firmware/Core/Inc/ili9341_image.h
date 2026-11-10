#ifndef ILI9341_IMAGE_H
#define ILI9341_IMAGE_H

#include <stdint.h>

#define ILI9341_IMAGE_WIDTH 320U
#define ILI9341_IMAGE_HEIGHT 240U
#define ILI9341_IMAGE_DATA_SIZE 153600U

extern const uint8_t g_ili9341_image_rgb565[ILI9341_IMAGE_DATA_SIZE];

#endif /* ILI9341_IMAGE_H */
