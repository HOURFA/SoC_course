/*
 * I2C_ctrl.h
 *
 *  Created on: 2017Äê6ÔÂ4ÈÕ
 *      Author: luotong
 */

#ifndef I2C_CTRL_H_
#define I2C_CTRL_H_

void I2C_start(void);
void I2C_end(void);


//void I2C_sendbyte( unsigned char value );

int I2C_senddata(unsigned char id, unsigned char subaddr,unsigned char value);
int I2C_readdata(unsigned char id, unsigned char addr, unsigned char *value);

#endif /* I2C_CTRL_H_ */
