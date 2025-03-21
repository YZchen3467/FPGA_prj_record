/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_example.c
*
* This file demonstrates how to use Xilinx HDMI TX Subsystem, HDMI RX Subsystem
* and Video PHY Controller drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         25/11/15 Initial release.
* 1.10         05/02/16 Updated function RxAuxCallback.
* 2.00  MG     02/03/15 Added upgraded with HDCP driver and overlay
* 2.10  MH     06/23/16 Added HDCP repeater support.
* 2.11  YH     04/08/16 Added two level validation routines
*                       Basic_validation will only check the received VmId
*                       PRBS_validation will check both video & audio contents
* 2.12  GM     07/10/16 Added onboard SI5324 Initialization API to enable
*                       125Mhz as NI-DRU reference clock
* 2.13  YH     03/01/16 Fixed a system hang issue by clearing TxBusy flag when
*                            a non-supportedvideo resolution is set
*                            during enable colorbar API
* 2.14  GM     23/01/17 Replace the Extraction Value of VPhy line rate with,
*                            XVphy_GetLineRateHz Rate API return value.
* 2.15  ms     04/10/17 Modified filename tag to include the file in doxygen
*                            examples.
* 2.16  mmo    05/05/17 Replace pre-processed interrupt vector ID with the
*                            pre-processed canonical interrupt vector ID for
*                            microblaze processor
* 2.17  YH     12/06/17 Removed unused PRBS validation related codes
*                       Added VPHY error processing APIs and typedef
*                       Placed Si5324 on reset on bonded mode in StartTxAfterRx
*                       Changed printf usage to xil_printf
*                       Changed "\n\r" in xil_printf calls to "\r\n"
* 2.18  mmo    21/07/17 Remove the i2c_dp159 API Call and
*                            XVphy_Clkout1OBufTdsEnable API Call from the
*                            TxStreamCallback API to avoid the race condition,
*                            and replace to be call at the global while loop.
*       MH     26/07/17 Set TMDS SCDC register after TX HPD toggle event
*       GM     18/08/17 Added SI Initialization after the SI Reset in
*                            StartTxAfterRx API
*       YH     18/08/17 Add HDCP Ready checking before set down streams
*       GM     28/08/17 Replace XVphy_HdmiInitialize API Call during
*                            Initialization with XVphy_Hdmi_CfgInitialize API
*                            Call
*       mmo    04/10/17 Updated function TxStreamUpCallback to include
*                            XhdmiACRCtrl_TMDSClkRatio API Call
*       EB     06/11/17 Updated function RxAudCallback to allow pass-through
*                            of audio format setting
* 3.00  mmo    29/12/17 Added EDID Parsing Capability
*       EB     16/01/18 Added InfoFrame capability
*       YH     16/01/18 Added video_bridge overflow interrupt
*                       Added video_bridge unlock interrupt
*       GM     16/01/18 Updated EnableColorBar to skip TX reconfiguration
*                            when the requested TX video resolution
*                            is not supported
*       EB     23/01/18 Modified RxStreamUpCallback so that scrambling flag
*                            is always enabled for HDMI 2.0 resolutions and
*                            always disabled for HDMI 1.4 resolutions during
*                            pass-through mode
*       EB     26/01/18 Updated function UpdateFrameRate to use the API
*                            XVidC_GetVideoModeIdExtensive
*       MMO    08/02/18 Updated the EnableColorBar, UpdateFrameRate,
*                            UpdateColorDepth, UpdateColorFormat API for
*                            clean flow.
*       GM              Added support for ZCU104
*       SM     28/02/18 Added code to call API for setting App version to
*                            support backward compatibility related issues.
* 3.01  EB     09/04/18 Updated XV_ConfigTpg and EnableColorBar APIs
*              18/04/18 Updated RxBrdgOverflowCallback to remove printing
* 3.02  mmo    23/04/18 Added checking Sink Capability whether it's a DVI sink
*                             or HDMI sink based on EDID-HDMI VSDB.
*                       Fixed system flow to avoid RX Buffer Overflow during
*                              transition.
*                       Code Clean-Up on comments and 80 Characted per line.
*                       Improve audio configuration during Pass-through mode.
*                       Disable HDMI RX Video Stream when EnableColorBar API
*                              is called.
*                       Added TX Bridge Overflow and TX Bridge Underflow
* 3.03  YB     08/14/18 Clubbing Repeater specific code under the
*                       'ENABLE_HDCP_REPEATER' macro.
*       EB     09/21/18 Added new API ToggleHdmiRxHpd and SetHdmiRxHpd
*                       Updated CloneTxEdid API
* 3.04  EB     03/01/19 Fixed an issue where TX's color space is not up-to-date
*                              in pass-through mode
*                       Fixed an issue where SCDC is not cleared when HPD is
*                              toggled
*                       Fixed an issue where TX stream doesn't come up when
*                              hotplug is performed on HDMI 2.0 resolution in
*                              loopback mode
*       EB     03/08/19 Fixed an issue where loading of default EDID doesn't
*                              toggle HPD
*       mmo    03/08/19 Added "IsStreamUpHDCP" to enable the HDCP
*                              Authentication on the first VSYNC of TX
* 3.05  ssh    03/17/21 Added EdidHdmi20_t, PsIic0 and PsIic1 declarations
* 3.06  ssh    04/19/21 Conditional checks added for instances of
*                              XVphy_Clkout10BufTdsEnable function
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include "xhdmi_menu.h"
#include "xhdmi_hdcp_keys_table.h"
#include "xhdmi_example.h"

/***************** Macros (Inline Functions) Definitions *********************/
/* These macro values need to changed whenever there is a change in version */
#define APP_MAJ_VERSION 5
#define APP_MIN_VERSION 4

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int I2cMux(void);
int I2cClk(u32 InFreq, u32 OutFreq);

void Info(void);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
void CloneTxEdid(void);
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
void TxConnectCallback(void *CallbackRef);
void TxToggleCallback(void *CallbackRef);
void TxVsCallback(void *CallbackRef);
void TxStreamUpCallback(void *CallbackRef);
void TxStreamDownCallback(void *CallbackRef);
void VphyHdmiTxInitCallback(void *CallbackRef);
// void VphyHdmiTxReadyCallback(void *CallbackRef);
void TxInfoFrameReset(void);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
void RxConnectCallback(void *CallbackRef);
void RxStreamUpCallback(void *CallbackRef);
void RxStreamDownCallback(void *CallbackRef);
void VphyHdmiRxInitCallback(void *CallbackRef);
void VphyHdmiRxReadyCallback(void *CallbackRef);
#endif
void VphyErrorCallback(void *CallbackRef);
void VphyProcessError(void);

/************************* Variable Definitions *****************************/
/* VPHY structure */
XVphy              Vphy;
u8                 VphyErrorFlag;
u8                 VphyPllLayoutErrorFlag;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/* HDMI TX SS structure */
XV_HdmiTxSs        HdmiTxSs;
XV_HdmiTxSs_Config *XV_HdmiTxSs_ConfigPtr;

EdidHdmi20 EdidHdmi20_t;

#ifdef USE_HDMI_AUDGEN
/* Audio Generator structure */
XhdmiAudioGen_t    AudioGen;
#endif

XHdmiC_Aux         AuxFifo[AUXFIFOSIZE];
u8                 AuxFifoStartIndex;
u8                 AuxFifoEndIndex;
u8                 AuxFifoCount;
u8				   AuxFifoOvrFlowCnt;

/* Flag indicates whether the TX Cable is connected or not */
u8                 TxCableConnect = (FALSE);

/* TX busy flag. This flag is set while the TX is initialized*/
u8                 TxBusy = (TRUE);

/* TX Stream Up Status Flag, Avoiding Race condition */
u8                 IsStreamUp = (FALSE);
u64                TxLineRate = 0;

/* Sink Ready: Become true when the EDID parsing is completed
 * upon cable connect */
u8                 SinkReady = (FALSE);

/* Variable for pass-through operation */
u8                 AuxFifoStartFlag = (FALSE);
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/* HDMI RX SS structure */
XV_HdmiRxSs        HdmiRxSs;
XV_HdmiRxSs_Config *XV_HdmiRxSs_ConfigPtr;
#endif

/* Interrupt Controller Data Structure */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
static XScuGic     Intc;
#endif

/* HDMI Application Menu: Data Structure */
XHdmi_Menu         HdmiMenu;

/**< Demo mode IsPassThrough
 * (TRUE)  = Pass-through mode
 * (FALSE) = Color Bar mode
 *  */
u8                 IsPassThrough = (FALSE);
u8                 StartTxAfterRxFlag = (FALSE);

/* General HDMI Application Variable Declaration (Scratch Pad) */

/************************** Function Definitions *****************************/

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function clones the EDID of the connected sink device to the HDMI RX
* @return None.
*
* @note   None.
*
******************************************************************************/
void CloneTxEdid(void)
{
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
    u8 Buffer[256];
    u32 Status;

    /* Read TX edid */
    Status = XV_HdmiTxSs_ReadEdid(&HdmiTxSs, (u8*)&Buffer);

    /* Check if read was successful */
    if (Status == (XST_SUCCESS)) {
        /* Load new EDID */
        XV_HdmiRxSs_LoadEdid(&HdmiRxSs, (u8*)&Buffer, sizeof(Buffer));

        /* Toggle HPD after loading new HPD */
        ToggleHdmiRxHpd(&Vphy, &HdmiRxSs);

        xil_printf("\r\n");
        xil_printf("Successfully cloned EDID and toggled HPD.\r\n");
    }
#else
    xil_printf("\r\nEdid Cloning no possible with HDMI RX SS.\r\n");
#endif
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the AuxFifo.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetAuxFifo(void)
{
	AuxFifoStartFlag   = (FALSE);
	AuxFifoStartIndex  = 0;
	AuxFifoEndIndex    = 0;
	AuxFifoCount	   = 0;
	AuxFifoOvrFlowCnt  = 0;
}
#endif

/*****************************************************************************/
/**
*
* This function checks the TX Busy flag, and returns TRUE with prompt. and
* FALSE
* @return TRUE/FALSE.
*
* @note   None.
*
******************************************************************************/
u8 CheckTxBusy (void)
{
	if (TxBusy) {
		xil_printf("Either TX still on transition to a new video"
		            " format\r\nor the TX cable is not connected\r\n");
	}
	return (TxBusy);
}

/* Send Vendor Specific InfoFrame */
void SendVSInfoframe(XV_HdmiTxSs *HdmiTxSsPtr)
{
	XHdmiC_VSIF *VSIFPtr;
	VSIFPtr = XV_HdmiTxSs_GetVSIF(HdmiTxSsPtr);

	XHdmiC_Aux Aux;

	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
	(void)memset((void *)&Aux, 0, sizeof(XHdmiC_Aux));

	VSIFPtr->Version = 0x1;
	VSIFPtr->IEEE_ID = 0xC03;

	if (XVidC_IsStream3D(&(HdmiTxSsPtr->HdmiTxPtr->Stream.Video))) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_3D;
		VSIFPtr->Info_3D.Stream =
				HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Info_3D;
		VSIFPtr->Info_3D.MetaData.IsPresent = FALSE;
	} else if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_24_P ||
			   HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_25_P ||
			   HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_30_P ||
			   HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_4096x2160_24_P ||
			   HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
			   	   	   XVIDC_VM_1920x1200_60_P	) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_EXTRES;

		/* Set HDMI VIC */
		switch(HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId) {
			case XVIDC_VM_1920x1200_60_P :
				VSIFPtr->HDMI_VIC = 5;
				break;
			case XVIDC_VM_4096x2160_24_P :
				VSIFPtr->HDMI_VIC = 4;
				break;
			case XVIDC_VM_3840x2160_24_P :
				VSIFPtr->HDMI_VIC = 3;
				break;
			case XVIDC_VM_3840x2160_25_P :
				VSIFPtr->HDMI_VIC = 2;
				break;
			case XVIDC_VM_3840x2160_30_P :
				VSIFPtr->HDMI_VIC = 1;
				break;
			default :
				break;
		}
	} else {
		VSIFPtr->Format = XHDMIC_VSIF_VF_NOINFO;
	}

	Aux = XV_HdmiC_VSIF_GeneratePacket(VSIFPtr);

	XV_HdmiTxSs_SendGenericAuxInfoframe(HdmiTxSsPtr, &Aux);
}

/* Send out all the InfoFrames in the AuxFifo during PassThrough mode
 * or send out AVI, Audio, Vendor Specific InfoFrames.
 */
void SendInfoframe(XV_HdmiTxSs *HdmiTxSsPtr)
{
	u32 Status;
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
	XHdmiC_VSIF *VSIFPtr;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	AviInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(HdmiTxSsPtr);
	AudioInfoFramePtr = XV_HdmiTxSs_GetAudioInfoframe(HdmiTxSsPtr);
	VSIFPtr = XV_HdmiTxSs_GetVSIF(HdmiTxSsPtr);
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
	Status = (XST_FAILURE);

	if (!IsPassThrough) {
		/* Generate Aux from the current TX InfoFrame */
		AuxFifo[0] = XV_HdmiC_AVIIF_GeneratePacket(AviInfoFramePtr);
		XV_HdmiTxSs_SendGenericAuxInfoframe(HdmiTxSsPtr,
						    &(AuxFifo[0]));

		/* GCP does not need to be sent out because GCP packets on
		 * the TX side is handled by the HDMI TX core fully.
		 */

		AuxFifo[0] =
			XV_HdmiC_AudioIF_GeneratePacket(AudioInfoFramePtr);
		XV_HdmiTxSs_SendGenericAuxInfoframe(HdmiTxSsPtr,
						    &(AuxFifo[0]));
		SendVSInfoframe(HdmiTxSsPtr);
	} else {
		if(AuxFifoCount > AUXFIFOSIZE) {
			AuxFifoStartIndex = AuxFifoEndIndex;
		}

		/* If PassThrough, update TX's InfoFrame Data Structure
		 * from AuxFiFO
		 */
		while (AuxFifoStartIndex != AuxFifoEndIndex) {
			if(AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
							AUX_VSIF_TYPE) {
				/* Reset Vendor Specific InfoFrame */
				(void)memset((void *)VSIFPtr,
					     0,
					     sizeof(XHdmiC_VSIF));

				XV_HdmiC_VSIF_ParsePacket
						(&AuxFifo[AuxFifoStartIndex],
						 VSIFPtr);
			} else if(AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
					AUX_AVI_INFOFRAME_TYPE) {
				/* Reset Avi InfoFrame */
				(void)memset((void *)AviInfoFramePtr, 0,
						sizeof(XHdmiC_AVI_InfoFrame));

				XV_HdmiC_ParseAVIInfoFrame
						(&AuxFifo[AuxFifoStartIndex],
						 AviInfoFramePtr);

				if (IsPassThrough && AviInfoFramePtr->ColorSpace !=
						XV_HdmiC_XVidC_To_IfColorformat(HdmiTxSsVidStreamPtr->ColorFormatId)) {

					/* The color space decoded from the RX's InfoFrame
					 * indicates a color space change. Proceed to update the
					 * TX stream color space to the new value.
					 */
					switch (AviInfoFramePtr->ColorSpace) {
						case XHDMIC_COLORSPACE_RGB :
							HdmiTxSsVidStreamPtr->ColorFormatId =
									XVIDC_CSF_RGB;
							break;

						case XHDMIC_COLORSPACE_YUV422 :
							HdmiTxSsVidStreamPtr->ColorFormatId =
									XVIDC_CSF_YCRCB_422;
							break;

						case XHDMIC_COLORSPACE_YUV444 :
							HdmiTxSsVidStreamPtr->ColorFormatId =
									XVIDC_CSF_YCRCB_444;
							break;

						case XHDMIC_COLORSPACE_YUV420 :
							HdmiTxSsVidStreamPtr->ColorFormatId =
									XVIDC_CSF_YCRCB_420;
							break;

						default:
							break;
					}

					xil_printf(ANSI_COLOR_YELLOW "TX Color space changed to %s"
						ANSI_COLOR_RESET "\r\n",
						XVidC_GetColorFormatStr(HdmiTxSsVidStreamPtr->ColorFormatId));

				}

				/* Modify the TX's InfoFrame here before
				 * sending out
				 * E.g:
				 *     AviInfoFramePtr->VIC = 107;
				 */

				/* Generate Aux from the modified TX's
				 * InfoFrame before sending out
				 * E.g:
				 * 	AuxFifo[AuxFifoStartIndex] =
				 * 		XV_HdmiC_AVIIF_GeneratePacket
				 *			     (AviInfoFramePtr);
				 */
			} else if(AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
					AUX_AUDIO_INFOFRAME_TYPE) {
				/* Reset Audio InfoFrame */
				(void)memset((void *)AudioInfoFramePtr, 0,
						sizeof(XHdmiC_AudioInfoFrame));

				XV_HdmiC_ParseAudioInfoFrame
						(&AuxFifo[AuxFifoStartIndex],
						 AudioInfoFramePtr);

				/* Modify the TX's InfoFrame here
				 * before sending out
				 * E.g:
				 * 	AudioInfoFramePtr->ChannelCount =
				 * 		XHDMIC_AUDIO_CHANNEL_COUNT_3;
				 */

				/* Generate Aux from the modified TX's
				 * InfoFrame beforesending out
				 * E.g :
				 * 	AuxFifo[AuxFifoStartIndex] =
				 * 		XV_HdmiC_AudioIF_GeneratePacket
				 *			   (AudioInfoFramePtr);
				 */
			}

			Status = XV_HdmiTxSs_SendGenericAuxInfoframe
					(HdmiTxSsPtr,
					 &(AuxFifo[AuxFifoStartIndex]));

			/* If TX Core's hardware Aux FIFO is full,
			 * from the while loop, retry during the
			 * next main while iteration.
			 */
			if (Status != (XST_SUCCESS)) {
				xil_printf(ANSI_COLOR_RED
				           "HW Aux Full"
					   ANSI_COLOR_RESET
				           "\r\n");
			}

			if(AuxFifoStartIndex < (AUXFIFOSIZE - 1)) {
				AuxFifoStartIndex++;
			} else {
				AuxFifoStartIndex = 0;
			}
		}

		AuxFifoCount = 0;
	}
}

void TxInfoFrameReset(void)
{
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	AviInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(&HdmiTxSs);
	AudioInfoFramePtr = XV_HdmiTxSs_GetAudioInfoframe(&HdmiTxSs);

	/* Reset Avi InfoFrame */
	(void)memset((void *)AviInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AVI_InfoFrame));

	/* Reset Audio InfoFrame */
	(void)memset((void *)AudioInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AudioInfoFrame));

	AviInfoFramePtr->Version = 2;
	AviInfoFramePtr->ColorSpace = XHDMIC_COLORSPACE_RGB;
	AviInfoFramePtr->VIC = 16;
	AviInfoFramePtr->PicAspectRatio = XHDMIC_PIC_ASPECT_RATIO_16_9;
    /* AudioInfoFramePtr->ChannelCount = XHDMIC_AUDIO_CHANNEL_COUNT_3; */
}
#endif

/*****************************************************************************/
/**
*
* This function setup SI5324 clock generator either in free or locked mode.
*
* @param  Index specifies an index for selecting mode frequency.
* @param  Mode specifies either free or locked mode.
*
* @return
*   - Zero if error in programming external clock.
*   - One if programmed external clock.
*
* @note   None.
*
******************************************************************************/
int I2cClk(u32 InFreq, u32 OutFreq)
{
	int Status;
	/* Reset I2C controller before issuing new transaction.
	 * This is required torecover the IIC controller in case a previous
	 * transaction is pending.
	 */
	XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
				  XIIC_RESET_MASK);

	/* Free running mode */
	if (InFreq == 0) {
		Status = IDT_8T49N24x_SetClock((XPAR_IIC_0_BASEADDR),
					       (I2C_CLK_ADDR),
					       (IDT_8T49N24X_XTAL_FREQ),
					       OutFreq,
					       TRUE);

		if (Status != (XST_SUCCESS)) {
			print("Error programming IDT_8T49N241\r\n");
			return 0;
		}
	}/* Locked mode */
	else {
		Status = IDT_8T49N24x_SetClock((XPAR_IIC_0_BASEADDR),
					       (I2C_CLK_ADDR),
					       InFreq,
					       OutFreq,
					       FALSE);

		if (Status != (XST_SUCCESS)) {
			print("Error programming SI5324\n\r");
			return 0;
		}
	}
	return 1;
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function reports the stream mode
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ReportStreamMode(XV_HdmiTxSs *HdmiTxSsPtr, u8 IsPassThrough)
{
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	if (IsPassThrough) {
		xil_printf("--------\r\nPass-Through :\r\n");
	} else {
		xil_printf("--------\r\nColorbar :\r\n");
	}

	XVidC_ReportStreamInfo(HdmiTxSsVidStreamPtr);
	xil_printf("--------\r\n");
}
#endif

/*****************************************************************************/
/**
*
* This function outputs the video timing , Audio, Link Status, HDMI RX state of
* HDMI RX core. In addition, it also prints information about HDMI TX, and
* HDMI GT cores.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Info(void)
{
	u32 Data;
	xil_printf("\r\n-----\r\n");
	xil_printf("Info\r\n");
	xil_printf("-----\r\n\r\n");

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XV_HdmiTxSs_ReportInfo(&HdmiTxSs);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XV_HdmiRxSs_ReportInfo(&HdmiRxSs);
#endif

	/* GT */
	xil_printf("------------\r\n");
	xil_printf("HDMI PHY\r\n");
	xil_printf("------------\r\n");
	Data = XVphy_GetVersion(&Vphy);
	xil_printf("  VPhy version : %02d.%02d (%04x)\r\n",
			   ((Data >> 24) & 0xFF),
			   ((Data >> 16) & 0xFF),
			   (Data & 0xFFFF));
	xil_printf("\r\n");
	xil_printf("GT status\r\n");
	xil_printf("---------\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	xil_printf("TX reference clock frequency: %0d Hz\r\n",
			   XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_TX));
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	xil_printf("RX reference clock frequency: %0d Hz\r\n",
			   XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_RX));
	if(Vphy.Config.DruIsPresent == (TRUE)) {
		xil_printf("DRU reference clock frequency: %0d Hz\r\n",
				   XVphy_DruGetRefClkFreqHz(&Vphy));
	}
#endif
	XVphy_HdmiDebugInfo(&Vphy, 0, XVPHY_CHANNEL_ID_CH1);

#if(defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES))
	xil_printf("------------\r\n");
	xil_printf("Debugging\r\n");
	xil_printf("------------\r\n");
#endif
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("AuxFifo Overflow Count: %d\r\n", AuxFifoOvrFlowCnt);
#endif
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when a TX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxConnectCallback(void *CallbackRef) {
	XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;
	if(HdmiTxSsPtr->IsStreamConnected == (FALSE)) {
		/* TX Cable is disconnected */
		TxCableConnect = (FALSE);

		if (IsPassThrough) {
			/* If the system in the Pass-through
			 * reset the AUX FIFO
			 */
			ResetAuxFifo();
			/* Clearing the Restarting the TX after RX up flag */
			StartTxAfterRxFlag = (FALSE);
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			/* Mute/Stop the HDMI RX SS Output
			 * Enable only when the downstream is ready
			 * (TPG & HDMI TX)
			 */
			XV_HdmiRxSs_VRST(&HdmiRxSs,TRUE);
#endif
		}

		/* Cable is disconnected, don't allow any TX operation */
		TxBusy = (TRUE);

		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_TX, (FALSE));

	} else {
		/* Set TX Cable Connect Flag to (TRUE) as the cable is
		 * connected
		 */
		TxCableConnect = (TRUE);

		/* Set Flag when the cable is connected
		 * this call back take in to account two scneario
		 * cable connect and cable disconnect
		 * Stable RX stream is available
		 */
		if (IsPassThrough) {
			/* Restart Stream */
			StartTxAfterRxFlag = (TRUE);
		}
		else { 
			TxBusy = (FALSE);
		}

		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_TX, (TRUE));

		/* Initialize EDID App during cable connect */
		EDIDConnectInit(&EdidHdmi20_t);
		/* Read the EDID and the SCDC */
		EdidScdcCheck(HdmiTxSsPtr, &EdidHdmi20_t);
	}
}

/*****************************************************************************/
/**
*
* This function is called when a TX toggle event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxToggleCallback(void *CallbackRef) {
	XV_HdmiTxSs_StreamStart(&HdmiTxSs);
}

/*****************************************************************************/
/**
*
* This function is called when the GT TX reference input clock has changed.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiTxInitCallback(void *CallbackRef) {
	XV_HdmiTxSs_RefClockChangeInit(&HdmiTxSs);
}

#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is used to toggle RX's HPD Line
*
* @param  VphyPtr is a pointer to the VPHY instance.
* @param  HdmiRxSsPtr is a pointer to the HDMI RX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ToggleHdmiRxHpd(XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr) {
	SetHdmiRxHpd(VphyPtr, HdmiRxSsPtr, FALSE);
	/* Wait 500 ms */
	usleep(500000);
	SetHdmiRxHpd(VphyPtr, HdmiRxSsPtr, TRUE);
}

/*****************************************************************************/
/**
*
* This function sets the HPD on the HDMI RXSS.
*
* @param  VphyPtr is a pointer to the VPHY instance.
* @param  HdmiRxSsPtr is a pointer to the HDMI RX Subsystem instance.
* @param  Hpd is a flag used to set the HPD.
*   - TRUE drives HPD high
*   - FALSE drives HPD low
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void SetHdmiRxHpd(XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr, u8 Hpd) {
	if (Hpd == TRUE) {
		XV_HdmiRxSs_Start(HdmiRxSsPtr);
		XVphy_IBufDsEnable(VphyPtr, 0, XVPHY_DIR_RX, (TRUE));
	} else {
		XVphy_MmcmPowerDown(VphyPtr, 0, XVPHY_DIR_RX, FALSE);
		XVphy_Clkout1OBufTdsEnable(VphyPtr, XVPHY_DIR_RX, (FALSE));
		XVphy_IBufDsEnable(VphyPtr, 0, XVPHY_DIR_RX, (FALSE));
		XV_HdmiRxSs_Stop(HdmiRxSsPtr);
	}
}

/*****************************************************************************/
/**
*
* This function is called when a RX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxConnectCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

	/* RX cable is disconnected */
	if(HdmiRxSsPtr->IsStreamConnected == (FALSE)) {
		/* Clear GT RX TMDS clock ratio */
		Vphy.HdmiRxTmdsClockRatio = 0;

#if(LOOPBACK_MODE_EN != 1)
		/* Check for Pass-through:
		 * Doesnt require to restart colorbar
		 * if the system is in colorbar mode
		 */
		if (IsPassThrough) {
			/* Clear pass-through flag
			 * as the system is in TX-Only
			 * mode
			 */
			IsPassThrough = (FALSE);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			/* Start colorbar with same
			 * video parameters
			 */
			if (TxCableConnect) {
				TxBusy = (FALSE);
			} else {
				TxBusy = (TRUE);
			}
#endif
		}
#endif
		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));
	} else {
		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));
	}

}

/*****************************************************************************/
/**
*
* This function is called when the GT RX reference input clock has changed.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiRxInitCallback(void *CallbackRef) {
	XVphy *VphyPtr = (XVphy *)CallbackRef;

	XV_HdmiRxSs_RefClockChangeInit(&HdmiRxSs);
	VphyPtr->HdmiRxTmdsClockRatio = HdmiRxSs.TMDSClockRatio;
}

/*****************************************************************************/
/**
*
* This function is called when the GT RX has been initialized.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiRxReadyCallback(void *CallbackRef) {
	XVphy *VphyPtr = (XVphy *)CallbackRef;
	XVphy_PllType RxPllType;

	/* Reset the menu to main */
	XHdmi_MenuReset(&HdmiMenu);

	RxPllType = XVphy_GetPllType(VphyPtr,
	                             0,
				     XVPHY_DIR_RX,
				     XVPHY_CHANNEL_ID_CH1);

	if (!(RxPllType == XVPHY_PLL_TYPE_CPLL)) {
		XV_HdmiRxSs_SetStream(&HdmiRxSs,
		        VphyPtr->HdmiRxRefClkHz,
			(XVphy_GetLineRateHz(&Vphy,
				             0,
					     XVPHY_CHANNEL_ID_CMN0)/1000000));

	} else {
		XV_HdmiRxSs_SetStream(&HdmiRxSs, VphyPtr->HdmiRxRefClkHz,
			 (XVphy_GetLineRateHz(&Vphy,
			                      0,
					      XVPHY_CHANNEL_ID_CH1)/1000000));
	}
}
#endif

/*****************************************************************************/
/**
*
* This function is called whenever an error condition in VPHY occurs.
* This will fill the FIFO of VPHY error events which will be processed outside
* the ISR.
*
* @param  CallbackRef is the VPHY instance pointer
* @param  ErrIrqType is the VPHY error type
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyErrorCallback(void *CallbackRef) {
	VphyErrorFlag = TRUE;
}

/*****************************************************************************/
/**
*
* This function is called in the application to process the pending
* VPHY errors
*
* @param  None.
*
* @return None.
*
* @note   This function can be expanded to perform necessary actions depending
*		on the error type. For example, XVPHY_ERR_PLL_LAYOUT can be
*		used to automatically switch in and out of bonded mode for
*               GTXE2 devices
*
******************************************************************************/
void VphyProcessError(void) {
	if (VphyErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED "VPHY Error: See log for details"
				   ANSI_COLOR_RESET "\r\n");
	}
	/* Clear Flag */
	VphyErrorFlag = FALSE;
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when a TX vsync has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxVsCallback(void *CallbackRef) {
	/* When the TX stream is confirmed to have started, start accepting
	 * Aux from RxAuxCallback
	 */
	if (IsPassThrough) {
		AuxFifoStartFlag = (TRUE);
	}

	/* Check whether the sink is DVI/HDMI Supported
	 * If the sink is DVI, don't send Info-frame
	 */
	if (EdidHdmi20_t.EdidCtrlParam.IsHdmi == XVIDC_ISHDMI) {
		SendInfoframe(&HdmiTxSs);
	}
}
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when a RX aux irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxAuxCallback(void *CallbackRef) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XHdmiC_Aux *AuxPtr;
	AuxPtr = XV_HdmiRxSs_GetAuxiliary(&HdmiRxSs);

	/* In pass-through mode copy some aux packets into local buffer.
	 * GCP does not need to be sent out because GCP packets on the TX side
	 * is handled by the HDMI TX core fully. Starts storing Aux only
	 * when TX stream has started to prevent AuxFifo Overflow.
	 */
	if (IsPassThrough && AuxPtr->Header.Byte[0] !=
			AUX_GENERAL_CONTROL_PACKET_TYPE &&
						AuxFifoStartFlag == TRUE) {
		memcpy(&(AuxFifo[AuxFifoEndIndex]),
		       AuxPtr,
		       sizeof(XHdmiC_Aux));

		if(AuxFifoEndIndex < (AUXFIFOSIZE - 1)) {
			AuxFifoEndIndex++;
		} else {
			AuxFifoEndIndex = 0;
		}

		if(AuxFifoCount >= AUXFIFOSIZE) {
			AuxFifoOvrFlowCnt++;
		}

		AuxFifoCount++;
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called when a RX audio irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxAudCallback(void *CallbackRef) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

	if (IsPassThrough) {
		/* Set TX Audio params:
		 * Audio Channels:
		 */
		XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs,
				XV_HdmiRxSs_GetAudioChannels(HdmiRxSsPtr));

		/* 3D audio */
		if (XV_HdmiRxSs_GetAudioFormat(HdmiRxSsPtr) ==
						XV_HDMIRX_AUDFMT_3D) {
			XV_HdmiTxSs_SetAudioFormat(&HdmiTxSs,
						   XV_HDMITX_AUDFMT_3D);
		}
		/* HBR audio */
		else if (XV_HdmiRxSs_GetAudioFormat(HdmiRxSsPtr) ==
						XV_HDMIRX_AUDFMT_HBR) {
			XV_HdmiTxSs_SetAudioFormat(&HdmiTxSs,
						   XV_HDMITX_AUDFMT_HBR);
		}
		/* L-PCM audio */
		else {
			if (XV_HdmiRxSs_GetAudioFormat(HdmiRxSsPtr) !=
					XV_HDMIRX_AUDFMT_LPCM) {
				xil_printf(ANSI_COLOR_YELLOW "Undefined audio "
						"format detected\r\n"
						ANSI_COLOR_RESET);
			}
			XV_HdmiTxSs_SetAudioFormat(&HdmiTxSs,
						   XV_HDMITX_AUDFMT_LPCM);
		}

	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called when a RX link status irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxLnkStaCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

	if (IsPassThrough) {
		/* Reset RX when the link error has reached its maximum */
		if ((HdmiRxSsPtr->IsLinkStatusErrMax) &&
				(Vphy.Quads[0].Plls[0].RxState ==
						XVPHY_GT_STATE_READY)) {

			/* Pulse RX PLL reset */
			XVphy_ClkDetFreqReset(&Vphy, 0, XVPHY_DIR_RX);
		}
	}
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamDownCallback(void *CallbackRef) {

#if(LOOPBACK_MODE_EN != 1)
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	ResetAuxFifo();
	/* Check for Pass-through
	* Doesnt require to restart colorbar
	* if the system is in colorbar mode
	*/
	if (IsPassThrough) {
		TxBusy = (TRUE);
	}
#endif
#endif

}

/*****************************************************************************/
/**
*
* This function is called when the RX stream init
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamInitCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	u32 Status;

	/* Calculate RX MMCM parameters
	 * In the application the YUV422 colordepth is 12 bits
	 * However the HDMI transports YUV422 in 8 bits.
	 * Therefore force the colordepth to 8 bits when the
	 * colorspace is YUV422
	 */

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(HdmiRxSsPtr);

	if (HdmiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_422) {
		Status =
			XVphy_HdmiCfgCalcMmcmParam(&Vphy,
					       0,
					       XVPHY_CHANNEL_ID_CH1,
					       XVPHY_DIR_RX,
					       HdmiRxSsVidStreamPtr->PixPerClk,
					       XVIDC_BPC_8);
	}

	/* Other colorspaces */
	else {
		Status = XVphy_HdmiCfgCalcMmcmParam(&Vphy,
					     0,
					     XVPHY_CHANNEL_ID_CH1,
					     XVPHY_DIR_RX,
					     HdmiRxSsVidStreamPtr->PixPerClk,
					     HdmiRxSsVidStreamPtr->ColorDepth);
	}

	if (Status == XST_FAILURE) {
		return;
	}

	/* Enable and configure RX MMCM */
	XVphy_MmcmStart(&Vphy, 0, XVPHY_DIR_RX);

	usleep(10000);
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamUpCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
	xil_printf("RX stream is up\r\n");
#if(LOOPBACK_MODE_EN != 1 && XPAR_XV_HDMITXSS_NUM_INSTANCES > 0)
	u32 Status;
	u64 LineRate;
	XVphy_PllType RxPllType;
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(HdmiRxSsPtr);
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);

	/* Copy video parameters */
	*HdmiTxSsVidStreamPtr = *HdmiRxSsVidStreamPtr;
	XV_HdmiTxSs_SetVideoIDCode(&HdmiTxSs,
				XV_HdmiRxSs_GetVideoIDCode(HdmiRxSsPtr));
	XV_HdmiTxSs_SetVideoStreamType(&HdmiTxSs,
				XV_HdmiRxSs_GetVideoStreamType(HdmiRxSsPtr));

	RxPllType = XVphy_GetPllType(&Vphy,
				     0,
				     XVPHY_DIR_RX,
				     XVPHY_CHANNEL_ID_CH1);

	if (Vphy.Config.XcvrType != XVPHY_GT_TYPE_GTPE2) {
		if (!(RxPllType == XVPHY_PLL_TYPE_CPLL)) {
			LineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN0 -
				XVPHY_CHANNEL_ID_CH1].LineRateHz;
		} else {
			LineRate = Vphy.Quads[0].Plls[0].LineRateHz;
		}
	} else { /* GTP */
		if (RxPllType == XVPHY_PLL_TYPE_PLL0) {
			LineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN0 -
				XVPHY_CHANNEL_ID_CH1].LineRateHz;
		} else {
			LineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN1 -
				XVPHY_CHANNEL_ID_CH1].LineRateHz;
		}
	}


	/* Check GT line rate
	 * For 4k60p the reference clock must be multiplied by four
	 */
	if ((LineRate / 1000000) > 3400) {
		/* TX reference clock */
		Vphy.HdmiTxRefClkHz = Vphy.HdmiRxRefClkHz * 4;
		XV_HdmiTxSs_SetTmdsClockRatio(&HdmiTxSs, 1);

		XV_HdmiTxSs_SetVideoStreamScramblingFlag(&HdmiTxSs, (TRUE));
	}

	/* Other resolutions */
	else {
		Vphy.HdmiTxRefClkHz = Vphy.HdmiRxRefClkHz;
		XV_HdmiTxSs_SetTmdsClockRatio(&HdmiTxSs, 0);

		XV_HdmiTxSs_SetVideoStreamScramblingFlag(&HdmiTxSs, (FALSE));
	}

	/* Set GT TX parameters */
	Status = XVphy_SetHdmiTxParam(&Vphy,
	                              0,
				      XVPHY_CHANNEL_ID_CHA,
				      HdmiTxSsVidStreamPtr->PixPerClk,
				      HdmiTxSsVidStreamPtr->ColorDepth,
				      HdmiTxSsVidStreamPtr->ColorFormatId);

	if (Status == XST_FAILURE) {
		return;
	}

	/* Disable the Colorbar */
	if (TxCableConnect) {
		/* Restart the TX if the TX Cable Connected */
		StartTxAfterRxFlag = (TRUE);

		/* Mute/Stop the HDMI RX SS Output
		 * Enable only when the downstream is ready
		 * (TPG & HDMI TX)
		 */
		XV_HdmiRxSs_VRST(HdmiRxSsPtr,TRUE);
	}
	/* Enable pass-through */
	IsPassThrough = (TRUE);
#else
	XVidC_ReportStreamInfo(&HdmiRxSsPtr->HdmiRxPtr->Stream.Video);

#endif

#ifdef VIDEO_FRAME_CRC_EN
	/* Reset Video Frame CRC */
	XVidFrameCrc_Reset();
#endif
}
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when the TX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxStreamUpCallback(void *CallbackRef) {

#if defined(XPAR_XV_HDMITXSS_NUM_INSTANCES)
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
#endif
	IsStreamUp = TRUE;

	XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;
	XVphy_PllType TxPllType;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(&HdmiRxSs);

	/* In passthrough copy the RX stream parameters to the TX stream */
	if (IsPassThrough) {
		XV_HdmiTxSs_SetVideoStream(HdmiTxSsPtr, *HdmiRxSsVidStreamPtr);
		/* If the Start TX after RX Flag is (TRUE), hence,
		 * no need to set thethe TX, let the StartTxAfterRX API
		 * executed which will trigger TX Stream up eventually
		 */
		if (StartTxAfterRxFlag) {
			return;
		}
	}
#endif

	/* Obtain the stream information:
	 * Notes: XV_HdmiTxSs_GetVideoStream are with updated value, either
	 * colorbar or pass-through
	 */
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	/* Check whether the sink is DVI/HDMI Supported */
	if (EdidHdmi20_t.EdidCtrlParam.IsHdmi == XVIDC_ISDVI) {
		if (HdmiTxSsVidStreamPtr->ColorDepth != XVIDC_BPC_8 ||
			HdmiTxSsVidStreamPtr->ColorFormatId != XVIDC_CSF_RGB) {
			xil_printf(ANSI_COLOR_YELLOW "Un-able to set TX "
						"stream, sink is DVI\r\n"
						ANSI_COLOR_RESET "\r\n");
			/* Clear TX busy flag */
			TxBusy = (FALSE);
			/* Don't set TX, if the Sink is DVI, but the source
			 * properties are:
			 *      - Color Depth more than 8 BPC
			 *      - Color Space not RGB
			 */
			return;
		} else {
			xil_printf(ANSI_COLOR_YELLOW "Set TX stream to DVI,"
				" sink is DVI\r\n" ANSI_COLOR_RESET "\r\n");
			XV_HdmiTxSs_AudioMute(HdmiTxSsPtr, TRUE);
			XV_HdmiTxSS_SetDviMode(HdmiTxSsPtr);
		}
	} else {
		XV_HdmiTxSS_SetHdmiMode(HdmiTxSsPtr);
		XV_HdmiTxSs_AudioMute(HdmiTxSsPtr, FALSE);
	}

	xil_printf("TX stream is up\r\n");

	/* Check for the 480i/576i during color bar mode
	 * When it's (TRUE), set the Info Frame Pixel Repetition to x2
	 */
	// if (!IsPassThrough) {
	// 	AVIInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(HdmiTxSsPtr);

	// 	if ( (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x480_60_I) ||
	// 	     (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x576_50_I) ) {
	// 		AVIInfoFramePtr->PixelRepetition =
	// 				XHDMIC_PIXEL_REPETITION_FACTOR_2;
	// 	} else {
	// 		AVIInfoFramePtr->PixelRepetition =
	// 				XHDMIC_PIXEL_REPETITION_FACTOR_1;
	// 	}
	// }
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	// else {
		/* Parse RX AVI Infoframe to TX AVI Infoframe
		 * as parameters from AVI Infoframe are used
		 * in TX Subsystem
		 */
		AVIInfoFramePtr = XV_HdmiRxSs_GetAviInfoframe(&HdmiRxSs);
		(void)memset((void *) &(HdmiTxSsPtr->AVIInfoframe),
				0x00, sizeof(XHdmiC_AVI_InfoFrame));
		memcpy (&(HdmiTxSsPtr->AVIInfoframe),
				AVIInfoFramePtr,
				sizeof(XHdmiC_AVI_InfoFrame));
	// }
#endif
	TxPllType = XVphy_GetPllType(&Vphy,
	                             0,
				     XVPHY_DIR_TX,
				     XVPHY_CHANNEL_ID_CH1);

	if ((TxPllType == XVPHY_PLL_TYPE_CPLL)) {
		TxLineRate = XVphy_GetLineRateHz(&Vphy,
		                                 0,
						 XVPHY_CHANNEL_ID_CH1);

	} else if((TxPllType == XVPHY_PLL_TYPE_QPLL) ||
			  (TxPllType == XVPHY_PLL_TYPE_QPLL0) ||
			  (TxPllType == XVPHY_PLL_TYPE_PLL0)) {
		TxLineRate =
			XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CMN0);
	} else {
		TxLineRate =
			XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CMN1);
	}

	/* Copy Sampling Rate */
	XV_HdmiTxSs_SetSamplingRate(HdmiTxSsPtr, Vphy.HdmiTxSampleRate);

#if defined(XPAR_XV_HDMITXSS_NUM_INSTANCES)
#if defined(USE_HDMI_AUDGEN)
	XhdmiACRCtrl_TMDSClkRatio(&AudioGen,
				HdmiTxSsPtr->HdmiTxPtr->Stream.TMDSClockRatio);
	/* Select the Audio source */
	if (IsPassThrough) {

		/* Disable audio generator */
		XhdmiAudGen_Start(&AudioGen, FALSE);

		/* Select ACR from RX */
		XhdmiACRCtrl_Sel(&AudioGen, ACR_SEL_IN);

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* Set TX Audio Channels (From HDMI RX Audio) */
		XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs,
				XV_HdmiRxSs_GetAudioChannels(&HdmiRxSs));
#endif
		/* Re-program audio clock */
		XhdmiAudGen_SetAudClk(&AudioGen, XAUD_SRATE_192K);
	} else {
		AudioInfoFramePtr = XV_HdmiTxSs_GetAudioInfoframe(&HdmiTxSs);

		/* Reset Audio InfoFrame */
		(void)memset((void *)AudioInfoFramePtr,
			0,
			sizeof(XHdmiC_AudioInfoFrame));

		/* Enable audio generator */
		XhdmiAudGen_Start(&AudioGen, TRUE);

		/* Select ACR from ACR Ctrl */
		XhdmiACRCtrl_Sel(&AudioGen, ACR_SEL_GEN);

		/* Enable 2-channel audio */
		XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs, 2);
		XhdmiAudGen_SetEnabChannels(&AudioGen, 2);
		XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_PING);
		XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
		XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs_GetTmdsClockFreqHz(HdmiTxSsPtr),
				   XAUD_SRATE_48K);
		/* Refer to CEA-861-D for Audio InfoFrame Channel Allocation
		 * - - - - - - FR FL
		 */
		AudioInfoFramePtr->ChannelAllocation = 0x0;
		/* Refer to Stream Header */
		AudioInfoFramePtr->SampleFrequency = 0x0;
	}
#endif
#endif

	ReportStreamMode(HdmiTxSsPtr, IsPassThrough);

#ifdef VIDEO_FRAME_CRC_EN
	/* Reset Video Frame CRC */
	XVidFrameCrc_Reset();
#endif

	/* Clear TX busy flag */
	TxBusy = (FALSE);

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	/* Release RX Video Bridge Reset */
	if (IsPassThrough) {
		XV_HdmiRxSs_VRST(&HdmiRxSs,FALSE);
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called when the TX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxStreamDownCallback(void *CallbackRef) {

	/* If the system in the Pass-through
	 * reset the AUX FIFO
	 */
	if (IsPassThrough) {
		ResetAuxFifo();
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* Mute/Stop the HDMI RX SS Output
		 * Enable only when the downstream is ready
		 * (TPG & HDMI TX)
		 */
		XV_HdmiRxSs_VRST(&HdmiRxSs,TRUE);
#endif
	}
	xil_printf("TX stream is down\r\n");
}

/*****************************************************************************/
/**
*
* This function is called to start the TX stream after the RX stream
* was up and running.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void StartTxAfterRx(void) {

	/* Clear the Start Tx After Rx Flag */
	StartTxAfterRxFlag = (FALSE);

	/* Disable TX TDMS clock */
#if (!XPAR_VPHY_0_USE_GT_CH4_HDMI)
	XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_TX, (FALSE));
#endif

	XV_HdmiTxSs_StreamStart(&HdmiTxSs);

	/* Enable RX clock forwarding */
	XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (TRUE));

	/* Program external clock generator in locked mode
	 * Only when the GT TX and RX are not coupled
	 */
	if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		I2cClk(Vphy.HdmiRxRefClkHz,Vphy.HdmiTxRefClkHz);
	} else {
		IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
	}

}
#endif
/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* HDMI cores. The function is application-specific since the actual system
* may or may not have an interrupt controller. The HDMI cores could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if interrupt setup was successful.
*   - A specific error code defined in "xstatus.h" if an error
*   occurs.
*
* @note   This function assumes a Microblaze system and no operating
*   system is used.
*
******************************************************************************/
int SetupInterruptSystem(void) {
	int Status;
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic *IntcInstPtr = &Intc;
#endif

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if(IntcCfgPtr == NULL) {
		xil_printf("ERR:: Interrupt Controller not found");
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr,
				IntcCfgPtr,
				IntcCfgPtr->CpuBaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\r\n");
		return XST_FAILURE;
	}

	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				(XScuGic *)IntcInstPtr);
#endif

	return (XST_SUCCESS);
}

void Xil_AssertCallbackRoutine(u8 *File, s32 Line) {
	xil_printf("Assertion in File %s, on line %0d\r\n", File, Line);
}

/*****************************************************************************/
/**
*
* Main function to call example with HDMI TX, HDMI RX and HDMI GT drivers.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if HDMI example was successfully.
*   - XST_FAILURE if HDMI example failed.
*
* @note   None.
*
******************************************************************************/
int main() {
	u32 Status = XST_FAILURE;
	XVphy_Config *XVphyCfgPtr;
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	AuxFifoStartFlag = FALSE;
	AuxFifoEndIndex = 0;
	AuxFifoStartIndex = 0;
	AuxFifoCount = 0;
#endif

	xil_printf("\r\n\r\n");
	xil_printf("--------------------------------------\r\n");
	xil_printf("---  HDMI SS + VPhy Example v%d.%d   ---\r\n",
			APP_MAJ_VERSION, APP_MIN_VERSION);
	xil_printf("---  (c) 2018 by Xilinx, Inc.      ---\r\n");
	xil_printf("--------------------------------------\r\n");
	xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
	xil_printf("--------------------------------------\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	StartTxAfterRxFlag = (FALSE);

#if(LOOPBACK_MODE_EN != 1)
	TxBusy            = (TRUE);
#endif
#endif
	VphyErrorFlag = FALSE;
	VphyPllLayoutErrorFlag = FALSE;

	/* Start in color bar */
	IsPassThrough = (FALSE);

	/* Initialize platform */
	init_platform();

	/* Initialize external clock generator */
	usleep(200000);
	IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);

#if defined(XPAR_XV_HDMITXSS_NUM_INSTANCES)
#if defined(USE_HDMI_AUDGEN)
	/* Initialize the Audio Generator */
	XhdmiAudGen_Init(&AudioGen,
			XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR,
			XPAR_AUDIO_SS_0_HDMI_ACR_CTRL_BASEADDR,
			XPAR_AUDIO_SS_0_CLK_WIZ_BASEADDR);
#endif
#endif

	/* Initialize IRQ */
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\r\n\r\n");
		return XST_FAILURE;
	}

#ifdef VIDEO_FRAME_CRC_EN
	XVidFrameCrc_Initialize(&VidFrameCRC);
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES

	/* Initialize HDMI TX Subsystem */

	XV_HdmiTxSs_ConfigPtr =
		XV_HdmiTxSs_LookupConfig(XPAR_XV_HDMITX_0_DEVICE_ID);

	if(XV_HdmiTxSs_ConfigPtr == NULL) {
		HdmiTxSs.IsReady = 0;
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_HdmiTxSs_CfgInitialize(&HdmiTxSs, XV_HdmiTxSs_ConfigPtr,
					XV_HdmiTxSs_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf
		       ("ERR:: HDMI TX Subsystem Initialization failed %d\r\n",
		        Status);
	}

	/* Set the Application version in TXSs driver structure */
	XV_HdmiTxSS_SetAppVersion(&HdmiTxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	/* Register HDMI TX SS Interrupt Handler with Interrupt Controller */
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_HdmiTxIntrHandler,
			(void *)&HdmiTxSs);
#endif
#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_VEC_ID);
#endif
#endif
	} else {
		xil_printf
			("ERR:: Unable to register HDMI TX interrupt handler");
		xil_printf("HDMI TX SS initialization error\r\n");
		return XST_FAILURE;
	}

	/* HDMI TX SS callback setup */
	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_CONNECT,
				(void *)TxConnectCallback,
				(void *)&HdmiTxSs);
	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_TOGGLE,
				(void *)TxToggleCallback,
				(void *)&HdmiTxSs);
	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_VS,
				(void *)TxVsCallback,
				(void *)&HdmiTxSs);
	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_STREAM_UP,
				(void *)TxStreamUpCallback,
				(void *)&HdmiTxSs);
	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_STREAM_DOWN,
				(void *)TxStreamDownCallback,
				(void *)&HdmiTxSs);
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES

	/* Initialize HDMI RX Subsystem */

	/* Get User Edid Info */
	XV_HdmiRxSs_SetEdidParam(&HdmiRxSs, (u8*)&Edid, sizeof(Edid));
	XV_HdmiRxSs_ConfigPtr =
		XV_HdmiRxSs_LookupConfig(XPAR_XV_HDMIRX_0_DEVICE_ID);

	if(XV_HdmiRxSs_ConfigPtr == NULL) {
		HdmiRxSs.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_HdmiRxSs_CfgInitialize(&HdmiRxSs,
					XV_HdmiRxSs_ConfigPtr,
					XV_HdmiRxSs_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf
		("ERR:: HDMI RX Subsystem Initialization failed %d\r\n",
		Status);
		return(XST_FAILURE);
	}

	/* Set the Application version in RXSs driver structure */
	XV_HdmiRxSS_SetAppVersion(&HdmiRxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	/* Register HDMI RX SS Interrupt Handler with Interrupt Controller */
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_HdmiRxIntrHandler,
			(void *)&HdmiRxSs);
#endif
#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
		XScuGic_Enable(&Intc,
				XPAR_FABRIC_V_HDMIRXSS_0_VEC_ID);
#endif
#endif
	} else {
		xil_printf
			("ERR:: Unable to register HDMI RX interrupt handler");
		xil_printf("HDMI RX SS initialization error\r\n");
		return XST_FAILURE;
	}

	/* RX callback setup */
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_CONNECT,
				(void *)RxConnectCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_AUX,
				(void *)RxAuxCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_AUD,
				(void *)RxAudCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_LNKSTA,
				(void *)RxLnkStaCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_STREAM_DOWN,
				(void *)RxStreamDownCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_STREAM_INIT,
				(void *)RxStreamInitCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_STREAM_UP,
				(void *)RxStreamUpCallback,
				(void *)&HdmiRxSs);
#endif

	/*
	 *  Initialize Video PHY
	 *  The GT needs to be initialized after the HDMI RX and TX.
	 *  The reason for this is the GtRxInitStartCallback
	 *  calls the RX stream down callback.
	 *
         */
	XVphyCfgPtr = XVphy_LookupConfig(XPAR_VPHY_0_DEVICE_ID);
	if (XVphyCfgPtr == NULL) {
		xil_printf("Video PHY device not found\r\n\r\n");
		return XST_FAILURE;
	}

	/* Register VPHY Interrupt Handler */
#if defined(__arm__) || (__aarch64__)
	Status = XScuGic_Connect(&Intc,
				XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR,
				(XInterruptHandler)XVphy_InterruptHandler,
				(void *)&Vphy);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("HDMI VPHY Interrupt Vec ID not found!\r\n");
		return XST_FAILURE;
	}

	/* Initialize HDMI VPHY */
	Status = XVphy_Hdmi_CfgInitialize(&Vphy, 0, XVphyCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("HDMI VPHY initialization error\r\n");
		return XST_FAILURE;
	}

	/* Enable VPHY Interrupt */
#if defined(__arm__) || (__aarch64__)
	XScuGic_Enable(&Intc,
			XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR);
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	/* VPHY callback setup */
	XVphy_SetHdmiCallback(&Vphy,
				XVPHY_HDMI_HANDLER_TXINIT,
				(void *)VphyHdmiTxInitCallback,
				(void *)&Vphy);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XVphy_SetHdmiCallback(&Vphy,
				XVPHY_HDMI_HANDLER_RXINIT,
				(void *)VphyHdmiRxInitCallback,
				(void *)&Vphy);
	XVphy_SetHdmiCallback(&Vphy,
				XVPHY_HDMI_HANDLER_RXREADY,
				(void *)VphyHdmiRxReadyCallback,
				(void *)&Vphy);
#endif

	XVphy_SetErrorCallback(&Vphy,
				(void *)VphyErrorCallback,
				(void *)&Vphy);

	xil_printf("---------------------------------\r\n");

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	/* Initialize menu */
	XHdmi_MenuInitialize(&HdmiMenu, UART_BASEADDR);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	/* Start with 1080p stream */
	TxInfoFrameReset();
	XV_HdmiTxSs_SetStream(
		&HdmiTxSs,
		XVIDC_VM_1920x1080_60_P,
		XVIDC_CSF_RGB,
		XVIDC_BPC_8,
		NULL);
#endif

	/* Enable Scrambling Override
	 * Note: Setting the override to TRUE will allow scrambling to be
	 *       disabled for video where TMDS Clock > 340 MHz which breaks the
	 *       HDMI Specification
	 * E.g.:
	 *   XV_HdmiTxSs_SetVideoStreamScramblingOverrideFlag(&HdmiTxSs, TRUE);
	 */

	/* Main loop */
	do {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		SinkReady = SinkReadyCheck(&HdmiTxSs, &EdidHdmi20_t);

		if (StartTxAfterRxFlag && SinkReady) {
			StartTxAfterRx();
		}

		if (IsStreamUp && SinkReady) {
			IsStreamUp = FALSE;

			i2c_dp159(&Vphy, 0, TxLineRate);
			XVphy_Clkout1OBufTdsEnable
				(&Vphy, XVPHY_DIR_TX, (TRUE));
		}
#endif

		/* HDMI Menu */
		XHdmi_MenuProcess(&HdmiMenu);

		/* VPHY error */
		VphyProcessError();

	}
	while (1);

	return 0;
}
