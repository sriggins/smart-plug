/******************************************************************************/
/** \file       Logger.c
 *******************************************************************************
 *
 *  \brief
 *
 *  \author     fonts2
 *
 *  \date       10.06.2013
 *
 *  \remark     Last Modification
 *               \li fonts2, 10.06.2013, Created
 *
 ******************************************************************************/
/*
 *  functions  global:
 *              vLoggerTask
 *  functions  local:
 *              .
 *
 ******************************************************************************/

//----- Header-Files -----------------------------------------------------------
#include "Logger.h"


//----- Macros -----------------------------------------------------------------

//----- Data types -------------------------------------------------------------

//----- Function prototypes ----------------------------------------------------

//----- Data -------------------------------------------------------------------
xQueueHandle xQueuePowerValue;
xQueueHandle xQueuePowerMessage;
xQueueHandle xQueueTime;

struct Value PowerValueLogger;
struct Message PowerMessageLogger;
struct Time TimeLogger;
uint8_t isTime;

//----- Implementation ---------------------------------------------------------
/*******************************************************************************
 *  function :    Logger
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
void vLoggerTask(void *pvData) {
	
		isTime=0;
	
    while (1) {
			// Checking if there are Messages in the Queue
			if(uxQueueMessagesWaiting(xQueueTime) != 0 )
			{
				// Receive Message from the Queue
				if( xQueueReceive( xQueueTime, &( TimeLogger ), ( portTickType ) 10 ) )
				{
					// Set the Time Flag
					isTime=1;
				}
				
			}		
			
			// Checking if there are Messages in the Queue
			if( uxQueueMessagesWaiting(xQueuePowerValue ) != 0 )
			{
				// Receive Message from the Queue
				if( xQueueReceive( xQueuePowerValue, &( PowerValueLogger ), ( portTickType ) 10 ) )
				{
					//Merge the Messages
					PowerMessageLogger.active=PowerValueLogger.active;
					PowerMessageLogger.reactive=PowerValueLogger.reactive;
					PowerMessageLogger.apparent=PowerValueLogger.apparent;
					PowerMessageLogger.voltage=PowerValueLogger.voltage;
					PowerMessageLogger.current=PowerValueLogger.current;
					PowerMessageLogger.hour[0]=TimeLogger.hour[0];
					PowerMessageLogger.hour[1]=TimeLogger.hour[1];
					PowerMessageLogger.minute[0]=TimeLogger.minute[0];
					PowerMessageLogger.minute[1]=TimeLogger.minute[1];
					
					//Check if the Time Flag is set
					if(isTime == 1)
					{
						//write the merged message in the Queue
						xQueueSend(xQueuePowerMessage, (void *) &PowerMessageLogger, portMAX_DELAY);
						//reset Time Flag
						isTime=0;
					}
			
				}
			}
			
	
 				vTaskDelay(10);
    }
}
