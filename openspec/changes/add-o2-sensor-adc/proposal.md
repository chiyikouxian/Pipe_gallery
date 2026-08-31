# Change: Add Oxygen Sensor Acquisition via ADC1 Channel 18 (PA4)

## Why
The monitoring node needs oxygen concentration data for the pipe gallery environment. The sensor was initially planned for PC5, but Ethernet RMII now uses PC5 as RXD1. The analog output is therefore connected to ADC1 channel 18 on PA4, exposed on core-board header J1-35.

## What Changes
- Add `applications/Apps/o2SensorApp.c` and `applications/Apps/o2SensorApp.h`: oxygen sensor driver using RT-Thread ADC device framework.
- Use ADC1 channel 18 (PA4) via `rt_device_find("adc1")` and `rt_adc_read()`.
- Export global variable `g_o2_concentration` (float, O₂%).
- Create a dedicated periodic read thread in `main.c`: read every `O2_READ_INTERVAL_MS` and update the global.
- Apply linear conversion formula from the STM32 reference example: `O2%(×10) = (ADC_mV - ZERO_OFFSET) × 209 / (FULL_SCALE_MV - ZERO_OFFSET)`.
- Apply air-stabilize logic: if O2×10 is within 207–211, force to 209 (20.9%).
- No changes to existing UART, Modbus, LoRa, MQTT, ADC multi-channel (flame sensor), or I2C/SHT30 paths.
- Configure PA4 as ADC1_INP18 in the board ADC MSP initialization and use the RT-Thread ADC device framework.
- Huawei Cloud and LoRa reporting integration for O2 data is marked as a **future task**.

## Impact
- Affected specs: `sensor-acquisition` (new requirement for O2 ADC acquisition)
- Affected code:
  - `applications/Apps/o2SensorApp.h` (new)
  - `applications/Apps/o2SensorApp.c` (new)
  - `applications/main.c` (thread creation)
  - `applications/heads.h` (include new header)
  - `drivers/board.c` (configure PA4 as ADC1_INP18)
  - `drivers/board.h` (ADC1 remains enabled)
