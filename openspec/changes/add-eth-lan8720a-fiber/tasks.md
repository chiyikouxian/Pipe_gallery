## 1. Hardware Preparation & Verification
- [x] 1.1 Connect LAN8720A differential output to Fiber Media Converter A via Ethernet cable.
- [x] 1.2 Connect Fiber Media Converter A -> Fiber cable -> Fiber Media Converter B -> PC RJ45.
- [x] 1.3 Configure PC server: IP `192.168.1.100`, subnet `255.255.255.0`, gateway `192.168.1.1`, TCP listener on port 8080.

## 2. Driver-Level Fixes & Validation
- [x] 2.1 Resolve PHY reset pin by neutralizing `phy_reset()` for LAN8720A modules without exposed `nRST`.
- [x] 2.2 Verify `ETH_RESET_PIN "PD3"` in `drivers/board.h` and document it as unused (software reset via MDIO).
- [x] 2.3 Confirm that `PHY_Status_REG 0x1F` is correct for LAN8720A.
- [x] 2.4 Verify `HAL_ETH_MODULE_ENABLED` is defined in the active HAL config.
- [x] 2.5 Disable DHCP for this point-to-point link so LWIP uses static IP `192.168.1.30`.
- [x] 2.6 Enable `ETH_IRQn` in `drivers/board.c` so RX complete interrupts reach `HAL_ETH_RxCpltCallback()`.

## 3. Build & Deploy
- [x] 3.1 Flash updated image to STM32H743XI board and verify runtime behavior on hardware.
- [x] 3.2 Verify boot log shows ethernet init success, PHY found at `0x01`, and stable `link up / 100Mbps / full-duplex`.
- [x] 3.3 Verify log: `ETH TCP client thread started` and `connected to 192.168.1.100:8080`.
- [x] 3.4 Verify log: periodic `Sent N bytes via ETH` messages.

## 4. End-to-End Data Verification
- [x] 4.1 Confirm PC server receives JSON data on port 8080 from the MCU.
- [x] 4.2 Verify JSON fields: `device_id`, ADC channels, methane ppm/lel, voltage/current, flow, temperature, humidity, o2, flame.
- [x] 4.3 Verify direct copper path `MCU -> Ethernet cable -> PC`.
- [x] 4.4 Verify fiber path `MCU -> cable -> Converter A -> fiber -> Converter B -> cable -> PC`.

## 5. Documentation
- [x] 5.1 Document the final PHY address and wiring/debug notes in `design.md`.
- [x] 5.2 Update `CLAUDE.md` with Ethernet/LAN8720A hardware notes.
