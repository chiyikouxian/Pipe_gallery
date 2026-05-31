# Change: Add Oxygen Sensor Acquisition via ADC1 Channel 8 (PC5)

## Why
The monitoring node needs oxygen concentration data for the pipe gallery environment. An analog-output oxygen sensor module is available and can be connected to an unused ADC1 channel on PC5 without conflicting with existing ADC inputs or other peripherals.

## What Changes
- Add `applications/Apps/o2SensorApp.c` and `applications/Apps/o2SensorApp.h`: oxygen sensor driver using RT-Thread ADC device framework.
- Use ADC1 channel 8 (PC5) via `rt_device_find("adc1")` and `rt_adc_read()`.
- Export global variable `g_o2_concentration` (float, O₂%).
- Create a dedicated periodic read thread in `main.c`: read every `O2_READ_INTERVAL_MS` and update the global.
- Apply linear conversion formula from the STM32 reference example: `O2%(×10) = (ADC_mV - ZERO_OFFSET) × 209 / (FULL_SCALE_MV - ZERO_OFFSET)`.
- Apply air-stabilize logic: if O2×10 is within 207–211, force to 209 (20.9%).
- No changes to existing UART, Modbus, LoRa, MQTT, ADC multi-channel (flame sensor), or I2C/SHT30 paths.
- No CubeMX changes; uses RT-Thread ADC device framework (already enabled).
- Huawei Cloud and LoRa reporting integration for O2 data is marked as a **future task**.

## Impact
- Affected specs: `sensor-acquisition` (new requirement for O2 ADC acquisition)
- Affected code:
  - `applications/Apps/o2SensorApp.h` (new)
  - `applications/Apps/o2SensorApp.c` (new)
  - `applications/main.c` (thread creation)
  - `applications/heads.h` (include new header)
  - `drivers/board.h` (no change needed — ADC1 already enabled, just document PC5 usage)
