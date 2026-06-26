/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-13     RealThread   first version
 */

#include <rtthread.h>
#include <board.h>
#include <drv_common.h>

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hadc->Instance==ADC1)
    {
        __HAL_RCC_ADC12_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_SYSCFG_CLK_ENABLE();

        /**ADC1 GPIO Configuration (ETH-safe mapping):
        PA0_C  ------> ADC1_INP0 (A1 flame)
        PA3    ------> ADC1_INP15 (A2 flame, moved from PA1)
        PA6    ------> ADC1_INP3 (A3 flame)
        PA5    ------> ADC1_INP19 (A4 flame, moved from PC4)
        PB1    ------> ADC1_INP5 (A5 flame)
        PC0    ------> ADC1_INP10 (O2 sensor, moved from PC5)
        */
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

        /* A1: PA0 (INP0) */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);

        /* A2: PA3 (INP15) */
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* A3: PA6 (INP3) */
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* A4: PA5 (INP19) */
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* A5: PB1 (INP5) */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* O2: PC0 (INP10) */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        for (volatile int i = 0; i < 10000; i++);
    }
}


RT_WEAK void rt_hw_board_init()
{
    extern void hw_board_init(char *clock_src, int32_t clock_src_freq, int32_t clock_target_freq);

    /* Heap initialization */
#if defined(RT_USING_HEAP)
    rt_system_heap_init((void *) HEAP_BEGIN, (void *) HEAP_END);
#endif

    hw_board_init(BSP_CLOCK_SOURCE, BSP_CLOCK_SOURCE_FREQ_MHZ, BSP_CLOCK_SYSTEM_FREQ_MHZ);

    /* Set the shell console output device */
#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

}

#ifdef BSP_USING_ETH
/**
 * @brief ETH GPIO Configuration (RMII mode for LAN8720A)
 *   PA1  -> ETH_REF_CLK
 *   PA2  -> ETH_MDIO (UART2 moved to PD5/PD6)
 *   PA7  -> ETH_CRS_DV
 *   PC1  -> ETH_MDC
 *   PC4  -> ETH_RXD0 (ADC A4 moved to PA5)
 *   PC5  -> ETH_RXD1 (O2 moved to PC0)
 *   PG11 -> ETH_TX_EN
 *   PG13 -> ETH_TXD0
 *   PG14 -> ETH_TXD1
 */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* Close PA1 analog switch so digital AF11 works for ETH_REF_CLK */
    HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);

    /* PA1: ETH_REF_CLK, PA2: ETH_MDIO, PA7: ETH_CRS_DV */
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PC1: ETH_MDC, PC4: ETH_RXD0, PC5: ETH_RXD1 */
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PG11: ETH_TX_EN, PG13: ETH_TXD0, PG14: ETH_TXD1 */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* Enable ETH global interrupt so Rx complete callbacks can run. */
    HAL_NVIC_SetPriority(ETH_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
}
#endif /* BSP_USING_ETH */
