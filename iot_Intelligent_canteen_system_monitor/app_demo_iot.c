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
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "app_demo_multi_sample.h"
#include "app_demo_environment.h"
#include "app_demo_config.h"
#include "wifi_connecter.h"

/* report environment */
static int g_iState = 0;
float temp;
float humi;
float gas;
int gas_alarm = 0;
int refri = 0;
int humi_flag = 0;

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
/* oc request id */
#define CN_COMMADN_INDEX                    "commands/request_id="

#define TASK_SLEEP_1000MS (1000)

/* report temperature */
void ReportTemperature(void)
{
    temp = GetTemperatureValue();
    IoTProfileService service;
    IoTProfileKV property;
    char str1[50];
    char str2[50] = "℃";
    sprintf(str1, "%.1f", temp);
    strcat(str1, str2);

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Temperature";
    property.value = str1;
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "IOT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);

    if(temp < 5)
        refri = 1;
    else if(temp > 32)
        refri = 2;
    else
        refri = 0;

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Refrigeration";
    switch(refri) {
        case 0:
            property.value = "OFF";
            break;
        case 1:
            property.value = "HEAT";
            break;
        case 2:
            property.value = "COOLING";
        default:
            break;
    }
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "IOT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);
}

/* report humidity */
void ReportHumidity(void)
{
    humi = GetHumidityValue();
    IoTProfileService service;
    IoTProfileKV property;
    char str1[50];
    char str2[50] = "%";
    sprintf(str1, "%.1f", humi);
    strcat(str1, str2);

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Humidity";
    property.value = str1;
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "IOT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);

    humi_flag = (humi > 50);

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Dehumidifier";
    switch(humi_flag) {
        case 0:
            property.value = "OFF";
            break;
        case 1:
            property.value = "ON";
            break;
        default:
            break;
    }
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "IOT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);
}

/* report combustible gas */
void ReportGasValue(void)
{
    gas = GetCombustibleGasValue();
    IoTProfileService service;
    IoTProfileKV property;
    char str1[50];
    sprintf(str1, "%.2f", gas);

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Combustible_Gas";
    property.value = str1;
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "IOT";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_DEVICE_ID, &service);

    gas_alarm = (gas > 0);

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_STRING;
    property.key = "Combustible_Gas_Alarm";
    switch(gas_alarm) {
        case 0:
            property.value = "OFF";
            break;
        case 1:
            property.value = "ON";
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
        ReportTemperature();
        ReportHumidity();
        ReportGasValue();
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