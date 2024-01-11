// SPDX-License-Identifier: GPL-2.0
/*
 * menu.c
 *
 *  Created on: 18 March 2019
 *      Author: florentw
 */
 
 #include "menu.h"
 
void display_main_menu()
{
	 xil_printf("\r\n-----------------------------\r\n");

	 xil_printf("1. Change TPG background\r\n");
	 xil_printf("2. Enable/Disable TPG moving box\r\n");
	 xil_printf("3. TPG Configure\r\n");
	 xil_printf("4. Change input colorformat\r\n");
	 xil_printf("5. VPSS Configure\r\n");
	 xil_printf("6. Enable TPG \r\n");
	 xil_printf("7. Show Image\r\n");
	 xil_printf("8. VDMA\r\n");
	 xil_printf("9. Show SD_img\r\n");
	 xil_printf("\r\n-----------------------------\r\n");
}

int tpg_menu()
{
	char userInput;

	xil_printf("\r\n-----------------------------\r\n");
	xil_printf("Change Background pattern:\r\n");
	xil_printf("1. Horizontal Ramp\r\n");
	xil_printf("2. Vertical Ramp\r\n");
	xil_printf("3. Temporal Ramp\r\n");
	xil_printf("4. Solid Red\r\n");
	xil_printf("5. Solid Green\r\n");
	xil_printf("6. Solid Blue\r\n");
	xil_printf("7. Solid Black\r\n");
	xil_printf("8. Solid White\r\n");
	xil_printf("9. Color Bar\r\n");
	xil_printf("\r\n-----------------------------\r\n");

	userInput = inbyte();

	if((userInput >= '1')&&(userInput <= '9'))
	{
		return (userInput - 48);
	}
	else
	{
		xil_printf("Invalid Option\r\n");
		return 10;
	}
}

int resolution_menu()
{
	int i;
	char userInput;

	xil_printf("\r\n-----------------------------\r\n");
	xil_printf("Select the new resolution:\r\n");
	for(i=0;i<=SUPPORTED_VIDEO_FORMATS-1;i++)
	{
		xil_printf("%d. ",i);
		print_resolution_name(i);
		xil_printf("\r\n");
	}
	xil_printf("\r\n-----------------------------\r\n");

	userInput = inbyte();

	if((userInput >= '0')&&(userInput <= '8'))
	{
		return (userInput - 48);
	}
	else
	{
		xil_printf("Invalid Option\r\n");
		return 10;
	}
}
int colorfromat_menu(int direction)
{
	int i;
	char userInput;

	xil_printf("\r\n-----------------------------\r\n");
	if(direction)
			xil_printf("Select the new input colorfromat:\r\n");
		else
			xil_printf("Select the new output colorfromat:\r\n");
	for(i=0;i<=SUPPORTED_COLOR_FORMATS-1;i++)
	{
		xil_printf("%d. ",i);
		print_colorformat_name(i);
		xil_printf("\r\n");
	}
	xil_printf("\r\n-----------------------------\r\n");

	userInput = inbyte();

	if((userInput >= '0')&&(userInput <= '4'))
	{
		return (userInput - 48);
	}
	else
	{
		xil_printf("Invalid Option\r\n");
		return 10;
	}
}
int input_menu()
{
	int i;
	char userInput;

	userInput = inbyte();
	if((userInput >= '0')&&(userInput <= '9'))
	{
		return (userInput - 48);
	}
	else
	{
		xil_printf("Invalid input\r\n");
		return 10;
	}
}
void menu(app_periphs_t *periphs_ptr)
{
	char userInput;

	display_main_menu();
	userInput = inbyte();
	int selection;
	int read_byteperpixels ,write_byteperpixels;

	switch(userInput)
	{
		case '1':
			selection = tpg_menu();
			if(selection !=0)
			{
				periphs_ptr->tpg_config.bckgndId=selection;
				configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
			}
			break;
		case '2':
			if(periphs_ptr->tpg_config.overlay_en)
			{
				periphs_ptr->tpg_config.overlay_en=0;
				xil_printf("Overlay disabled\r\n");
			}
			else
			{
				periphs_ptr->tpg_config.overlay_en=1;
				xil_printf("Overlay enabled\r\n");
			}
			configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
			break;
		case '3':
			print_tpg_configure(&periphs_ptr->tpg_config);
			break;
		case '4':
			selection = colorfromat_menu(1);
			if(selection !=10)
			{
				set_input_resolution(periphs_ptr, selection,colorformat_mode);
			}
			break;
		case '5':
			display_vpss_log(periphs_ptr->Vproc_ptr);
			break;
		case '6':
			XV_tpg_EnableAutoRestart(periphs_ptr->tpg_ptr);
			XV_tpg_Start(periphs_ptr->tpg_ptr);
			break;
		case '7':
			XV_tpg_DisableAutoRestart(periphs_ptr->tpg_ptr);
			show_img(FRAME_BUFFER_1,&gImage_beijing[0],1920,1080);
			show_img(FRAME_BUFFER_2,&gImage_beijing[0],1920,1080);
			show_img(FRAME_BUFFER_3,&gImage_beijing[0],19280,1080);
			break;
		case '8':
			xil_printf("\r\n-------Configure VDMA--------\r\n");
			xil_printf("read_bytePerPixels(0~9) : ");
			read_byteperpixels = input_menu();
			if (read_byteperpixels == 10)
				break;
			xil_printf("%d\r\n",read_byteperpixels);
			xil_printf("write_bytePerPixels(0~9) : ");
			write_byteperpixels = input_menu();
			if (write_byteperpixels == 10)
				break;
			xil_printf("%d\r\n",write_byteperpixels);
			configure_vdma(XPAR_AXI_VDMA_0_BASEADDR,&periphs_ptr->video_pipe_config.Stream_out, write_byteperpixels,read_byteperpixels);
			xil_printf("\r\n-----------------------------\r\n");
			break;
		case '9':
			XV_tpg_DisableAutoRestart(periphs_ptr->tpg_ptr);
			read_bmp(FRAME_BUFFER_1,"1.bmp");
			read_bmp(FRAME_BUFFER_2,"1.bmp");
			read_bmp(FRAME_BUFFER_3,"1.bmp");
			break;
		default:
			xil_printf("\nWrong Command\r\n");
			break;
	}
}
