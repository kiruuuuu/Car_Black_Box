
#include <xc.h>

void init_i2c(void)
{
	/* Set SCL and SDA pins as inputs */
	TRISC3 = 1;
	TRISC4 = 1;
	/* Set I2C master mode */
	SSPCON1 = 0x28;      //setting serial port and setting to master mode 

	SSPADD = 0x31;           //we are storing address to SSPADD
    
	/* Use I2C levels, worked also with '0' */   
	CKE = 0;
	/* Disable slew rate control  worked also with '0' */
	SMP = 1;
	/* Clear SSPIF interrupt flag */
	SSPIF = 0;
	/* Clear bus collision flag */
	BCLIF = 0;
}

void i2c_idle(void)   //to check whether instruction executed or not
{
	while (!SSPIF); // 0 instruction not completed
                    // 1 instruction completed
	SSPIF = 0;
}

void i2c_ack(void)
{
	if (ACKSTAT)
	{
		/* Do debug print here if required */
	}
}

void i2c_start(void)    //start the communication
{
	SEN = 1;
	i2c_idle();
}
   
void i2c_stop(void)     //stop the communication
{
	PEN = 1;
	i2c_idle();
}

void i2c_rep_start(void)   //to repeat start operation
{
	RSEN = 1;
	i2c_idle();
}

void i2c_write(unsigned char data)     
{
	SSPBUF = data;        //storing data to the buffer
	i2c_idle();
}

void i2c_rx_mode(void)    // rx mode
{
	RCEN = 1;        
	i2c_idle();
}

void i2c_no_ack(void)   //to inform slave master is not sending ack
{
	ACKDT = 1;     //Data bit is 1 no ack recevied
	ACKEN = 1;     //if you are using data bit we should enable bit
}

unsigned char i2c_read(void)
{
	i2c_rx_mode();
	i2c_no_ack();

	return SSPBUF;
}