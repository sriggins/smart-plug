#ifndef _NRF8001_H
#define _NRF8001_H

#include <stm32f10x_it.h>
#include <stdio.h>
#include "services.h"
#include "string.h"
	// variable used in data.h
	typedef uint8_t nRFLen;
	typedef uint8_t nRFPipe;
#include "constants.h"
#include "data.h" // data structures for requests and responses

	typedef struct {
			uint8_t status_byte;
			uint8_t buffer[32];
	} hal_aci_data_t;
	
	extern struct nRFEvent *rxEvent;

	extern GPIO_InitTypeDef GPIO_InitStructure;
	extern SPI_InitTypeDef SPI_InitStructure;

	// Intern functions
	enum nRFTxStatus transmitReceive(struct nRFCommand *txCmd, uint16_t timeout);
	enum nRFTxStatus transmitCommand(uint8_t command);
	enum nRFTxStatus transmitPipeCommand(uint8_t command, nRFPipe pipe);
	
	// Init functions
	extern enum nRFCmd setup(void);
	extern void nRF8001(void);
	
	// Informational functions
  extern void addressToString(char *str, uint8_t *address);
	extern enum nRFDeviceState getDeviceState(void);
	extern uint8_t creditsAvailable(void);
	extern void isConnected(void);
	extern enum nRFConnectionStatus getConnectionStatus(void);

	
	// Receive functions
	extern enum nRFTxStatus poll(uint16_t timeout);
	extern enum nRFTxStatus pollvoid(void);
	extern uint8_t isPipeOpen(nRFPipe servicePipeNo);
				
	// Transmit functions
	extern enum nRFTxStatus test(uint8_t feature);
	extern enum nRFTxStatus sleep(void);
	extern enum nRFTxStatus getDeviceVersion(void);
	extern enum nRFTxStatus echo(nRFLen dataLength, uint8_t *data);
	extern enum nRFTxStatus wakeup(void);
	extern enum nRFTxStatus getBatteryLevel(void);
	extern enum nRFTxStatus getTemperature(void);
	extern enum nRFTxStatus setTxPower(uint8_t powerLevel);
	extern enum nRFTxStatus getDeviceAddress(void);
	extern enum nRFTxStatus connect(uint8_t timeout1, uint8_t timeout2, uint8_t advInterval1, uint8_t advInterval2);
	extern enum nRFTxStatus radioReset(void);
	extern enum nRFTxStatus bond(uint16_t timeout, uint16_t advInterval);
	extern enum nRFTxStatus disconnect(uint8_t reason);
	extern enum nRFTxStatus changeTimingRequest(uint16_t intervalMin,
														 uint16_t intervalMax,
														 uint16_t slaveLatency,
														 uint16_t timeout);
	extern enum nRFTxStatus openRemotePipe(nRFPipe servicePipeNo);
	extern enum nRFTxStatus closeRemotePipe(nRFPipe servicePipeNo);
	extern enum nRFTxStatus dtmCommand(uint16_t dtmCmd);
	extern enum nRFTxStatus readDynamicData(void);
	extern enum nRFTxStatus writeDynamicData(uint8_t seqNo,
														 nRFLen dataLength,
														 uint8_t *data);
	extern enum nRFTxStatus setApplLatency(uint8_t applLatencyMode,
													 uint16_t latency);
	extern enum nRFTxStatus setKey(uint8_t keyType, uint8_t *key);
	extern enum nRFTxStatus openAdvPipe(void);
	extern enum nRFTxStatus broadcast(uint16_t timeout, uint16_t advInterval);
	extern enum nRFTxStatus bondSecurityRequest(void);
	extern enum nRFTxStatus directedConnect(void);
	extern enum nRFTxStatus sendData(nRFPipe servicePipeNo,nRFLen dataLength,char Message[20]);
	extern enum nRFTxStatus requestData(nRFPipe servicePipeNo);
	extern enum nRFTxStatus setLocalData(nRFPipe servicePipeNo,
												 nRFLen dataLength,
												 uint8_t *data);
	extern enum nRFTxStatus sendDataAck(nRFPipe servicePipeNo);
	extern enum nRFTxStatus sendDataNack(nRFPipe servicePipeNo,
												 uint8_t errorCode);
															 
	// System functions
	extern void wait( u32 timeout );
	extern int8_t SPI1_IO(int8_t data);


#endif /* _NRF8001_H */
