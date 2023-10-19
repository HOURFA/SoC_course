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
#include "xscugic.h"
#include "xparameters.h"		// 參數集.
#include "xgpio.h"	// 簡化PS對PL的GPIO操作的函數庫.
#include "sleep.h"

#define GPIO_INTERRUPT_ID	XPAR_FABRIC_AXI_GPIO_1_IP2INTC_IRPT_INTR //GPIO中斷編號
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID

XGpio LED_XGpio;		// 宣告一個GPIO用的結構.
XGpio BTN_XGpio;		// 宣告一個GPIO用的結構.

XScuGic_Config *IntcConfig;
XScuGic Intc;
int delay_num = 500000;
int handler_times = 0;
int set_times = 0;
void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpio *Gpio, u16 GpioIntrId);
void IntrHandler();
int main()
{
    int LED_num = 0b00000000;
    XGpio_Initialize(&LED_XGpio, XPAR_AXI_GPIO_0_DEVICE_ID);	// 初始化LED_XGpio.
    XGpio_SetDataDirection(&LED_XGpio, 1, 0);		// 設置通道.

    XGpio_Initialize(&BTN_XGpio, XPAR_AXI_GPIO_1_DEVICE_ID);
    XGpio_SetDataDirection(&BTN_XGpio, 1, 0);

    SetupInterruptSystem(&Intc, &BTN_XGpio, GPIO_INTERRUPT_ID);

    //XGpio_InterruptEnable(&BTN_XGpio,XGPIO_IR_CH1_MASK);
    while(1) {
    	XGpio_DiscreteWrite(&LED_XGpio, 1, LED_num);		// LED_XGpio通道,送LED_num值進去.
    	LED_num = LED_num +1;
    	printf("Delay : %d\n",delay_num);
    	printf("INTERRUPT_TIMES = %d\n",handler_times);
    	usleep(delay_num);
    }

    return 0;
}



void IntrHandler() {

	handler_times ++;
	printf("INTERRUPT_START\n");
	usleep(2000000);
    XGpio_InterruptClear(&BTN_XGpio, XGPIO_IR_CH1_MASK);
    XGpio_InterruptDisable(&BTN_XGpio, XGPIO_IR_CH1_MASK);

    if (delay_num > 50000)
   		delay_num = delay_num - 100000;
    else
   		delay_num = 500000;
    printf("INTERRUPT_Done\n");
    XGpio_InterruptEnable(&BTN_XGpio,XGPIO_IR_CH1_MASK);
}

void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpio *Gpio, u16 GpioIntrId){
	printf("Start setup\n");

	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

	XScuGic_CfgInitialize(GicInstancePtr, IntcConfig,IntcConfig->CpuBaseAddress);

	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,GicInstancePtr);

	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

	XScuGic_Connect(GicInstancePtr, GpioIntrId,(Xil_ExceptionHandler)IntrHandler,(void *)Gpio);

	XScuGic_Enable(GicInstancePtr, GpioIntrId);

	XScuGic_SetPriorityTriggerType(GicInstancePtr, GpioIntrId,0xA0, 0x0);

	XGpio_InterruptGlobalEnable(Gpio);

	printf("DONE\n");
}
