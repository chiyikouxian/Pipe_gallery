# Design: Ethernet LAN8720A Fiber Optic Data Link

## Context

This change formalizes the existing LAN8720A RMII Ethernet implementation as an official project capability. The software stack (HAL → driver → LWIP → TCP client) is already written and partially tested; this design documents the hardware interface, data flow, and remaining verification items.

### Hardware Topology

```
┌──────────────┐    RMII 10-wire    ┌──────────────┐   Diff. Ethernet   ┌───────────────┐
│ STM32H743XI  │ ◄────────────────► │  LAN8720A    │ ◄────────────────► │ Fiber Media   │
│  (MAC + DMA) │   PA1=REF_CLK(50M)│  PHY Module  │  TX+/TX- RX+/RX-  │ Converter A   │
└──────────────┘   PA2=MDIO        └──────┬───────┘                   └───────┬───────┘
                    PA7=CRS_DV             │                                   │
                    PC1=MDC               │                                   │
                    PC4=RXD0              │                                   │
                    PC5=RXD1              │                                   │
                    PG11=TX_EN            │                                   │
                    PG13=TXD0             │                                   │
                    PG14=TXD1             │                                   │
                                          │  ┌─────────┐ Fiber Optic Cable    │
                                          └──┤  RJ45   │──────────────────────┤
                                             │ Jack    │                      │
                                             └─────────┘                      │
                                                                              ▼
                                                                     ┌───────────────┐
                                                                     │ Fiber Media   │
                                                                     │ Converter B   │
                                                                     └───────┬───────┘
                                                                             │
                                                                     Standard RJ45
                                                                             │
                                                                             ▼
                                                                     ┌───────────────┐
                                                                     │  PC (Server)  │
                                                                     │192.168.1.100  │
                                                                     │  port 8080    │
                                                                     └───────────────┘
```

### Pin Mapping (STM32H743 → LAN8720A RMII)

| STM32 Pin | RMII Signal | LAN8720A Pin | Notes |
|-----------|-------------|-------------|-------|
| PA1 | REF_CLK (50MHz) | CLKIN (Pin 5) | 50MHz from STM32 ETH MAC PLL; board.c closes PA1 analog switch |
| PA2 | MDIO | MDIO (Pin 16) | UART2 relocated to PD5/PD6 |
| PA7 | CRS_DV | CRS_DV (Pin 10) | |
| PC1 | MDC | MDC (Pin 15) | |
| PC4 | RXD0 | RXD0 (Pin 11) | ADC A4 relocated to PA5 |
| PC5 | RXD1 | RXD1 (Pin 12) | O2 sensor relocated to PA4 (ADC1 CH18) |
| PG11 | TX_EN | TX_EN (Pin 2) | |
| PG13 | TXD0 | TXD0 (Pin 3) | |
| PG14 | TXD1 | TXD1 (Pin 4) | |
| PD3 | — | — | ⚠️ Module has NO reset pin; current PD3 assignment needs resolution |

## Goals / Non-Goals

### Goals
- Validate the existing LAN8720A RMII driver on real hardware.
- Resolve PHY reset strategy for modules without exposed RST pin.
- Verify PHY address auto-detection works with LAN8720A.
- Confirm TCP data reporting via Ethernet + fiber reaches the PC server.
- Document the complete hardware chain and wiring reference.

### Non-Goals
- No changes to the existing UART/Modbus/LoRa/MQTT/ADC/I2C paths.
- No new application-layer protocol (existing TCP JSON client in `ethApp.c` is sufficient).
- No DHCP — static IP `192.168.1.30` is used.
- No web server on the MCU side — the MCU is a TCP client, the PC is the server.
- No VLAN, QoS, or other advanced Ethernet features.

## Design Decisions

### Decision 1: RMII interface (not MII)
- **Rationale**: Fewer pins required (10 vs 16). All STM32H7 and LAN8720A boards support RMII. All existing code is already set to `HAL_ETH_RMII_MODE`.
- **Trade-off**: RMII runs at 50MHz (MII uses 25MHz TX_CLK/RX_CLK separately). The single 50MHz REF_CLK must be stable.

### Decision 2: REF_CLK from STM32 ETH MAC PLL (not external crystal on LAN8720A module)
- **Rationale**: The STM32H743 ETH peripheral can internally generate a 50MHz reference clock output on PA1 when configured for RMII. The `board.c` code already closes PA1's analog switch for this (`HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE)`). This eliminates the need for a crystal on the LAN8720A module.
- **Alternative considered**: LAN8720A can also use an external 25MHz crystal and output 50MHz on nINT/REFCLKO. This is simpler but requires the module to have the crystal populated. The F1 demo used MCU-side MCO. The F4 demo used MCU-side MCO.
- **LAN8720A clock mode conflict**: If the module HAS a 25MHz crystal installed, it will generate its own 50MHz REFCLKO output (pin 6). The PHY must NOT have both an external 50MHz on CLKIN AND an active internal oscillator. **This must be verified on the module** — check strap pins (LED2/nINTSEL) to determine if nINT/REFCLKO is in interrupt or clock-output mode.

### Decision 3: Software PHY reset via MDIO (no hardware reset pin needed)
- **Rationale**: The user's LAN8720A module does not expose a nRST pin. Per LAN8720A datasheet, a software reset can be performed by writing to the PHY Basic Control Register (address 0x00, bit 15 = 1), which the current `drv_eth.c` already does via `HAL_ETH_WritePHYRegister(&EthHandle, PHY_ADDR, PHY_BASIC_CONTROL_REG, PHY_RESET_MASK)`.
- **Action needed**: Remove or neutralize the PD3 hardware reset code (`phy_reset()` in `drv_eth.c`) since no pin is connected. Options: (a) configure PD3 as unused and let `phy_reset()` be a no-op, (b) remove the reset call from `rt_stm32_eth_init()`, or (c) use PD3 as a power-control GPIO for the module.

### Decision 4: PHY address auto-scan from 0x1F down to 0x00
- **Rationale**: The current `drv_eth.c` scanning logic starts at `0x1F` (broadcast) and iterates down. LAN8720A address is set by PHYAD[0:4] pins. Typical values:
  - PHYAD0 low → address 0x00 (commonly hardwired on standalone modules)
  - PHYAD0 high → address 0x01
  - STM32 EVAL boards often use 0x01
- **Risk**: The scan condition `(regvalue & PHY_BASIC_STATUS_REG) == i` reads register `PHY_SPECIAL_MODES_REG` (0x12) and checks if the low bits match the address. This is a heuristic — works on many PHYs but must be validated on LAN8720A.
- **Mitigation**: If the scan fails, fall back to a fixed `PHY_ADDR` via board.h macro.

### Decision 5: Keep existing TCP client application (`ethApp.c`) unchanged
- **Rationale**: The application layer is already functional: JSON-building, TCP socket, reconnect logic, sensor data assembly. This proposal focuses on the physical and driver layers; the application layer is a consumer.

## Data Flow (Software Stack)

```
Sensor Globals (g_adc_ch0..5, g_methane_ppm, Current[], Voltage[], etc.)
        │
        ▼
ethApp.c (eth_client_thread_entry, every 3s)
  build_sensor_json() → JSON string
  send(sock, json_buf, len, 0)
        │
        ▼
LWIP Socket Layer (SAL_USING_LWIP)
        │
        ▼
LWIP TCP/IP Stack (lwip-2.1.2)
        │
        ▼
netif → ethernetif (RT-Thread eth_device framework)
        │
        ▼
drv_eth.c (STM32 MAC + DMA driver)
  HAL_ETH_Transmit() → RMII MAC frame
        │
        ▼
STM32H743 ETH MAC → RMII pins → LAN8720A PHY → differential Ethernet
        │
        ▼
Fiber Media Converters (physical only, no software role)
        │
        ▼
PC Server (listening on 192.168.1.100:8080)
```

## Thread Model

| Thread Name | Priority | Stack | Role |
|------------|----------|-------|------|
| `eth_tcp` | 24 | 2048 | TCP client: connect, build JSON, send every 3s, reconnect on failure |
| `erx` | 12 | 1024 | LWIP ethernetif RX (kernel) |
| `etx` | 10 | 1024 | LWIP TCP/IP stack (kernel) |
| `phy` | MAX-2 | 1024 | PHY link monitor (polling timer or interrupt) |

## Network Configuration

```c
// rtconfig.h — already configured
#define RT_LWIP_IPADDR  "192.168.1.30"    // STM32 MCU static IP
#define RT_LWIP_GWADDR  "192.168.1.1"     // Gateway
#define RT_LWIP_MSKADDR "255.255.255.0"   // Subnet mask

// ethApp.h — already configured
#define ETH_SERVER_IP    "192.168.1.100"  // PC server IP
#define ETH_SERVER_PORT  8080             // TCP port
```

## PHY Register Reference (LAN8720A-Specific)

From `drivers/include/drv_eth.h` (already defined):

| Register | Address | Purpose |
|----------|---------|---------|
| BCR | 0x00 | Basic Control — bit 15 = software reset |
| BSR | 0x01 | Basic Status — bit 2 = link up, bit 5 = autoneg complete |
| PHY Special Modes | 0x12 | Used for PHY address detection |
| Interrupt Flag | 0x1D | Link status change etc. |
| Interrupt Mask | 0x1E | Enable/disable interrupt sources |
| Status | 0x1F | Speed (bits 2-3) and duplex (bit 4) |

## Risks / Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-----------|--------|-----------|
| LAN8720A module has no RST pin; PD3 toggles an unused pin | High | Low | Remove/neutralize `phy_reset()` call in drv_eth.c; rely on MDIO software reset |
| REF_CLK clock mode mismatch (module has 25MHz crystal, MCU also drives 50MHz) | Med | High | Inspect module's LED2/nINTSEL strapping; if conflicts, choose one clock source |
| PHY address scan fails (wrong register heuristic for LAN8720A) | Med | Med | Fall back to hardcoded PHY_ADDR=0x00 or 0x01; add board.h macro for fixed address |
| Fiber media converters incompatible (100Base-FX vs 100Base-TX auto-negotiation) | Low | Med | Ensure converters support 100Mbps full-duplex; LAN8720A auto-negotiates with converter |
| D2 SRAM3 (0x30040000) is cacheable → DMA corruption | Med | High | stm32h7 uses DTCM for most; D2 SRAM3 is in non-cacheable area. Verify SCB MPU configuration |
| Float formatting in log output (`%f`) | Low | Low | The ethApp.c already uses `append_float()` with integer math |

## Open Questions

1. **Does the user's LAN8720A module have a 25MHz crystal or does it rely on external 50MHz?** LED2/nINTSEL strap determines the nINT/REFCLKO pin function. If strapped as REFCLKO output (50MHz) and the MCU also outputs 50MHz on PA1 → conflict. If strapped as nINT interrupt, MCU's PA1 50MHz is the sole clock.

2. **Is the PD3 GPIO pin currently connected to anything on the module?** If the module has no RST pin at all, PD3 should either be left floating or repurposed (e.g., power control via MOSFET).

3. **What is the exact IP configuration of the PC side?** Does the PC server software already exist? The ethApp.c sends JSON to `192.168.1.100:8080` — is a server running there?
## Validation Notes

- Verified PHY address on the current LAN8720A module is `0x01`.
- Verified PC side uses `192.168.1.100/24` and a TCP listener on port `8080`.
- Verified this point-to-point setup must use static IPv4. Leaving `RT_LWIP_DHCP` enabled caused `e0` to stay at `0.0.0.0` with `DHCP_ENABLE`, which blocked ping and TCP connect.
- Verified ETH RX path required explicitly enabling `ETH_IRQn` in `HAL_ETH_MspInit()`. Before NVIC enable, ARP entries appeared on the PC and the MCU could transmit, but ping replies and TCP connect failed because RX complete interrupts never fired.
- Verified final end-to-end behavior on hardware:
  - Direct copper path `MCU -> Ethernet cable -> PC` succeeds
  - Fiber path `MCU -> cable -> Converter A -> fiber -> Converter B -> cable -> PC` succeeds
  - PC `ping 192.168.1.30` succeeds
  - MCU `ping 192.168.1.100` succeeds
  - `EthApp` logs `connected to 192.168.1.100:8080`
  - PC receives periodic JSON payloads from `192.168.1.30`
