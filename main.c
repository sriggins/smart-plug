/******************************************************************************/
/** \file       main.c
 *******************************************************************************
 *
 *  \brief      This is the freeRTOS base project for STM32f103.
 *
 *  \author     fonts2
 *
 *  \date       16.05.2013
*
 *  \remark     Last Modification
 *               \li  fonts2, 16.05.2013   created
 *
 ******************************************************************************/
/*
 *  functions  global:
 *              main
 *  functions  local:
 *              vCreateTasks
 *
 ******************************************************************************/
 
//----- Header-Files -----------------------------------------------------------
/* Standard includes. */
#include <stdio.h>
#include "stm32f10x_it.h"

#include "Metering.h"
#include "Connection.h"
#include "Logger.h"

#include <FreeRTOS.h>      
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
#include "constants.h"


//----- Macros -----------------------------------------------------------------
#define	SIZE_OF_POWER_MESSAGE_QUEUE			2
#define	SIZE_OF_TASK_MESSAGE_QUEUE			20
#define	SIZE_OF_VALUE_ITEM				sizeof(struct Value)
#define	SIZE_OF_MESSAGE_ITEM			sizeof(struct Message)
#define	SIZE_OF_TIME_ITEM					sizeof(struct Time)

#define PRIORITY_METERING ( 7 )     		 /* Priority of METERING                 	  */
#define PRIORITY_CONNECTION ( 7 )      	/* Priority of CONNECTION                 	*/
#define PRIORITY_LOGGER ( 7 )      			/* Priority of LOGGER			                 	*/

#define STACKSIZE_TASK  ( 256 )       /* Stacksize of Task1 [Number of Words] */

//----- Data types -------------------------------------------------------------

//----- Function prototypes ----------------------------------------------------
static void prvSetupHardware( void );
static void Init( void );
static void  vCreateTasks(void);

//----- Data -------------------------------------------------------------------




/*-----------------------------------------------------------*/

/*******************************************************************************
 *  function :    main
 ******************************************************************************/
/** \brief        Initialize Hardware
 *
 *  \type         global
 *
 *  \return       error code
 *
 ******************************************************************************/
int main( void )
{
	//Messagequeues initialisieren
	xQueuePowerValue			 		= xQueueCreate(SIZE_OF_POWER_MESSAGE_QUEUE, SIZE_OF_VALUE_ITEM);
	xQueuePowerMessage				= xQueueCreate(SIZE_OF_POWER_MESSAGE_QUEUE, SIZE_OF_MESSAGE_ITEM);
	xQueueTime								= xQueueCreate(SIZE_OF_POWER_MESSAGE_QUEUE, SIZE_OF_TIME_ITEM);
	
	prvSetupHardware();
	Init();
	
	/* Create all application tasks and start the scheduler --------------------*/
  vCreateTasks();
  vTaskStartScheduler();

  /* If there is no failure, then this code is never reached */
	while ( 1 )
	{
		
	}
	
}

/*******************************************************************************
 *  function :    vCreateTasks
 ******************************************************************************/
/** \brief        Create all application task
 *
 *  \type         local
 *
 *  \return       void
 *
 ******************************************************************************/
static void vCreateTasks(void)  {

   xTaskCreate(vMeteringTask,
                (signed char *)"vMeteringTask",
                STACKSIZE_TASK,
                NULL,
                PRIORITY_METERING,
                NULL);

   xTaskCreate(vConnectionTask,
                (signed char *)"vConnectionTask",
                STACKSIZE_TASK,
                NULL,
                PRIORITY_CONNECTION,
                NULL);
	
	 xTaskCreate(vLoggerTask,
                (signed char *)"vLoggerTask",
                STACKSIZE_TASK,
                NULL,
                PRIORITY_LOGGER,
                NULL);
}

/*-----------------------------------------------------------*/

/*******************************************************************************
 *  function :    init
 ******************************************************************************/
/** \brief        Initialize Hardware
 *
 *  \type         global
 *
 *  \return       error code
 *
 ******************************************************************************/
static void Init( void )
{
	/* Configure the clocks, GPIO and other peripherals */
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBase_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
	// enable SPI 1 & 2 peripheral clock and GPIO
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	/****** LED and Switches *******/
	/*-----------------------------------------------*/
	// === INIT LED ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// === ZCR0 ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// === INIT Solid-State Relais ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// === INIT TASTER ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/****** nRF8001 Prepare pins and start SPI *******/
	/*-----------------------------------------------*/
	// === INIT reqn ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_9);
	
	// === INIT rdyn ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// === INIT reset ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_6);

	// === SPI1 ===

	/* configure SPI SCK -------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	/* configure SPI MISO ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	
	/* configure SPI MOSI ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOA, &GPIO_InitStructure );


	/* configure SPI1 as master-------------------------------------------------*/
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  SPI_Init( SPI1, &SPI_InitStructure );
	
	/* SPI1 ENABLE -------------------------------------------------------------*/
	SPI_Cmd(SPI1, ENABLE);
	
	/****** STPM01 Prepare pins and start SPI *******/
	/*-----------------------------------------------*/
	// === SYN0  ===
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_9);
	
	// === SPI2 ===
		/* configure SPI NSS ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	
	/* configure SPI MOSI ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	
	/* configure SPI SCK -------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	/* configure SPI MISO ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	

	/* configure SPI2 as master-------------------------------------------------*/
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//18MHz
  SPI_Init( SPI2, &SPI_InitStructure );
	
	/* SPI2 ENABLE -------------------------------------------------------------*/
	SPI_Cmd(SPI2, ENABLE);
	
	// === Timer ===
	/* MCO ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	RCC_MCOConfig(RCC_MCO_HSI);
	
	/* SCK ------------------------------------------------------*/	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_TimeBase_InitStructure.TIM_ClockDivision  = TIM_CKD_DIV1;
  TIM_TimeBase_InitStructure.TIM_CounterMode 	  = TIM_CounterMode_Up;
  TIM_TimeBase_InitStructure.TIM_Period 				= 799;
  TIM_TimeBase_InitStructure.TIM_Prescaler 			= 0;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBase_InitStructure);
 
  NVIC_InitStructure.NVIC_IRQChannel 						= TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_Init(&NVIC_InitStructure);


}

/*******************************************************************************
 *  function :    prvSetupHardware
 ******************************************************************************/
/** \brief        prvSetupHardware
 *
 *  \type         global
 *
 *  \return       error code
 *
 ******************************************************************************/
static void prvSetupHardware( void )
{
	/* Start with the clocks in their expected state. */
	RCC_DeInit();

	/* Enable HSE (high speed external clock). */
	RCC_HSEConfig( RCC_HSE_ON );

	/* Wait till HSE is ready. */
	while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
	{
	}

	/* 2 wait states required on the flash. */
	*( ( unsigned portLONG * ) 0x40022000 ) = 0x02;

	/* HCLK = SYSCLK */
	RCC_HCLKConfig( RCC_SYSCLK_Div1 );

	/* PCLK2 = HCLK */
	RCC_PCLK2Config( RCC_HCLK_Div1 );

	/* PCLK1 = HCLK/2 */
	RCC_PCLK1Config( RCC_HCLK_Div2 );

	/* PLLCLK = (16MHz / 2) * 9 = 72 MHz. */
	RCC_PLLConfig( RCC_PLLSource_HSE_Div2, RCC_PLLMul_9 );

	/* Enable PLL. */
	RCC_PLLCmd( ENABLE );

	/* Wait till PLL is ready. */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

	/* Wait till PLL is used as system clock source. */
	while( RCC_GetSYSCLKSource() != 0x08 )
	{
	}

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
							RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE );
	

	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
	
	
}
/*-----------------------------------------------------------*/

#ifdef  DEBUG
/* Keep the linker happy. */
void assert_failed( unsigned portCHAR* pcFile, unsigned portLONG ulLine )
{
	for( ;; )
	{
	}
}
#endif

