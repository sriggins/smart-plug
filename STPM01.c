/******************************************************************************/
/** \file       STPM01.c
 *******************************************************************************
 *
 *  \brief      These are the functions for the STPM01 Modul
 *
 *  \author     fonts2
 *
 *  \date       16.05.2013
*
 *  \remark     Last Modification
 *               \li  fonts2, 16.05.2013   created
 *               \li  fonts2, 03.06.2013   modified
 *
 ******************************************************************************/
/*
 *  functions  global:
 *              
 *  functions  local:
 *              
 *
 ******************************************************************************/
#include <STPM01.h>
int8_t SCK_Flag;

/**
  * @brief  This function sends command over the SPI2.
  * @param  Data to send
  * @retval Received data
  */
int8_t SPI2_IO(int8_t data)
{
  SPI_I2S_SendData(SPI2,data);
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET){;}
  return SPI_I2S_ReceiveData(SPI2);
}

/**
  * @brief  This function sends command to the STPM01 its a selfmade SPI.
  * @param  Data to send
  * @retval none
  */
void SendtoSTPM01(uint8_t data)
{
	// define count varible
	int8_t count;
	/* Configure GPIO */
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* SPI2 DISABLE -------------------------------------------------------------*/
	SPI_Cmd(SPI2, DISABLE);
	//SCK on
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	

	
	/* configure SPI MISO as Output------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	/* configure SPI SCK -------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	GPIO_SetBits(GPIOB,GPIO_Pin_13);


	wait10ns(50);
	//NSS Active
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	wait10ns(10);
	//SYN0 write
	GPIO_ResetBits(GPIOA,GPIO_Pin_9);
	SCK_Flag=0;

	for(count=7; count>=0; count--)
	{
			while(!SCK_Flag);
		
			GPIO_ResetBits(GPIOB,GPIO_Pin_13);
			if((data&(1<<count))&&(1<<count))
			{
				GPIO_SetBits(GPIOB,GPIO_Pin_14);
			}
			else
			{
				GPIO_ResetBits(GPIOB,GPIO_Pin_14);
			}
			while(SCK_Flag);
			GPIO_SetBits(GPIOB,GPIO_Pin_13);
	}
	while(!SCK_Flag);
	wait10ns(3);
	//SYN0 read
	GPIO_SetBits(GPIOA,GPIO_Pin_9);
	wait10ns(5);
	//NSS not Active
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	wait10ns(200);
	//SCK off
	TIM_Cmd(TIM2, DISABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
	
	/* configure SPI MISO as Input ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	/* configure SPI SCK -------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	
	/* SPI2 ENABLE -------------------------------------------------------------*/
	SPI_Cmd(SPI2, ENABLE);
}

/**
  * @brief  This function sends the Setup data to the STPM01.
  * @param  none
  * @retval none
  */
void SetupSTPM01(void)
{
	SendtoSTPM01(0xFB);				//RD=1;
	wait10ns(5);
	SendtoSTPM01(0x00);				//TSTD=0;
	SendtoSTPM01(0x82);				//MDIV=1;
	SendtoSTPM01(0x04);				//RC=0;
	SendtoSTPM01(0x06);				//APL=0;
	SendtoSTPM01(0x08);				
	SendtoSTPM01(0x0A);				//PST=2;
	SendtoSTPM01(0x8C);				
	SendtoSTPM01(0x0E);				
	SendtoSTPM01(0x10);				//FRS=0;
	SendtoSTPM01(0x12);				//MSBF=0;
	SendtoSTPM01(0x14);				//FUND=0;
	SendtoSTPM01(0x18);				//LTCH=0;
	SendtoSTPM01(0x1A);				
	SendtoSTPM01(0x1C);				//KMOT=0;
	SendtoSTPM01(0x1E);			
	
	SendtoSTPM01(0x40);				//-9.364% STROM 
	SendtoSTPM01(0x42);
	SendtoSTPM01(0x44);
	SendtoSTPM01(0x46);
	SendtoSTPM01(0x48);
	SendtoSTPM01(0xCA);
	SendtoSTPM01(0x4C);
	SendtoSTPM01(0x4E);
	
	SendtoSTPM01(0xB0);				//10.138% Spannung 
	SendtoSTPM01(0xB2);
	SendtoSTPM01(0xB4);
	SendtoSTPM01(0x36);
	SendtoSTPM01(0x38);
	SendtoSTPM01(0xBA);
	SendtoSTPM01(0xBC);
	SendtoSTPM01(0xBE);

// 	SendtoSTPM01(0x40);				//-9.995% STROM 
// 	SendtoSTPM01(0xC2);
// 	SendtoSTPM01(0x44);
// 	SendtoSTPM01(0xC6);
// 	SendtoSTPM01(0xC8);
// 	SendtoSTPM01(0x4A);
// 	SendtoSTPM01(0x4C);
// 	SendtoSTPM01(0x4E);

// 	SendtoSTPM01(0xB0);				//10.138% Spannung 
// 	SendtoSTPM01(0xB2);
// 	SendtoSTPM01(0xB4);
// 	SendtoSTPM01(0x36);
// 	SendtoSTPM01(0x38);
// 	SendtoSTPM01(0xBA);
// 	SendtoSTPM01(0xBC);
// 	SendtoSTPM01(0xBE);
	
// 	SendtoSTPM01(0xA8);
// 	SendtoSTPM01(0xAC);
}

/**
  * @brief  This function is a reset
  * @param  timeout time
  * @retval none
  */
void reset_STPM01(void)
{
	SendtoSTPM01(0x82);
	SendtoSTPM01(0x82);
	SendtoSTPM01(0x82);
	wait10ns(50);
	SetupSTPM01();
}
/**
  * @brief  This function waits for a few ns.
  * @param  timeout time
  * @retval none
  */
void wait10ns( u32 timeout )
{
	u32 i;
	for(i=timeout; i>0; i --)
	{
		//wait
	}
}
