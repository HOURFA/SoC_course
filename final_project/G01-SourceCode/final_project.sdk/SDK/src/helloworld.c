/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "app_hdmi.h"



#define DDR_BASEADDR        0x01000000

#define VDMA_BASEADDR       XPAR_AXI_VDMA_0_BASEADDR
#define VIDEO_BASEADDR 0x10000000

#define FILE_NAME "1.bmp"
FATFS FatfsInst;

#define DEMO_STRIDE (H_ACTIVE * BYTES_PIXEL)
u8 frameBuf[H_ACTIVE*V_ACTIVE*3];

app_periphs_t app_periphs;

int main()
{
	int resID;
	init_platform();
	print("+-------------------------------------------+\n\r");
	print("|----------------System Start---------------|\r\n");
	print("+-------------------------------------------+\n\n\r");
	res_menu:
	resID = resolution_menu();
	if(resID ==10)
	{
		xil_printf("error section\r\n");
		goto res_menu;
	}
	init_periphs(&app_periphs);

	//Start the application
	init_application(&app_periphs,resID);

	while(1)
	{
		menu(&app_periphs);
		Xil_DCacheFlush();
	}


	cleanup_platform();
	return 0;
}
