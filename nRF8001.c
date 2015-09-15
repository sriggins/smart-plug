/******************************************************************************/
/** \file       nRF8001.c
 *******************************************************************************
 *
 *  \brief      These are the functions for the nRF8001 Modul
 *
 *  \author     fonts2
 *
 *  \date       16.05.2013
*
 *  \remark     Last Modification
 *               \li  fonts2, 16.05.2013   created
 *               \li  fonts2, 03.06.2013   modified
 *
 ******************************************************************************/
/*
 *  functions  global:
 *              
 *  functions  local:
 *              
 *
 ******************************************************************************/

#include <nRF8001.h>

hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] = SETUP_MESSAGES_CONTENT;

//definitions
uint64_t pipesOpen;
int8_t rdyn_pin = 0;
int8_t reqn_pin = 0;

uint8_t credits;
enum nRFDeviceState deviceState;
enum nRFConnectionStatus connectionStatus;
int8_t nextSetupMessage;


enum nRFDeviceState getDeviceState()
{
    return deviceState;
}

enum nRFCmd setup()
{
    int8_t previousMessageSent = -1;
    for (;;) {
        transmitReceive(0, 0);
        if (deviceState == Setup || deviceState == PreSetup) {
           // Start the setup process
            nextSetupMessage = 0;
            break;
        } else {
           // not Ready
        }
    }

    for (;;) {
        if (nextSetupMessage >= 0
            && nextSetupMessage < NB_SETUP_MESSAGES
            && nextSetupMessage > previousMessageSent) {
							
            transmitReceive((struct nRFCommand *)setup_msgs[nextSetupMessage].buffer, 0);
            previousMessageSent = nextSetupMessage;
        } else if (nextSetupMessage >= 0
            && nextSetupMessage > previousMessageSent) {
						//invalid setup
            deviceState = Invalid;
            return cmdSetupError;
        } else if (nextSetupMessage == -1) {
            // Setup done
        }

        if (deviceState == Standby) {

          // setup successfull
					return cmdSuccess;
        }

        transmitReceive(0, 0);
    }

}

void nRF8001()
{
	// initialize start Data
	deviceState = Initial;
	credits = 0;
	nextSetupMessage = -2;
	connectionStatus = Disconnected;
}

void addressToString(char *str, uint8_t *address)
{
		int8_t i;
    for (i = NRF_ADDRESS_LENGTH - 1; i >= 0; i--) {
        uint8_t c = address[i];
        sprintf(str + (NRF_ADDRESS_LENGTH-1-i)*2, "%02x", c);
    }
}


// Send a Command and receive a event
enum nRFTxStatus transmitReceive(struct nRFCommand *txCmd, uint16_t timeout)
{
		  uint8_t *txBuffer = (uint8_t *)txCmd;
	    uint8_t txLength, txCommand;
			uint8_t nextByte;
    // Buffer that we will receive into
			uint8_t rxBuffer[sizeof(struct nRFEvent)];
			rxEvent=(struct nRFEvent*)rxBuffer;
			memset(&rxBuffer, 0, sizeof(struct nRFEvent));

    // Transmit length
    if (txCmd != NULL) {
        txLength = txCmd->length;
        txCommand = txCmd->command;
    } else {
        txLength = 0;
        txCommand = 0;
    }

    // Enough credits?
    if (txLength &&
       (txCmd->command == NRF_SENDDATA_OP
     || txCmd->command == NRF_REQUESTDATA_OP
     || txCmd->command == NRF_SETLOCALDATA_OP
     || txCmd->command == NRF_SENDDATAACK_OP
     || txCmd->command == NRF_SENDDATANACK_OP)) {
        if (credits < 1) {
            //nrf_debug("transmitReceive fail, not enough credits");
            return InsufficientCredits;
        }

        // Use a credit
        credits--;
    }

    // Bring REQN low
    if (txLength > 0) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    } 
    
    // TODO: Timeout

    if (txLength > 0 || timeout == 0) {
			
        // Wait for RDYN low indefinitely
        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)==1);

    } else {
        uint8_t rdy = 0;
        // Wait for RDYN low for 1 ms at a time
				uint16_t waitPeriods;
        for (waitPeriods = 0; waitPeriods < timeout; waitPeriods++) {
            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)== 0) {
                rdy = 1;
                break;
            } else {
                wait(1);
            }
        }

        if (!rdy) {
            return Timeout;
        }
    }

    if (txLength == 0) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
				wait(10);
    }

    // Send length and command bytes,
    // receive debug and length bytes
		  rxEvent->debug = SPI1_IO(txLength);
			rxEvent->length = SPI1_IO(txCommand);
		
    for (nextByte = 2; (nextByte < txLength + 1) || (nextByte < rxEvent->length + 2); nextByte++) 
		{
        uint8_t c;
        if (nextByte < txLength + 1) {
						SPI1_IO(txBuffer[nextByte]);// transmit
					
        } else {
            c = SPI1_IO(0x00); // receive only
        }

        if (nextByte < rxEvent->length + 2) { // receive
            rxBuffer[nextByte] = c;
        }
    }

    // Bring REQN high
		GPIO_SetBits(GPIOB, GPIO_Pin_9);
    // Wait for RDYN high
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == 0);

    // Return immediately if we didn't receive anything
    if (!rxEvent->length) {
        return Success;
    }

    // Handle response
    switch (rxEvent->event) {
				// DeviceStartedEvent
        case NRF_DEVICESTARTEDEVENT:
					  rxEvent->msg.deviceStarted.operatingMode				=rxEvent->aci.aci1;
            rxEvent->msg.deviceStarted.hwError							=rxEvent->aci.aci2;
            rxEvent->msg.deviceStarted.dataCreditAvailable	=rxEvent->aci.aci3;
						credits = rxEvent->msg.deviceStarted.dataCreditAvailable;

            switch (rxEvent->msg.deviceStarted.operatingMode) {
                case 0x01:
                    deviceState = Test;
                    break;
                case 0x02:
                    deviceState = Setup;
                    break;
                case 0x03:
                    if (deviceState == Initial) {
                        deviceState = PreSetup;
                    } 
										else 
										{
                        deviceState = Standby;
                    }
            }
            break;
						
				//Command Respond
        case NRF_COMMANDRESPONSEEVENT: 
							rxEvent->msg.commandResponse.opcode								=rxEvent->aci.aci1;
							rxEvent->msg.commandResponse.status								=rxEvent->aci.aci2;  

            if (rxEvent->msg.commandResponse.status	 != 0x00) {  
            
                if (rxEvent->msg.commandResponse.opcode	 == NRF_SETUP_OP &&
                    rxEvent->msg.commandResponse.status ==
                    NRF_STATUS_TRANSACTION_CONTINUE) {
                    nextSetupMessage++;
                    //nrf_debug("ready for next setup message");
                } else if (rxEvent->msg.commandResponse.opcode	== NRF_SETUP_OP &&
                    rxEvent->msg.commandResponse.status ==
                    NRF_STATUS_TRANSACTION_COMPLETE) {
                    //nrf_debug("setup complete");
                    nextSetupMessage = -1;
                }
                break;
            }

							switch (rxEvent->msg.commandResponse.opcode	) {
									case NRF_SETUP_OP:
											break;
									case NRF_GETTEMPERATURE_OP:             
													rxEvent->msg.commandResponse.data.temperature =(((rxEvent->aci.aci3)<<8)|(rxEvent->aci.aci4))/4;
	 
											break;
									case NRF_GETBATTERYLEVEL_OP:
													rxEvent->msg.commandResponse.data.voltage 		=(((rxEvent->aci.aci3)<<8)|(rxEvent->aci.aci4))* 0.00352;
											break;
									case NRF_GETDEVICEVERSION_OP:
													rxEvent->msg.commandResponse.data.getDeviceVersion.configurationId 				=((rxEvent->aci.aci3)<<8)|(rxEvent->aci.aci4);
													rxEvent->msg.commandResponse.data.getDeviceVersion.aciVersion							=rxEvent->aci.aci5;
													rxEvent->msg.commandResponse.data.getDeviceVersion.setupFormat						=rxEvent->aci.aci6;
													rxEvent->msg.commandResponse.data.getDeviceVersion.configurationStatus		=rxEvent->aci.aci11;
											break;
									case NRF_GETDEVICEADDRESS_OP:
	 											rxEvent->msg.commandResponse.data.getDeviceAddress.deviceAddress[0]	=rxEvent->aci.aci3;
												rxEvent->msg.commandResponse.data.getDeviceAddress.deviceAddress[1]	=rxEvent->aci.aci4;
												rxEvent->msg.commandResponse.data.getDeviceAddress.deviceAddress[2]	=rxEvent->aci.aci5;
												rxEvent->msg.commandResponse.data.getDeviceAddress.deviceAddress[3]	=rxEvent->aci.aci6;
												rxEvent->msg.commandResponse.data.getDeviceAddress.deviceAddress[4]	=rxEvent->aci.aci7;
												rxEvent->msg.commandResponse.data.getDeviceAddress.deviceAddress[5]	=rxEvent->aci.aci8;
												rxEvent->msg.commandResponse.data.getDeviceAddress.addressType 			=rxEvent->aci.aci9;
											break;
									case NRF_READDYNAMICDATA_OP:
												rxEvent->msg.commandResponse.data.readDynamicData.sequenceNo				=rxEvent->aci.aci3;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[0]    =rxEvent->aci.aci4;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[1]    =rxEvent->aci.aci5;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[2]    =rxEvent->aci.aci6;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[3]    =rxEvent->aci.aci7;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[4]    =rxEvent->aci.aci8;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[5]    =rxEvent->aci.aci9;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[6]    =rxEvent->aci.aci10;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[7]    =rxEvent->aci.aci11;
												rxEvent->msg.commandResponse.data.readDynamicData.dynamicData[8]    =rxEvent->aci.aci12;
											break;
									default:
											break;
							}
            break;
				
				//Conncted Event
        case NRF_CONNECTEDEVENT:
            connectionStatus = Connected;
//                     rxEvent->msg.connected.addressType,
//                     rxEvent->msg.connected.peerAddress,
                // TODO: put in other data
            break;
        case NRF_DISCONNECTEDEVENT:
            connectionStatus = Disconnected;
            pipesOpen = 0;
						rxEvent->msg.disconnected.aciStatus 		=rxEvent->aci.aci1;
						rxEvent->msg.disconnected.btLeStatus		=rxEvent->aci.aci2;
            break;
        case NRF_DATACREDITEVENT:
						rxEvent->msg.dataCredits 								=rxEvent->aci.aci1;
            credits += rxEvent->msg.dataCredits;
            break;
        case NRF_PIPESTATUSEVENT:
 						rxEvent->msg.pipeStatus.pipesOpen[0]		=rxEvent->aci.aci1;
						rxEvent->msg.pipeStatus.pipesOpen[1]		=rxEvent->aci.aci2;
						rxEvent->msg.pipeStatus.pipesOpen[2]		=rxEvent->aci.aci3;
						rxEvent->msg.pipeStatus.pipesOpen[3]		=rxEvent->aci.aci4;
						rxEvent->msg.pipeStatus.pipesOpen[4]		=rxEvent->aci.aci5;
						rxEvent->msg.pipeStatus.pipesOpen[5]		=rxEvent->aci.aci6;
						rxEvent->msg.pipeStatus.pipesOpen[6]		=rxEvent->aci.aci7;
						rxEvent->msg.pipeStatus.pipesOpen[7]		=rxEvent->aci.aci8;
            break;

        case NRF_DATARECEIVEDEVENT:
							rxEvent->msg.dataReceived.servicePipeNo 								=rxEvent->aci.aci1;
							rxEvent->msg.dataReceived.data[0]    										=rxEvent->aci.aci2;
							rxEvent->msg.dataReceived.data[1]     									=rxEvent->aci.aci3;
							rxEvent->msg.dataReceived.data[2]     									=rxEvent->aci.aci4;
							rxEvent->msg.dataReceived.data[3]     									=rxEvent->aci.aci5;
							rxEvent->msg.dataReceived.data[4]     									=rxEvent->aci.aci6;
							rxEvent->msg.dataReceived.data[5]     									=rxEvent->aci.aci7;
							rxEvent->msg.dataReceived.data[6]     									=rxEvent->aci.aci8;
							rxEvent->msg.dataReceived.data[7]     									=rxEvent->aci.aci9;
							rxEvent->msg.dataReceived.data[8]     									=rxEvent->aci.aci10;
							credits ++;
       
            break;
        case NRF_HARDWAREERROREVENT:
            break;
        default: 
            break;
        }
    return Success;
}

// Informational functions

uint8_t creditsAvailable()
{
    return credits;
}

void isConnected() {
    connectionStatus = Connected;
}

enum nRFConnectionStatus getConnectionStatus()
{
    return connectionStatus;
}

// Receive functions

enum nRFTxStatus poll(uint16_t timeout)
{
	return transmitReceive(0, timeout);
}

enum nRFTxStatus pollvoid()
{
  return transmitReceive(0, 0);
}

uint8_t isPipeOpen(nRFPipe servicePipeNo)
{
    return (rxEvent->msg.pipeStatus.pipesOpen[servicePipeNo]) != 0;
}

// Transmit functions

enum nRFTxStatus transmitCommand(uint8_t command)
{	
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			//nrf_debug("calling transmitCommand");
			struct nRFCommand cmd;
			cmd.length = 1;
			cmd.command = command;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus transmitPipeCommand(uint8_t command, nRFPipe pipe)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			//nrf_debug("calling transmitPipeCommand");

			struct nRFCommand cmd;
			cmd.length = 2;
			cmd.command = command;
			cmd.aci.aci1 = pipe;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus test(uint8_t feature)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			//nrf_debug("calling test");

			struct nRFCommand cmd;
			cmd.length = 2;
			cmd.command = NRF_TEST_OP;
			cmd.content.testFeature = feature;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus sleep()
{
    return transmitCommand(NRF_SLEEP_OP);
}

enum nRFTxStatus echo(nRFLen dataLength, uint8_t *data)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			if (dataLength > NRF_MAX_ECHO_MESSAGE_LENGTH) {
					//nrf_debug("data too long");
					return DataTooLong;
			}
			else{
				//nrf_debug("calling echo");

				struct nRFCommand cmd;
				cmd.length = dataLength + 1;
				cmd.command = NRF_ECHO_OP;
				memcpy(cmd.content.echoData, data, dataLength);
				return transmitReceive(&cmd, 0);
			}
		}
}

enum nRFTxStatus wakeup()
{
    return transmitCommand(NRF_WAKEUP_OP);
}

enum nRFTxStatus getBatteryLevel()
{
    return transmitCommand(NRF_GETBATTERYLEVEL_OP);
}

enum nRFTxStatus getTemperature()
{
    return transmitCommand(NRF_GETTEMPERATURE_OP);
}

enum nRFTxStatus setTxPower(uint8_t powerLevel)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 2;
			cmd.command = NRF_SETTXPOWER_OP;
			cmd.content.radioTxPowerLevel = powerLevel;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus getDeviceAddress()
{
    return transmitCommand(NRF_GETDEVICEADDRESS_OP);
}

enum nRFTxStatus getDeviceVersion()
{
    return transmitCommand(NRF_GETDEVICEVERSION_OP);
}

enum nRFTxStatus connect(uint8_t timeout1, uint8_t timeout2, uint8_t advInterval1, uint8_t advInterval2)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			//nrf_debug("connecting");

			struct nRFCommand cmd;
			cmd.length = 5;
			cmd.command = NRF_CONNECT_OP;
			cmd.aci.aci1 = timeout1;
			cmd.aci.aci2 = timeout2;
			cmd.aci.aci3 = advInterval1;
			cmd.aci.aci4 = advInterval2;
			
			connectionStatus = Connecting;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus radioReset()
{
    //nrf_debug("sending radio reset");
    struct nRFCommand cmd;
    cmd.length = 1;
    cmd.command = NRF_RADIORESET_OP;
    return transmitReceive(&cmd, 0);
}

enum nRFTxStatus bond(uint16_t timeout, uint16_t advInterval)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 5;
			cmd.command = NRF_BOND_OP;
			cmd.content.bond.timeout = timeout;
			cmd.content.bond.advInterval = advInterval;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus disconnect(uint8_t reason)
{
    if (deviceState != Standby || connectionStatus != Connected) {
        //nrf_debug("device not in standby and connected state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 2;
			cmd.command = NRF_DISCONNECT_OP;
			cmd.content.disconnectReason = reason;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus changeTimingRequest(uint16_t intervalMin,
                                   uint16_t intervalMax,
                                   uint16_t slaveLatency,
                                   uint16_t timeout)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
    struct nRFCommand cmd;
    cmd.command = NRF_CHANGETIMINGREQUEST_OP;

    if (intervalMin > 0 || intervalMax > 0
        || slaveLatency > 0 || timeout >> 0) {
        cmd.length = 9;
        cmd.content.changeTimingRequest.intervalMin = intervalMin;
        cmd.content.changeTimingRequest.intervalMax = intervalMax;
        cmd.content.changeTimingRequest.slaveLatency = slaveLatency;
        cmd.content.changeTimingRequest.timeout = timeout;
    } else {
        cmd.length = 1;
    }

    return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus openRemotePipe(nRFPipe servicePipeNo)
{
    return transmitPipeCommand(NRF_OPENREMOTEPIPE_OP, servicePipeNo);
}

enum nRFTxStatus closeRemotePipe(nRFPipe servicePipeNo)
{
    return transmitPipeCommand(NRF_CLOSEREMOTEPIPE_OP, servicePipeNo);
}

enum nRFTxStatus dtmCommand(uint16_t dtmCmd)
{
    struct nRFCommand cmd;
    cmd.length = 3;
    cmd.command = NRF_DTMCOMMAND_OP;
    cmd.content.dtmCommand = dtmCmd;
    return transmitReceive(&cmd, 0);
}

enum nRFTxStatus readDynamicData()
{
    return transmitCommand(NRF_READDYNAMICDATA_OP);
}

enum nRFTxStatus writeDynamicData(uint8_t seqNo, nRFLen dataLength,
    uint8_t *data)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = dataLength + 2;
			cmd.command = NRF_WRITEDYNAMICDATA_OP;
			cmd.content.writeDynamicData.sequenceNo = seqNo;
			memcpy(cmd.content.writeDynamicData.dynamicData, data, dataLength);
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus setApplLatency(uint8_t applLatencyMode, uint16_t latency)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 4;
			cmd.command = NRF_SETAPPLICATIONLATENCY_OP;
			cmd.content.setApplLatency.applLatencyMode = applLatencyMode;
			cmd.content.setApplLatency.latency = latency;
			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus setKey(uint8_t keyType, uint8_t *key)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.command = NRF_SETKEY_OP;
			cmd.content.setKey.keyType = keyType;

			if (keyType == 0) {
					cmd.length = 2;
			} else if (keyType == 1) {
					cmd.length = 2 + NRF_PASSKEY_LENGTH;
					memcpy(cmd.content.setKey.key, key, NRF_PASSKEY_LENGTH);
			} else {
					return InvalidParameter;
			}

			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus openAdvPipe(void)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 9;
			cmd.command = NRF_OPENADVPIPE_OP;
			cmd.aci.aci1 = 0x00;
			cmd.aci.aci2 = 0x00;
			cmd.aci.aci3 = 0x00;
			cmd.aci.aci4 = 0x00;
			cmd.aci.aci5 = 0x00;
			cmd.aci.aci6 = 0x00;
			cmd.aci.aci7 = 0x00;
			cmd.aci.aci8 = 0x01;

			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus broadcast(uint16_t timeout, uint16_t advInterval)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 5;
			cmd.command = NRF_BROADCAST_OP;
			cmd.content.broadcast.timeout = timeout;
			cmd.content.broadcast.advInterval = advInterval;

			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus bondSecurityRequest()
{
    return transmitCommand(NRF_BONDSECREQUEST_OP);
}

enum nRFTxStatus directedConnect()
{
    return transmitCommand(NRF_DIRECTEDCONNECT_OP);
}

enum nRFTxStatus sendData(nRFPipe servicePipeNo,nRFLen dataLength,char Message[20])
{
		
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }

    else if (connectionStatus != Connected) {
        //nrf_debug("device not connected");
        return NotConnected;
    }

    else if (!(rxEvent->msg.pipeStatus.pipesOpen[0])&(1<<servicePipeNo)) {
        //nrf_debug("pipe not open");
        return PipeNotOpen;
    }

    else if (credits <= 0) {
        //nrf_debug("not enough credits");
        return InsufficientCredits;
    }

    else if (dataLength > NRF_DATA_LENGTH) {
        //nrf_debug("data too long");
        return DataTooLong;
    }
		else{				
			struct nRFCommand cmd;
			cmd.command = NRF_SENDDATA_OP;
			cmd.length = dataLength + 2;
			cmd.aci.aci1 = servicePipeNo;
			cmd.aci.aci2 = Message[0];
			cmd.aci.aci3 = Message[1];	
			cmd.aci.aci4 = Message[2];
			cmd.aci.aci5 = Message[3];
			cmd.aci.aci6 = Message[4];
			cmd.aci.aci7 = Message[5];
			cmd.aci.aci8 = Message[6];
			cmd.aci.aci9 = Message[7];
			cmd.aci.aci10 = Message[8];
			cmd.aci.aci11 = Message[9];
			cmd.aci.aci12 = Message[10];
			cmd.aci.aci13 = Message[11];
			cmd.aci.aci14 = Message[12];
			cmd.aci.aci15 = Message[13];
			cmd.aci.aci16 = Message[14];
			cmd.aci.aci17 = Message[15];
			cmd.aci.aci18 = Message[16];
			cmd.aci.aci19 = Message[17];
			cmd.aci.aci20 = Message[18];
			cmd.aci.aci21 = Message[19];

			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus requestData(nRFPipe servicePipeNo)
{
    return transmitPipeCommand(NRF_REQUESTDATA_OP, servicePipeNo);
}

enum nRFTxStatus setLocalData(nRFPipe servicePipeNo, nRFLen dataLength,
    uint8_t *data)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 2 + dataLength;
			cmd.command = NRF_SETLOCALDATA_OP;
			cmd.content.data.servicePipeNo = servicePipeNo;

			memcpy(cmd.content.data.data, data, dataLength);

			return transmitReceive(&cmd, 0);
		}
}

enum nRFTxStatus sendDataAck(nRFPipe servicePipeNo)
{
    return transmitPipeCommand(NRF_SENDDATAACK_OP, servicePipeNo);
}

enum nRFTxStatus nsendDataNack(nRFPipe servicePipeNo, uint8_t errorCode)
{
    if (deviceState != Standby) {
        //nrf_debug("device not in Standby state");
        return InvalidState;
    }
		else{
			struct nRFCommand cmd;
			cmd.length = 3;
			cmd.command = NRF_SENDDATANACK_OP;
			cmd.content.sendDataNack.servicePipeNo = servicePipeNo;
			cmd.content.sendDataNack.errorCode = errorCode;

			return transmitReceive(&cmd, 0);
		}
}

// System functions
void wait( u32 timeout )
{
	u32 i;
	timeout=timeout*72000;
	for(i=timeout; i>0; i --)
	{
		//wait
	}
}

int8_t SPI1_IO(int8_t data)
{
  SPI_I2S_SendData(SPI1,data);
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET){;}
  return SPI_I2S_ReceiveData(SPI1);
}
