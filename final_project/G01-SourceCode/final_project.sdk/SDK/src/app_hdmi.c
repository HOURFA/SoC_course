// SPDX-License-Identifier: GPL-2.0
/*
 * app_hdmi.c
 *
 *  Created on: 07 Jan 2018
 *      Author: Florent Werbrouck
 */

#include "app_hdmi.h"
/* Peripheral IP driver Instance */
XV_tpg tpg_inst;
XVtc VtcInst;
XVprocSs VprocInst;

XScuGic_Config *IntcConfig;
XScuGic Intc;



XGpio BTN_XGpio;
static FIL fil;		/* File object */
static FATFS fatfs;

const VideoTimingClk_t VideoTimingClk[SUPPORTED_VIDEO_FORMATS]=
{
		{XVIDC_VM_1920x1080_60_P, 	CLKWIZ_CLKOUT0_148_5_MHz},
		{XVIDC_VM_1280x1024_60_P,	CLKWIZ_CLKOUT0_108_MHz},
		{XVIDC_VM_1280x960_60_P,	CLKWIZ_CLKOUT0_108_MHz},
		{XVIDC_VM_1280x720_60_P, 	CLKWIZ_CLKOUT0_74_25_MHz},
		{XVIDC_VM_1024x768_60_P,	CLKWIZ_CLKOUT0_65_MHz},
		{XVIDC_VM_800x600_60_P,		CLKWIZ_CLKOUT0_40_MHz},
		{XVIDC_VM_640x480_60_P,		CLKWIZ_CLKOUT0_25_175MHz}
};
const  int ColorType[SUPPORTED_VIDEO_FORMATS]=
{
		XVIDC_CSF_RGB,
		XVIDC_CSF_YCRCB_444,
		XVIDC_CSF_YCRCB_422,
		XVIDC_CSF_YCRCB_420,
		XVIDC_CSF_YONLY
};
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function configures the TPG core.
* @param
* @param
*
*
******************************************************************************/
void configure_tpg(XV_tpg *tpg_ptr, tpg_config_t *tpg_config)
{
	// Set Resolution
    XV_tpg_Set_height(tpg_ptr, tpg_config->height);
    XV_tpg_Set_width(tpg_ptr, tpg_config->width);

    // Set Color Space
    XV_tpg_Set_colorFormat(tpg_ptr, tpg_config->colorFormat);

    // Change the pattern to color bar
    XV_tpg_Set_bckgndId(tpg_ptr, tpg_config->bckgndId);

    if(tpg_config->overlay_en)
    {
    	// Set Overlay to moving box
		// Set the size of the box
		XV_tpg_Set_boxSize(tpg_ptr, tpg_config->boxSize);
		// Set the speed of the box
		XV_tpg_Set_motionSpeed(tpg_ptr, tpg_config->motionSpeed);
    }

    XV_tpg_Set_ovrlayId(tpg_ptr, tpg_config->overlay_en);
}

/*****************************************************************************/
/**
*
* This function configures the VPSS core.
* @param
* @param
*
*
******************************************************************************/
int configure_vpss(XVprocSs *InstancePtr, const XVidC_VideoStream *StrmIn,
					const XVidC_VideoStream *StrmOut)
{
	int Status;

	XVprocSs_SetVidStreamIn(InstancePtr, StrmIn);
	XVprocSs_SetVidStreamOut(InstancePtr, StrmOut);
	Status = XVprocSs_SetSubsystemConfig(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures the VDMA core.
* @param
* @param
*
*
******************************************************************************/
void configure_vdma(UINTPTR BaseAddr,const XVidC_VideoStream *Strm, u32 write_bytePerPixels,u32 read_bytePerPixels)
{

	u32 width, height;

	width = Strm->Timing.HActive;
	height = Strm->Timing.VActive;

	/* Start of VDMA Configuration */
    /* Configure the Write interface (S2MM)*/
    // S2MM Control Register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x30, 0x8B);
    //S2MM Start Address 1
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xAC, FRAME_BUFFER_1);
    //S2MM Start Address 2
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB0, FRAME_BUFFER_2);
    //S2MM Start Address 3
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB4, FRAME_BUFFER_3);
    //S2MM Frame delay / Stride register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA8, width*write_bytePerPixels);
    // S2MM HSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA4, width*write_bytePerPixels);
    // S2MM VSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA0, height);

    /* Configure the Read interface (MM2S)*/
    // MM2S Control Register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x8B);
    // MM2S Start Address 1
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x5C, FRAME_BUFFER_1);
    // MM2S Start Address 2
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x60, FRAME_BUFFER_2);
    // MM2S Start Address 3
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x64, FRAME_BUFFER_3);
    // MM2S Frame delay / Stride register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x58, width*read_bytePerPixels);
    // MM2S HSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x54, width*read_bytePerPixels);
    // MM2S VSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x50, height);
}

/*****************************************************************************/
/**
*
* This function configures the VTC Generator core.
* @param
* @param
*
*
******************************************************************************/
void configure_vtc_gen(XVtc *InstancePtr, const XVidC_VideoStream *Strm)
{
	XVtc_Reset(InstancePtr);
	XVtc_DisableGenerator(InstancePtr);
//	XVtc_Disable(InstancePtr);

	XVtc_Timing XVtc_Timingconf;

	XVtc_Timingconf.HActiveVideo 	= Strm->Timing.HActive;
	XVtc_Timingconf.HBackPorch		= Strm->Timing.HBackPorch;
	XVtc_Timingconf.HFrontPorch		= Strm->Timing.HFrontPorch;
	XVtc_Timingconf.HSyncPolarity	= Strm->Timing.HSyncPolarity;
	XVtc_Timingconf.HSyncWidth		= Strm->Timing.HSyncWidth;
	XVtc_Timingconf.Interlaced		= Strm->IsInterlaced;
	XVtc_Timingconf.V0BackPorch		= Strm->Timing.F0PVBackPorch;
	XVtc_Timingconf.V0FrontPorch	= Strm->Timing.F0PVFrontPorch;
	XVtc_Timingconf.V0SyncWidth		= Strm->Timing.F0PVSyncWidth;
	XVtc_Timingconf.V1BackPorch		= Strm->Timing.F1VBackPorch;
	XVtc_Timingconf.V1FrontPorch	= Strm->Timing.F1VFrontPorch;
	XVtc_Timingconf.V1SyncWidth		= Strm->Timing.F1VSyncWidth;
	XVtc_Timingconf.VActiveVideo	= Strm->Timing.VActive;
	XVtc_Timingconf.VSyncPolarity	= Strm->Timing.VSyncPolarity;

	//Configure the VTC
	//XVtc_ConvVideoMode2Timing(&VtcInst,XVTC_VMODE_576P,&XVtc_Timingconf);
	XVtc_SetGeneratorTiming(&VtcInst, &XVtc_Timingconf);
	XVtc_RegUpdate(InstancePtr);

	//Start the VTC generator
//	XVtc_Enable(InstancePtr);
	XVtc_EnableGenerator(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function configures the input stream.
* @param
* @param
*
*
******************************************************************************/
void set_input_resolution(app_periphs_t *periphs_ptr, int resID, int mode)
{

	const XVidC_VideoTimingMode *VmPtr;
	int   color_type = APP_COLOR_FORMAT;

	//Set Stream In
	switch (mode)
	{
		case 1:
			VmPtr=XVidC_GetVideoModeData(VideoTimingClk[resID].VmId);
			periphs_ptr->video_pipe_config.Stream_in.Timing = VmPtr->Timing;
			periphs_ptr->video_pipe_config.Stream_in.VmId = VmPtr->VmId;
			periphs_ptr->video_pipe_config.Stream_in.FrameRate = VmPtr->FrameRate;
			break;
		case 2:
			color_type = ColorType[resID];
			periphs_ptr->video_pipe_config.Stream_in.ColorFormatId = color_type;
			break;
	}
	periphs_ptr->video_pipe_config.Stream_in.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_in.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_in.IsInterlaced = 0;

	configure_vpss(periphs_ptr->Vproc_ptr, &periphs_ptr->video_pipe_config.Stream_in,&periphs_ptr->video_pipe_config.Stream_out);
	display_vpss_log(periphs_ptr->Vproc_ptr);

}
void set_resolution(app_periphs_t *periphs_ptr, int resID, int mode)
{

	const XVidC_VideoTimingMode *VmPtr;
	VmPtr=XVidC_GetVideoModeData(VideoTimingClk[resID].VmId);
	periphs_ptr->video_pipe_config.Stream_in.Timing = periphs_ptr->video_pipe_config.Stream_out.Timing = VmPtr->Timing;
	periphs_ptr->video_pipe_config.Stream_in.VmId = periphs_ptr->video_pipe_config.Stream_out.VmId = VmPtr->VmId;
	periphs_ptr->video_pipe_config.Stream_in.FrameRate = periphs_ptr->video_pipe_config.Stream_out.FrameRate = VmPtr->FrameRate;
	//Configure the TPG
	periphs_ptr->tpg_config.height = periphs_ptr->video_pipe_config.Stream_in.Timing.VActive;
	periphs_ptr->tpg_config.width = periphs_ptr->video_pipe_config.Stream_in.Timing.HActive;
	//Configure the CLKOUT0 DIV
	ClkWiz_Set_Output_Clock(XPAR_CLK_WIZ_0_BASEADDR, VideoTimingClk[resID].clkFreq);
	/* End of clocking wizard configuration */
	configure_vtc_gen(periphs_ptr->Vtc_ptr,&periphs_ptr->video_pipe_config.Stream_out);

	configure_vpss(periphs_ptr->Vproc_ptr, &periphs_ptr->video_pipe_config.Stream_in,&periphs_ptr->video_pipe_config.Stream_out);


}
/*****************************************************************************/
/**
*
* This function configures the out stream.
* @param
* @param
*
*
******************************************************************************/
void set_output_resolution(app_periphs_t *periphs_ptr, int resID, int mode)
{

	const XVidC_VideoTimingMode *VmPtr ;
	int   color_type = XVIDC_CSF_YCRCB_422;

	//Set Stream In
	switch (mode)
	{
		case 1:
			VmPtr=XVidC_GetVideoModeData(VideoTimingClk[resID].VmId);
			periphs_ptr->video_pipe_config.Stream_out.Timing = VmPtr->Timing;
			periphs_ptr->video_pipe_config.Stream_out.VmId = VmPtr->VmId;
			periphs_ptr->video_pipe_config.Stream_out.FrameRate = VmPtr->FrameRate;
			break;
		case 2:
			color_type = ColorType[resID];
			periphs_ptr->video_pipe_config.Stream_out.ColorFormatId = color_type;
			break;
	}
	//Set Stream Out
	periphs_ptr->video_pipe_config.Stream_out.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_out.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_out.IsInterlaced = 0;

	configure_vpss(periphs_ptr->Vproc_ptr, &periphs_ptr->video_pipe_config.Stream_in,&periphs_ptr->video_pipe_config.Stream_out);
	display_vpss_log(periphs_ptr->Vproc_ptr);
}
/*****************************************************************************/
/**
*
* This function configure the clock output of the clocking wizard
* @param
* @param
* @param
*
*
******************************************************************************/
void ClkWiz_Set_Output_Clock(UINTPTR ClkWiz_BaseAddr, u8 outClockFreq)
{
	int status;

	//Soft Reset is required from 2018.3
//	Xil_Out32(ClkWiz_BaseAddr, 0xA);
//	usleep(800);
	xil_printf("ClkWiz_output_Clock : ");
	switch(outClockFreq)
	{
		case CLKWIZ_CLKOUT0_148_5_MHz:
			//Set clock to 148.5 MHz
			// [7:0] = DIVCLK_DIVIDE = 8 (0x8)
			// [15:8] = CLKFBOUT_MULT = 37 (0x25)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D2508);
			// DIV = 6.250 (0x06.0xFA)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0xFA06);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("148.5MHz\r\n");
			break;

		case CLKWIZ_CLKOUT0_108_MHz:
			//Set clock to 108 MHz
			// [7:0] = DIVCLK_DIVIDE = 2 (0x2)
			// [15:8] = CLKFBOUT_MULT = 10 (0x0A)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D0A02);
			// DIV = 9.375 (0x09.0x177)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x17709);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("108MHz\r\n");
			break;

		case CLKWIZ_CLKOUT0_74_25_MHz:
			//Set clock to 74.25 MHz
			// [7:0] = DIVCLK_DIVIDE = 8 (0x8)
			// [15:8] = CLKFBOUT_MULT = 37 (0x25)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D2508);
			// DIV = 12.500 (0x0C.0x1FA)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x1FA0C);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("74.25MHz\r\n");
			break;

		case CLKWIZ_CLKOUT0_74_MHz:
			//Set clock to 74 MHz
			// [7:0] = DIVCLK_DIVIDE = 10 (0xA)
			// [15:8] = CLKFBOUT_MULT = 50 (0x32)
			// [25:16] = CLKFBOUT_FRAC = 875 (0x36B)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x36B320A);
			// DIV = 13.750 (0x0D.0x2EE)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x2EE0D);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("74MHz\r\n");
			break;

		case CLKWIZ_CLKOUT0_65_MHz:
			//Set clock to 65 MHz
			// [7:0] = DIVCLK_DIVIDE = 10 (0x0A)
			// [15:8] = CLKFBOUT_MULT = 50 (0x32)
			// [25:16] = CLKFBOUT_FRAC = 375 (0x177)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x177320A);
			// DIV = 15.500 (0x0F.0x1F4)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x1F40F);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("65MHz\r\n");
			break;

		case CLKWIZ_CLKOUT0_40_MHz:
			//Set clock to 40 MHz
			// [7:0] = DIVCLK_DIVIDE = 1 (0x01)
			// [15:8] = CLKFBOUT_MULT = 5 (0x05)
			// [25:16] = CLKFBOUT_FRAC = 0 (0x0)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x501);
			// DIV = 25.000 (0x19.0x00)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x19);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("40MHz\r\n");
			break;

		case CLKWIZ_CLKOUT0_27_MHz:
			//Set clock to 27 MHz
			// [7:0] = DIVCLK_DIVIDE = 2 (0x02)
			// [15:8] = CLKFBOUT_MULT = 10 (0xA)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D0A02);
			// DIV = 37.500 (0x25.0x1F4)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x1F425);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("27MHz\r\n");
			break;
		case CLKWIZ_CLKOUT0_25_175MHz:
			//Set clock to 27 MHz
			// [7:0] = DIVCLK_DIVIDE = 7 (0x07)
			// [15:8] = CLKFBOUT_MULT = 38 (0x26)
			// [25:16] = CLKFBOUT_FRAC = 0 (0x7D)
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x002607);
			// DIV = 43.125 (0x2B.0x07D)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x07D2B);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			xil_printf("25.175MHz\r\n");
			break;
		default:
			return XST_FAILURE;
			break;
	}

	status = ClkWiz_Wait_For_Lock(ClkWiz_BaseAddr);
	return status;

}
/****************************************************************************/
/**
*
*	This function wait for some time and check if the clocking wizard is locked
*
*
* @param	outClockFreq frequency to be set.
*
* @return	status.
*
* @note		None.
*
******************************************************************************/
int ClkWiz_Wait_For_Lock(UINTPTR Addr)
{
	usleep(800);
	if(Xil_In32(Addr + 0x04)& CLK_LOCK)
	{
		return XST_SUCCESS;
	}
	else
	{
		return XST_FAILURE;
	}
}

/*****************************************************************************/
/**
*
* This function configure initialize the SD Card
* @param
* @param
* @param
*
*
**********************************************************11********************/
u32 SD_Init()
{
    FRESULT result;
    TCHAR *Path = "0:/";
    result =f_mount(&fatfs,Path, 0);
        if(result){
		   printf("error : f_mount returned error \r\n");
		   return XST_FAILURE;
		  }

    return XST_SUCCESS ;
}
/*****************************************************************************/
/**
*
* This function configure initialize the peripherals
* @param
* @param
* @param
*
*
******************************************************************************/
int init_periphs(app_periphs_t *periphs_ptr)
{
	int Status;
	XVprocSs_Config *VprocSsConfigPtr;

	periphs_ptr->tpg_ptr 	= &tpg_inst;
	periphs_ptr->Vtc_ptr 	= &VtcInst;
	periphs_ptr->Vproc_ptr	= &VprocInst;

	//Configure the SD Card
	SD_Init();
	//Configure the GPIO(I2C)
	EMIO_I2C_init();
	//Configure the ADV7511
	init_adv7511();

	//Initialize the TPG IP
	Status = XV_tpg_Initialize(periphs_ptr->tpg_ptr, XPAR_V_TPG_0_DEVICE_ID);
	if(Status!= XST_SUCCESS)
	{
		xil_printf("TPG configuration failed\r\n");
		return(XST_FAILURE);
	}

	// Initialize the VTC
	XVtc_Config *VTC_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
	XVtc_CfgInitialize(periphs_ptr->Vtc_ptr, VTC_Config, VTC_Config->BaseAddress);

	//Configure the VPSS
	VprocSsConfigPtr = XVprocSs_LookupConfig(XPAR_V_PROC_SS_0_DEVICE_ID);
	if(VprocSsConfigPtr == NULL)
	{
		xil_printf("ERR:: VprocSs device not found\r\n");
		return (XST_DEVICE_NOT_FOUND);
	}

	XVprocSs_LogReset(periphs_ptr->Vproc_ptr);
	Status = XVprocSs_CfgInitialize(periphs_ptr->Vproc_ptr,
									VprocSsConfigPtr,
									VprocSsConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS)
	{
		 xil_printf("ERR:: Video Processing Subsystem Init. error\n\r");
		 return(XST_FAILURE);
	}
    XGpio_Initialize(&BTN_XGpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    XGpio_SetDataDirection(&BTN_XGpio, 1, 0);
	SetupInterruptSystem(&Intc, &BTN_XGpio, GPIO_INTERRUPT_ID);
	XGpio_InterruptEnable(&BTN_XGpio,XGPIO_IR_CH1_MASK);
	return Status;
}

/*****************************************************************************/
/**
*
* This function configure initialize the application
* @param
* @param
* @param
*
*
******************************************************************************/
int init_application(app_periphs_t *periphs_ptr, int resID)
{

	//int Status;
	const XVidC_VideoTimingMode *VmPtrIn, *VmPtrOut ;

	//Set Stream In
	VmPtrIn=XVidC_GetVideoModeData(VideoTimingClk[resID].VmId);//XVIDC_VM_1920x1080_60_P
	periphs_ptr->video_pipe_config.Stream_in.Timing = VmPtrIn->Timing;
	periphs_ptr->video_pipe_config.Stream_in.VmId = VmPtrIn->VmId;
	periphs_ptr->video_pipe_config.Stream_in.ColorFormatId = XVIDC_CSF_RGB;
	periphs_ptr->video_pipe_config.Stream_in.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_in.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_in.FrameRate = VmPtrIn->FrameRate;
	periphs_ptr->video_pipe_config.Stream_in.IsInterlaced = 0;

	//Set Stream Out
	VmPtrOut=XVidC_GetVideoModeData(VideoTimingClk[resID].VmId);
	periphs_ptr->video_pipe_config.Stream_out.Timing = VmPtrOut->Timing;
	periphs_ptr->video_pipe_config.Stream_out.VmId = VmPtrOut->VmId;
	periphs_ptr->video_pipe_config.Stream_out.ColorFormatId = XVIDC_CSF_YCRCB_422;
	periphs_ptr->video_pipe_config.Stream_out.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_out.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_out.FrameRate = VmPtrOut->FrameRate;
	periphs_ptr->video_pipe_config.Stream_out.IsInterlaced = 0;

	/* Clocking Wizard Configuration */
	//Configure the CLKOUT0 DIV
	ClkWiz_Set_Output_Clock(XPAR_CLK_WIZ_0_BASEADDR, VideoTimingClk[resID].clkFreq);

	/* End of clocking wizard configuration */

	//Configure the TPG
	periphs_ptr->tpg_config.colorFormat=periphs_ptr->video_pipe_config.Stream_in.ColorFormatId;
	periphs_ptr->tpg_config.bckgndId=XTPG_BKGND_COLOR_BARS;
	periphs_ptr->tpg_config.overlay_en=1;
	periphs_ptr->tpg_config.motionSpeed=5;
	periphs_ptr->tpg_config.boxSize=100;
	periphs_ptr->tpg_config.height = periphs_ptr->video_pipe_config.Stream_in.Timing.VActive;
	periphs_ptr->tpg_config.width = periphs_ptr->video_pipe_config.Stream_in.Timing.HActive;

	xil_printf("Starting VPSS...");
		configure_vpss(periphs_ptr->Vproc_ptr, &periphs_ptr->video_pipe_config.Stream_in,
						&periphs_ptr->video_pipe_config.Stream_out);
	xil_printf("Done!\r\n");

	//Start the TPG
	xil_printf("Starting TPG...");
	configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
	XV_tpg_EnableAutoRestart(periphs_ptr->tpg_ptr);
	XV_tpg_Start(periphs_ptr->tpg_ptr);
	xil_printf("Done!\r\n");

	xil_printf("Starting VDMA...");
	configure_vdma(XPAR_AXI_VDMA_0_BASEADDR,&periphs_ptr->video_pipe_config.Stream_out, 3,3);
	xil_printf("Done!\r\n");

	xil_printf("Starting VTC...");
	configure_vtc_gen(periphs_ptr->Vtc_ptr,&periphs_ptr->video_pipe_config.Stream_out);
	xil_printf("Done!\r\n");

	return 1;
}
/*****************************************************************************/
/**
*
* This function shows the VPSS log
* @param
* @param
*
*
******************************************************************************/
void display_vpss_log(XVprocSs *InstancePtr)
{

	XVprocSs_LogDisplay(InstancePtr);
	XVprocSs_LogReset(InstancePtr);
	XVprocSs_ReportSubsystemConfig(InstancePtr);
}
void show_img(u32 disp_base_addr, const unsigned char * addr, u32 size_x, u32 size_y)
{
		u32 i=0;
		u32 j=0;
		u32 r,g,b;
		u32 start_addr=disp_base_addr;
		for(j=0;j<size_y;j++)
		{
			for(i=0;i<size_x;i++)
			{
				b = *(addr+(i+j*size_x)*4+1);

				g = *(addr+(i+j*size_x)*4+2);

				r = *(addr+(i+j*size_x)*4+3);
				Xil_Out8((start_addr+(i+j*size_x)*3+3),g);
				Xil_Out8((start_addr+(i+j*size_x)*3+2),r);
				Xil_Out8((start_addr+(i+j*size_x)*3+1),b);
			}
		}
		Xil_DCacheFlush();
		xil_printf("show img\r\n");
}
void read_bmp(u8 *frame)
{
	static 	FATFS fatfs;
	FIL 	fil;
	u8		bmp_head[54];
	UINT 	*bmp_width,*bmp_height,*bmp_size;
	unsigned char read_line_buf[1920 * 3];
	UINT 	br;
	int 	i , j;
	u32 r,g,b;

//	f_mount(&fatfs,"",1);

	f_open(&fil,"1.bmp",FA_READ);

	f_lseek(&fil,0);

	f_read(&fil,bmp_head,54,&br);
	xil_printf("bmp head: \n\r");

	for(i=0;i<54;i++)
		xil_printf(" %x",bmp_head[i]);

	bmp_width  = (UINT *)(bmp_head + 0x12);
	bmp_height = (UINT *)(bmp_head + 0x16);
	bmp_size   = (UINT *)(bmp_head + 0x22);
	xil_printf("\n width = %d, height = %d, size = %d bytes \n\r",*bmp_width,*bmp_height,*bmp_size);

	for(i=*bmp_height-1;i>=0;i--){
		f_read(&fil,read_line_buf,(*bmp_width)*3,&br);
//		f_read(&fil,frame+i*(*bmp_width)*3,(*bmp_width)*3,&br);
		for(j=*bmp_width-1;j>=0;j--){
			b = read_line_buf[j * 3 + 0];

			g = read_line_buf[j * 3 + 1];

			r = read_line_buf[j * 3 + 2];

			Xil_Out8((frame+((j)+(i)*1920)*3-3),g);
			Xil_Out8((frame+((j)+(i)*1920)*3-2),b);
			Xil_Out8((frame+((j)+(i)*1920)*3-1),r);
		}
	}


	f_close(&fil);

	Xil_DCacheFlush();     //��Cache嚗����DDR3銝�
	xil_printf("show bmp\n\r");
}
/*****************************************************************************/
/**
*
* This function print the resolution name
* @param
* @param
*
*
******************************************************************************/
void print_resolution_name(int resID)
{
	xil_printf("%s",XVidC_GetVideoModeStr(VideoTimingClk[resID].VmId));
}
/*****************************************************************************/
/**
*
* This function print the colorformat name
* @param
* @param
*
*
******************************************************************************/
void print_colorformat_name(int resID)
{
	xil_printf("%s",XVidC_GetColorFormatStr(ColorType[resID]));
}
/*****************************************************************************/
/**
*
* This function print the tpg configure
* @param
* @param
*
*
******************************************************************************/
void print_tpg_configure(tpg_config_t *tpg_config)
{
	xil_printf("\r\n------TPG_configure----------\r\n");
	xil_printf("colorFormat : ");
	print_colorformat(tpg_config->colorFormat);
	xil_printf("bckgndId : ");
	tpg_background(tpg_config->bckgndId);
	xil_printf("width : %d\r\n",tpg_config->width);
	xil_printf("height : %d\r\n",tpg_config->height);
	xil_printf("boxSize : %d\r\n",tpg_config->boxSize);
	xil_printf("motionSpeed : %d\r\n",tpg_config->motionSpeed);
	xil_printf("\r\n------TPG_configure----------\r\n");
}
void print_VTC_configure(const XVidC_VideoStream *Strm)
{
	xil_printf("\r\n------VTC_configure----------\r\n");
	xil_printf("HActiveVideo : %d\r\n",Strm->Timing.HActive);
	xil_printf("HBackPorch : %d\r\n",Strm->Timing.HBackPorch);
	xil_printf("HFrontPorch : %d\r\n",Strm->Timing.HFrontPorch);
	xil_printf("HSyncPolarity : %d\r\n",Strm->Timing.HSyncPolarity);
	xil_printf("HSyncWidthHSyncWidth : %d\r\n",Strm->Timing.HSyncWidth);
	xil_printf("Interlaced : %d\r\n",Strm->IsInterlaced);
	xil_printf("V0BackPorch : %d\r\n",Strm->Timing.F0PVBackPorch);
	xil_printf("V0FrontPorch : %d\r\n",Strm->Timing.F0PVFrontPorch);
	xil_printf("V0SyncWidth : %d\r\n",Strm->Timing.F0PVSyncWidth);
	xil_printf("V1BackPorch : %d\r\n",Strm->Timing.F1VBackPorch);
	xil_printf("V1FrontPorch : %d\r\n",Strm->Timing.F1VFrontPorch);
	xil_printf("V1SyncWidth : %d\r\n",Strm->Timing.F1VSyncWidth);
	xil_printf("VActiveVideo : %d\r\n",Strm->Timing.VActive);
	xil_printf("VSyncPolarity : %d\r\n",Strm->Timing.VSyncPolarity);
	xil_printf("\r\n------VTC_configure----------\r\n");
}
void print_colorformat(int ID)
{
	switch(ID)
	{
		case 0:
			xil_printf("RGB \r\n");
			break;
		case 1:
			xil_printf("YUV_444\r\n");
			break;
		case 2:
			xil_printf("YUV_422\r\n");
			break;
		case 3:
			xil_printf("YUV_420\r\n");
			break;
		case 4:
			xil_printf("Y_ONLY\r\n");
			break;
	}
}
void tpg_background(int ID)
{
	switch(ID)
		{
			case 1:
				xil_printf("Horizontal Ramp \r\n");
				break;
			case 2:
				xil_printf("Vertical Ramp\r\n");
				break;
			case 3:
				xil_printf("Temporal Ramp\r\n");
				break;
			case 4:
				xil_printf("Solid Red\r\n");
				break;
			case 5:
				xil_printf("Solid Green\r\n");
				break;
			case 6:
				xil_printf("Solid Blue\r\n");
				break;
			case 7:
				xil_printf("Solid Black\r\n");
				break;
			case 8:
				xil_printf("Solid White\r\n");
				break;
			case 9:
				xil_printf("Color Bar\r\n");
				break;
		}
}
void IntrHandler() {
	u32 i=0;
	u32 j=0;
	u8 r,g,b,data;
	u32 height = 24;
	u32 width = 32;

	xil_printf("INTERRUPT_START\r\n");
	usleep(2000000);
    XGpio_InterruptClear(&BTN_XGpio, XGPIO_IR_CH1_MASK);
    XGpio_InterruptDisable(&BTN_XGpio, XGPIO_IR_CH1_MASK);
    for(j=0;j<height;j++)
    		{
    			for(i=0;i<width;i++)
    			{
    				g = Xil_In8((FRAME_BUFFER_1+(i+j*width)*3+3));
    				r = Xil_In8((FRAME_BUFFER_1+(i+j*width)*3+2));
    				b = Xil_In8((FRAME_BUFFER_1+(i+j*width)*3+1));
//    				data = r&g&b;
//    				xil_printf("%d\r\n",i+j*height,data);
    				XUartPs_SendByte(STDOUT_BASEADDRESS,r);
    				XUartPs_SendByte(STDOUT_BASEADDRESS,g);
    				XUartPs_SendByte(STDOUT_BASEADDRESS,b);
    			}
    		}
    xil_printf("INTERRUPT_Done\n");
    XGpio_InterruptEnable(&BTN_XGpio,XGPIO_IR_CH1_MASK);
    usleep(10000);
}

void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpio *Gpio, u16 GpioIntrId){
	printf("Setup Interrupt...");

	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

	XScuGic_CfgInitialize(GicInstancePtr, IntcConfig,IntcConfig->CpuBaseAddress);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,GicInstancePtr);

	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

	XScuGic_Connect(GicInstancePtr, GpioIntrId,(Xil_ExceptionHandler)IntrHandler,(void *)Gpio);

	XScuGic_Enable(GicInstancePtr, GpioIntrId);

	XScuGic_SetPriorityTriggerType(GicInstancePtr, GpioIntrId,0xA0, 0x0);

	XGpio_InterruptGlobalEnable(Gpio);

	printf("DONE!\r\n");
}
