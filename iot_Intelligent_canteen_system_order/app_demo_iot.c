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
#include <hi_task.h>
#include <string.h>
#include <unistd.h>
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "iot_watchdog.h"
#include "wifi_connecter.h"
#include "adc_demo.h"

/* report environment */
static int g_iState = 0;
float light;
float man;
static int light_blue = 0;
static int man_red = 0;

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
/* oc request id */
#define CN_COMMADN_INDEX                    "commands/request_id="

#define TASK_SLEEP_1000MS (1000)

/* report temperature */
void ReportLight(void)
{
    light = GetLight();
    printf("light value is %.f \r\n", light);
    IoTProfileService service;
    IoTProfileKV property;
    if(light > 1800)
        light_blue = 1;
    else
        light_blue = 0;

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Light";
    switch(light_blue) {
        case 0:
            property.value = "Light OFF";
            break;
        case 1:
            property.value = "Light ON";
            break;
        default:
            break;
    }
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "IOT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);
}

/* report humidity */
void ReportSomeMan(void)
{
    man = GetSomeMan();
    IoTProfileService service;
    IoTProfileKV property;

    if(man > 1800)
        man_red = 1;
    else
        man_red = 0;

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "SomeMan";
    switch(man_red) {
        case 1:
            property.value = "Person";
            break;
        case 0:
            property.value = "No Person";
            break;
        default:
            break;
    }
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "IOT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);
}

///< this is the demo main task entry,here we will set the wifi/cjson/mqtt ready ,and
///< wait if any work to do in the while
static void *DemoEntry(const char *arg)
{
    hi_watchdog_disable();
    WifiStaReadyWait();
    CJsonInit();
    printf("cJsonInit init \r\n");
    IoTMain();

/* 主动上报 */
#ifdef TAKE_THE_INITIATIVE_TO_REPORT
    while (1) {
        // /< here you could add your own works here--we report the data to the IoTplatform
        hi_sleep(TASK_SLEEP_1000MS);
        // /< now we report the data to the iot platform
        ReportLight();
        ReportSomeMan();
        if (g_iState == 0xffff) {
            g_iState = 0;
            break;
        }
    }
#endif
}

///< This is the demo entry, we create a task here, and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE  0x1000
#define CN_IOT_TASK_PRIOR 28
#define CN_IOT_TASK_NAME "IOTDEMO"
static void AppDemoIot(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();

    attr.name = "IOTDEMO";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CN_IOT_TASK_STACKSIZE;
    attr.priority = CN_IOT_TASK_PRIOR;

    if (osThreadNew((osThreadFunc_t)DemoEntry, NULL, &attr) == NULL) {
        printf("[IOT] Falied to create IOTDEMO!\n");
    }
}

SYS_RUN(AppDemoIot);

#define IOT_GPIO_IDX_10 10 // for hispark_pegasus
#define IOT_GPIO_IDX_11 11 // for hispark_pegasus
#define IOT_GPIO_IDX_12 12 // for hispark_pegasus

static void *DeviceCntrol(const char *arg)
{
    printf("LedTask start\r\n");
    // 配置GPIO引脚号和输出值
    IoTGpioSetOutputVal(IOT_GPIO_IDX_10,1);
    IoTGpioSetOutputVal(IOT_GPIO_IDX_11,1);
    IoTGpioSetOutputVal(IOT_GPIO_IDX_12,1);
    return NULL;
}

static void LedControlTask(void)
{
    osThreadAttr_t attr;
    // 初始化GPIO
    IoTGpioInit(IOT_GPIO_IDX_10);
    // 设置GPIO为输出方向
    IoTGpioSetDir(IOT_GPIO_IDX_10, IOT_GPIO_DIR_OUT);
    // 初始化GPIO
    IoTGpioInit(IOT_GPIO_IDX_11);
    // 设置GPIO为输出方向
    IoTGpioSetDir(IOT_GPIO_IDX_11, IOT_GPIO_DIR_OUT);
    // 初始化GPIO
    IoTGpioInit(IOT_GPIO_IDX_12);
    // 设置GPIO为输出方向
    IoTGpioSetDir(IOT_GPIO_IDX_12, IOT_GPIO_DIR_OUT);

    attr.name = "LedCntrolDemo";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024; /* 堆栈大小为1024 */
    attr.priority = osPriorityNormal;
    // 报错
    if (osThreadNew((osThreadFunc_t)DeviceCntrol, NULL, &attr) == NULL) {
        printf("[LedExample] Failed to create LedTask!\n");
    }
}

SYS_RUN(LedControlTask);