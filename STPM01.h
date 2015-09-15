#ifndef _STPM01_H
#define _STPM01_H

#include <stm32f10x_it.h>
#include <stdio.h>
extern int8_t SCK_Flag;
extern int8_t SCK_rxFlag;

struct ReceiveData{ 
union {
	uint8_t MeteringData[32];
	
	struct {
		uint32_t DAP;
		uint32_t DRP;
		uint32_t DSP;
		uint32_t DFP;
		uint32_t DEV;
		uint32_t DMV;
		uint32_t CFL;
		uint32_t CFH;
	} msg;
	
}STMP01Data;
};


extern int8_t SPI2_IO(int8_t data);
extern void SendtoSTPM01(uint8_t data);
extern void SetupSTPM01(void);
extern void reset_STPM01(void);
extern void wait10ns( u32 timeout );
extern uint8_t ReadfromSTPM01(void);


#endif /* _STPM01_H */
