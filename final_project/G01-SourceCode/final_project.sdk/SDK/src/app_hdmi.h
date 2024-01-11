// SPDX-License-Identifier: GPL-2.0
/*
 * app_hdmi.h
 *
 *  Created on: 07 Jan 2019
 *      Author: florentw
 */

#ifndef APP_HDMI_H_
#define APP_HDMI_H_

	#include "xv_tpg.h"
	#include "sleep.h"
	#include "xvprocss.h"
	#include "ff.h"
	#include "xvtc.h"
	#include "xscugic.h"
	#include "adv7511_init.h"
	
	#define CLK_LOCK			1
	//---------------------------------------------Constant Define
	//Output clocks
	#define CLKWIZ_CLKOUT0_148_5_MHz		0
	#define CLKWIZ_CLKOUT0_108_MHz			1
	#define CLKWIZ_CLKOUT0_74_25_MHz		2
	#define CLKWIZ_CLKOUT0_74_MHz			3
	#define CLKWIZ_CLKOUT0_65_MHz			4
	#define CLKWIZ_CLKOUT0_40_MHz			5
	#define CLKWIZ_CLKOUT0_27_MHz			6
	#define	CLKWIZ_CLKOUT0_25_175MHz		7
	#define SUPPORTED_VIDEO_FORMATS	7
	//Color format
	#define SUPPORTED_COLOR_FORMATS 5
	#define APP_COLOR_FORMAT	XVIDC_CSF_YCRCB_422
	// VPSS configure mode
	#define resoloution_mode 1
	#define colorformat_mode 2

	#define FRAME_BUFFER_BASE_ADDR	0x10000000
	#define FRAME_BUFFER_SIZE		0x1000000	//0x400000 for max 1080p YCbCr422 8bpc
	#define FRAME_BUFFER_1			FRAME_BUFFER_BASE_ADDR
	#define FRAME_BUFFER_2			FRAME_BUFFER_BASE_ADDR + FRAME_BUFFER_SIZE
	#define FRAME_BUFFER_3			FRAME_BUFFER_BASE_ADDR + (FRAME_BUFFER_SIZE*2)

	#define H_STRIDE            1920
	#define H_ACTIVE            1920
	#define V_ACTIVE            1080
	#define BYTES_PIXEL            3

	#define GPIO_INTERRUPT_ID	XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR //GPIO¤¤Â_½s¸¹
	#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	//---------------------------------------------
	//---------------------------------------------Struct Define
	typedef struct {
		 u32 	colorFormat;
		 u32 	bckgndId;
		 u8 	overlay_en;
		 u32 	boxSize;
		 u32	motionSpeed;
		 u32	width;
		 u32	height;
	} tpg_config_t;
	typedef struct {
		XVidC_VideoStream Stream_in;
		XVidC_VideoStream Stream_out;
	} video_pipe_config_t;

	typedef struct {
		XVtc  					*Vtc_ptr;
		XVprocSs 				*Vproc_ptr;
		XV_tpg 					*tpg_ptr;
		tpg_config_t 			tpg_config;
		video_pipe_config_t		video_pipe_config;
	} app_periphs_t;
	typedef struct {
		XVidC_VideoMode		VmId;
		u8					clkFreq;
	} VideoTimingClk_t;
	//---------------------------------------------
	int init_periphs(app_periphs_t *periphs_ptr);
	int init_application(app_periphs_t *periphs_ptr, int resID);


#endif /* APP_HDMI_H_ */
