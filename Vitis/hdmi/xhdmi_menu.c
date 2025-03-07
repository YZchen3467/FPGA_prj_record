/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_menu.h"
#include "xhdcp.h"
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#endif
#include "xvidc_edid_ext.h"

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
/* Create entry for each mode in the custom table */
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode
	[(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] = {
	/* Custom Modes . */
	{
		XVIDC_VM_1152x864_60_P, "1152x864@60Hz", XVIDC_FR_60HZ,
		{
			1152, 64, 120, 184, 1520, 0,
			864, 3, 4, 26, 897, 0, 0, 0, 0, 1
		}
	}
};
#endif
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/* Pointer to the menu handling functions */
typedef XHdmi_MenuType XHdmi_MenuFuncType(XHdmi_Menu *InstancePtr, u8 Input);

/************************** Function Prototypes ******************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_AudioChannelMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif

static void XHdmi_DisplayMainMenu(void);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static void XHdmi_DisplayEdidMenu(void);
static void XHdmi_DisplayAudioMenu(void);
static void XHdmi_DisplayAudioChannelMenu(void);
static void XHdmi_DisplayVideoMenu(void);
#endif
extern void Info(void);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern void XV_HdmiTxSs_ShowEdid(XV_HdmiTxSs *InstancePtr);
extern void CloneTxEdid(void);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
extern void XV_HdmiRxSs_LoadEdid(XV_HdmiRxSs *InstancePtr, u8 *EdidData, u16 Length);
extern void XV_HdmiRxSs_ToggleRxHpd(XV_HdmiRxSs *InstancePtr);
extern void HDCPXILCMD_ProcessKey(char theCmdKey);
#endif

/************************* Variable Definitions *****************************/

/**
* This table contains the function pointers for all possible states.
* The order of elements must match the XHdmi_MenuType enumerator definitions.
*/
static XHdmi_MenuFuncType* const XHdmi_MenuTable[XHDMI_NUM_MENUS] = {
	XHdmi_MainMenu,
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XHdmi_EdidMenu,
	XHdmi_AudioMenu,
	XHdmi_AudioChannelMenu,
	XHdmi_VideoMenu,
#endif
};

extern XVphy Vphy;               /* VPhy structure */
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern XV_HdmiTxSs HdmiTxSs;       /* HDMI TX SS structure */
extern XhdmiAudioGen_t AudioGen;

extern u8 TxCableConnect;
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
extern XV_HdmiRxSs HdmiRxSs;       /* HDMI RX SS structure */
#endif
extern u8 IsPassThrough;         /**< Demo mode 0-colorbar 1-pass through */
extern u8 TxBusy;                // TX busy flag. This flag is set while the TX is initialized
// extern XHdcp_Repeater HdcpRepeater;

/*HDMI EDID*/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern EdidHdmi20 EdidHdmi20_t;
#endif
extern u8 Buffer[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function takes care of the HDMI menu initialization.
*
* @param InstancePtr is a pointer to the XHdmi_Menu instance.
*
* @return None
*
*
******************************************************************************/
void XHdmi_MenuInitialize(XHdmi_Menu *InstancePtr, u32 UartBaseAddress)
{
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	u32 Status;
#endif
#endif
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->CurrentMenu = XHDMI_MAIN_MENU;
	InstancePtr->UartBaseAddress = UartBaseAddress;
	InstancePtr->Value = 0;
	InstancePtr->WaitForColorbar = (FALSE);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	//Initialize and Add Custom Resolution in to the Video Table
	//Added for the resolution menu
	/* Example : User registers custom timing table */
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
			 (XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\n\r\n");
	}
#endif
#endif

	// Show main menu
	XHdmi_DisplayMainMenu();
}


/*****************************************************************************/
/**
*
* This function resets the menu to the main menu.
*
* @param InstancePtr is a pointer to the XHdmi_Menu instance.
*
* @return None
*
*
******************************************************************************/
void XHdmi_MenuReset(XHdmi_Menu *InstancePtr)
{
	InstancePtr->CurrentMenu = XHDMI_MAIN_MENU;
}

/*****************************************************************************/
/**
*
* This function displays the HDMI main menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayMainMenu(void)
{
	xil_printf("\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("---   MAIN MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("i - Info\r\n");
	xil_printf("       => Shows information about the HDMI RX stream, HDMI TX stream, \r\n");
	xil_printf("          GT transceivers and PLL settings.\r\n");
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("p - Pass-through\r\n");
	xil_printf("       => Passes the sink input to source output.\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("p - Toggle HPD\r\n");
	xil_printf("       => Toggles the HPD of HDMI RX.\r\n");
#endif
	xil_printf("z - GT & HDMI TX/RX log\r\n");
	xil_printf("       => Shows log information for GT & HDMI TX/RX.\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	xil_printf("e - Edid\r\n");
	xil_printf("       => Display and set edid.\r\n");
	xil_printf("a - Audio\r\n");
	xil_printf("       => Audio options.\r\n");
	xil_printf("v - Video\r\n");
	xil_printf("       => Video pattern options.\r\n");
	xil_printf("m - Set HDMI Mode\r\n");
	xil_printf("n - Set DVI Mode\r\n");
#endif

	xil_printf("\r\n\r\n");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI main menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_MAIN_MENU;

	switch (Input) {
			// Info
		case ('i') :
		case ('I') :
			Info();
			Menu = XHDMI_MAIN_MENU;
			break;

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			// Pass-through
		case ('p') :
		case ('P') :
			// Check if a source is connected
			if (HdmiRxSs.IsStreamConnected == (TRUE)) {
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
				xil_printf("Force pass-through\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
				xil_printf("Toggle HDMI RX HPD\r\n");
#endif
				ToggleHdmiRxHpd(&Vphy, &HdmiRxSs);
			}// No source
			else {
				xil_printf(ANSI_COLOR_YELLOW "No source device detected.\r\n"
							ANSI_COLOR_RESET);
			}
			Menu = XHDMI_MAIN_MENU;
			break;
#endif

			// GT & HDMI TX/RX log
		case ('z') :
		case ('Z') :
			XVphy_LogDisplay(&Vphy);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_LogDisplay(&HdmiTxSs);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_LogDisplay(&HdmiRxSs);
#endif
			Menu = XHDMI_MAIN_MENU;
			break;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			// HDMI Mode
		case ('m') :
		case ('M') :
			xil_printf("Set TX Mode To HDMI.\r\n");
			XV_HdmiTxSS_SetHdmiMode(&HdmiTxSs);
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, FALSE);
			Menu = XHDMI_MAIN_MENU;
			break;

			// DVI Mode
		case ('n') :
		case ('N') :
			xil_printf("Set TX Mode To DVI .\r\n");
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, TRUE);
			XV_HdmiTxSS_SetDviMode(&HdmiTxSs);
			Menu = XHDMI_MAIN_MENU;
			break;
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			// Edid
		case ('e') :
		case ('E') :
			XHdmi_DisplayEdidMenu();
			Menu = XHDMI_EDID_MENU;
			break;

			// Audio
		case ('a') :
		case ('A') :
			XHdmi_DisplayAudioMenu();
			Menu = XHDMI_AUDIO_MENU;
			break;

			// Video
		case ('v') :
		case ('V') :
			XHdmi_DisplayVideoMenu();
			Menu = XHDMI_VIDEO_MENU;
			break;
#endif

		default :
			XHdmi_DisplayMainMenu();
			Menu = XHDMI_MAIN_MENU;
			break;
	}

	return Menu;
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function displays the HDMI edid menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayEdidMenu(void) {
	xil_printf("\r\n");
	SinkCapabilityCheck(&EdidHdmi20_t);
	SinkCapWarningMsg(&EdidHdmi20_t);
	xil_printf("---------------------\r\n");
	xil_printf("---   EDID MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("  1 - Display the edid of the connected sink device.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	xil_printf("  2 - Clone the edid of the connected sink edid to HDMI Rx.\r\n");
	xil_printf("  3 - Load default edid to HDMI Rx.\r\n");
#endif
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI edid menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	u8 Buffer[256];
	int Status = XST_FAILURE;

	// Default
	Menu = XHDMI_EDID_MENU;
	switch (Input) {

			// Show source edid
		case 1 :
			XV_HdmiTxSs_ShowEdid(&HdmiTxSs);
			// Read TX edid
			xil_printf("\r\n");

			Status = XV_HdmiTxSs_ReadEdid(&HdmiTxSs, (u8*)&Buffer);
			/* Only Parse the EDID when the Read EDID success */
			if (Status == XST_SUCCESS) {
				XV_VidC_parse_edid((u8*)&Buffer,
									&EdidHdmi20_t.EdidCtrlParam,
									XVIDC_VERBOSE_ENABLE);
			} else {
				xil_printf(ANSI_COLOR_YELLOW "EDID parsing has failed.\r\n"
							ANSI_COLOR_RESET);
			}
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			// Clone edid
		case 2 :
			CloneTxEdid();
			break;

			// Load edid
		case 3 :
			XV_HdmiRxSs_LoadDefaultEdid(&HdmiRxSs);
			/* Toggle HPD after loading new HPD */
			ToggleHdmiRxHpd(&Vphy, &HdmiRxSs);
			xil_printf("HPD is toggled.\r\n");
			break;
#endif

			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayEdidMenu();
			break;
	}


	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the video menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayVideoMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("---   VIDEO MENU   ---\r\n");
	xil_printf("----------------------\r\n");
#if (XPAR_XV_TPG_0_COLOR_BAR == 1)
	xil_printf("  1 - Color bars\r\n");
#endif
	xil_printf(" 10 - Checker board\r\n");
	xil_printf(" 11 - Cross hatch\r\n");
	xil_printf(" 12 - Noise\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the video menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_VIDEO_MENU;

	// Insert carriage return
	xil_printf("\r\n");

	switch (Input) {
			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayVideoMenu();
			break;
	}

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the audio menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayAudioMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("---   AUDIO MENU   ---\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("  1 - Mute audio.\r\n");
	xil_printf("  2 - Unmute audio.\r\n");
	xil_printf("  3 - Configure audio channels.\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI audio menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_AUDIO_MENU;

	switch (Input) {
			// Mute
		case 1 :
			xil_printf("Mute audio.\r\n");
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, TRUE);
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

			// Unmute
		case 2 :
			xil_printf("Unmute audio.\r\n");
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, FALSE);
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

			// Audio channels
		case 3 :
			xil_printf("Display Audio Channels menu.\r\n");
			XHdmi_DisplayAudioChannelMenu();
			Menu = XHDMI_AUDIO_CHANNEL_MENU;
			break;

			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayAudioMenu();
			break;
	}

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the audio channel menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayAudioChannelMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("- AUDIO CHANNEL MENU -\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("  1 - 2 Audio Channels.\r\n");
	xil_printf("  2 - 8 Audio Channels.\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI audio channel menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_AudioChannelMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	XHdmiC_AudioInfoFrame *AudioInfoframePtr;

	AudioInfoframePtr = XV_HdmiTxSs_GetAudioInfoframe(&HdmiTxSs);

	// Default
	Menu = XHDMI_AUDIO_CHANNEL_MENU;

	switch (Input) {
			// 2 Audio Channels
		case 1 :
			print("2 Audio Channels.\r\n");
			XhdmiAudGen_SetEnabChannels(&AudioGen, 2);
			XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
			XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs, 2);
			// Refer to CEA-861-D for Audio InfoFrame Channel Allocation
			// - - - - - - FR FL
			AudioInfoframePtr->ChannelAllocation = 0x0;
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

			// 8 Audio Channels
		case 2 :
			print("8 Audio Channels.\r\n");
			XhdmiAudGen_SetEnabChannels(&AudioGen, 8);
			XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
			XhdmiAudGen_SetPattern(&AudioGen, 3, XAUD_PAT_RAMP);
			XhdmiAudGen_SetPattern(&AudioGen, 4, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 5, XAUD_PAT_PING);
			XhdmiAudGen_SetPattern(&AudioGen, 6, XAUD_PAT_RAMP);
			XhdmiAudGen_SetPattern(&AudioGen, 7, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 8, XAUD_PAT_PING);
			XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs, 8);
			// Refer to CEA-861-D for Audio InfoFrame Channel Allocation
			// RRC RLC RR RL FC LFE FR FL
			AudioInfoframePtr->ChannelAllocation = 0x13;
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

			// Exit
		case 99 :
			xil_printf("Returning to audio menu.\r\n");
			Menu = XHDMI_AUDIO_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayAudioChannelMenu();
			break;
	}

	return Menu;
}
#endif

/*****************************************************************************/
/**
*
* This function is called to trigger the HDMI menu statemachine.
*
* @param InstancePtr is a pointer to the XHdmi_Menu instance.
*
* @param input is the value used for the next menu state decoder.
*
* @return None
*
******************************************************************************/
void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr) {
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	if ((InstancePtr->WaitForColorbar) && (!TxBusy)) {
		InstancePtr->WaitForColorbar = (FALSE);
		xil_printf("Enter Selection -> ");
	}
#endif

	// Check if the uart has any data
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	else if (XUartPs_IsReceiveData(InstancePtr->UartBaseAddress)) {
#endif
		// Read data from uart
		Data = XUartPs_RecvByte(InstancePtr->UartBaseAddress);

		// Main menu
		if (InstancePtr->CurrentMenu == XHDMI_MAIN_MENU) {
			InstancePtr->CurrentMenu = XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, Data);
			InstancePtr->Value = 0;
		}

		// Sub menu
		else {
			// Send response to user
			XUartPs_SendByte(InstancePtr->UartBaseAddress, Data);

			// Alpha numeric data
			if (isalpha(Data)) {
				xil_printf("Invalid input. Valid entry is only digits 0-9. Try again\r\n\r\n");
				xil_printf("Enter Selection -> ");
				InstancePtr->Value = 0;
			}

			// Numeric data
			else if ((Data >= '0') && (Data <= '9')) {
				InstancePtr->Value = InstancePtr->Value * 10 + (Data-'0');
			}

			// Backspace
			else if (Data == '\b' || Data == 127) {
				InstancePtr->Value = InstancePtr->Value / 10; //discard previous input
			}

			// Execute
			else if ((Data == '\n') || (Data == '\r')) {
				xil_printf("\r\n");
				InstancePtr->CurrentMenu = XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, InstancePtr->Value);
				InstancePtr->Value = 0;
			}
		}
	}
}
