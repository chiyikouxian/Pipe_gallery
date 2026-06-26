# Pipe Gallery Node

Embedded monitoring node based on `STM32H743XI + RT-Thread`.

## Current Status

The Ethernet reporting path is working end to end.

Validated paths:
- Direct copper: `MCU -> Ethernet cable -> PC`
- Fiber path: `MCU -> cable -> Converter A -> fiber -> Converter B -> cable -> PC`

Validated behavior:
- PHY link up at `100Mbps full-duplex`
- MCU static IP: `192.168.1.30`
- PC static IP: `192.168.1.100`
- TCP connection to `192.168.1.100:8080`
- Periodic JSON sensor reports received on the PC

## Ethernet Bring-Up Notes

Key fixes required for LAN8720A bring-up:
- Disable `RT_LWIP_DHCP` and use static IPv4 for direct PC testing
- Enable `ETH_IRQn` in `drivers/board.c`, otherwise TX may work while RX silently fails
- LAN8720A on current hardware is detected at PHY address `0x01`
- Current module does not expose `nRST`; software reset via MDIO is used

## Test Topology

Direct copper:

`MCU -> RJ45 cable -> PC`

Fiber:

`MCU -> RJ45 cable -> Fiber Converter A -> fiber -> Fiber Converter B -> RJ45 cable -> PC`

## PC Test Setup

- IP: `192.168.1.100`
- Netmask: `255.255.255.0`
- TCP listener port: `8080`

Test server:

```powershell
python tcp_server.py
```

Expected MCU behavior:
- `connected to 192.168.1.100:8080`
- `Sent N bytes via ETH`

Expected PC behavior:
- `ping 192.168.1.30` succeeds
- `tcp_server.py` receives JSON payloads from `192.168.1.30`

## Key Files

- `drivers/drv_eth.c`
- `drivers/board.c`
- `applications/Apps/ethApp.c`
- `openspec/changes/add-eth-lan8720a-fiber/`

## Build

Typical RT-Thread / Studio build output is under `Debug/`.
