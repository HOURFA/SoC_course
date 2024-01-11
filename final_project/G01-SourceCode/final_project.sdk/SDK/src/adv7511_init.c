#include "xil_types.h"
#include "I2C_ctrl.h"
#include "sleep.h"

//#define IIC_CLK         XPAR_XIICPS_0_I2C_CLK_FREQ_HZ
#define IIC_CLK         400000
#define ADV7511_ADDR    0x72 >> 1
#define config_lenth    17

u8 WriteBuffer[1 + 1];

u8 ReadBuffer[1];	/* Read buffer for reading a page. */

struct sensor_register {
	u8 addr;
	u8 data;
};

int adv7511_read_reg(unsigned char reg, unsigned char *value)
{
	return I2C_readdata(ADV7511_ADDR,reg,value);
}


void dump_reg(unsigned char reg)
{
	unsigned char data;
	adv7511_read_reg(reg, &data);
	xil_printf("0x%02x : 0x%02x\r\n", reg, data);
}

int adv7511_write_reg(unsigned char reg, unsigned char value)
{
	I2C_senddata(ADV7511_ADDR,reg,value);

	dump_reg(reg);
}



int adv7511_change_reg(unsigned char reg, unsigned char mask, unsigned char value)
{
	unsigned char data,i;

	adv7511_read_reg(reg,&data);
	for(i=0;i<8;i++)
	{
		if(mask & 0x80)
		{
			if(value & 0x80)
			{
				data |= 0x80;
			}
			else
			{
				data &= ~0x80;
			}
		}

		mask <<= 1;
		value <<= 1;
	}
}

void adv7511_init0()
{
	//---------------------------------------------------
	adv7511_write_reg(0x41,0x10); // POWER UP
	adv7511_write_reg(0x98,0x03);
	adv7511_write_reg(0x9a,0xe0);
	adv7511_write_reg(0x9c,0x30);
	adv7511_write_reg(0x9d,0x61);
	adv7511_write_reg(0xa2,0xa4);
	adv7511_write_reg(0xa3,0xa4);
	adv7511_write_reg(0xe0,0xD0);
	adv7511_write_reg(0xF9,0x00);
	//---------------------------------------------------
	//------------------------------------------+
	// HDMI/DVI mode select 0xAF[1]             |
	//---------------------------- 0 = DVI  Mode|
	//---------------------------- 1 = HDMI Mode|
	//------------------------------------------+
	adv7511_write_reg(0xaf,0x04);
	//-----------------------------------------------------------------------------------------------------+
	// Input ID 0x15[3:0]                                                                                  |
	//------------------- 0000 = 24         bit   RGB 4:4:4 or YCbCr 4:4:4 (separate syncs)                |
	//------------------- 0001 = 16, 20, 24 bit YCbCr 4:2:2                (separate syncs)                |
	//------------------- 0010 = 16, 20, 24 bit YCbCr 4:2:2                (embedded syncs)                |
	//------------------- 0011 =  8, 10, 12 bit YCbCr 4:2:2                (2x pixel clock, separate syncs)|
	//------------------- 0100 =  8, 10, 12 bit YCbCr 4:2:2                (2x pixel clock, embedded syncs)|
	//------------------- 0101 = 12, 15, 16 bit   RGB 4:4:4 or YCbCr       (DDR with separate syncs)       |
	//------------------- 0110 =  8, 10, 12 bit YCbCr 4:2:2                (DDR with separate syncs)       |
	//------------------- 0111 =  8, 10, 12 bit YCbCr 4:2:2                (DDR separate syncs)            |
	//------------------- 1000 =  8, 10, 12 bit YCbCr 4:2:2                (DDR embedded syncs)            |
	//-----------------------------------------------------------------------------------------------------+
	adv7511_write_reg(0x15,0x01);
	//------------------------------------------------------------+
	// Video Input Justification 0x48[4:3]                        |
	//----------------------------------- 00 =  evenly distributed|
	//----------------------------------- 01 =   right   justified|
	//----------------------------------- 10 =    left   justified|
	//----------------------------------- 11 = invalid            |
	//------------------------------------------------------------+
	adv7511_write_reg(0x48,0x08);
	//------------------------------------------------------+
	// Color Space Converter Enable 0x18[7]                 |
	//------------------------------------- 0 = CSC Disabled|
	//------------------------------------- 1 = CSC Enabled |
	//------------------------------------------------------+
	adv7511_write_reg(0x18,0xE7); // Default
	//------------------------------------------+
	// Output format 0x55[6:5]                  |
	//------------------------ 00 = RGB         |
	//------------------------ 01 = YCbCr 4:2:2 |
	//------------------------ 10 = YCbCr 4:4:4 |
	//------------------------------------------+
	adv7511_write_reg(0x55,0x00);
	//------------------------------------------+
	// Output format 0x16[7]                    |
	//---------------------- 0 = 4:4:4          |
	//---------------------- 1 = 4:2:2          |
	//------------------------------------------+
	// Input Color Depth 0x16[5:4]              |
	//---------------------------- 00 = invalid |
	//---------------------------- 10 = 12  bit |
	//---------------------------- 01 = 10  bit |
	//---------------------------- 11 =  8  bit |
	//------------------------------------------+
	// Input Style 0x16[3:2]                    |
	//---------------------- 00 = Not Valid     |
	//---------------------- 01 =    Style2     |
	//---------------------- 10 =    Style1     |
	//---------------------- 11 =    Style3     |
	adv7511_write_reg(0x16,0x3C); // 4:2:2 , 8bit , Style3
	adv7511_write_reg(0x56,0x28);
	adv7511_write_reg(0xD6,0xC0);
	adv7511_write_reg(0xF9,0x00);

}
//********************************************************************************
void init_adv7511()
{
    u32 i;
    int test;
    unsigned char data;
	unsigned int addr,value;

    xil_printf("--------------init_adv7511_start--------------\r\n");
    adv7511_read_reg(0xF5,&data);
	if(data != 0x75)
	{
		xil_printf("--------------init_adv7511_failed--------------\r\n");
		goto out;
	}
	initial:
	adv7511_init0();
    adv7511_read_reg(0x41,&data);
	if(data != 0x10)
	{
		xil_printf("--------------init_adv7511_failed--------------\r\n");
		goto initial;
	}
	xil_printf("--------------init_adv7511_finish--------------\r\n");
    return 0;
out:
	return 1;
}
