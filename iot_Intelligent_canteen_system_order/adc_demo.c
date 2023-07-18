/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "iot_adc.h"
#include "iot_errno.h"
#include "adc_demo.h"
#define NUM 1
#define STACK_SIZE 4096

#define IOT_GPIO_IDX_10 10 // for hispark_pegasus
#define IOT_GPIO_IDX_12 12 // for hispark_pegasus

float GetLight(void)
{
    unsigned int ret = 0;
    unsigned short data = 0;
    float value = 0.0;
    ret = AdcRead(IOT_ADC_CHANNEL_4, &data, IOT_ADC_EQU_MODEL_4, IOT_ADC_CUR_BAIS_DEFAULT, 0xf0);
    value = (float)data;
    return value;
}

float GetSomeMan(void)
{
    unsigned int ret = 0;
    unsigned short data = 0;
    float value = 0.0;
    ret = AdcRead(IOT_ADC_CHANNEL_3, &data, IOT_ADC_EQU_MODEL_2, IOT_ADC_CUR_BAIS_DEFAULT, 0xf0);
    value = (float)data;
    return value;
}

static void ADCLightTask(int *arg)
{
    (void)arg;
    unsigned int ret1 = 0;
    unsigned int ret2 = 0;
    unsigned short data1 = 0;
    unsigned short data2 = 0;
    float value1 = 0.0;
    unsigned int value2 = 0;

    while (NUM) {
        // 光敏电阻对应的是ADC channel 4，
        ret1 = AdcRead(IOT_ADC_CHANNEL_4, &data1, IOT_ADC_EQU_MODEL_4, IOT_ADC_CUR_BAIS_DEFAULT, 0xf0);
        if (ret1 != IOT_SUCCESS) {
            printf("ADC Read Fail \r\n");
            return;
        } else {
            /* vlt * 1.8 * 4 / 4096.0 为将码字转换为电压 */
            value1 = (float)data1 * (1.8) * 4 /4096;
            //printf("ADC Read value is %.3f \r\n", value1);
            if(data1 > 1800)
            IoTGpioSetOutputVal(IOT_GPIO_IDX_12, IOT_GPIO_VALUE1);
            else
            IoTGpioSetOutputVal(IOT_GPIO_IDX_12, IOT_GPIO_VALUE0);
        }
        TaskMsleep(100); /* 20:sleep 20ms */
        ret2 = AdcRead(IOT_ADC_CHANNEL_3, &data2, IOT_ADC_EQU_MODEL_2, IOT_ADC_CUR_BAIS_DEFAULT, 0x0f);
        if(ret2 != IOT_SUCCESS){
            printf("ADC RED Read Fail \r\n");
            return;
        }else{
            value2 = (unsigned int)data2;
            //printf("ADC RED Read value is %.d \r\n", data2);
            if(data2 > 1800)
            IoTGpioSetOutputVal(IOT_GPIO_IDX_10, IOT_GPIO_VALUE1);
            else
            IoTGpioSetOutputVal(IOT_GPIO_IDX_10, IOT_GPIO_VALUE0);
        }
        TaskMsleep(100); /* 20:sleep 20ms */
    }
}

static void ADCLightDemo(void)
{
    osThreadAttr_t attr;
    IoTGpioInit(IOT_GPIO_IDX_10);
    // 设置GPIO为输出方向
    IoTGpioSetDir(IOT_GPIO_IDX_10, IOT_GPIO_DIR_OUT);
    IoTGpioInit(IOT_GPIO_IDX_12);
    // 设置GPIO为输出方向
    IoTGpioSetDir(IOT_GPIO_IDX_12, IOT_GPIO_DIR_OUT);
    attr.name = "ADCLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(ADCLightTask, NULL, &attr) == NULL) {
        printf("[ADCLightDemo] Failed to create ADCLightTask!\n");
    }
}

APP_FEATURE_INIT(ADCLightDemo);
