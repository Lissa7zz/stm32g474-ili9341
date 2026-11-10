/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : ILI9341 RGB test
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341_image.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_WIDTH                 320U
#define LCD_HEIGHT                240U
#define LCD_SPI_TIMEOUT_MS        1000U

#define LCD_CS_Pin                GPIO_PIN_6
#define LCD_CS_GPIO_Port          GPIOB
#define LCD_RST_Pin               GPIO_PIN_8
#define LCD_RST_GPIO_Port         GPIOA
#define LCD_DC_Pin                GPIO_PIN_9
#define LCD_DC_GPIO_Port          GPIOA
#define LCD_BL_Pin                GPIO_PIN_10
#define LCD_BL_GPIO_Port          GPIOB

/*
 * 0x28 = MV + BGR: landscape orientation, 320x240.
 * If the image is upside down, change this value from 0x28 to 0xE8.
 */
#define LCD_MADCTL_VALUE          0x28U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void LCD_Init(void);
static void LCD_DrawImage(const uint8_t *image, uint32_t length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void LCD_SPI_Send(const uint8_t *data, uint16_t length)
{
  if ((data == NULL) || (length == 0U))
  {
    return;
  }

  if (HAL_SPI_Transmit(&hspi1,
                       (uint8_t *)data,
                       length,
                       LCD_SPI_TIMEOUT_MS) != HAL_OK)
  {
    Error_Handler();
  }
}

static void LCD_WriteCommandData(uint8_t command,
                                 const uint8_t *data,
                                 uint16_t length)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
  LCD_SPI_Send(&command, 1U);

  if ((data != NULL) && (length != 0U))
  {
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
    LCD_SPI_Send(data, length);
  }

  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void LCD_WriteCommand(uint8_t command)
{
  LCD_WriteCommandData(command, NULL, 0U);
}

static void LCD_ConfigureControlPins(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStruct.Pin = LCD_RST_Pin | LCD_DC_Pin;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LCD_CS_Pin | LCD_BL_Pin;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void LCD_SetWindow(uint16_t x0,
                          uint16_t y0,
                          uint16_t x1,
                          uint16_t y1)
{
  uint8_t column[4];
  uint8_t row[4];

  column[0] = (uint8_t)(x0 >> 8);
  column[1] = (uint8_t)(x0 & 0xFFU);
  column[2] = (uint8_t)(x1 >> 8);
  column[3] = (uint8_t)(x1 & 0xFFU);

  row[0] = (uint8_t)(y0 >> 8);
  row[1] = (uint8_t)(y0 & 0xFFU);
  row[2] = (uint8_t)(y1 >> 8);
  row[3] = (uint8_t)(y1 & 0xFFU);

  LCD_WriteCommandData(0x2AU, column, sizeof(column));
  LCD_WriteCommandData(0x2BU, row, sizeof(row));
  LCD_WriteCommand(0x2CU);
}

static void LCD_Init(void)
{
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
  static const uint8_t command_madctl[] = {LCD_MADCTL_VALUE};
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

  LCD_ConfigureControlPins();

  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20U);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120U);

  LCD_WriteCommand(0x01U);
  HAL_Delay(150U);

  LCD_WriteCommandData(0xEFU, command_ef, sizeof(command_ef));
  LCD_WriteCommandData(0xCFU, command_cf, sizeof(command_cf));
  LCD_WriteCommandData(0xEDU, command_ed, sizeof(command_ed));
  LCD_WriteCommandData(0xE8U, command_e8, sizeof(command_e8));
  LCD_WriteCommandData(0xCBU, command_cb, sizeof(command_cb));
  LCD_WriteCommandData(0xF7U, command_f7, sizeof(command_f7));
  LCD_WriteCommandData(0xEAU, command_ea, sizeof(command_ea));
  LCD_WriteCommandData(0xC0U, command_c0, sizeof(command_c0));
  LCD_WriteCommandData(0xC1U, command_c1, sizeof(command_c1));
  LCD_WriteCommandData(0xC5U, command_c5, sizeof(command_c5));
  LCD_WriteCommandData(0xC7U, command_c7, sizeof(command_c7));
  LCD_WriteCommandData(0x36U, command_madctl, sizeof(command_madctl));
  LCD_WriteCommandData(0x3AU, command_pixel_format, sizeof(command_pixel_format));
  LCD_WriteCommandData(0xB1U, command_frame_rate, sizeof(command_frame_rate));
  LCD_WriteCommandData(0xB6U,
                       command_display_function,
                       sizeof(command_display_function));
  LCD_WriteCommandData(0xF2U, command_gamma_disable, sizeof(command_gamma_disable));
  LCD_WriteCommandData(0x26U, command_gamma_curve, sizeof(command_gamma_curve));
  LCD_WriteCommandData(0xE0U,
                       command_positive_gamma,
                       sizeof(command_positive_gamma));
  LCD_WriteCommandData(0xE1U,
                       command_negative_gamma,
                       sizeof(command_negative_gamma));

  LCD_WriteCommand(0x11U);
  HAL_Delay(120U);
  LCD_WriteCommand(0x29U);
  HAL_Delay(20U);

  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
}

static void LCD_DrawImage(const uint8_t *image, uint32_t length)
{
  uint32_t offset = 0U;

  if ((image == NULL) || (length != ILI9341_IMAGE_DATA_SIZE))
  {
    Error_Handler();
  }

  LCD_SetWindow(0U, 0U, LCD_WIDTH - 1U, LCD_HEIGHT - 1U);

  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  while (offset < length)
  {
    uint16_t chunk = (uint16_t)((length - offset) > 4096U
                                    ? 4096U
                                    : (length - offset));

    LCD_SPI_Send(&image[offset], chunk);
    offset += chunk;
  }

  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
  LCD_Init();
  LCD_DrawImage(g_ili9341_image_rgb565, ILI9341_IMAGE_DATA_SIZE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_Delay(1000U);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: source line number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  (void)file;
  (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
