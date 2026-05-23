## Context

SHT30 is a digital temperature/humidity sensor with I2C interface. This project already has the RT-Thread software I2C driver (`drivers/drv_soft_i2c.c`) compiled in the tree. The `board.h` I2C section provides the standard template for enabling software I2C1. The sensor will be the only device on this bus in the current design.

### Constraints
- Must use **software I2C1**, not hardware HAL I2C, consistent with the existing `drv_soft_i2c.c` pattern.
- Must use RT-Thread I2C device API (`rt_i2c_bus_device_find`, `rt_i2c_transfer`).
- Must avoid `%f` in `rt_kprintf` logs; float values are converted to scaled integers for printing.
- Must not touch existing UART, Modbus, LoRa, MQTT, or ADC modules.
- The sensor shares no hardware resources with existing peripherals.

### SHT30 Technical Details
| Parameter | Value |
|-----------|-------|
| Default 7-bit address | `0x44` |
| Recommended measurement command | Single-shot, high repeatability: `0x2C 0x06` |
| CRC polynomial | `0x31` (x⁸ + x⁵ + x⁴ + 1) |
| CRC initial value | `0xFF` |
| Temperature conversion | `T(°C) = -45 + 175 × raw / 65535` |
| Humidity conversion | `RH(%) = 100 × raw / 65535` |
| Measurement duration (high repeatability) | ~15 ms |
| Acquisition period (app-level) | Configurable via `SHT30_READ_INTERVAL_MS` |

### Data Flow
```
SHT30 ──I2C1──> sht30App.c ──> g_temperature_c / g_humidity_rh
                                   │
                                   └── (future) Huawei / LoRa reporting threads
```

## Goals / Non-Goals

### Goals
- Enable software I2C1 on PB6/PB7.
- Implement SHT30 single-shot read with CRC8 verification.
- Export `g_temperature_c` (float) and `g_humidity_rh` (float) as shared globals.
- Run periodic acquisition in a dedicated RT-Thread thread.
- On I2C or CRC failure, keep the last valid value and log the error.

### Non-Goals
- No dynamic device enumeration or multi-device I2C bus scanning.
- No reporting-thread integration in this change (Huawei payload, LoRa text payload).
- No SHT30 heater control, alert mode, or periodic mode—only single-shot read.
- No HAL I2C migration or hardware I2C1 configuration.
- No persistent storage or calibration.

## Design Decisions

### Decision 1: Software I2C1 via existing `drv_soft_i2c.c`
- **Rationale**: `board.h` already provides the software I2C enable pattern. The driver `drv_soft_i2c.c` is already present. Adding a new hardware I2C HAL init path would be inconsistent with the project baseline.
- **Trade-off**: Software I2C is slower than hardware I2C, but SHT30 measurement timing (~15 ms) dominates over bus bit-banging overhead.

### Decision 2: Single-shot mode, not periodic auto-measurement
- **Rationale**: The SHT30 supports a "periodic" mode where it auto-samples at configurable MPS and the MCU just fetches results. However, periodic mode requires more complex state management (start periodic → wait → fetch → stop). Single-shot (`0x2C 0x06`) is simpler, stateless, and aligns with the project's thread-based polling model.
- **Trade-off**: Single-shot adds a ~15 ms blocking wait per read. Given the expected read interval (≥ 2000ms), this is acceptable.

### Decision 3: CRC8 verification + last-valid-value fallback
- **Rationale**: SHT30 datasheet mandates CRC8 on each 16-bit data word. Skipping CRC risks accepting corrupted readings. On failure, retaining the last valid value avoids reporting garbage or zero.
- **Implementation**: A small `sht30_crc8(const uint8_t *data, int len)` function; polynomial `0x31`, init `0xFF`.

### Decision 4: Separate `sht30App.c/h` module, not embedded in an existing App
- **Rationale**: Follows the project convention of one App module per sensor type (e.g., `MethaneSensorApp`, `linesensor`). Keeps scope isolated and reviewable.

## Data Model

```c
/* sht30App.h */
extern float g_temperature_c;   /* Temperature in Celsius */
extern float g_humidity_rh;     /* Relative humidity in % */
```

Both are initialized to `0.0f`. Updates are atomic at the RT-Thread thread context (single writer). Readers (future reporting threads) can safely read stale copies.

## Thread Model

| Property | Value |
|----------|-------|
| Thread name | `sht30_rd` |
| Entry function | `sht30_thread_entry` |
| Priority | 25 (same as other sensor threads) |
| Stack | 1024 bytes |
| Tick | 10 |

## I2C Bus Configuration

In `drivers/board.h`, uncomment and modify the I2C1 template:
```c
#define BSP_USING_I2C1
#define BSP_I2C1_SCL_PIN    GET_PIN(B, 6)
#define BSP_I2C1_SDA_PIN    GET_PIN(B, 7)
```

The RT-Thread I2C device framework must also be enabled in RT-Thread Settings (`RT_USING_I2C`, `RT_USING_I2C_BITOPS`), and the software I2C driver (`RT_USING_I2C1`) must be enabled. This is typically done via `menuconfig` or by directly editing `rtconfig.h`.

## Risks / Mitigations

| Risk | Mitigation |
|------|-----------|
| I2C bus not enabled in rtconfig.h before compilation | `tasks.md` explicitly lists this as the first implementation step |
| SHT30 not responding (wrong address, wiring) | Log a clear error on first `rt_i2c_transfer` failure; `rt_i2c_transfer` return value checked |
| Float formatting in `rt_kprintf` | Convert to scaled int: `(int)(g_temperature_c * 100)` prints as `XX.YY` |
| Thread stack overflow | 1024 bytes is conservative for an I2C read + variable update loop |
