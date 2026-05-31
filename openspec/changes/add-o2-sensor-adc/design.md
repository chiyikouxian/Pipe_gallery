## Context

The oxygen sensor is an analog-output module. Its output voltage is proportional to O₂ concentration (0–~2.41V maps to 0–20.9% O₂). The STM32F103 reference example uses ADC1 with Standard Peripheral Library; this project is on STM32H743XI with RT-Thread and must adapt the conversion logic while using RT-Thread's ADC device framework.

### Constraints
- Must use **RT-Thread ADC device framework** (`rt_device_find`, `rt_adc_enable`, `rt_adc_read`), not HAL direct calls.
- Must use pin **PC5 = ADC12_INP8** (channel 8).
- Must not modify CubeMX HAL ADC init path (the existing flame sensor ADC1 init is left intact).
- Must avoid `%f` in `rt_kprintf`; O₂ values are printed as scaled integers (×10).
- Must not touch existing UART, Modbus, LoRa, MQTT, SHT30 I2C, or flame sensor ADC modules.

### Oxygen Sensor Reference Example (F103 → H743 Adaptation)
| Parameter | F103 Example | H743 Adaptation |
|-----------|-------------|-----------------|
| ADC instance | ADC1 | ADC1 (shared) |
| Channel | Channel 9 (PB1) | Channel 8 (PC5) |
| Resolution | 12-bit | 12-bit |
| Vref | 3.3V | 3.3V |
| Full-scale voltage | ~2.41V | ~2.41V (same sensor) |
| Conversion formula | `(mV - 0) × 209 / (2410 - 0)` | Same |
| Air-stabilize zone | 207–211 → 209 | Same |
| Output unit | O₂% × 10 | O₂% (float) |
| Sampling method | 100 samples, bubble sort, trim 20 each end | Single-shot per cycle (simpler, sufficient for 2000ms interval) |

### Data Flow
```
O2 Sensor ──ADC1 Ch8(PC5)──> o2SensorApp.c ──> g_o2_concentration (float)
                               rt_adc_read()      │
                                                  └── (future) Huawei / LoRa reporting
```

## Goals / Non-Goals

### Goals
- Read O₂ sensor via RT-Thread ADC device framework on ADC1 channel 8 (PC5).
- Apply conversion formula and air-stabilize logic from reference example.
- Export `g_o2_concentration` (float, O₂%) as shared global.
- Run periodic acquisition in a dedicated RT-Thread thread.
- On ADC read failure, keep the last valid value and log the error.

### Non-Goals
- No CubeMX re-generation or HAL ADC layer modification.
- No calibration persistence — calibration values are compile-time macros.
- No multi-sample averaging with bubble sort (simpler single-read per cycle).
- No reporting-thread integration in this change (Huawei payload, LoRa text payload).
- No modification to existing flame sensor ADC path.

## Design Decisions

### Decision 1: RT-Thread ADC device framework, not HAL direct calls
- **Rationale**: `RT_USING_ADC` is already enabled. The framework provides `rt_adc_read()` which handles channel selection and conversion transparently. This avoids adding HAL-specific code in the application layer and keeps the module independent of the flame sensor's HAL ADC path.
- **Trade-off**: RT-Thread ADC framework adds a thin abstraction layer, but it's already compiled in and proven for this project.

### Decision 2: Single-read per cycle, no multi-sample averaging
- **Rationale**: The F103 example uses 100-sample averaging because it has a fast loop with no RTOS delay. In our RT-Thread model with a 2000ms cycle, single readings are sufficient. If noise becomes an issue on real hardware, a simple moving average can be added later.
- **Trade-off**: Slightly noisier than the 100-sample approach, but acceptable for environmental monitoring at 2-second intervals.

### Decision 3: PC5 = ADC12_INP8 (channel 8)
- **Rationale**: PB1 is already occupied by the flame sensor (CH5). PC5 is physically near PC4 (flame sensor CH4), is free, and maps to ADC1 channel 8.
- **Trade-off**: None. This channel is unused.

### Decision 4: Separate `o2SensorApp.c/h` module
- **Rationale**: Follows project convention of one App module per sensor type (MethaneSensorApp, linesensor, sht30App). Keeps scope isolated and reviewable.

## Conversion Formula

```
// Step 1: ADC raw → millivolts
mV = raw * 3300 / 4096    // 12-bit ADC, 3.3V Vref

// Step 2: mV → O2 concentration (×10 for integer math)
o2_x10 = (mV - O2_ZERO_OFFSET_MV) * 209 / (O2_FULL_SCALE_MV - O2_ZERO_OFFSET_MV)

// Step 3: Air-stabilize
if (o2_x10 >= 207 && o2_x10 <= 211)
    o2_x10 = 209;

// Step 4: Store as float %
g_o2_concentration = (float)o2_x10 / 10.0f;
```

## Calibration Macros (compile-time, user-adjustable)

```c
#define O2_ZERO_OFFSET_MV       0       // mV at 0% O2
#define O2_FULL_SCALE_MV        2410    // mV at 20.9% O2 (calibrate per sensor)
#define O2_AIR_STABILIZE_LOW    207     // ×10, lower bound
#define O2_AIR_STABILIZE_HIGH   211     // ×10, upper bound
#define O2_AIR_STABILIZE_VALUE  209     // ×10, force to 20.9%
```

## Thread Model

| Property | Value |
|----------|-------|
| Thread name | `o2_rd` |
| Entry function | `o2_thread_entry` |
| Priority | 25 (same as other sensor threads) |
| Stack | 1024 bytes (ADC read is lightweight) |
| Tick | 10 |

## ADC Device Configuration

No board.h or rtconfig.h changes needed:
- `RT_USING_ADC` is already enabled in `rtconfig.h`.
- `BSP_USING_ADC1` is already enabled in `drivers/board.h`.
- The new channel 8 is accessed at application level via `rt_adc_read(adc_dev, 8, &raw)`.
- The existing HAL ADC init in `board.c` / CubeMX already configures ADC1; no modification required.

## Risks / Mitigations

| Risk | Mitigation |
|------|-----------|
| ADC1 channel 8 not enabled in CubeMX HAL init | If read fails, log clear error; user re-generates CubeMX to add channel 8 |
| ADC device "adc1" not found | Check `rt_device_find` return value; log error |
| Calibration values differ per sensor unit | Expose as macros in header; user adjusts before compile |
| Float formatting in `rt_kprintf` | Print as `X.X` via integer math: `(int)val` and `(int)((val-int_val)*10)` |
