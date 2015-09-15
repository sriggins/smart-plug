#ifndef LOGGER_H_
#define LOGGER_H_

/******************************************************************************/
/** \file       Logger.h
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
#include <stm32f10x_it.h>
#include <stdio.h>

#include <FreeRTOS.h>      
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>

//----- Macros -----------------------------------------------------------------
#define MAX_NBR_QUEUED_MESSAGES ( 3 )       /* Max. number of Messages      */

//----- Data types -------------------------------------------------------------


//----- Function prototypes ----------------------------------------------------
extern void  vLoggerTask(void *pdata);

//----- Data -------------------------------------------------------------------
struct Value
{
	uint32_t active;
	uint32_t reactive;
	uint32_t apparent;
	uint16_t voltage;
	uint16_t current;
};

struct Message
{
	uint32_t active;
	uint32_t reactive;
	uint32_t apparent;
	uint16_t voltage;
	uint16_t current;
	uint8_t hour[2];
	uint8_t minute[2];
};
struct Time
{
	uint8_t hour[2];
	uint8_t minute[2];
};





#endif /* LOGGER_H_ */
