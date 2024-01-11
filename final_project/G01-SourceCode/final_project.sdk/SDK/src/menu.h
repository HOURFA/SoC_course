// SPDX-License-Identifier: GPL-2.0
/*
 * app_hdmi.h
 *
 *  Created on: 18 Mar 2019
 *      Author: florentw
 */

#ifndef MENU_H_
#define MENU_H_

	#include "xil_printf.h"
	#include "app_hdmi.h"
	extern const unsigned char gImage_beijing[8294400];
	
	void display_main_menu();
	int resolution_menu();
	void menu(app_periphs_t *periphs_ptr);

#endif /* MENU_H_ */
