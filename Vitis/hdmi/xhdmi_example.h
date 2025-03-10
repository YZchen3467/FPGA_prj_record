/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_example.h
*
* This file contains set of definition for the main application
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         12/02/18 Initial release.
* 3.03  YB     08/14/18 Adding macro 'ENABLE_HDCP_REPEATER' to allow application
*                       to select/deselect the Repeater specific code.
*       EB     09/21/18 Added new API ToggleHdmiRxHpd
* </pre>
*
******************************************************************************/
#ifndef _XHDMI_EXAMPLE_H_
/**  prevent circular inclusions by using protection macros */
#define _XHDMI_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xparameters.h"
#include "xiic.h"
#include "xil_io.h"
#include "xuartps.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "string.h"
#if (defined XPS_BOARD_ZCU104)
#include "idt_8t49n24x.h"
#endif
#include "xvidc.h"
#include "xv_hdmic.h"
#include "xv_hdmic_vsif.h"
#include "dp159.h"
#include "sleep.h"
#include "xhdmi_edid.h"
#include "xhdmi_menu.h"
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#include "xv_hdmirxss.h"
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#include "audiogen_drv.h"
#ifdef XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR
/* This is only required for the audio over HDMI */
#define USE_HDMI_AUDGEN
#endif
#endif
#include "xvphy.h"
#if defined (ARMR5) || (__aarch64__) || (__arm__)
#include "xscugic.h"
#endif
#include "xhdmi_hdcp_keys.h"
#include "xhdcp.h"
#include "xvidframe_crc.h"

/* AUXFIFOSIZE: Must be set to 3 or higher*/
#define AUXFIFOSIZE 10

#define UART_BASEADDR XPAR_XUARTPS_0_BASEADDR

/************************** Constant Definitions *****************************/
#define I2C_MUX_ADDR    0x74  /**< I2C Mux Address */
#if (defined XPS_BOARD_ZCU104)
#define I2C_CLK_ADDR    0x6C  /**< I2C Clk Address IDT_8T49N241*/
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"


/************************** Constant Definitions *****************************/

/******************************** OPTIONS ************************************/
/* Enabling this will disable Pass-through mode and TX and RX will operate
 * separately
 */
#define LOOPBACK_MODE_EN 0

/* Enabling this will enable a debug UART menu */
#define HDMI_DEBUG_TOOLS 0

/* Enabling this will register a custom resolution to the video timing table
 */
#define CUSTOM_RESOLUTION_ENABLE 1

/* Enabling this will enable HDCP Debug menu */
#define HDCP_DEBUG_MENU_EN 0

/* Enabling this will enable Video Masking menu */
#define VIDEO_MASKING_MENU_EN 0

/************************** Variable Definitions *****************************/
/* VPhy structure */
extern XVphy     Vphy;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/* HDMI TX SS structure */
extern XV_HdmiTxSs HdmiTxSs;

#ifdef USE_HDMI_AUDGEN
extern XhdmiAudioGen_t AudioGen;
#endif

extern u8 TxCableConnect;
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/* HDMI RX SS structure */
extern XV_HdmiRxSs HdmiRxSs;
#endif

/* TX busy flag. This flag is set while the TX is initialized*/
extern u8 TxBusy;
extern u8 IsPassThrough;

/************************** Function Prototypes ******************************/
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
void ToggleHdmiRxHpd (XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr);
void SetHdmiRxHpd(XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr, u8 Hpd);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _XHDMI_EXAMPLE_H_ */
