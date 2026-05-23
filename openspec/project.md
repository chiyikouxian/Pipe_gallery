# Project Context

## Purpose
This repository implements an embedded monitoring node based on **STM32H743XI + RT-Thread**.
The node acquires and reports field data including methane, ADC channels, three-phase electrical values, and water flow.

Current valid reporting/communication paths in repository code are:
- Modbus RTU (meter polling over UART3)
- Huawei Cloud IoTDA MQTT (ESP8266 AT command path over UART4)
- LoRa reporting (ATK-LORA-01 over UART5)

No external HTTP reporting feature is active in current implementation baseline.

## Tech Stack
- **MCU**: STM32H743XI (ARM Cortex-M7)
- **RTOS**: RT-Thread v4.0.3
- **Language**: C
- **Build**: SCons / RT-Thread Studio workflow
- **Communication**:
- Modbus RTU
- MQTT over ESP8266 AT commands
- LoRa (UART5)

## Project Conventions

### Code Style
- Application modules are under `applications/Apps/`.
- Function naming is primarily `snake_case`.
- Macros use `UPPER_SNAKE_CASE`.
- Shared sensor state commonly uses global variables (`g_*`) and/or `node[]`.
- Thread names are concise lowercase with underscores (for example `md_m_poll`, `lora_tx`, `uart4_rx`).

### Architecture Patterns
- Layer split:
- `applications/Apps/`: app logic and communication orchestration
- `drivers/`: board/peripheral drivers
- `rt-thread/`: RTOS kernel/components
- Multi-threaded runtime with separate threads for acquisition and reporting paths.
- Shared data model through global values and structures consumed by reporting threads.

### Testing Strategy
- Primary validation is on real hardware.
- Typical process:
- Build with RT-Thread Studio/SCons
- Deploy to board
- Verify serial logs and cloud/LoRa receiving side behavior

## Domain Context

### Hardware/Ports
- UART1: console (`PA9` TX, `PA10` RX)
- UART2: methane sensor (`PA2` TX, `PA3` RX)
- UART3: Modbus RTU master (`PB10` TX, `PB11` RX)
- Modbus direction control: `PA15`
- UART4: ESP8266 (Huawei MQTT path, `PA11` TX, `PA12` RX)
- UART5: LoRa module (ATK-LORA-01, `PC12` TX, `PD2` RX)
- ADC1: multi-channel analog acquisition (`PA0`, `PA1`, `PA6`, `PC4`, `PB1`)

### Data Sources
- Methane (PPM / LEL)
- ADC channels
- Three-phase voltage/current
- Water flow

### Reporting Baseline
- Huawei Cloud IoTDA MQTT path is present in current code.
- LoRa reporting path is present in current code.
- No active external HTTP reporting module in code baseline.

## Important Constraints
- RT-Thread `printf` float-format limitations exist in parts of the codebase.
- ESP8266 AT-command sequencing and serial response timing are critical.
- Validation still relies heavily on board-level integration tests.

## External Dependencies
- Huawei Cloud IoTDA (MQTT path via ESP8266 AT commands)
- FreeModbus / RT-Thread Modbus components
- ATK-LORA-01 module

## Scope Notes
- `Pipe_gallery.md` is a **historical/archived reference** from a removed direction and is **not** the current implementation target.
- Any future cloud migration or new reporting path must be handled as a new, explicitly approved change.
