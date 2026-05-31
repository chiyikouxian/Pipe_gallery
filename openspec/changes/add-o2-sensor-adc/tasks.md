## 1. Add O2 Sensor Application Module
- [x] 1.1 Create `applications/Apps/o2SensorApp.h` with:
  - ADC device name macro `O2_ADC_DEVICE_NAME "adc1"`
  - ADC channel macro `O2_ADC_CHANNEL 8`
  - Read interval macro `O2_READ_INTERVAL_MS 2000`
  - Calibration macros: `O2_ZERO_OFFSET_MV 0`, `O2_FULL_SCALE_MV 2410`
  - Stabilize macros: `O2_AIR_STABILIZE_LOW 207`, `O2_AIR_STABILIZE_HIGH 211`, `O2_AIR_STABILIZE_VALUE 209`
  - Global variable declaration: `extern float g_o2_concentration;`
  - Thread entry declaration: `void o2_thread_entry(void *parameter);`
  - Init function declaration: `rt_err_t o2_sensor_init(void);`
- [x] 1.2 Create `applications/Apps/o2SensorApp.c` with:
  - ADC read via `rt_device_find("adc1")` → `rt_adc_enable()` → `rt_adc_read(dev, 8, &raw)`
  - ADC raw-to-mV conversion: `mV = raw * 3300 / 4096`
  - O2 concentration conversion (integer ×10): `o2_x10 = (mV - 0) * 209 / (2410 - 0)`
  - Air-stabilize: if `o2_x10` in [207, 211], force to 209
  - Store `g_o2_concentration = (float)o2_x10 / 10.0f`
  - On ADC read failure: keep last valid value, log error via LOG_E
  - Thread entry: init ADC device → loop { read → convert → update global → log scaled int → sleep interval }
  - Log output as scaled integer: `[O2] XX.X %` (avoid `%f`)

## 2. Integrate into System Startup
- [x] 2.1 Add `#include "o2SensorApp.h"` to `applications/heads.h`.
- [x] 2.2 In `applications/main.c`, call `o2_sensor_init()` after SHT30 init and before LoRa thread.
- [x] 2.3 Log init success/failure to serial console via `rt_kprintf`.

## 3. Build Verification
- [ ] 3.1 Run build (RT-Thread Studio) and confirm zero errors.
- [ ] 3.2 Confirm no new warnings introduced.

## 4. Board-Level Verification
- [ ] 4.1 Confirm serial log shows "O2 sensor initialized on adc1 ch8" at boot.
- [ ] 4.2 Confirm periodic log output: `[O2] XX.X %` (scaled int format).
- [ ] 4.3 Disconnect O2 sensor and confirm ADC read error log appears without crash.
- [ ] 4.4 Confirm all existing threads (Modbus, LoRa, MQTT, ADC flame, SHT30, UART) continue to run normally.
- [ ] 4.5 Verify O2 reading is ~20.9% in normal air; adjust `O2_FULL_SCALE_MV` if needed.

## 5. (Future Task, Blocked) Huawei Cloud Reporting Integration
- [ ] 5.1 Add O2 concentration field to Huawei Cloud property-report JSON in `huaweiCloudApp.c`.

## 6. (Future Task, Blocked) LoRa Reporting Integration
- [ ] 6.1 Add O2 concentration field to LoRa text payload in `loraApp.c`.
