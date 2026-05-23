# Change: Add SHT30 Temperature/Humidity Sensor Acquisition via I2C1

## Why
The monitoring node currently lacks ambient temperature and humidity data. Adding an SHT30 sensor over I2C1 (PB6 SCL / PB7 SDA) fills this gap without occupying any remaining UART or ADC resource.

## What Changes
- Enable RT-Thread software I2C1 bus in `drivers/board.h` with PB6/PB7 pin mapping.
- Enable RT-Thread I2C device framework in `rtconfig.h` / RT-Thread Settings.
- Add `applications/Apps/sht30App.c` and `applications/Apps/sht30App.h`: SHT30 driver using RT-Thread I2C device API.
- Export global variables `g_temperature_c` (float, Celsius) and `g_humidity_rh` (float, %RH).
- Create a periodic acquisition thread in `main.c`: read every defined interval and update globals.
- Reserve CRCs/error-handling so that on I2C failure the last valid reading is preserved and an error is logged.
- No changes to existing UART, Modbus, LoRa, MQTT, or ADC paths.
- Huawei Cloud and LoRa reporting integration for the new data is marked as a **future task**; this change only covers acquisition.

## Impact
- Affected specs: `sensor-acquisition` (new requirement for SHT30 I2C1 acquisition)
- Affected code:
  - `drivers/board.h` (enable `BSP_USING_I2C1` + pin definitions)
  - `rtconfig.h` or RT-Thread Settings (enable I2C framework, software I2C driver)
  - `applications/Apps/sht30App.h` (new)
  - `applications/Apps/sht30App.c` (new)
  - `applications/main.c` (thread creation)
  - `applications/heads.h` (include new header)
