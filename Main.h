#ifndef MAIN_H_
#define MAIN_H_

/******************************************************************************/
/** \file       Main.h
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
/* Standard includes. */
#include <stdio.h>
#include "stm32f10x_it.h"

#include "Metering.h"
#include "Connection.h"

#include <FreeRTOS.h>      
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
#include "constants.h"


//----- Macros -----------------------------------------------------------------
#define	SIZE_OF_POWER_MESSAGE_QUEUE			40
#define	SIZE_OF_TASK_MESSAGE_QUEUE			20
#define	SIZE_OF_MESSAGE_ITEM			sizeof(Power_Message)

#define PRIORITY_METERING ( 7 )     	 /* Priority of METERING                   */
#define PRIORITY_CONNECTION ( 7 )      /* Priority of CONNECTION                 */

#define STACKSIZE_TASK  ( 256 )       /* Stacksize of Task1 [Number of Words] */

//----- Data types -------------------------------------------------------------
typedef struct sp{
	uint16_t real;
	uint16_t reactiv;
}Power_Message;

//----- Function prototypes ----------------------------------------------------

//----- Data -------------------------------------------------------------------


#endif /* MAIN_H_ */
