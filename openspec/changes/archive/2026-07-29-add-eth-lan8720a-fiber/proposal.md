# Change: Add Ethernet LAN8720A Fiber Optic Data Link

## Why
The monitoring node needs a reliable wired data path to a PC. Using LAN8720A Ethernet PHY in RMII mode, data flows through fiber optic transceivers to a remote PC. This provides galvanic isolation and long-distance reach — critical for pipe gallery environments. The existing RS485/Modbus, LoRa (UART5), and ESP8266 WiFi (UART4) paths remain untouched; Ethernet becomes the primary high-speed reporting channel.

## What Changes
- **Validate** the existing LAN8720A RMII driver code (already written in `drivers/drv_eth.c`/`.h`, `drivers/board.c`/`.h`).
- **Fix PHY reset strategy**: The LAN8720A module used does NOT expose a RESET pin. Current code (`drivers/board.h:313`) defines `ETH_RESET_PIN "PD3"` which will fail. Mitigation: either configure PD3 as the MCU-side power-control GPIO, or remove the hardware reset and rely on software PHY reset via MDIO register write.
- **Verify REF_CLK path**: STM32H743 generates 50MHz on PA1 via ETH MAC internal PLL (HAL_ETH_RMII_MODE). The LAN8720A module must be configured to use external 50MHz clock input (XTAL1/CLKIN = PA1). Confirm the module's clock mode matches.
- **Verify PHY address auto-scan**: Current `drv_eth.c` scans PHY addresses 0x1F..0x00 but LAN8720A typical address is 0x00 or 0x01 (per PHYAD0 pin strapping on the module). The scanning logic condition `(regvalue & PHY_BASIC_STATUS_REG) == i` must be validated against LAN8720A register behavior.
- **Document complete hardware chain**: STM32 MCU → RMII → LAN8720A → differential Ethernet → Fiber Media Converter A → Fiber cable → Fiber Media Converter B → PC (192.168.1.100:8080).
- **No changes** to existing UART, Modbus, LoRa, MQTT, ADC, I2C, or SHT30/O2 sensor paths.

## Impact
- Affected specs: `ethernet-communication` (new capability)
- Affected code:
  - `drivers/board.h` (ETH_RESET_PIN may change)
  - `drivers/board.c` (reset pin handling)
  - `drivers/drv_eth.c` (PHY scan logic review)
  - `applications/Apps/ethApp.c`/`.h` (already exists: TCP client, JSON reporting)
  - `applications/main.c` (already calls `eth_app_init()`)
  - `rtconfig.h` (already configured: LWIP + static IP 192.168.1.30)
  - `cubemx/Inc/stm32h7xx_hal_conf.h` (already has `HAL_ETH_MODULE_ENABLED`)
  - `linkscripts/STM32H743XIHx/link.lds` (already has D2 SRAM3 for ETH DMA)
