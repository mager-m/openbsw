ESP32
=====

BSP
---

The ESP32 platform BSP modules provide hardware abstraction for the Arduino Nano ESP32
(ESP32-S3 SoC) using ESP-IDF drivers:

- **bspMcu**: MCU reset via ``esp_restart()``
- **bspUart**: UART communication via ESP-IDF UART driver
- **bspGpio**: GPIO control via ESP-IDF GPIO driver
- **bspAdc**: ADC reading via ESP-IDF ADC oneshot driver
- **bspLedc**: PWM output via ESP-IDF LEDC peripheral
- **bspTwai**: CAN bus communication via ESP-IDF TWAI driver
- **bspEepromDriver**: Persistent storage via NVS (Non-Volatile Storage)
- **bspSystemTime**: System time via ``esp_timer``
- **bspInterruptsImpl**: Interrupt management via FreeRTOS critical sections

Safety
------

- **safeBspMcuWatchdog**: Task watchdog via ESP-IDF task watchdog API
