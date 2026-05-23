## 1. Enable RT-Thread I2C Framework
- [x] 1.1 In RT-Thread Settings or `rtconfig.h`, enable `RT_USING_I2C` and `RT_USING_I2C_BITOPS` (software I2C).
- [x] 1.2 Enable software I2C1 driver: `BSP_USING_I2C1` in `drivers/board.h` (this BSP uses `BSP_USING_I2C1` not `RT_USING_I2C1`; the `drv_soft_i2c.h` macro `I2C1_BUS_CONFIG` is gated on `BSP_USING_I2C1`).
- [x] 1.3 Verify `drivers/drv_soft_i2c.c` is included in the build (already present in `drivers/`, compiled when `RT_USING_I2C` is defined).

## 2. Configure I2C1 Pins in board.h
- [x] 2.1 Uncomment `#define BSP_USING_I2C1` in `drivers/board.h`.
- [x] 2.2 Set `BSP_I2C1_SCL_PIN` to `GET_PIN(B, 6)`.
- [x] 2.3 Set `BSP_I2C1_SDA_PIN` to `GET_PIN(B, 7)`.

## 3. Add SHT30 Application Module
- [x] 3.1 Create `applications/Apps/sht30App.h` with:
  - I2C bus name macro `SHT30_I2C_BUS_NAME "i2c1"`
  - I2C address macro `SHT30_I2C_ADDR 0x44`
  - Measurement command macro `SHT30_CMD_MEAS_HIGH_REP {0x2C, 0x06}`
  - Read interval macro `SHT30_READ_INTERVAL_MS 2000`
  - Global variable declarations: `extern float g_temperature_c; extern float g_humidity_rh;`
  - Thread entry declaration: `void sht30_thread_entry(void *parameter);`
  - Init function declaration: `rt_err_t sht30_init(void);`
- [x] 3.2 Create `applications/Apps/sht30App.c` with:
  - CRC8 helper: `static uint8_t sht30_crc8(const uint8_t *data, int len)` (poly 0x31, init 0xFF)
  - I2C read function `sht30_read_once()` using `rt_i2c_transfer` (send cmd `0x2C 0x06`, wait 20ms, read 6 bytes, CRC8 verify)
  - Temperature conversion: `T = -45.0f + 175.0f * raw_t / 65535.0f`
  - Humidity conversion: `RH = 100.0f * raw_h / 65535.0f`
  - On I2C/CRC failure: keep last valid value, log error via LOG_E / LOG_W
  - Thread entry `sht30_thread_entry`: find I2C bus → loop { read → update globals → log scaled int → sleep 2000ms }
  - Init `sht30_init()`: find I2C bus → create thread "sht30_rd" (prio 25, stack 1024, tick 10) → start

## 4. Integrate into System Startup
- [x] 4.1 Add `#include "sht30App.h"` to `applications/heads.h`.
- [x] 4.2 In `applications/main.c`, call `sht30_init()` after line_sensor_init() and before LoRa thread.
- [x] 4.3 Log init success/failure to serial console via `rt_kprintf`.

## 5. Build Verification
- [ ] 5.1 Run `scons` (or RT-Thread Studio build) and confirm zero errors.
- [ ] 5.2 Confirm no new warnings introduced.

## 6. Board-Level Verification
- [x] 6.1 Confirm serial log shows "SHT30 initialized on i2c1" at boot.
- [x] 6.2 Confirm periodic log output: `[SHT30] T=XX.X C, RH=XX.X %` (scaled int format).
- [ ] 6.3 Disconnect SHT30 and confirm I2C error log appears without crash.
- [ ] 6.4 Confirm all existing threads (Modbus, LoRa, MQTT, ADC, UART) continue to run normally.

## 7. (Future Task, Blocked) Huawei Cloud Reporting Integration
- [ ] 7.1 Add temperature/humidity fields to Huawei Cloud property-report JSON in `huaweiCloudApp.c`.
- [ ] 7.2 Create or update service definitions on the cloud platform side.

## 8. (Future Task, Blocked) LoRa Reporting Integration
- [ ] 8.1 Add temperature/humidity fields to LoRa text payload in `loraApp.c`.
