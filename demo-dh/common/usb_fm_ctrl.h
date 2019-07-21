/***************************************************************************
 *
 * Copyright 2015-2019 xxx.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by xxx.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __USB_FM_CTRL_H__
#define __USB_FM_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define USB_FM_PRIMITIVE_SET     0xA1
#define USB_FM_PRIMITIVE_GET     0xA2
#define USB_FM_PRIMITIVE_QUERY   0xA3

typedef enum {
    USB_FM_SET_POWER,            // 0x00
    USB_FM_SET_BAND,             // 0x01
    USB_FM_SET_CHAN_RSSI_TH,     // 0x02
    USB_FM_SET_CHAN_SPACING,     // 0x03
    USB_FM_SET_MUTE,             // 0x04
    USB_FM_SET_VOLUME,           // 0x05
    USB_FM_SET_MONO,             // 0x06
    USB_FM_SET_SEEK_START,       // 0x07
    USB_FM_SET_SEEK_STOP,        // 0x08
    USB_FM_SET_TUNING_CHAN,      // 0x09
    USB_FM_SET_RDS,              // 0x0A
    //SS_FM_SET_DNS,
    //SS_FM_SET_AF,
    USB_FM_SET_DC_THRES,         // 0x0B
    USB_FM_SET_SPIKE_THRES,      // 0x0C

    USB_FM_SET_MAX_QTY
} USB_FM_SET_CMD_E;

typedef enum {
    USB_FM_GET_FM_IC = 1,            // 0x01
    USB_FM_GET_FM_POWER_STATE,       // 0x02
    USB_FM_GET_BAND,                 // 0x03
    USB_FM_GET_SEEK_CHAN_RSSI_TH,    // 0x04
    USB_FM_GET_SEEK_CHAN_SPACING,    // 0x05
    USB_FM_GET_MUTE_STATE,           // 0x06
    USB_FM_GET_MONO_STATE,           // 0x07
    USB_FM_GET_VOL,                  // 0x08
    USB_FM_GET_SEEK_STATE,           // 0x09
    USB_FM_GET_RDS_STATE,            // 0x0A
    USB_FM_GET_DNS_STATE,            // 0x0B
    USB_FM_GET_AF_STATE,             // 0x0C
    USB_FM_GET_CUR_CHAN,             // 0x0D
    USB_FM_GET_DC_THRES,             // 0x0E
    USB_FM_GET_SPIKE_THRES,          // 0x0F

    USB_FM_GET_MAX_QTY
} USB_FM_GET_CMD_E;

typedef enum {
    RTC6226_REG_DEVICE_ID = 0x00,
    RTC6226_REG_CHIP_ID,
    RTC6226_REG_MPXCFG,
    RTC6226_REG_TUNE_CHAN,
    RTC6226_REG_SYSCFG,
    RTC6226_REG_SEEKCFG1,
    RTC6226_REG_PWRCFG,
    RTC6226_REG_PADCFG,

    RTC6226_REG_STATUS = 0x0A,
    RTC6226_REG_RSSI,
    RTC6226_REG_RDS_BA_INFO,
    RTC6226_REG_RDS_BB_INFO,
    RTC6226_REG_RDS_BC_INFO,
    RTC6226_REG_RDS_BD_INFO,

    RTC6226_REG_AUDCFG = 0x12,
    RTC6226_REG_RADIOCFG0,
    RTC6226_REG_RADIOCFG_SEEK1,
    RTC6226_REG_RADIOCFG_SEEK2,

    RTC6226_REG_I2SCFG = 0x1C,

    RTC6226_REG_CHAN = 0x1E,

    RTC6226_REG_MAX_NUM = 0x20
} RTC6226_REG_E;

#ifdef __cplusplus
}
#endif

#endif // __USB_FM_CTRL_H__
