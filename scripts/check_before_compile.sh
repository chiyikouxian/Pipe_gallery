#!/bin/bash
# 传感器调试项目编译前检查脚本
# 使用方法：bash scripts/check_before_compile.sh

echo "========================================"
echo "  传感器调试项目 - 编译前检查"
echo "========================================"
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查计数
PASS=0
FAIL=0
WARN=0

# 检查函数
check_pass() {
    echo -e "${GREEN}✓${NC} $1"
    ((PASS++))
}

check_fail() {
    echo -e "${RED}✗${NC} $1"
    ((FAIL++))
}

check_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
    ((WARN++))
}

echo "1. 检查项目结构..."
echo "-----------------------------------"

# 检查关键目录
if [ -d "applications" ]; then
    check_pass "applications/ 目录存在"
else
    check_fail "applications/ 目录不存在"
fi

if [ -d "applications/Apps" ]; then
    check_pass "applications/Apps/ 目录存在"
else
    check_fail "applications/Apps/ 目录不存在"
fi

if [ -d "drivers" ]; then
    check_pass "drivers/ 目录存在"
else
    check_fail "drivers/ 目录不存在"
fi

if [ -d "rt-thread" ]; then
    check_pass "rt-thread/ 目录存在"
else
    check_fail "rt-thread/ 目录不存在"
fi

echo ""
echo "2. 检查传感器应用文件..."
echo "-----------------------------------"

# 传感器应用文件列表
SENSOR_FILES=(
    "applications/Apps/linesensor.c"
    "applications/Apps/linesensor.h"
    "applications/Apps/sht30App.c"
    "applications/Apps/sht30App.h"
    "applications/Apps/MethaneSensorApp.c"
    "applications/Apps/MethaneSensorApp.h"
    "applications/Apps/o2SensorApp.c"
    "applications/Apps/o2SensorApp.h"
    "applications/Apps/displacementSensorApp.c"
    "applications/Apps/displacementSensorApp.h"
    "applications/Apps/stressSensorApp.c"
    "applications/Apps/stressSensorApp.h"
    "applications/Apps/freeModbusApp.c"
    "applications/Apps/freeModbusApp.h"
)

for file in "${SENSOR_FILES[@]}"; do
    if [ -f "$file" ]; then
        check_pass "$(basename $file) 存在"
    else
        check_fail "$(basename $file) 不存在"
    fi
done

echo ""
echo "3. 检查新增调试文件..."
echo "-----------------------------------"

# 新增文件列表
NEW_FILES=(
    "applications/Apps/debugCommands.c"
    "applications/Apps/debugCommands.h"
    "applications/Apps/sensor_debug_config.h"
    "docs/SENSOR_DEBUG_GUIDE.md"
    "docs/QUICK_REFERENCE.md"
    "docs/SUMMARY.md"
    "docs/README.md"
)

for file in "${NEW_FILES[@]}"; do
    if [ -f "$file" ]; then
        check_pass "$(basename $file) 已创建"
    else
        check_warn "$(basename $file) 未创建（可选）"
    fi
done

echo ""
echo "4. 检查主文件..."
echo "-----------------------------------"

if [ -f "applications/main.c" ]; then
    check_pass "main.c 存在"

    # 检查是否包含stressSensorApp.h
    if grep -q "stressSensorApp.h" applications/main.c; then
        check_pass "main.c 已包含 stressSensorApp.h"
    else
        check_warn "main.c 未包含 stressSensorApp.h（应力传感器未集成）"
    fi
else
    check_fail "main.c 不存在"
fi

echo ""
echo "5. 检查构建文件..."
echo "-----------------------------------"

if [ -f "SConstruct" ]; then
    check_pass "SConstruct 存在"
else
    check_fail "SConstruct 不存在"
fi

if [ -f "rtconfig.h" ]; then
    check_pass "rtconfig.h 存在"
else
    check_fail "rtconfig.h 不存在"
fi

if [ -f "rtconfig.py" ]; then
    check_pass "rtconfig.py 存在"
else
    check_fail "rtconfig.py 不存在"
fi

echo ""
echo "6. 检查SCons..."
echo "-----------------------------------"

if command -v scons &> /dev/null; then
    check_pass "SCons 已安装"
    scons --version 2>&1 | head -1
else
    check_fail "SCons 未安装"
fi

echo ""
echo "7. 检查Python..."
echo "-----------------------------------"

if command -v python &> /dev/null; then
    check_pass "Python 已安装"
    python --version 2>&1
else
    check_fail "Python 未安装"
fi

echo ""
echo "8. 检查ARM工具链..."
echo "-----------------------------------"

if command -v arm-none-eabi-gcc &> /dev/null; then
    check_pass "ARM GCC 工具链已安装"
    arm-none-eabi-gcc --version 2>&1 | head -1
else
    check_warn "ARM GCC 工具链未在PATH中（需要设置RTT_EXEC_PATH）"
fi

echo ""
echo "9. 检查原理图网表..."
echo "-----------------------------------"

if [ -f "Netlist_Schematic1_2026-09-18.tel" ]; then
    check_pass "最新原理图网表存在 (2026-09-18)"
else
    check_warn "原理图网表不存在或版本过旧"
fi

echo ""
echo "========================================"
echo "  检查结果汇总"
echo "========================================"
echo -e "${GREEN}通过: $PASS${NC}"
echo -e "${YELLOW}警告: $WARN${NC}"
echo -e "${RED}失败: $FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}✓ 所有必要检查通过，可以编译！${NC}"
    echo ""
    echo "下一步："
    echo "  1. 运行 'scons' 编译项目"
    echo "  2. 烧录 rt-thread.elf 到STM32"
    echo "  3. 连接串口终端（115200波特率）"
    echo "  4. 使用 'sensors' 命令查看所有传感器"
    echo ""
else
    echo -e "${RED}✗ 存在 $FAIL 个错误，请先修复！${NC}"
    echo ""
fi

echo "详细文档："
echo "  - docs/SENSOR_DEBUG_GUIDE.md    (完整调试指南)"
echo "  - docs/QUICK_REFERENCE.md       (快速参考)"
echo "  - docs/SUMMARY.md               (项目总结)"
echo ""
