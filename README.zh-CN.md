[English](README.md) | [简体中文](README.zh-CN.md)

# STM32G474 NUCLEO + ILI9341

使用 NUCLEO-G474RE 通过 SPI1 驱动 8 针 ILI9341 屏幕的 STM32CubeIDE 示例工程。

项目默认使用横屏 `320 x 240`、RGB565，并在上电后显示一张内置演示图片。图片转换与头文件生成可以配合 [Pixel2Panel](https://lissa7zz.github.io/ili9341-image-studio/) 完成。

![STM32G474 ILI9341 demo](assets/demo-preview.png)

## 硬件

- NUCLEO-G474RE，MCU 为 STM32G474RET6
- ILI9341 SPI 屏幕，常见 8 针接口：`GND / VCC / SCL / SDA / RES / DC / CS / BLK`
- ST-LINK USB 线和杜邦线

## 接线

| ILI9341 | STM32 | Arduino 信号 | Nucleo 排针 | 配置 |
|---|---|---|---|---|
| GND | GND | GND | CN6 pin 6 或 7 | 共地 |
| VCC | 3V3 | 3V3 | CN6 pin 4 | 按屏幕模块标称电压 |
| SCL | PA5 | D13 | CN5 pin 6 | SPI1_SCK |
| SDA | PA7 | D11 | CN5 pin 4 | SPI1_MOSI |
| RES | PA8 | D7 | CN9 pin 8 | GPIO Output，初始高 |
| DC | PA9 | D8 | CN5 pin 1 | GPIO Output，初始低 |
| CS | PB6 | D10 | CN5 pin 3 | GPIO Output，初始高 |
| BLK | PB10 | D6 | CN9 pin 7 | 当前代码拉高开启背光 |

注意：

- 屏幕模块明确支持 5 V 时，VCC 才可以接 5 V；SPI 和 GPIO 始终使用 3.3 V 电平。
- 如果 BLK 直接驱动背光 LED，不能把 GPIO 当电源使用。应串限流电阻，或通过三极管/MOS 管控制。
- PA5 默认还连接板载用户 LED LD2。如果高速 SPI 波形不稳定，可以评估断开 SB6。
- 当前固件只在 `LCD_ConfigureControlPins()` 中将 PB10 配为普通 GPIO。CubeMX 文件中 PB10 标为 TIM2_CH3，但还没有使用 PWM 调光。

## CubeMX 配置

MCU：`STM32G474RETx`，开发板：`NUCLEO-G474RE`。

| 项目 | 当前配置 |
|---|---|
| Debug | Serial Wire |
| SYSCLK | 170 MHz |
| 时钟源 | HSI 16 MHz + PLL |
| PLL | M=4, N=85, R=2 |
| SPI1 | Master, 8 Bit, MSB First |
| 时钟模式 | CPOL Low, CPHA 1 Edge |
| NSS | Software |
| 预分频 | 16，SPI 约 10.625 MHz |
| PA5 / PA7 | SPI1_SCK / SPI1_MOSI |
| PA8 | LCD_RST，Output Push Pull |
| PA9 | LCD_DC，Output Push Pull |
| PB6 | LCD_CS，Output Push Pull |

工程文件：[firmware/ILI9341.ioc](firmware/ILI9341.ioc)。

## 打开和烧录

1. 使用 STM32CubeIDE 导入 `firmware` 目录中的现有工程。
2. 编译工程。
3. 通过板载 ST-LINK 下载到 NUCLEO-G474RE。
4. 上电后应显示 `assets/demo-preview.png` 中的横屏演示图。

`Debug` 等本地编译产物没有提交到仓库，首次导入后重新编译即可。

## 代码入口

- `firmware/Core/Src/main.c`：ILI9341 初始化、绘制和主循环
- `firmware/Core/Src/spi.c`：SPI1 配置
- `firmware/Core/Src/gpio.c`：CubeMX GPIO 初始化
- `firmware/Core/Inc/ili9341_image.h`：演示图片尺寸和数组声明
- `firmware/Core/Src/ili9341_image.c`：320 x 240 RGB565 演示图片

当前 `LCD_DrawImage()` 只接受 `ILI9341_IMAGE_DATA_SIZE` 大小的横屏图片，即 `153600` 字节。

## 替换演示图片

可以直接在 [Pixel2Panel](https://lissa7zz.github.io/ili9341-image-studio/) 中转换自己的图片，然后替换：

- `firmware/Core/Inc/ili9341_image.h`
- `firmware/Core/Src/ili9341_image.c`

也可以重新生成仓库中的演示图片：

```bash
python -m pip install -r tools/requirements.txt
python tools/generate_demo_header.py
```

## 板卡文档

- [ST UM2505：STM32G4 Nucleo-64 板卡指导书](https://www.st.com/resource/en/user_manual/um2505-stm32g4-nucleo64-boards-mb1367-stmicroelectronics.pdf)

## 相关项目

- [Pixel2Panel 图片转换工具](https://github.com/Lissa7zz/ili9341-image-studio)

## License

项目原创代码、文档和演示图片使用 [MIT License](LICENSE)。STM32 HAL 和 CMSIS 组件适用各自的许可，详见 [NOTICE.md](NOTICE.md)。
