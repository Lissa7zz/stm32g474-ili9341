#ifndef ILI9341_H
#define ILI9341_H

#include "main.h"

#include <stdint.h>

#define ILI9341_MADCTL_LANDSCAPE 0x28U

HAL_StatusTypeDef ILI9341_Init(void);
HAL_StatusTypeDef ILI9341_SetWindow(uint16_t x0,
                                    uint16_t y0,
                                    uint16_t x1,
                                    uint16_t y1);
HAL_StatusTypeDef ILI9341_DrawRGB565Image(const uint8_t *image,
                                          uint32_t length,
                                          uint16_t width,
                                          uint16_t height);
void ILI9341_SetBacklight(uint8_t enabled);

#endif /* ILI9341_H */
