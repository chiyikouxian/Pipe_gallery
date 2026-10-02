/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-02     Administrator       the first version
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include "allInFile.h"
#include "rtthread.h" // RT-Thread RTOS头文件
#include <rthw.h>  // RT-Thread Hardware-related Header
#include <rtdevice.h>  // RT-Thread device-related headers

// 字符串操作示例
void string_operations(void) {
    printf("==== 字符串操作 ====\n");

    // 字符串拼接
    char str1[50] = "Hello, ";
    char str2[] = "World!";
    strcat(str1, str2);
    printf("字符串拼接: %s\n", str1);

    // 字符串比较
    if (strcmp(str1, "Hello, World!") == 0) {
        printf("字符串相同\n");
    } else {
        printf("字符串不同\n");
    }

    // 字符串长度
    printf("字符串长度: %zu\n", strlen(str1));
}

// 内存管理操作示例
void memory_management_operations(void) {
    printf("\n==== 内存管理操作 ====\n");

    // 动态分配内存
    int *arr = (int *)malloc(5 * sizeof(int));
    if (arr == NULL) {
        printf("内存分配失败\n");
        return;
    }

    // 初始化并打印数组
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;
    }
    printf("动态分配的数组内容: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // 释放内存
    free(arr);
}

// 数学计算操作示例
void math_operations(void) {
    printf("\n==== 数学计算操作 ====\n");

    double num1 = 25.0;
    double num2 = 4.0;

    // 平方根
    printf("平方根(%.2f): %.2f\n", num1, sqrt(num1));

    // 除法
    printf("除法(%.2f / %.2f): %.2f\n", num1, num2, num1 / num2);

    // 取余数
    printf("取余数(%.2f %% %.2f): %.2f\n", num1, num2, fmod(num1, num2));
}

// 文件操作示例
void file_operations(void) {
    printf("\n==== 文件操作 ====\n");

    FILE *file = fopen("test_file.txt", "w");
    if (file == NULL) {
        printf("文件打开失败\n");
        return;
    }

    // 写入文件
    fprintf(file, "This is a test file.\n");
    fprintf(file, "This is a second line.\n");
    fclose(file);

    // 读取文件
    file = fopen("test_file.txt", "r");
    if (file == NULL) {
        printf("文件打开失败\n");
        return;
    }

    char buffer[100];
    printf("文件内容:\n");
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }
    fclose(file);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 字符串操作示例
void string_operation(void) {
    printf("==== 字符串操作 ====\n");

    // 字符串拼接
    char str1[50] = "Hello, ";
    char str2[] = "World!";
    strcat(str1, str2);
    printf("字符串拼接: %s\n", str1);

    // 字符串比较
    if (strcmp(str1, "Hello, World!") == 0) {
        printf("字符串相同\n");
    } else {
        printf("字符串不同\n");
    }

    // 字符串长度
    printf("字符串长度: %zu\n", strlen(str1));
}

// 内存管理操作示例
void memory_management_operation(void) {
    printf("\n==== 内存管理操作 ====\n");

    // 动态分配内存
    int *arr = (int *)malloc(5 * sizeof(int));
    if (arr == NULL) {
        printf("内存分配失败\n");
        return;
    }

    // 初始化并打印数组
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 2;
    }
    printf("动态分配的数组内容: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // 释放内存
    free(arr);
}

// 数学计算操作示例
void math_operation(void) {
    printf("\n==== 数学计算操作 ====\n");

    double num1 = 25.0;
    double num2 = 4.0;

    // 平方根
    printf("平方根(%.2f): %.2f\n", num1, sqrt(num1));

    // 除法
    printf("除法(%.2f / %.2f): %.2f\n", num1, num2, num1 / num2);

    // 取余数
    printf("取余数(%.2f %% %.2f): %.2f\n", num1, num2, fmod(num1, num2));
}

// 文件操作示例
void file_operation(void) {
    printf("\n==== 文件操作 ====\n");

    FILE *file = fopen("test_file.txt", "w");
    if (file == NULL) {
        printf("文件打开失败\n");
        return;
    }

    // 写入文件
    fprintf(file, "This is a test file.\n");
    fprintf(file, "This is a second line.\n");
    fclose(file);

    // 读取文件
    file = fopen("test_file.txt", "r");
    if (file == NULL) {
        printf("文件打开失败\n");
        return;
    }

    char buffer[100];
    printf("文件内容:\n");
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }
    fclose(file);
}

// 用于记录CPU时间片
static rt_tick_t last_tick = 0;

// 监测 CPU 使用率
void monitor_cpu_usage(void) {
    rt_tick_t current_tick;
    unsigned long cpu_usage;

    current_tick = rt_tick_get();  // 获取当前系统 tick 时间
    cpu_usage = current_tick - last_tick;  // 计算本次时间片使用的 tick 数
    last_tick = current_tick;  // 更新为当前 tick

    printf("==== CPU 使用率监测 ====\n");
    printf("当前CPU使用时间: %lu ticks\n", cpu_usage);
    // 在实际应用中，可能需要更精确的方式来计算 CPU 时间片比例
}

rt_size_t rt_memory_pool_get_total_size()
{
    rt_size_t total_memory = 0;
    return total_memory;
}

rt_size_t rt_memory_pool_get_free_size()
{
    rt_size_t free_memory = 0;
    return free_memory;
}

rt_size_t rt_memory_pool_get_one_size()
{
    rt_size_t one_memory = 0;
    return one_memory;
}

rt_size_t rt_memory_pool_get_many_size()
{
    rt_size_t many_memory = 0;
    return many_memory;
}

// 监测内存使用情况
void monitor_memory_usage(void) {
    rt_size_t total_memory = rt_memory_pool_get_total_size();   // 获取总内存
    rt_size_t free_memory  = rt_memory_pool_get_free_size();    // 获取剩余内存
    rt_size_t used_memory  = total_memory - free_memory;        // 已用内存

    printf("\n==== 内存使用情况监测 ====\n");
    printf("总内存: %lu bytes\n", total_memory);
    printf("可用内存: %lu bytes\n", free_memory);
    printf("已用内存: %lu bytes\n", used_memory);
}

// 监测文件操作
void monitor_file_operations(void) {
    FILE *file;
    char buffer[100];

    // 打开文件并写入内容
    file = fopen("test_file.txt", "w");
    if (file == NULL) {
        printf("文件打开失败\n");
        return;
    }

    fprintf(file, "这是一个测试文件\n");
    fprintf(file, "用于单片机监测\n");
    fclose(file);

    // 读取文件内容并监测
    file = fopen("test_file.txt", "r");
    if (file == NULL) {
        printf("文件打开失败\n");
        return;
    }

    printf("文件内容:\n");
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }
    fclose(file);
}

// 定时任务：每隔一段时间调用一次监测函数
void monitor_task(void* parameter) {
    monitor_cpu_usage();
    monitor_memory_usage();
    monitor_file_operations();
}


