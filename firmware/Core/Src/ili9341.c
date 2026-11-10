#include "ili9341.h"

#include "spi.h"

#define ILI9341_CS_PIN GPIO_PIN_6
#define ILI9341_CS_PORT GPIOB
#define ILI9341_RST_PIN GPIO_PIN_8
#define ILI9341_RST_PORT GPIOA
#define ILI9341_DC_PIN GPIO_PIN_9
#define ILI9341_DC_PORT GPIOA
#define ILI9341_BL_PIN GPIO_PIN_10
#define ILI9341_BL_PORT GPIOB

#define ILI9341_SPI_TIMEOUT_MS 1000U
#define ILI9341_SPI_CHUNK_SIZE 4096U

#define ILI9341_TRY(call)      \
  do                           \
  {                            \
    HAL_StatusTypeDef status = (call); \
    if (status != HAL_OK)      \
    {                          \
      return status;           \
    }                          \
  } while (0)

static const uint8_t command_ef[] = {0x03U, 0x80U, 0x02U};
static const uint8_t command_cf[] = {0x00U, 0xC1U, 0x30U};
static const uint8_t command_ed[] = {0x64U, 0x03U, 0x12U, 0x81U};
static const uint8_t command_e8[] = {0x85U, 0x00U, 0x78U};
static const uint8_t command_cb[] = {0x39U, 0x2CU, 0x00U, 0x34U, 0x02U};
static const uint8_t command_f7[] = {0x20U};
static const uint8_t command_ea[] = {0x00U, 0x00U};
static const uint8_t command_c0[] = {0x23U};
static const uint8_t command_c1[] = {0x10U};
static const uint8_t command_c5[] = {0x3EU, 0x28U};
static const uint8_t command_c7[] = {0x86U};
static const uint8_t command_madctl[] = {ILI9341_MADCTL_LANDSCAPE};
static const uint8_t command_pixel_format[] = {0x55U};
static const uint8_t command_frame_rate[] = {0x00U, 0x18U};
static const uint8_t command_display_function[] = {0x08U, 0x82U, 0x27U};
static const uint8_t command_gamma_disable[] = {0x00U};
static const uint8_t command_gamma_curve[] = {0x01U};
static const uint8_t command_positive_gamma[] =
{
  0x0FU, 0x31U, 0x2BU, 0x0CU, 0x0EU, 0x08U, 0x4EU, 0xF1U,
  0x37U, 0x07U, 0x10U, 0x03U, 0x0EU, 0x09U, 0x00U
};
static const uint8_t command_negative_gamma[] =
{
  0x00U, 0x0EU, 0x14U, 0x03U, 0x11U, 0x07U, 0x31U, 0xC1U,
  0x48U, 0x08U, 0x0FU, 0x0CU, 0x31U, 0x36U, 0x0FU
};

static HAL_StatusTypeDef ILI9341_Send(const uint8_t *data, uint16_t length)
{
  if ((data == NULL) || (length == 0U))
  {
    return HAL_ERROR;
  }

  return HAL_SPI_Transmit(&hspi1,
                          (uint8_t *)data,
                          length,
                          ILI9341_SPI_TIMEOUT_MS);
}

static HAL_StatusTypeDef ILI9341_WriteCommandData(uint8_t command,
                                                  const uint8_t *data,
                                                  uint16_t length)
{
  HAL_StatusTypeDef status;

  HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_RESET);
  status = ILI9341_Send(&command, 1U);

  if ((status == HAL_OK) && (data != NULL) && (length != 0U))
  {
    HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_SET);
    status = ILI9341_Send(data, length);
  }

  HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
  return status;
}

static HAL_StatusTypeDef ILI9341_WriteCommand(uint8_t command)
{
  return ILI9341_WriteCommandData(command, NULL, 0U);
}

void ILI9341_SetBacklight(uint8_t enabled)
{
  HAL_GPIO_WritePin(ILI9341_BL_PORT,
                    ILI9341_BL_PIN,
                    enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

HAL_StatusTypeDef ILI9341_SetWindow(uint16_t x0,
                                    uint16_t y0,
                                    uint16_t x1,
                                    uint16_t y1)
{
  uint8_t column[4];
  uint8_t row[4];

  if ((x1 < x0) || (y1 < y0))
  {
    return HAL_ERROR;
  }

  column[0] = (uint8_t)(x0 >> 8);
  column[1] = (uint8_t)(x0 & 0xFFU);
  column[2] = (uint8_t)(x1 >> 8);
  column[3] = (uint8_t)(x1 & 0xFFU);

  row[0] = (uint8_t)(y0 >> 8);
  row[1] = (uint8_t)(y0 & 0xFFU);
  row[2] = (uint8_t)(y1 >> 8);
  row[3] = (uint8_t)(y1 & 0xFFU);

  ILI9341_TRY(ILI9341_WriteCommandData(0x2AU, column, sizeof(column)));
  ILI9341_TRY(ILI9341_WriteCommandData(0x2BU, row, sizeof(row)));
  ILI9341_TRY(ILI9341_WriteCommand(0x2CU));
  return HAL_OK;
}

HAL_StatusTypeDef ILI9341_Init(void)
{
  ILI9341_SetBacklight(0U);

  HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ILI9341_RST_PORT, ILI9341_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(20U);
  HAL_GPIO_WritePin(ILI9341_RST_PORT, ILI9341_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(120U);

  ILI9341_TRY(ILI9341_WriteCommand(0x01U));
  HAL_Delay(150U);

  ILI9341_TRY(ILI9341_WriteCommandData(0xEFU, command_ef, sizeof(command_ef)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xCFU, command_cf, sizeof(command_cf)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xEDU, command_ed, sizeof(command_ed)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xE8U, command_e8, sizeof(command_e8)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xCBU, command_cb, sizeof(command_cb)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xF7U, command_f7, sizeof(command_f7)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xEAU, command_ea, sizeof(command_ea)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xC0U, command_c0, sizeof(command_c0)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xC1U, command_c1, sizeof(command_c1)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xC5U, command_c5, sizeof(command_c5)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xC7U, command_c7, sizeof(command_c7)));
  ILI9341_TRY(ILI9341_WriteCommandData(0x36U,
                                       command_madctl,
                                       sizeof(command_madctl)));
  ILI9341_TRY(ILI9341_WriteCommandData(0x3AU,
                                       command_pixel_format,
                                       sizeof(command_pixel_format)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xB1U,
                                       command_frame_rate,
                                       sizeof(command_frame_rate)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xB6U,
                                       command_display_function,
                                       sizeof(command_display_function)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xF2U,
                                       command_gamma_disable,
                                       sizeof(command_gamma_disable)));
  ILI9341_TRY(ILI9341_WriteCommandData(0x26U,
                                       command_gamma_curve,
                                       sizeof(command_gamma_curve)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xE0U,
                                       command_positive_gamma,
                                       sizeof(command_positive_gamma)));
  ILI9341_TRY(ILI9341_WriteCommandData(0xE1U,
                                       command_negative_gamma,
                                       sizeof(command_negative_gamma)));

  ILI9341_TRY(ILI9341_WriteCommand(0x11U));
  HAL_Delay(120U);
  ILI9341_TRY(ILI9341_WriteCommand(0x29U));
  HAL_Delay(20U);
  ILI9341_SetBacklight(1U);
  return HAL_OK;
}

HAL_StatusTypeDef ILI9341_DrawRGB565Image(const uint8_t *image,
                                          uint32_t length,
                                          uint16_t width,
                                          uint16_t height)
{
  uint32_t offset = 0U;
  uint64_t expected_length;
  HAL_StatusTypeDef status;

  if ((image == NULL) || (width == 0U) || (height == 0U))
  {
    return HAL_ERROR;
  }

  expected_length = (uint64_t)width * (uint64_t)height * 2U;
  if ((expected_length > UINT32_MAX) || ((uint32_t)expected_length != length))
  {
    return HAL_ERROR;
  }

  status = ILI9341_SetWindow(0U, 0U, width - 1U, height - 1U);
  if (status != HAL_OK)
  {
    return status;
  }

  HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);

  while (offset < length)
  {
    uint32_t remaining = length - offset;
    uint16_t chunk = (uint16_t)((remaining > ILI9341_SPI_CHUNK_SIZE)
                                    ? ILI9341_SPI_CHUNK_SIZE
                                    : remaining);

    status = ILI9341_Send(&image[offset], chunk);
    if (status != HAL_OK)
    {
      HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
      return status;
    }
    offset += chunk;
  }

  HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
  return HAL_OK;
}
