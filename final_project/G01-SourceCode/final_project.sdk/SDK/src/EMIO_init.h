/*
 * EMIO_init.h
 *
 *  Created on: 2017Äê6ÔÂ4ÈÕ
 *      Author: luotong
 */

#ifndef EMIO_INIT_H_
#define EMIO_INIT_H_

#include "xgpiops.h"

int EMIO_I2C_init(void);


#define SIOC_PIN 	54
#define SIOD_PIN 	55
#define RESET_PIN 	56


#define DIRECTION_INPUT 0
#define DIRECTION_OUTPUT 1

void CLOCK_HIGH(void);

void CLOCK_LOW(void);

void DATA_HIGH(void);

void DATA_LOW(void);

void DATA_INPUT(void);

void DATA_OUTPUT(void);

int GET_DATA(void);


#endif /* EMIO_INIT_H_ */
