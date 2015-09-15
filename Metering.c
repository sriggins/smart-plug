/******************************************************************************/
/** \file       Metering.c
 *******************************************************************************
 *
 *  \brief
 *
 *  \author     fonts2
 *
 *  \date       16.05.2013
 *
 *  \remark     Last Modification
 *               \li fonts2, 16.05.2013, Created
 *
 ******************************************************************************/
/*
 *  functions  global:
 *              vMeteringTask
 *  functions  local:
 *              .
 *
 ******************************************************************************/

//----- Header-Files -----------------------------------------------------------
#include "Metering.h"


//----- Macros -----------------------------------------------------------------

//----- Data types -------------------------------------------------------------

//----- Function prototypes ----------------------------------------------------

//----- Data -------------------------------------------------------------------
		struct Value PowerValueIn;
		struct Message PowerValueOut;
		struct Value PowerValueOld;
		struct ReceiveData rxData;
		uint8_t rxcount;
		portTickType xLastWakeTime;
		//100Hz frequency to proceed the Task
		const portTickType xFrequency = 1000;

//----- Implementation ---------------------------------------------------------
/*******************************************************************************
 *  function :    Metering
 ******************************************************************************/
/** \brief
 *
 *  \type         local
 *
 *  \param[in]	  pvData    not used
 *
 *  \return       void
 *
 ******************************************************************************/
void vMeteringTask(void *pvData) {
	
		// send the Setup Messages to the STPM01 Device
 		SetupSTPM01();
		
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    while (1) 
			{
				// Wait for the next cycle.
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
				
				//SYN0 = >Puls 500ns
				GPIO_ResetBits(GPIOA,GPIO_Pin_9);
				wait10ns(50);
				GPIO_SetBits(GPIOA,GPIO_Pin_9);
				wait10ns(5);	

				//NSS Active
				GPIO_ResetBits(GPIOB,GPIO_Pin_12);			
				wait10ns(5);	
			
				//SYN0 = >Puls 30ns
				GPIO_ResetBits(GPIOA,GPIO_Pin_9);
				wait10ns(5);		
				GPIO_SetBits(GPIOA,GPIO_Pin_9);
				wait10ns(5);	
			
				//Read the data from th STPM01 Device
				for(rxcount=0; rxcount<32; rxcount++)
				{
					rxData.STMP01Data.MeteringData[rxcount]=SPI2_IO(0xff);
				}
				
				wait10ns(5);		
				//NSS high
				GPIO_SetBits(GPIOB,GPIO_Pin_12);	

				//unmask the value
				PowerValueIn.active=((rxData.STMP01Data.msg.DAP&0x0FFFFF00)>>8);
				PowerValueIn.reactive=((rxData.STMP01Data.msg.DRP&0x0FFFFF00)>>8);
				PowerValueIn.apparent=((rxData.STMP01Data.msg.DSP&0x0FFFFF00)>>8);
				PowerValueIn.voltage=((rxData.STMP01Data.msg.DEV&0x07FF0000)>>16);// /4
				PowerValueIn.current=(rxData.STMP01Data.msg.DEV&0x0000FFFF);// *4

				if(PowerValueIn.active<PowerValueOld.active)
				{
					PowerValueOut.active=((1048575+PowerValueIn.active)-PowerValueOld.active);
				}
				else
				{
					PowerValueOut.active=(PowerValueIn.active-PowerValueOld.active);//*16.8089
				}
				
				if(PowerValueOut.voltage<40)
				{
					PowerValueOld.active=0;
				}
				PowerValueOld.active=PowerValueIn.active;
				PowerValueOut.voltage=PowerValueIn.voltage;
				PowerValueOut.current=PowerValueIn.current;
				
				//Send the calculated PowerValue to the Logger Task
				xQueueSend(xQueuePowerValue, (void *) &PowerValueOut, portMAX_DELAY);

    }
}
