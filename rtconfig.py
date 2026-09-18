import os

# toolchains options
ARCH = 'arm'
CPU = 'cortex-m7'
CROSS_TOOL = 'gcc'

# cross_tool provides the cross compiler
# EXEC_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR
PLATFORM = 'gcc'
EXEC_PATH = r'D:\RT-ThreadStudio\repo\Extract\ToolChain_Support_Packages\ARM\GNU_Tools_for_ARM_Embedded_Processors\10.2.1\bin'

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

PREFIX = 'arm-none-eabi-'
CC = PREFIX + 'gcc'
AS = PREFIX + 'gcc'
AR = PREFIX + 'ar'
CXX = PREFIX + 'g++'
LINK = PREFIX + 'gcc'
TARGET_EXT = 'elf'
SIZE = PREFIX + 'size'
OBJDUMP = PREFIX + 'objdump'
OBJCPY = PREFIX + 'objcopy'

# Compiler flags for STM32H7 Cortex-M7
DEVICE = ' -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections'
CFLAGS = DEVICE + ' -Dgcc -Wall'
AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb '
LFLAGS = DEVICE + ' -lm -lgcc -lc -specs=nano.specs -specs=nosys.specs -nostartfiles -Wl,--gc-sections,-Map=rt-thread.map,-cref,-u,Reset_Handler -T linkscripts/STM32H743XIHx/link.lds'

CPATH = ''
LPATH = ''
CXXFLAGS = DEVICE + ' -Dgcc -Wall'

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'
