/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_watchdog.h"
#include "app_demo_multi_sample.h"

#define ENVIRONMENT_TASK_STACK  (1024 *4)
#define INTERRUPT_TASK_PRIO (26)

void KeyInterruptScan(const char *arg)
{
    (void)arg;
    AppMultiSampleDemo();
}

void EnvironmentDemo(const char *arg)
{
    printf("environmental monitoring open ok\r\n");
    EnvironmentFunc();
}

static void InterruptTask(void)
{
    osThreadAttr_t attr;

    attr.name = "Interrupt";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ENVIRONMENT_TASK_STACK;
    attr.priority = INTERRUPT_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)KeyInterruptScan, NULL, &attr) == NULL) {
        printf("[TrafficLight] Falied to create TrafficLightDemo!\n");
    }
}

SYS_RUN(InterruptTask);

static void StartTask(void)
{
    osThreadAttr_t attr = {0};
    IoTWatchDogDisable();

    attr.name = "EnvironmentDemo";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ENVIRONMENT_TASK_STACK;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)EnvironmentDemo, NULL, &attr) == NULL) {
        printf("[EnvironmentDemo] Falied to create EnvironmentDemo!\n");
    }
}

SYS_RUN(StartTask);
