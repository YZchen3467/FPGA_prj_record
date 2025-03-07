/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_edid.h
*
* This file contains set of EDID demonstrates different capability
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         12/02/18 Initial release.
* 1.01  EB     05/04/18 Updated EDID
* </pre>
*
******************************************************************************/
#ifndef _XHDMI_EDID_H_
/**  prevent circular inclusions by using protection macros */
#define _XHDMI_EDID_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xvidc_edid_ext.h"
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#include "xv_hdmirxss.h"
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#endif

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*Magic Number: Maximum Number of Retries for EDID Read*/
#define READEDIDRETRY 5
/*Magic Number: Maximum Number of Retries for SCDC Read*/
#define READSCDCRETRY 5
/*Magic Number: Maximum Number of Retries for EDID Read Interval*/
#define READINTERVAL 25000

/**
* These constants specify the flags of warning msg
*/
#define	XV_HDMI_SINK_NO_WARNINGS                  0x00
#define XV_HDMI_SINK_EDID_SCDC_MISMATCH           0x01
	/* Sink's EDID indicates supported TMDS more than 340Mbps
	 * and the HF-VSDB HDMI 2.0 is available but the
	 * RR_Capable/Read Ready is not asserted */
#define	XV_HDMI_SINK_EDID_20_VSDB20_NA_SCDC_PASS  0x02
	/* Sink's EDID indicates supported TMDS more than 340Mbps
	 * and the HF-VSDB HDMI 2.0 is not available but the SCDC
	 * is accessible*/
#define	XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL 0x04
	/* Sink's EDID indicates supported TMDS more than 340Mbps
	 * and the HF-VSDB HDMI 2.0 is available but the SCDC
	 * is not accessible*/
#define	XV_HDMI_SINK_EDID_14_SCDC_PASS            0x08
	/* Sink's EDID indicates supported TMDS less than 340Mbps
	 * (HDMI 1.4) but the SCDC Register is accessible*/
#define	XV_HDMI_SINK_20_NOT_CAPABLE               0x10
	/*Sink not HDMI 2.0 Capable */
#define	XV_HDMI_SINK_DEEP_COLOR_10_NOT_SUPP       0x20
	/*Deep Color 10bpc is not supported */
#define	XV_HDMI_SINK_DEEP_COLOR_12_NOT_SUPP       0x40
	/*Deep Color 12bpc is not supported */
#define XV_HDMI_SINK_DEEP_COLOR_16_NOT_SUPP       0x80
	/*Deep Color 16bpc is not supported */
#define XV_SINK_NOT_HDMI                          0x100
	/*Sink is not HDMI */

/*EDID Parsing Data Structure (Application)*/
typedef struct {
	XV_VidC_EdidCntrlParam EdidCtrlParam;
	/*Control Parameter from the EDID Driver*/

	u32 HdmiSinkWarningFlag;
	/*Hdmi Sink Warning Flag*/

	/*Scratch Pad */
	u8 EdidCableConnectRead;
	/* Flag indicate EDID Read during cable
	 * connect
	 */
	u8 IsReReadSinkEdid;
	/*Status Flag for Re-Read EDID*/
	u8 IsReReadSCDC;
	/*Status Flag for Re-Read SCDC*/
	u8 IsHDMI20SinkCapable;
	/*Status Flag for HDMI 2.0 Capable*/
	u32 SinkCheckRetryCount;
	/*Counter for Re-Read EDID Interval
	 * (Magic Number)*/
	u8 IsReReadSinkEdidRetry;
	/*Counter for Flag for Re-Read EDID Retry*/
	u8 IsReReadScdcRetry;
	/*Counter for Re-Read SCDC of Sink Retry
	 * (Non-EDID related)*/
} EdidHdmi20;

extern EdidHdmi20 EdidHdmi20_t;
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*
  EDID
*/
/*EDID for 2018.1*/
static const u8 Edid[] = {
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x69, 0xF2, 0x22, 0x01, 0x01, 0x01, 0x01,
		0x1C, 0x14, 0x01, 0x03, 0x80, 0x30, 0x1B, 0x78, 0xEE, 0xC4, 0xF6, 0xA3, 0x54, 0x57, 0x9C, 0x23,
		0x11, 0x4F, 0x54, 0xBF, 0xEF, 0x00, 0x71, 0x4F, 0x81, 0x80, 0x81, 0x40, 0x81, 0x95, 0xA9, 0x40,
		0xB3, 0x00, 0xD1, 0xC0, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x5A, 0x2D, 0x58, 0x2C,
		0x45, 0x00, 0xDD, 0x0C, 0x11, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFD, 0x71, 0x00, 0x4C, 0x1F,
		0x53, 0x11, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56,
		0x48, 0x32, 0x32, 0x36, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFF,
		0x00, 0x41, 0x37, 0x4C, 0x4D, 0x51, 0x53, 0x30, 0x31, 0x35, 0x39, 0x38, 0x0A, 0x38, 0x01, 0x08,

		0x02, 0x03, 0x1E, 0xF1, 0x4B, 0x90, 0x05, 0x04, 0x03, 0x02, 0x01, 0x11, 0x12, 0x13, 0x14, 0x1F,
		0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x1A, 0x36,
		0x80, 0xA0, 0x70, 0x38, 0x1E, 0x40, 0x30, 0x20, 0x35, 0x00, 0xDD, 0x0C, 0x11, 0x00, 0x00, 0x1A,
		0x66, 0x21, 0x56, 0xAA, 0x51, 0x00, 0x1E, 0x30, 0x46, 0x8F, 0x33, 0x00, 0xDD, 0x0C, 0x11, 0x00,
		0x00, 0x1E, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xDD, 0x0C,
		0x11, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00,
		0xDD, 0x0C, 0x11, 0x00, 0x00, 0x18, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C,
		0x25, 0x00, 0x13, 0x2B, 0x21, 0x00, 0x00, 0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCD
};
#endif
/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
void EdidScdcCheck(XV_HdmiTxSs          *HdmiTxSsPtr,
                   EdidHdmi20           *CheckHdmi20Param);

u8 SinkReadyCheck (XV_HdmiTxSs          *HdmiTxSsPtr,
                    EdidHdmi20           *CheckHdmi20Param);

void EDIDConnectInit(EdidHdmi20           *CheckHdmi20Param);
void SinkCapWarningMsg(EdidHdmi20 *CheckHdmi20Param);
void SinkCapabilityCheck(EdidHdmi20 *CheckHdmi20Param);
#endif
#ifdef __cplusplus
}
#endif

#endif /* _XHDMI_EDID_H_ */
