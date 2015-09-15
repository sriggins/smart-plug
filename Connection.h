#ifndef CONNECTION_H_
#define CONNECTION_H_

/******************************************************************************/
/** \file       Connection.h
 *******************************************************************************
 *
 *  \brief
 *
 *  \author     fonts2
 *
 ******************************************************************************/
/*
 *  function
 *
 ******************************************************************************/

//----- Header-Files -----------------------------------------------------------
#include <FreeRTOS.h>      
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
/* Library includes. */
#include "stm32f10x_it.h"
/* Standard includes. */
#include <stdio.h>
#include <nRF8001.h>
#include <Logger.h>
#include <string.h>


//----- Macros -----------------------------------------------------------------
#define MAX_NBR_QUEUED_ECTS ( 3 )       /* Max. number of Messages      */

//----- Data types -------------------------------------------------------------


//----- Function prototypes ----------------------------------------------------
extern void  vConnectionTask(void *pdata);

// //----- Data -------------------------------------------------------------------
 extern xQueueHandle xQueuePowerMessage;
 extern xQueueHandle xQueueTime;

#endif /* CONNECTION_H_ */
