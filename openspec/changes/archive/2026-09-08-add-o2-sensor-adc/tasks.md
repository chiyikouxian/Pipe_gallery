## 1. Add O2 Sensor Application Module
- [x] 1.1 Create `applications/Apps/o2SensorApp.h` with:
  - ADC device name macro `O2_ADC_DEVICE_NAME "adc1"`
  - ADC channel macro `O2_ADC_CHANNEL 18`
  - Read interval macro `O2_READ_INTERVAL_MS 2000`
  - Calibration macros: `O2_ZERO_OFFSET_MV 0`, `O2_FULL_SCALE_MV 2410`
  - Stabilize macros: `O2_AIR_STABILIZE_LOW 207`, `O2_AIR_STABILIZE_HIGH 211`, `O2_AIR_STABILIZE_VALUE 209`
  - Global variable declaration: `extern float g_o2_concentration;`
  - Thread entry declaration: `void o2_thread_entry(void *parameter);`
  - Init function declaration: `rt_err_t o2_sensor_init(void);`
- [x] 1.2 Create `applications/Apps/o2SensorApp.c` with:
  - ADC read via `rt_device_find("adc1")` → `rt_adc_enable()` → `rt_adc_read(dev, 18)`
  - ADC raw-to-mV conversion: `mV = raw * 3300 / 4096`
  - O2 concentration conversion (integer ×10): `o2_x10 = (mV - 0) * 209 / (2410 - 0)`
  - Air-stabilize: if `o2_x10` in [207, 211], force to 209
  - Store `g_o2_concentration = (float)o2_x10 / 10.0f`
  - On ADC device lookup/channel-enable failure: return an error and do not create the thread
  - Thread entry: init ADC device → loop { read → convert → update global → sleep interval }

## 2. Integrate into System Startup
- [x] 2.1 Add `#include "o2SensorApp.h"` to `applications/heads.h`.
- [x] 2.2 In `applications/main.c`, call `o2_sensor_init()` after SHT30 init and before LoRa thread.
- [x] 2.3 Check the `o2_sensor_init()` return value in `main.c` and emit a visible success/failure log.

## 3. Board Pin Integration
- [x] 3.1 Configure PA4 as ADC1_INP18 in `drivers/board.c`.
- [x] 3.2 Keep PC5 reserved for Ethernet RMII RXD1.

## 4. Build Verification
- [ ] 4.1 Run build (RT-Thread Studio/SCons with a configured ARM toolchain) and confirm zero errors. *(requires ARM toolchain / hardware)*
- [ ] 4.2 Confirm no new warnings introduced. *(requires ARM toolchain / hardware)*

## 5. Board-Level Verification
- [ ] 5.1 Confirm serial log shows "O2 sensor initialized on adc1 ch18" at boot. *(requires hardware)*
- [ ] 5.2 Confirm periodic O2 values are updated without disrupting other threads. *(requires hardware)*
- [ ] 5.3 Disconnect O2 sensor and confirm the system remains stable. *(requires hardware)*
- [ ] 5.4 Confirm all existing threads (Modbus, LoRa, MQTT, ADC flame, SHT30, UART, Ethernet) continue to run normally. *(requires hardware)*
- [ ] 5.5 Verify O2 reading is ~20.9% in normal air; adjust `O2_FULL_SCALE_MV` if needed. *(requires hardware)*

## 6. Ethernet Reporting Integration
- [x] 6.1 Include O2 concentration in the Ethernet TCP JSON payload in `ethApp.c`.

## 7. (Deferred) Huawei Cloud Reporting Integration
- [ ] 7.1 Add O2 concentration field to Huawei Cloud property-report JSON in `huaweiCloudApp.c`. *(deferred - future task)*

## 8. (Deferred) LoRa Reporting Integration
- [ ] 8.1 Add O2 concentration field to LoRa text payload in `loraApp.c`. *(deferred - future task)*
