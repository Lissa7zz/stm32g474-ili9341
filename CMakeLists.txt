cmake_minimum_required(VERSION 3.20)

project(stm32g474_ili9341 C ASM)

set(FIRMWARE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/firmware")
set(LINKER_SCRIPT "${FIRMWARE_DIR}/STM32G474RETX_FLASH.ld")

set(CORE_SOURCES
  "${FIRMWARE_DIR}/Core/Src/gpio.c"
  "${FIRMWARE_DIR}/Core/Src/ili9341.c"
  "${FIRMWARE_DIR}/Core/Src/ili9341_image.c"
  "${FIRMWARE_DIR}/Core/Src/main.c"
  "${FIRMWARE_DIR}/Core/Src/spi.c"
  "${FIRMWARE_DIR}/Core/Src/stm32g4xx_hal_msp.c"
  "${FIRMWARE_DIR}/Core/Src/stm32g4xx_it.c"
  "${FIRMWARE_DIR}/Core/Src/syscalls.c"
  "${FIRMWARE_DIR}/Core/Src/sysmem.c"
  "${FIRMWARE_DIR}/Core/Src/system_stm32g4xx.c"
)

file(GLOB HAL_SOURCES CONFIGURE_DEPENDS
  "${FIRMWARE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/*.c"
)

add_executable(stm32g474_ili9341
  ${CORE_SOURCES}
  ${HAL_SOURCES}
  "${FIRMWARE_DIR}/Core/Startup/startup_stm32g474retx.s"
)

target_include_directories(stm32g474_ili9341 PRIVATE
  "${FIRMWARE_DIR}/Core/Inc"
  "${FIRMWARE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc"
  "${FIRMWARE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy"
  "${FIRMWARE_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include"
  "${FIRMWARE_DIR}/Drivers/CMSIS/Include"
)

target_compile_definitions(stm32g474_ili9341 PRIVATE
  STM32G474xx
  USE_HAL_DRIVER
)

target_compile_options(stm32g474_ili9341 PRIVATE
  -mcpu=cortex-m4
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=hard
  -ffunction-sections
  -fdata-sections
)

target_compile_options(stm32g474_ili9341 PRIVATE
  $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
)

target_link_options(stm32g474_ili9341 PRIVATE
  -mcpu=cortex-m4
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=hard
  -T${LINKER_SCRIPT}
  --specs=nosys.specs
  --specs=nano.specs
  -static
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_BINARY_DIR}/stm32g474_ili9341.map
  -Wl,--print-memory-usage
  -Wl,--start-group
  -lc
  -lm
  -Wl,--end-group
)

set_target_properties(stm32g474_ili9341 PROPERTIES
  OUTPUT_NAME "stm32g474_ili9341"
  SUFFIX ".elf"
)

find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
add_custom_command(TARGET stm32g474_ili9341 POST_BUILD
  COMMAND ${ARM_OBJCOPY}
          -O binary
          "$<TARGET_FILE:stm32g474_ili9341>"
          "${CMAKE_BINARY_DIR}/stm32g474_ili9341.bin"
  COMMENT "Generating stm32g474_ili9341.bin"
)
