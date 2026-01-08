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
        /* Peripheral clock enable */
        __HAL_RCC_ADC12_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_SYSCFG_CLK_ENABLE();

        /**ADC1 GPIO Configuration
        根据参考代码配置：
        PA0_C  ------> ADC1_INP0 (通道0) - 需要打开模拟开关
        PA1_C  ------> ADC1_INP1 (通道1) - 需要打开模拟开关
        PA6    ------> ADC1_INP6 (通道3)
        PC4    ------> ADC1_INP4 (通道4)
        PB1    ------> ADC1_INP5 (通道5)
        */
        /* 配置GPIO为模拟输入模式 */
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  /* 模拟输入不需要高速 */
        
        /* 配置通道0引脚PA0 */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        /* 打开PA0的模拟开关，使PA0连接到PA0_C（ADC输入） */
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);
        
        /* 配置通道1引脚PA1 */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        /* 打开PA1的模拟开关，使PA1连接到PA1_C（ADC输入） */
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_OPEN);
        
        /* 配置通道3引脚PA6 */
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        /* 配置通道4引脚PC4 */
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        
        /* 配置通道5引脚PB1 */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        /* 确保模拟开关配置生效，使用简单的循环延时（此时RT-Thread线程系统可能还未初始化） */
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
