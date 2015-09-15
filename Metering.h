#ifndef METERING_H_
#define METERING_H_

/******************************************************************************/
/** \file       Metering.h
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
#include <STPM01.h>
#include <Logger.h>

//----- Macros -----------------------------------------------------------------
#define MAX_NBR_QUEUED_MESSAGES ( 3 )       /* Max. number of Messages      */

//----- Data types -------------------------------------------------------------


//----- Function prototypes ----------------------------------------------------
extern void  vMeteringTask(void *pdata);

//----- Data -------------------------------------------------------------------
 extern xQueueHandle xQueuePowerValue;

#endif /* METERING_H_ */
