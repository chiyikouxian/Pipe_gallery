## 1. Hardware Preparation & Verification
- [ ] 1.1 Inspect LAN8720A module: identify clock source (crystal vs external), PHYAD strap pins, and whether nRST or INT pins are available.
- [ ] 1.2 Verify RMII wiring between STM32H743 and LAN8720A module per [design.md pin mapping](./design.md).
- [ ] 1.3 Connect LAN8720A differential output to Fiber Media Converter A via Ethernet cable (with or without magnetic isolation as needed).
- [x] 1.4 Connect Fiber Media Converter A -> Fiber cable -> Fiber Media Converter B -> PC RJ45.
- [x] 1.5 Configure PC server: IP `192.168.1.100`, subnet `255.255.255.0`, gateway `192.168.1.1`, TCP listener on port 8080.

## 2. Driver-Level Fixes & Validation
- [x] 2.1 Resolve PHY reset pin: if LAN8720A module has no nRST, modify `drivers/drv_eth.c` or `drivers/board.h` to neutralize `phy_reset()` (no-op or remove the PD3 toggle).
- [x] 2.2 Verify `ETH_RESET_PIN "PD3"` in `drivers/board.h` and document it as unused (software reset via MDIO).
- [ ] 2.3 Validate PHY address auto-scan logic in `drv_eth.c` against LAN8720A register behavior. Add fallback fixed address if scan fails.
- [x] 2.4 Confirm that `PHY_Status_REG 0x1F` is correct for LAN8720A (verified in `drv_eth.h`, matches datasheet).
- [x] 2.5 Verify `HAL_ETH_MODULE_ENABLED` is defined in the active HAL config (`cubemx/Inc/stm32h7xx_hal_conf.h:48`).
- [x] 2.6 Disable DHCP for this point-to-point link so LWIP uses static IP `192.168.1.30`.
- [x] 2.7 Enable `ETH_IRQn` in `drivers/board.c` so RX complete interrupts reach `HAL_ETH_RxCpltCallback()`.

## 3. Build & Deploy
- [ ] 3.1 Run `scons` and confirm zero errors and no new warnings.
- [x] 3.2 Flash updated image to STM32H743XI board and verify runtime behavior on hardware.
- [x] 3.3 Verify boot log shows ethernet init success, PHY found at `0x01`, and stable `link up / 100Mbps / full-duplex`.
- [x] 3.4 Verify log: `ETH TCP client thread started` and `connected to 192.168.1.100:8080`.
- [x] 3.5 Verify log: periodic `Sent N bytes via ETH` messages.

## 4. End-to-End Data Verification
- [x] 4.1 Confirm PC server receives JSON data on port 8080 from the MCU.
- [x] 4.2 Verify JSON fields: `device_id`, all ADC channels, methane ppm/lel, voltage_a/b/c, current_a/b/c, flow, temperature, humidity, o2, flame.
- [ ] 4.3 Test link resilience: disconnect/reconnect fiber cable and verify TCP reconnect logic (3 failures -> re-initiate connection).
- [ ] 4.4 Verify all existing threads (Modbus, LoRa, MQTT, ADC, SHT30, O2, UART) continue to run normally alongside ETH.

## 5. Documentation
- [x] 5.1 Document the final PHY address, clock mode, and wiring/debug notes in `design.md`.
- [x] 5.2 Update `CLAUDE.md` with Ethernet/LAN8720A hardware notes if significant.
