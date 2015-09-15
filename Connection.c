/******************************************************************************/
/** \file       Connection.c
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
 *              vConnetionTask
 *  functions  local:
 *              .
 *
 ******************************************************************************/

//----- Header-Files -----------------------------------------------------------
#include "Connection.h"

//----- Macros -----------------------------------------------------------------

//----- Data types -------------------------------------------------------------
struct nRFEvent *rxEvent;
enum nRFConnectionStatus connectionState;
enum nRFDeviceState deviceCurrentState;
//----- Function prototypes ----------------------------------------------------

//----- Data -------------------------------------------------------------------
struct Message PowerMessage;
struct Time CurrentTime;
int8_t DeviceAdress[6];
uint8_t Data[27];
int8_t txcount;
int8_t numberOfMessage;
int8_t length;
char OutMessage[20];
float active;
float voltage;
float current;


//----- Implementation ---------------------------------------------------------
/*******************************************************************************
 *  function :    Connection
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
void vConnectionTask(void *pvData) {
	
		//nRF8001 initialisieren
		nRF8001();
		vTaskDelay(10);

    while (1) {
			

			
			//Check the current DeviceState
			deviceCurrentState=getDeviceState();
			switch(deviceCurrentState)
			{
					case Initial:
						setup(); 				//DeviceStartedEvent
						break;
					
					case Setup:
						break;
					
					case Standby:
						
						//Check the current connectionState
						connectionState=getConnectionStatus();
						switch(connectionState)
						{
								//Is not connected
								case Disconnected:
									connect(0x3f,0xff,0x40,0x00);
									poll(0x200);		//CommandResponseEvent
									pollvoid();			//ConnectEvent
								break;
								
								//Is connected
								case Connected:
											//Open the Pipe to Receive
											openRemotePipe(1);
											pollvoid();				//CommandResponseEvent
								
											//Request the Data
											requestData(1);
											pollvoid();				//DataReceivedEvent
										
											//Write the Data in local varible
											length = rxEvent->length;
											for(txcount=0;txcount<=length-3;txcount++)
											{
												Data[txcount]=rxEvent->msg.dataReceived.data[txcount];
											}
											
											//Close the Pipe
											closeRemotePipe(1);
											pollvoid();								

											// Check the Received Message from the PeerDevice
											switch((Data[0]<<8)|Data[1])
											{
												//Turn the Socket on
												case PLUG_ON:
													//Sets the Control LED
													GPIO_SetBits(GPIOB, GPIO_Pin_11);
													//Solid State Relay active
													GPIO_SetBits(GPIOB, GPIO_Pin_0);
													break;
												
												//Turn the Socket off
												case PLUG_OFF:
													//Resets the Control LED
													GPIO_ResetBits(GPIOB, GPIO_Pin_11);
													//Resets the Solid State Relay
													GPIO_ResetBits(GPIOB, GPIO_Pin_0);
													break;
												
												// Get the data
												case GET_DATA:
														//write the received time in a local varible
														CurrentTime.hour[0]=Data[4];
														CurrentTime.hour[1]=Data[3];
														
														CurrentTime.minute[0]=Data[6];
														CurrentTime.minute[1]=Data[5];
												
														//update the time
														xQueueSend(xQueueTime, (void *) &CurrentTime, portMAX_DELAY);

														//Receive PowerMessage from Logger Task
													
														xQueueReceive( xQueuePowerMessage, &( PowerMessage ), portMAX_DELAY );
															active=(float)PowerMessage.active/60;
															voltage=(float)PowerMessage.voltage/4;
															current=(float)PowerMessage.current*4;
														
														switch(Data[2])
														{
															case ACTIVE_POWER:
																//write the active Power in the Outgoing Message
																sprintf(OutMessage,"P:%4.1f W %c%c:%c%c",active, PowerMessage.hour[1], PowerMessage.hour[0], PowerMessage.minute[1], PowerMessage.minute[0]);
																break;
															
															case REACTIVE_POWER:
																//write the reactive Power in the Outgoing Message
																sprintf(OutMessage,"Q:%d mVAr %c%c:%c%c",PowerMessage.reactive, PowerMessage.hour[1], PowerMessage.hour[0], PowerMessage.minute[1], PowerMessage.minute[0]);
																break;
															
															case APPARENT_POWER:
																//write the apparent Power in the Outgoing Message
																sprintf(OutMessage,"S:%d mVA %c%c:%c%c",PowerMessage.apparent, PowerMessage.hour[1], PowerMessage.hour[0], PowerMessage.minute[1], PowerMessage.minute[0]);
																break;
															
															case U_RMS:
																//write the RMS voltage in the Outgoing Message
																sprintf(OutMessage,"U:%4.1f V %c%c:%c%c",voltage, PowerMessage.hour[1], PowerMessage.hour[0], PowerMessage.minute[1], PowerMessage.minute[0]);
																break;
															
															case I_RMS:
																//write the RMS current in the Outgoing Message
																sprintf(OutMessage,"I:%4.1f mA %c%c:%c%c",current, PowerMessage.hour[1], PowerMessage.hour[0], PowerMessage.minute[1], PowerMessage.minute[0]);
																break;
															default:
																break;
												
															
														}		
														
														//Open the send Pipe
														openRemotePipe(2);
														pollvoid();				//CommandResponseEvent
												
														// Send the Output Message to the Peer Device
														sendData(2,20,OutMessage);//s135k
														
														for(txcount=0;txcount<20;txcount++)
														{
															OutMessage[txcount]=0;
														}
												
														//Close the Send Pipe
														closeRemotePipe(2);
														pollvoid();							
													break;
												default:
													break;
											}
									break;
									
								//Is connecting
								case Connecting:
									break;
							
								default:
									break;	
						}
						break;
					
					case Invalid:
						break;
					
					default:
						break;			
			}

			
			vTaskDelay(10);
	}
}
