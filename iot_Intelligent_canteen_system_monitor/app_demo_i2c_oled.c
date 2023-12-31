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

#include <unistd.h>
#include <hi_gpio.h>
#include <hi_io.h>
#include "ssd1306_oled.h"
#include "c081_nfc.h"
#include "iot_i2c.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "hi_time.h"
#include "app_demo_multi_sample.h"
#include "app_demo_environment.h"
#include "app_demo_config.h"

#define I2C_REG_ARRAY_LEN           (64)
#define OLED_SEND_BUFF_LEN          (28)
#define OLED_SEND_BUFF_LEN2         (25)
#define OLED_SEND_BUFF_LEN3         (27)
#define OLED_SEND_BUFF_LEN4         (29)
#define Max_Column                  (128)
#define OLED_DEMO_TASK_STAK_SIZE    (1024*2)
#define OLED_DEMO_TASK_PRIORITY     (25)
#define OLED_DISPLAY_INTERVAL_TIME  (1)
#define SEND_CMD_LEN                (2)
#define MAX_COLUM                  (128)

#define CHAR_SIZE   16
#define Y_PIXEL_POINT 16
#define X_PIXEL_POINT 8
#define X_REMAINING_PIXELS 6

#define X_PIXEL_POINT_POSITION_120  (120)
#define Y_LINES_PIXEL_2   (2)
#define X_COLUMNS_PIXEL_8 (8)

unsigned char  g_oled_demo_task_id = 0;
unsigned int g_mux_id = 0;
static unsigned char hi3861_board_led_test = 0;

/* 6*8的点阵 */
static const unsigned char f6X8[][6] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // sp
    { 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00 }, // !
    { 0x00, 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
    { 0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // #
    { 0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // $
    { 0x00, 0x62, 0x64, 0x08, 0x13, 0x23 }, // %
    { 0x00, 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
    { 0x00, 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
    { 0x00, 0x00, 0x1c, 0x22, 0x41, 0x00 }, // (
    { 0x00, 0x00, 0x41, 0x22, 0x1c, 0x00 }, // )
    { 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14 }, // *
    { 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
    { 0x00, 0x00, 0x00, 0xA0, 0x60, 0x00 }, // ,
    { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08 }, // -
    { 0x00, 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
    { 0x00, 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
    { 0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0
    { 0x00, 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 1
    { 0x00, 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
    { 0x00, 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 3
    { 0x00, 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 4
    { 0x00, 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
    { 0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 6
    { 0x00, 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
    { 0x00, 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
    { 0x00, 0x06, 0x49, 0x49, 0x29, 0x1E }, // 9
    { 0x00, 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
    { 0x00, 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;号
    { 0x00, 0x08, 0x14, 0x22, 0x41, 0x00 }, // <
    { 0x00, 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
    { 0x00, 0x00, 0x41, 0x22, 0x14, 0x08 }, // >
    { 0x00, 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
    { 0x00, 0x32, 0x49, 0x59, 0x51, 0x3E }, // @
    { 0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C }, // A
    { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x36 }, // B
    { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x22 }, // C
    { 0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C }, // D
    { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x41 }, // E
    { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01 }, // F
    { 0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A }, // G
    { 0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
    { 0x00, 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
    { 0x00, 0x20, 0x40, 0x41, 0x3F, 0x01 }, // J
    { 0x00, 0x7F, 0x08, 0x14, 0x22, 0x41 }, // K
    { 0x00, 0x7F, 0x40, 0x40, 0x40, 0x40 }, // L
    { 0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // M
    { 0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F }, // N
    { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
    { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x06 }, // P
    { 0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E }, // Q
    { 0x00, 0x7F, 0x09, 0x19, 0x29, 0x46 }, // R
    { 0x00, 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
    { 0x00, 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
    { 0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
    { 0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
    { 0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F }, // W
    { 0x00, 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
    { 0x00, 0x07, 0x08, 0x70, 0x08, 0x07 }, // Y
    { 0x00, 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
    { 0x00, 0x00, 0x7F, 0x41, 0x41, 0x00 }, // [
    { 0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55 }, // 55
    { 0x00, 0x00, 0x41, 0x41, 0x7F, 0x00 }, // ]
    { 0x00, 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
    { 0x00, 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
    { 0x00, 0x00, 0x01, 0x02, 0x04, 0x00 }, // '
    { 0x00, 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
    { 0x00, 0x7F, 0x48, 0x44, 0x44, 0x38 }, // b
    { 0x00, 0x38, 0x44, 0x44, 0x44, 0x20 }, // c
    { 0x00, 0x38, 0x44, 0x44, 0x48, 0x7F }, // d
    { 0x00, 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
    { 0x00, 0x08, 0x7E, 0x09, 0x01, 0x02 }, // f
    { 0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C }, // g
    { 0x00, 0x7F, 0x08, 0x04, 0x04, 0x78 }, // h
    { 0x00, 0x00, 0x44, 0x7D, 0x40, 0x00 }, // i
    { 0x00, 0x40, 0x80, 0x84, 0x7D, 0x00 }, // j
    { 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00 }, // k
    { 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00 }, // l
    { 0x00, 0x7C, 0x04, 0x18, 0x04, 0x78 }, // m
    { 0x00, 0x7C, 0x08, 0x04, 0x04, 0x78 }, // n
    { 0x00, 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
    { 0x00, 0xFC, 0x24, 0x24, 0x24, 0x18 }, // p
    { 0x00, 0x18, 0x24, 0x24, 0x18, 0xFC }, // q
    { 0x00, 0x7C, 0x08, 0x04, 0x04, 0x08 }, // r
    { 0x00, 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
    { 0x00, 0x04, 0x3F, 0x44, 0x40, 0x20 }, // t
    { 0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C }, // u
    { 0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
    { 0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
    { 0x00, 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
    { 0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C }, // y
    { 0x00, 0x44, 0x64, 0x54, 0x4C, 0x44 }, // z
    { 0x14, 0x14, 0x14, 0x14, 0x14, 0x14 }, // horiz lines
};
/* 8*16的点阵 */
static const unsigned char f8X16[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0
    0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x30, 0x00, 0x00, 0x00, // ! 1
    0x00, 0x10, 0x0C, 0x06, 0x10, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // " 2
    0x40, 0xC0, 0x78, 0x40, 0xC0, 0x78, 0x40, 0x00, 0x04, 0x3F, 0x04, 0x04, 0x3F, 0x04, 0x04, 0x00, // # 3
    0x00, 0x70, 0x88, 0xFC, 0x08, 0x30, 0x00, 0x00, 0x00, 0x18, 0x20, 0xFF, 0x21, 0x1E, 0x00, 0x00, // $ 4
    0xF0, 0x08, 0xF0, 0x00, 0xE0, 0x18, 0x00, 0x00, 0x00, 0x21, 0x1C, 0x03, 0x1E, 0x21, 0x1E, 0x00, // % 5
    0x00, 0xF0, 0x08, 0x88, 0x70, 0x00, 0x00, 0x00, 0x1E, 0x21, 0x23, 0x24, 0x19, 0x27, 0x21, 0x10, // & 6
    0x10, 0x16, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ' 7
    0x00, 0x00, 0x00, 0xE0, 0x18, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x07, 0x18, 0x20, 0x40, 0x00, // ( 8
    0x00, 0x02, 0x04, 0x18, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x18, 0x07, 0x00, 0x00, 0x00, // ) 9
    0x40, 0x40, 0x80, 0xF0, 0x80, 0x40, 0x40, 0x00, 0x02, 0x02, 0x01, 0x0F, 0x01, 0x02, 0x02, 0x00, // * 10
    0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x1F, 0x01, 0x01, 0x01, 0x00, // + 11
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xB0, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, // , 12
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, // - 13
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, // . 14
    0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x18, 0x04, 0x00, 0x60, 0x18, 0x06, 0x01, 0x00, 0x00, 0x00, // / 15
    0x00, 0xE0, 0x10, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x0F, 0x10, 0x20, 0x20, 0x10, 0x0F, 0x00, // 0 16
    0x00, 0x10, 0x10, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, // 1 17
    0x00, 0x70, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00, 0x30, 0x28, 0x24, 0x22, 0x21, 0x30, 0x00, // 2 18
    0x00, 0x30, 0x08, 0x88, 0x88, 0x48, 0x30, 0x00, 0x00, 0x18, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00, // 3 19
    0x00, 0x00, 0xC0, 0x20, 0x10, 0xF8, 0x00, 0x00, 0x00, 0x07, 0x04, 0x24, 0x24, 0x3F, 0x24, 0x00, // 4 20
    0x00, 0xF8, 0x08, 0x88, 0x88, 0x08, 0x08, 0x00, 0x00, 0x19, 0x21, 0x20, 0x20, 0x11, 0x0E, 0x00, // 5 21
    0x00, 0xE0, 0x10, 0x88, 0x88, 0x18, 0x00, 0x00, 0x00, 0x0F, 0x11, 0x20, 0x20, 0x11, 0x0E, 0x00, // 6 22
    0x00, 0x38, 0x08, 0x08, 0xC8, 0x38, 0x08, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, // 7 23
    0x00, 0x70, 0x88, 0x08, 0x08, 0x88, 0x70, 0x00, 0x00, 0x1C, 0x22, 0x21, 0x21, 0x22, 0x1C, 0x00, // 8 24
    0x00, 0xE0, 0x10, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x31, 0x22, 0x22, 0x11, 0x0F, 0x00, // 9 25
    0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, // : 26
    0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0x00, 0x00, 0x00, 0x00, // ;号 27
    0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, // < 28
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, // = 29
    0x00, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, // > 30
    0x00, 0x70, 0x48, 0x08, 0x08, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x30, 0x36, 0x01, 0x00, 0x00, // ? 31
    0xC0, 0x30, 0xC8, 0x28, 0xE8, 0x10, 0xE0, 0x00, 0x07, 0x18, 0x27, 0x24, 0x23, 0x14, 0x0B, 0x00, // @ 32
    0x00, 0x00, 0xC0, 0x38, 0xE0, 0x00, 0x00, 0x00, 0x20, 0x3C, 0x23, 0x02, 0x02, 0x27, 0x38, 0x20, // A 33
    0x08, 0xF8, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00, // B 34
    0xC0, 0x30, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x07, 0x18, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, // C 35
    0x08, 0xF8, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x10, 0x0F, 0x00, // D 36
    0x08, 0xF8, 0x88, 0x88, 0xE8, 0x08, 0x10, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x23, 0x20, 0x18, 0x00, // E 37
    0x08, 0xF8, 0x88, 0x88, 0xE8, 0x08, 0x10, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00, // F 38
    0xC0, 0x30, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x07, 0x18, 0x20, 0x20, 0x22, 0x1E, 0x02, 0x00, // G 39
    0x08, 0xF8, 0x08, 0x00, 0x00, 0x08, 0xF8, 0x08, 0x20, 0x3F, 0x21, 0x01, 0x01, 0x21, 0x3F, 0x20, // H 40
    0x00, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, // I 41
    0x00, 0x00, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x00, 0xC0, 0x80, 0x80, 0x80, 0x7F, 0x00, 0x00, 0x00, // J 42
    0x08, 0xF8, 0x88, 0xC0, 0x28, 0x18, 0x08, 0x00, 0x20, 0x3F, 0x20, 0x01, 0x26, 0x38, 0x20, 0x00, // K 43
    0x08, 0xF8, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x20, 0x30, 0x00, // L 44
    0x08, 0xF8, 0xF8, 0x00, 0xF8, 0xF8, 0x08, 0x00, 0x20, 0x3F, 0x00, 0x3F, 0x00, 0x3F, 0x20, 0x00, // M 45
    0x08, 0xF8, 0x30, 0xC0, 0x00, 0x08, 0xF8, 0x08, 0x20, 0x3F, 0x20, 0x00, 0x07, 0x18, 0x3F, 0x00, // N 46
    0xE0, 0x10, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x0F, 0x10, 0x20, 0x20, 0x20, 0x10, 0x0F, 0x00, // O 47
    0x08, 0xF8, 0x08, 0x08, 0x08, 0x08, 0xF0, 0x00, 0x20, 0x3F, 0x21, 0x01, 0x01, 0x01, 0x00, 0x00, // P 48
    0xE0, 0x10, 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x0F, 0x18, 0x24, 0x24, 0x38, 0x50, 0x4F, 0x00, // Q 49
    0x08, 0xF8, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x03, 0x0C, 0x30, 0x20, // R 50
    0x00, 0x70, 0x88, 0x08, 0x08, 0x08, 0x38, 0x00, 0x00, 0x38, 0x20, 0x21, 0x21, 0x22, 0x1C, 0x00, // S 51
    0x18, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x18, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x00, 0x00, // T 52
    0x08, 0xF8, 0x08, 0x00, 0x00, 0x08, 0xF8, 0x08, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00, // U 53
    0x08, 0x78, 0x88, 0x00, 0x00, 0xC8, 0x38, 0x08, 0x00, 0x00, 0x07, 0x38, 0x0E, 0x01, 0x00, 0x00, // V 54
    0xF8, 0x08, 0x00, 0xF8, 0x00, 0x08, 0xF8, 0x00, 0x03, 0x3C, 0x07, 0x00, 0x07, 0x3C, 0x03, 0x00, // W 55
    0x08, 0x18, 0x68, 0x80, 0x80, 0x68, 0x18, 0x08, 0x20, 0x30, 0x2C, 0x03, 0x03, 0x2C, 0x30, 0x20, // X 56
    0x08, 0x38, 0xC8, 0x00, 0xC8, 0x38, 0x08, 0x00, 0x00, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x00, 0x00, // Y 57
    0x10, 0x08, 0x08, 0x08, 0xC8, 0x38, 0x08, 0x00, 0x20, 0x38, 0x26, 0x21, 0x20, 0x20, 0x18, 0x00, // Z 58
    0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x40, 0x40, 0x40, 0x00, // [ 59
    0x00, 0x0C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x38, 0xC0, 0x00, // \ 60
    0x00, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x7F, 0x00, 0x00, 0x00, // ] 61
    0x00, 0x00, 0x04, 0x02, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ^ 62
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, // _ 63
    0x00, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ` 64
    0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x19, 0x24, 0x22, 0x22, 0x22, 0x3F, 0x20, // a 65
    0x08, 0xF8, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x11, 0x20, 0x20, 0x11, 0x0E, 0x00, // b 66
    0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x0E, 0x11, 0x20, 0x20, 0x20, 0x11, 0x00, // c 67
    0x00, 0x00, 0x00, 0x80, 0x80, 0x88, 0xF8, 0x00, 0x00, 0x0E, 0x11, 0x20, 0x20, 0x10, 0x3F, 0x20, // d 68
    0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x1F, 0x22, 0x22, 0x22, 0x22, 0x13, 0x00, // e 69
    0x00, 0x80, 0x80, 0xF0, 0x88, 0x88, 0x88, 0x18, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, // f 70
    0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x6B, 0x94, 0x94, 0x94, 0x93, 0x60, 0x00, // g 71
    0x08, 0xF8, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x20, 0x3F, 0x21, 0x00, 0x00, 0x20, 0x3F, 0x20, // h 72
    0x00, 0x80, 0x98, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, // i 73
    0x00, 0x00, 0x00, 0x80, 0x98, 0x98, 0x00, 0x00, 0x00, 0xC0, 0x80, 0x80, 0x80, 0x7F, 0x00, 0x00, // j 74
    0x08, 0xF8, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x20, 0x3F, 0x24, 0x02, 0x2D, 0x30, 0x20, 0x00, // k 75
    0x00, 0x08, 0x08, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, // l 76
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x20, 0x3F, 0x20, 0x00, 0x3F, 0x20, 0x00, 0x3F, // m 77
    0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x20, 0x3F, 0x21, 0x00, 0x00, 0x20, 0x3F, 0x20, // n 78
    0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00, // o 79
    0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xA1, 0x20, 0x20, 0x11, 0x0E, 0x00, // p 80
    0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x0E, 0x11, 0x20, 0x20, 0xA0, 0xFF, 0x80, // q 81
    0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x20, 0x20, 0x3F, 0x21, 0x20, 0x00, 0x01, 0x00, // r 82
    0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x33, 0x24, 0x24, 0x24, 0x24, 0x19, 0x00, // s 83
    0x00, 0x80, 0x80, 0xE0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x00, 0x00, // t 84
    0x80, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x20, 0x10, 0x3F, 0x20, // u 85
    0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x01, 0x0E, 0x30, 0x08, 0x06, 0x01, 0x00, // v 86
    0x80, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0x80, 0x0F, 0x30, 0x0C, 0x03, 0x0C, 0x30, 0x0F, 0x00, // w 87
    0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x20, 0x31, 0x2E, 0x0E, 0x31, 0x20, 0x00, // x 88
    0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x81, 0x8E, 0x70, 0x18, 0x06, 0x01, 0x00, // y 89
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x21, 0x30, 0x2C, 0x22, 0x21, 0x30, 0x00, // z 90
    0x00, 0x00, 0x00, 0x00, 0x80, 0x7C, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x40, 0x40, // { 91
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, // | 92
    0x00, 0x02, 0x02, 0x7C, 0x80, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x3F, 0x00, 0x00, 0x00, 0x00, // } 93
    0x00, 0x06, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ~ 94
};

#define NDEF_DATA_LEN_MAX           (1024)
#define MAX_COLUM                  (128)

hi_u8 ndefFileCloudMusic[NDEF_DATA_LEN_MAX] = {
    //0x00, 0x28,
    //0xd4, 0x0f, 0x16, 0x61, 0x6e, 0x64, 0x72, 0x6f,
    //0x69, 0x64, 0x2e, 0x63, 0x6f, 0x6d, 0x3a, 0x70,
    //0x6b, 0x67, 0x63, 0x6f, 0x6d, 0x2e, 0x6e, 0x65,
    //0x74, 0x65, 0x61, 0x73, 0x65, 0x2e, 0x63, 0x6c,
    //0x6f, 0x75, 0x64, 0x6d, 0x75, 0x73, 0x69, 0x63,
    0x00,0x0F,                    
    0xD1,0x01,0x0B,0x55,
    0x01,0x68,0x75,0x61,
    0x77,0x65,0x69,0x2E,
    0x63,0x6F,0x6D,
};

// extern  unsigned char ndefFile[NDEF_DATA_LEN_MAX];
static  unsigned char  Hi3861BoardLedTest = 0;
unsigned char *ndfe;

/*
    *@bref   向ssd1306 屏幕寄存器写入命令
    *status 0：表示写入成功，否则失败
*/
static unsigned int I2cWriteByte(unsigned char regAddr, unsigned char cmd)
{
    unsigned int status = 0;
    IotI2cIdx id = IOT_I2C_IDX_0; // I2C 0
    unsigned char sendLen = 2;
    unsigned char userData = cmd;
    IotI2cData oledI2cCmd = { 0 };
    IotI2cData oledI2cWriteCmd = { 0 };

    unsigned char sendUserCmd [SEND_CMD_LEN] = {OLED_ADDRESS_WRITE_CMD, userData};
    unsigned char sendUserData [SEND_CMD_LEN] = {OLED_ADDRESS_WRITE_DATA, userData};

    /* 如果是写命令，发写命令地址0x00 */
    if (regAddr == OLED_ADDRESS_WRITE_CMD) {
        oledI2cWriteCmd.sendBuf = sendUserCmd;
        oledI2cWriteCmd.sendLen = sendLen;
        status = IoTI2cWrite(id, OLED_ADDRESS, oledI2cWriteCmd.sendBuf, oledI2cWriteCmd.sendLen);
        if (status != 0) {
            return status;
        }
    } else if (regAddr == OLED_ADDRESS_WRITE_DATA) { /* 如果是写数据，发写数据地址0x40 */
        oledI2cCmd.sendBuf = sendUserData;
        oledI2cCmd.sendLen = sendLen;
        status = IoTI2cWrite(id, OLED_ADDRESS, oledI2cCmd.sendBuf, oledI2cCmd.sendLen);
        if (status != 0) {
            return status;
        }
    }
    return 0;
}

/* 写命令操作 */
static unsigned int WriteCmd(unsigned char cmd)
{
    unsigned char status = 0;
    /* 写设备地址 */
    status = I2cWriteByte(OLED_ADDRESS_WRITE_CMD, cmd);
    if (status != 0) {
        return -1;
    }
    return 0;
}

/* 写数据操作 */
static unsigned int WriteData(unsigned char i2cData)
{
    unsigned char status = 0;
    /* 写设备地址 */
    status = I2cWriteByte(OLED_ADDRESS_WRITE_DATA, i2cData);
    if (status != 0) {
        return -1;
    }
}

static unsigned int SetOledAddress(void)
{
    unsigned int status = 0;

    status = WriteCmd(DISPLAY_OFF); // --display off
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(SET_LOW_COLUMN_ADDRESS); // ---set low column address
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(SET_HIGH_COLUMN_ADDRESS); // ---set high column address
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(SET_START_LINE_ADDRESS); // --set start line address
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(SET_PAGE_ADDRESS); // --set page address
    if (status != 0) {
        return -1;
    }
    return 0;
}

static unsigned int SetOledControlCmd(void)
{
    unsigned int status = 0;

    status = WriteCmd(CONTRACT_CONTROL); // contract control
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(FULL_SCREEN); // --128
    if (status != 0) {
        return -1;
    }
    status= WriteCmd(SET_SEGMENT_REMAP); // set segment remap
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(NORMAL); // --normal / reverse
    if (status != 0) {
        return -1;
    }
    return 0;
}

static unsigned int SetOledScanDisplayCmd(void)
{
    unsigned int status = 0;

    status =WriteCmd(SET_MULTIPLEX); // --set multiplex ratio(1 to 64)
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(DUTY); // --1/32 duty
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(SCAN_DIRECTION); // Com scan direction
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(DISPLAY_OFFSET); // -set display offset
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(DISPLAY_TYPE);
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(OSC_DIVISION); // set osc division
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(DIVISION);
    if (status != 0) {
        return -1;
    }
    return 0;
}

static unsigned int SetOledColorPreChargeCmd(void)
{
    unsigned int status = 0;

    status = WriteCmd(COLOR_MODE_OFF); // set area color mode off
    if (status != 0) {
        return -1;
    }
    status= WriteCmd(COLOR);
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(PRE_CHARGE_PERIOD); // Set Pre-Charge Period
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(PERIOD);
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(PIN_CONFIGUARTION); // set com pin configuartion
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(CONFIGUARTION);
    if (status != 0) {
        return -1;
    }
    return 0;
}

static unsigned int SetOledVcomhChargePumpCmd(void)
{
    unsigned int status = 0;

    status = WriteCmd(SET_VCOMH); // set Vcomh
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(VCOMH);
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(SET_CHARGE_PUMP_ENABLE); // set charge pump enable
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(PUMP_ENABLE);
    if (status != 0) {
        return -1;
    }
    status = WriteCmd(TURN_ON_OLED_PANEL); // --turn on oled panel
    if (status != 0) {
        return -1;
    }
    return 0;
}

/* ssd1306 oled 初始化 */
unsigned int OledInit(void)
{
    unsigned int status = 0;
    hi_udelay(DELAY_100_MS); // 100ms 这里的延时很重要

    status = SetOledAddress();
    if (status != 0) {
        return -1;
    }
    status = SetOledControlCmd();
    if (status != 0) {
        return -1;
    }
    status = SetOledScanDisplayCmd();
    if (status != 0) {
        return -1;
    }
    status = SetOledColorPreChargeCmd();
    if (status != 0) {
        return -1;
    }
    status = SetOledVcomhChargePumpCmd();
    if (status != 0) {
        return -1;
    }
    return 0;
}
/*
 * bref: set start position 设置起始点坐标
 * char x:write start from x axis
 * y:write start from y axis
 */
void OledSetPosition(unsigned char x, unsigned char y)
{
    WriteCmd(0xb0 + y); /* 0xb0 */
    WriteCmd(((x & 0xf0) >> 4) | 0x10); /* 设置起点坐标0x10，4 */
    WriteCmd(x & 0x0f); /* 0x0f */
}
/* 全屏填充 */
#define Y_PIXEL_POINT_MAX   (128)
#define X_PIXEL_8   (8)
void OledFillScreen(unsigned char fiiData)
{
    for (unsigned char m = 0; m < X_PIXEL_8; m++) { /* 从OLED 的第0行开始，填充屏幕 */
        WriteCmd(0xb0 + m); /* 0xb0 */
        WriteCmd(0x00); /* 0x00 */
        WriteCmd(0x10); /* 0x10 */
        for (unsigned char n = 0; n < Y_PIXEL_POINT_MAX; n++) { /* 从OLED的第0列个像素点开始填充屏幕 */
            WriteData(fiiData);
        }
    }
}
/*
 * bref: Clear from a location
 * fill_data: write data to screen register
 * line:write positon start from Y axis
 * pos :write positon start from x axis
 * len:write data len
 */
void OledPositionCleanScreen(unsigned char fillData, unsigned char line,
    unsigned char pos, unsigned char len)
{
    unsigned char m = line;
    WriteCmd(0xb0 + m); /* 0xb0 */
    WriteCmd(0x00); /* 0x00 */
    WriteCmd(0x10); /* 0x10 */

    for (unsigned char n = pos; n < len; n++) {
        WriteData(fillData);
    }
}
/*
 *  bref: 8*16 typeface
 *  x:write positon start from x axis
 *  y:write positon start from y axis
 *  chr:write data
 *  char_size:select typeface
 */
void OledShowChar(unsigned char x, unsigned char y, unsigned char chr, unsigned char charSize)
{
    unsigned char c = chr - ' '; // 得到偏移后的值
    unsigned char xPosition = x;
    unsigned char yPosition = y;

    if (xPosition > MAX_COLUM - 1) {
        xPosition = 0;
        yPosition = yPosition + Y_LINES_PIXEL_2; /* 每2行写 */
    }
    if (charSize == CHAR_SIZE) { /* 当要写入的字为两个字节 */
        OledSetPosition(xPosition, yPosition);
        for (int i = 0; i < X_PIXEL_POINT; i++) { /* 从OLED的第0列个像素点开始 */
            WriteData(f8X16[c * Y_PIXEL_POINT + i]); /* 16,8 */
        }
        OledSetPosition(xPosition, yPosition + 1);
        for (int j = 0; j < X_PIXEL_POINT; j++) { /* 从OLED的第0列个像素点开始 */
            WriteData(f8X16[c * Y_PIXEL_POINT + j + X_PIXEL_POINT]); /* 16,8 */
        }
    } else {
        OledSetPosition(xPosition, yPosition);
        for (int k = 0; k < X_REMAINING_PIXELS; k++) { /* 从OLED的第0列个像素点开始 */
            WriteData(f6X8[c][k]);
        }
    }
}

/*
 * bref: display string
 * x:write positon start from x axis
 * y:write positon start from y axis
 * chr:write data
 * char_size:select typeface
 */
void OledShowStr(unsigned char x, unsigned char y, unsigned char *chr, unsigned char charSize)
{
    unsigned char j = 0;
    unsigned char xPosition = x;
    unsigned char yPosition = y;

    if (chr == NULL) {
        printf("param is NULL,Please check!!!\r\n");
        return;
    }
    while (chr[j] != '\0') {
        OledShowChar(xPosition, yPosition, chr[j], charSize);
        xPosition += X_COLUMNS_PIXEL_8; /* 8列组成一个字符位置 */
        if (xPosition > X_PIXEL_POINT_POSITION_120) { /* 120 */
            xPosition = 0;
            yPosition += Y_LINES_PIXEL_2; /* 每2行写 */
        }
        j++;
    }
}

/* 小数转字符串
 * 输入：double 小数
 * 输出：转换后的字符串
*/
unsigned char *FlaotToString(double d, unsigned char *str)
{
    unsigned char str1[40] = {0};
    double data = d;
    unsigned char *floatString = str;
    int j = 0;
    int m;

    if (str == NULL) {
        return;
    }

    m = (int)data; /* 浮点数的整数部分 */
    while (m > 0) {
        str1[j++] = m % 10 + '0'; /* 10 : 对10求余 */
        m = m / 10; /* 10 : 对10求模 */
    }

    for (int k = 0; k < j; k++) {
        floatString[k] = str1[j - 1 - k]; /* 1： 被提取的整数部分正序存放到另一个数组 */
    }
    floatString[j++] = '.';
 
    data = data - (int)data; /* 小数部分提取 */
    for (int i = 0; i < 1; i++) { /* 1: 取小数点1位 */
        data = data * 10; /* 10：取整数 */
        floatString[j++] = (int)data + '0';
        data = data - (int)d;
    }

    while (floatString[--j] == '0') {
        if (j == 0) {
            break;
        }
    }
    floatString[++j] = '\0';

    return floatString;
}

static void OledNfcDisplayInit(void)
{
    OledFillScreen(OLED_CLEAN_SCREEN); // clear screen
    OledShowStr(OLED_X_POSITION_0, OLED_Y_POSITION_0, \
                "WiFi-AP  ON  U:1", OLED_DISPLAY_STRING_TYPE_1); /* 0, 0, xx, 1 */
    OledShowStr(OLED_X_POSITION_0, OLED_Y_POSITION_3, \
                "    NFC Mode    ", OLED_DISPLAY_STRING_TYPE_1); /* 15, 1, xx, 1 */
}

static void NFCTagCloudMusicMode(void)
{
    OledShowStr(OLED_X_POSITION_0, OLED_Y_POSITION_6, \
                "   CloudMusic   ", OLED_DISPLAY_STRING_TYPE_1); /* 0, 2, xx, 1 */
    SetNdefData();
    int ret = GetNdefData(ndefFileCloudMusic, sizeof(ndefFileCloudMusic));
    if (ret != 0) {
        printf("get ndefFile Wechat Failed\r\n");
    }
}


static void OledScreenInitConfig(void)
{
    while (OledInit() != 0) {
        printf("Connecting oled board falied!Please access oled board\r\n");
        if (Hi3861BoardLedTest == 0) {
            Hi3861BoardLedTest = 1;
            /* test HiSpark board */
            FACTORY_HISPARK_BOARD_TEST("-----------HiSpark board test----------");
        }
        TaskMsleep(SLEEP_1S);
    }
}

void NfcOledReturnMode(void)
{
    OledShowStr(OLED_X_POSITION_0, OLED_Y_POSITION_6, \
                "   Return Menu   ", OLED_DISPLAY_STRING_TYPE_1); /* 0, 2, xx, 1 */
}

/* nfc menu display */
void OledNfcDisplay(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_13);
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoTGpioInit(IOT_IO_NAME_GPIO_14);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    /* 初始化时屏幕 i2c baudrate setting */
    IoTI2cInit(IOT_I2C_IDX_0, HI_I2C_IDX_BAUDRATE); /* baudrate: 400kbps */
    IoTI2cSetBaudrate(IOT_I2C_IDX_0, HI_I2C_IDX_BAUDRATE);
    /* ssd1306 config init */
    OledScreenInitConfig();
    /* 按键中断初始化 */
    TestGpioInit();
    AppMultiSampleDemo();
    /* display init */
    OledNfcDisplayInit();

    while (1) {
        switch (GetKeyStatus(CURRENT_MODE)) {
            case NFC_TAG_CloudMusic_MODE:
                NFCTagCloudMusicMode();
                break;
            case NFC_RETURN_MODE:
                NfcOledReturnMode();
                break;
            default:
                break;
        }
        TaskMsleep(SLEEP_10_MS); // 10ms
    }
}

/* shut down all led */
void AllLedOff(void)
{
    GpioControl(HI_IO_NAME_GPIO_10, HI_GPIO_IDX_10, HI_GPIO_DIR_OUT,
                HI_GPIO_VALUE0, HI_IO_FUNC_GPIO_10_GPIO);
    GpioControl(HI_IO_NAME_GPIO_11, HI_GPIO_IDX_11, HI_GPIO_DIR_OUT,
                HI_GPIO_VALUE0, HI_IO_FUNC_GPIO_10_GPIO);
    GpioControl(HI_IO_NAME_GPIO_12, HI_GPIO_IDX_12, HI_GPIO_DIR_OUT,
                HI_GPIO_VALUE0, HI_IO_FUNC_GPIO_12_GPIO);
}