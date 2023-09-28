/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

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
#include <stdlib.h>
#include "xparameters.h"		// 參數集.
#include "xgpio.h"	// 簡化PS對PL的GPIO操作的函數庫.

// 延遲用.
void delay(int dly)
{
    int i, j;
    for (i = 0; i < dly; i++) {
    	for (j = 0; j < 0xffff; j++) {
    		;
        }
    }
}

// 主程式.
int main()
{
    XGpio LED_XGpio;		// 宣告一個GPIO用的結構.

    int LED_num = 0b00000000;	// 宣告一個變數,做運算用暫存用.
    XGpio_Initialize(&LED_XGpio, XPAR_AXI_GPIO_0_DEVICE_ID);	// 初始化LED_XGpio.
    XGpio_SetDataDirection(&LED_XGpio, 1, 0);		// 設置通道.


    XGpio SWS_XGpio;		// 宣告一個GPIO用的結構.
    int Switch_num = 0b00000000;
    XGpio_Initialize(&SWS_XGpio, XPAR_AXI_GPIO_1_DEVICE_ID);	// 初始化LED_XGpio.
    XGpio_SetDataDirection(&SWS_XGpio, 1, 0);		// 設置通道.


    int delay_num = 1000;


    printf("Start!!!");

    while(1) {
    	Switch_num = XGpio_DiscreteRead(&SWS_XGpio, 1);		// LED_XGpio通道,送LED_num值進去.
    	switch (Switch_num) {
    	        case 0b00000001:
    	        	delay_num = 100;
    	            break;
    	        case 0b00000010:
    	        	delay_num = 500;
    	            break;
    	        default:
    	        	delay_num = 1000;
    	            break;
    	    }

    	printf("LED_num = 0x%x\n", LED_num);

    	XGpio_DiscreteWrite(&LED_XGpio, 1, LED_num);		// LED_XGpio通道,送LED_num值進去.

    	LED_num = LED_num + 1;		// LED_num變數反相.

    	delay(delay_num);
    }

    return 0;
}
