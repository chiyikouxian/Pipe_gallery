/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-17     ideapad15s       the first version
 */
#include "heads.h"
#include <drivers/serial.h>
#include "MethaneSensorApp.h"

#define UART2_DEVICE_NAME    "uart2" /* UART2_TX---PD5   UART2_RX---PD6 (NOT PA2/PA3!) */
#define PACKET_SIZE          19      /* fixed 19-byte packet */

/* IMPORTANT: Hardware connection for GM-402B methane sensor:
 * - Sensor TX  -> MCU PD6 (UART2_RX)
 * - Sensor RX  -> MCU PD5 (UART2_TX)
 * - Sensor GND -> MCU GND
 * - Sensor VIN -> 3.3V or 5V (check sensor spec)
 *
 * NOTE: PA2 and PA3 are NOT used for UART2!
 *       PA2 is used by ETH_MDIO (Ethernet)
 *       PA3 is used by ADC1_INP15 (Flame sensor A2)
 */

/* global methane sensor data, for cloud report */
rt_uint16_t g_methane_ppm = 0;
rt_uint8_t  g_methane_lel = 0;
rt_uint8_t  g_methane_status = 0;
rt_bool_t   g_methane_data_valid = RT_FALSE;

static rt_uint32_t methane_valid_frames = 0;
static rt_uint32_t methane_invalid_frames = 0;
static volatile rt_bool_t methane_debug_active = RT_FALSE;

/* receive packet buffer */
static rt_uint8_t uart2_received_data[PACKET_SIZE];

/**
 * @brief Validate and parse one complete methane sensor frame
 *
 * @param buffer_in input packet buffer, fixed 19 bytes
 */
static rt_bool_t Response_FrameCheck_Uart(const rt_uint8_t *buffer_in)
{
    rt_uint8_t checksum = 0;
    rt_uint16_t methane = 0;
    rt_uint8_t lel = 0;

    if (buffer_in[0] != 0xAC || buffer_in[1] != 0xAC ||
        buffer_in[2] != PACKET_SIZE || buffer_in[3] != 0xAA)
    {
        methane_invalid_frames++;
        return RT_FALSE;
    }

    /* checksum verification */
    for (rt_uint8_t i = 0; i < PACKET_SIZE - 1; i++)
    {
        checksum += buffer_in[i];
    }

    if (checksum != buffer_in[PACKET_SIZE - 1])
    {
        methane_invalid_frames++;
        return RT_FALSE;
    }

    /* Manufacturer reference implementation (Ver1.1):
     * [10] = PPM low byte, [11] = PPM high byte (little-endian)
     * [12] = LEL %, [13] = device/fault status.
     * Bytes [8] and [9] are not the PPM field. */
    methane = ((rt_uint16_t)buffer_in[11] << 8) | (rt_uint16_t)buffer_in[10];
    lel = buffer_in[12];
    g_methane_ppm = methane;
    g_methane_lel = lel;
    g_methane_status = buffer_in[13];
    g_methane_data_valid = RT_TRUE;
    methane_valid_frames++;
    return RT_TRUE;
}

/**
 * @brief Receive UART2 data and print in hexadecimal format
 *
 * @param timeout receive timeout in ms, -1 means wait forever
 * @return rt_err_t RT_EOK on success, negative value on failure
 */
rt_err_t uart2_receive_and_print(rt_int32_t timeout)
{
    rt_device_t serial;
    rt_uint8_t data;
    rt_size_t result;
    rt_uint8_t data_index = 0; /* number of bytes collected in current frame */

    /* find UART device */
    serial = rt_device_find(UART2_DEVICE_NAME);
    if (serial == RT_NULL)
    {
        /* rt_kprintf("Cannot find %s device!\n", UART2_DEVICE_NAME); */
        return -RT_ERROR;
    }

    /* open device in read/write mode */
    if (rt_device_open(serial, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        /* rt_kprintf("Failed to open %s device!\n", UART2_DEVICE_NAME); */
        return -RT_ERROR;
    }

    /* configure UART baud rate to 19200 */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_19200;
    if (rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config) != RT_EOK)
    {
        /* rt_kprintf("Failed to configure UART2 baud rate!\n"); */
        rt_device_close(serial);
        return -RT_ERROR;
    }

    /* rt_kprintf("UART2 receive started (19200 baud), waiting for data...\n"); */
    /* rt_kprintf("Received HEX data:\n"); */

    /* receive loop */
    while (1)
    {
        if (methane_debug_active)
        {
            data_index = 0;
            rt_thread_mdelay(10);
            continue;
        }

        /* read one byte from UART */
        result = rt_device_read(serial, 0, &data, 1);

        if (result > 0)
        {
            /* Synchronize on AC AC, then collect exactly one 19-byte frame. */
            if (data_index == 0)
            {
                if (data == 0xAC)
                {
                    uart2_received_data[0] = data;
                    data_index = 1;
                }
            }
            else if (data_index == 1)
            {
                if (data == 0xAC)
                {
                    uart2_received_data[1] = data;
                    data_index = 2;
                }
                else
                {
                    data_index = 0;
                }
            }
            else
            {
                uart2_received_data[data_index++] = data;
                if (data_index == PACKET_SIZE)
                {
                    (void)Response_FrameCheck_Uart(uart2_received_data);
                    data_index = 0;
                }
            }
        }
        else if (timeout > 0)
        {
            /* timeout mode: wait a little and decrease timeout */
            rt_thread_mdelay(10);
            timeout -= 10;
            if (timeout <= 0)
            {
                /* rt_kprintf("\nUART2 receive timeout!\n"); */
                break;
            }
        }
        else if (timeout == 0)
        {
            /* timeout is zero, exit directly */
            break;
        }
        else
        {
            /* timeout < 0 means wait forever */
            rt_thread_mdelay(10);
        }
    }

    /* close device */
    rt_device_close(serial);

    return RT_EOK;
}

/**
 * @brief MSH command: debug UART2 raw data reception
 *
 * Usage: uart2_debug [seconds]
 * Example: uart2_debug 10  (monitor for 10 seconds)
 */
static void uart2_debug(int argc, char **argv)
{
    rt_device_t serial;
    rt_uint8_t data;
    rt_size_t result;
    rt_uint32_t byte_count = 0;
    rt_uint32_t debug_valid_before;
    rt_uint32_t debug_invalid_before;
    rt_uint32_t timeout_ms = 5000;  /* default 5 seconds */
    rt_tick_t start_tick;
    rt_uint8_t frame[PACKET_SIZE];
    rt_uint8_t frame_index = 0;

    if (argc == 2)
    {
        int seconds = atoi(argv[1]);
        if (seconds > 0 && seconds <= 60)
        {
            timeout_ms = seconds * 1000;
        }
    }

    methane_debug_active = RT_TRUE;
    rt_thread_mdelay(20);

    rt_kprintf("\n========================================\n");
    rt_kprintf("  UART2 Raw Data Debug Monitor\n");
    rt_kprintf("========================================\n");
    rt_kprintf("Device: %s\n", UART2_DEVICE_NAME);
    rt_kprintf("Pins: PD5 (TX), PD6 (RX)\n");
    rt_kprintf("Baud: 19200\n");
    rt_kprintf("Duration: %d seconds\n", timeout_ms / 1000);
    rt_kprintf("Expected frame: AC AC 13 AA ...(19 bytes)\n");
    rt_kprintf("========================================\n\n");

    /* find UART device */
    serial = rt_device_find(UART2_DEVICE_NAME);
    if (serial == RT_NULL)
    {
        rt_kprintf("ERROR: Cannot find %s device!\n", UART2_DEVICE_NAME);
        methane_debug_active = RT_FALSE;
        return;
    }

    /* open device */
    if (rt_device_open(serial, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("ERROR: Failed to open %s device!\n", UART2_DEVICE_NAME);
        methane_debug_active = RT_FALSE;
        return;
    }

    /* configure UART baud rate to 19200 for GM-402B methane sensor */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_19200;
    if (rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config) != RT_EOK)
    {
        rt_kprintf("ERROR: Failed to configure UART2 baud rate!\n");
        rt_device_close(serial);
        methane_debug_active = RT_FALSE;
        return;
    }

    rt_kprintf("Receiving data (HEX format):\n");
    /* Temporarily stop the background parser.  Both readers consume the same
     * RT-Thread UART RX queue; allowing them to run together loses bytes and
     * makes a valid stream appear to have a non-19-byte length. */
    debug_valid_before = methane_valid_frames;
    debug_invalid_before = methane_invalid_frames;
    start_tick = rt_tick_get();

    /* receive loop with timeout */
    while (1)
    {
        /* check timeout */
        if ((rt_tick_get() - start_tick) >= rt_tick_from_millisecond(timeout_ms))
        {
            break;
        }

        /* read one byte from UART */
        result = rt_device_read(serial, 0, &data, 1);

        if (result > 0)
        {
            /* print byte in hex */
            rt_kprintf("%02X ", data);
            byte_count++;

            /* Independently detect and validate complete frames in the raw
             * stream, rather than judging validity by total byte count. */
            if (frame_index == 0)
            {
                if (data == 0xAC)
                {
                    frame[0] = data;
                    frame_index = 1;
                }
            }
            else if (frame_index == 1)
            {
                if (data == 0xAC)
                {
                    frame[1] = data;
                    frame_index = 2;
                }
                else
                {
                    frame_index = 0;
                }
            }
            else
            {
                frame[frame_index++] = data;
                if (frame_index == PACKET_SIZE)
                {
                    (void)Response_FrameCheck_Uart(frame);
                    frame_index = 0;
                    rt_kprintf("\n");
                }
            }

            if ((byte_count % PACKET_SIZE == 0) && (frame_index != 0))
            {
                rt_kprintf("\n");
            }
        }
        else
        {
            /* no data, wait a bit */
            rt_thread_mdelay(10);
        }
    }

    rt_device_close(serial);
    methane_debug_active = RT_FALSE;

    rt_kprintf("\n\n========================================\n");
    rt_kprintf("Total bytes received: %d\n", byte_count);
    if (byte_count == 0)
    {
        rt_kprintf("\nWARNING: No data received!\n");
        rt_kprintf("Possible causes:\n");
        rt_kprintf("  1. Sensor not powered (check VIN and GND)\n");
        rt_kprintf("  2. Wrong pin connection - should be:\n");
        rt_kprintf("     Sensor TX -> MCU PD6 (UART2_RX)\n");
        rt_kprintf("     Sensor RX -> MCU PD5 (UART2_TX)\n");
        rt_kprintf("  3. Sensor requires warm-up time (wait 2-3 minutes)\n");
        rt_kprintf("  4. Check if sensor is actually GM-402B module\n");
    }
    else
    {
        rt_uint32_t valid_count = methane_valid_frames - debug_valid_before;

        rt_kprintf("\nValid frames: %u\n", (unsigned int)valid_count);
        rt_kprintf("Invalid frames: %u\n",
                   (unsigned int)(methane_invalid_frames - debug_invalid_before));
        if (frame_index != 0)
            rt_kprintf("Incomplete trailing bytes: %d\n", frame_index);
        if (valid_count == 0)
            rt_kprintf("WARNING: Data received, but no valid GM-402B frame found.\n");
        else
            rt_kprintf("Last value: %u ppm, LEL %u%%, status 0x%02X\n",
                       g_methane_ppm, g_methane_lel, g_methane_status);
    }
    rt_kprintf("========================================\n\n");
}
MSH_CMD_EXPORT_ALIAS(uart2_debug, uart2_debug, Debug UART2 raw data (uart2_debug [seconds]));
