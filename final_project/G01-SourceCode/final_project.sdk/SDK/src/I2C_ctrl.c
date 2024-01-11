/*
 * I2C_ctrl.c
 *
 *  Created on: 2017Äê6ÔÂ4ÈÕ
 *      Author: luotong
 */


#include "sleep.h"
#include "EMIO_init.h"
#include "I2C_ctrl.h"


#define I2C_DELAY	usleep(10)

void I2C_start(void)
{
	CLOCK_HIGH();
	DATA_HIGH();
	I2C_DELAY;
	DATA_LOW();
	I2C_DELAY;
	CLOCK_LOW();
	I2C_DELAY;
}

void I2C_end(void)
{
	DATA_LOW();
	I2C_DELAY;
	CLOCK_HIGH();
	I2C_DELAY;
	DATA_HIGH();
	I2C_DELAY;
}

int I2C_sendbyte( unsigned char value )
{
	unsigned char tmp = value;
	unsigned char  i=0,ack;

	for(i=0; i<8; i++)
	{
		if(tmp & 0x80 )
				DATA_HIGH();
		else
				DATA_LOW();

		 I2C_DELAY;
		 CLOCK_HIGH();
		 I2C_DELAY;
		 CLOCK_LOW();
		 I2C_DELAY;

		 tmp<<=1;
	 }


	 DATA_HIGH();
	 DATA_INPUT();
	 I2C_DELAY;
	 CLOCK_HIGH();
	 ack = GET_DATA();
	 I2C_DELAY;
	 CLOCK_LOW();
	 I2C_DELAY;
	 DATA_OUTPUT();

	 if(ack==1)
	 {
		 return -1;
	 }

	 return 0;
}


unsigned char I2C_readbyte( unsigned char addr)
{
	unsigned char  i=0,data=0;

	DATA_HIGH();
	DATA_INPUT();

	for(i=0; i<8; i++)
	{
		CLOCK_HIGH();
		I2C_DELAY;

		data <<= 1;
		if(GET_DATA())
			data |= 1;

		 I2C_DELAY;
		 CLOCK_LOW();
		 I2C_DELAY;
	 }

	 DATA_OUTPUT();
	 DATA_HIGH();
	 I2C_DELAY;
	 CLOCK_HIGH();
	 I2C_DELAY;
	 CLOCK_LOW();
	 I2C_DELAY;
	 DATA_HIGH();

	 return data;
}

int I2C_readdata(unsigned char id, unsigned char addr, unsigned char *value)
{

	I2C_start();
	if(I2C_sendbyte(id<<1) != 0)
	{
		goto error;
	}

	if(I2C_sendbyte(addr) != 0)
	{
		goto error;
	}


	I2C_start();
	if(I2C_sendbyte((id<<1)|0x1) != 0)
	{
		goto error;
	}
	*value = I2C_readbyte(addr);
	I2C_end();

	return 0;

error:
	I2C_end();
	return -1;
}

int I2C_senddata(unsigned char id, unsigned char addr,unsigned char value)
{
	I2C_start();
	if(I2C_sendbyte(id<<1) != 0)
	{
		goto error;
	}
	if(I2C_sendbyte(addr) != 0)
	{
		goto error;
	}
	if(I2C_sendbyte(value)!= 0)
	{
		goto error;
	}
	I2C_end();

	return 0;

error:
	I2C_end();
	return -1;

}
