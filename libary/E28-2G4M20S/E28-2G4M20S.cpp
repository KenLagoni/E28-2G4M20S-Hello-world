/*
	E28-2G4M20S.cpp

	Copyright (c) 2019 Lagoni
	Not for commercial use
 */ 

#include "E28-2G4M20S.h"
#include "radio.h"
#include "sx1280-hal.h"
//#include "Arduino.h" // Needed for Serial.print
//#include <stdlib.h>



E28_2G4M20S::E28_2G4M20S(int chipSelectPin, int resetPin, int busyPin, int dio1Pin, int dio2Pin, int dio3Pin, int txEnablePin, int rxEnablePin)
{
	_chipSelectPin = chipSelectPin;
	_resetPin      = resetPin;
	_busyPin       = busyPin;
	_dio1Pin       = dio1Pin;
	_dio2Pin       = dio2Pin;
	_dio3Pin       = dio3Pin;
	_txEnablePin   = txEnablePin;
	_rxEnablePin   = rxEnablePin;
	
	Radio = new SX1280Hal(_chipSelectPin, _busyPin, _dio1Pin, _dio2Pin, _dio3Pin, _resetPin);	
		
	pinMode(_txEnablePin, OUTPUT);
	digitalWrite(_txEnablePin, LOW);	
	pinMode(_rxEnablePin, OUTPUT);
	digitalWrite(_rxEnablePin, HIGH);		
}

void E28_2G4M20S::SetTxModeActive( void )
{
	digitalWrite(_rxEnablePin, LOW);
	digitalWrite(_txEnablePin, HIGH);	
}

void E28_2G4M20S::SetRxModeActive( void )
{
	digitalWrite(_txEnablePin, LOW);
	digitalWrite(_rxEnablePin, HIGH);	
}

void E28_2G4M20S::Init()
{
	 Radio->Init( );	 
	 Radio->SetRegulatorMode( USE_DCDC ); // Can also be set in LDO mode but consume more power
	 memset( &Buffer, 0x00, MAX_PAYLOAD_LENGTH ); // Zero fills the buffer
	 
	#if defined( MODE_BLE )

    SerialUSB.print( "\nRunning in BLE mode\n\r");
    modulationParams.PacketType                   = PACKET_TYPE_BLE;
    modulationParams.Params.Ble.BitrateBandwidth  = GEN_BLE_BR_0_125_BW_0_3;
    modulationParams.Params.Ble.ModulationIndex   = GEN_BLE_MOD_IND_1_00;
    modulationParams.Params.Ble.ModulationShaping = RADIO_MOD_SHAPING_BT_1_0;

    packetParams.PacketType                 = PACKET_TYPE_BLE;
    packetParams.Params.Ble.BlePacketType   = BLE_EYELONG_1_0;
    packetParams.Params.Ble.ConnectionState = BLE_MASTER_SLAVE;
    packetParams.Params.Ble.CrcField        = BLE_CRC_3B;
    packetParams.Params.Ble.Whitening       = RADIO_WHITENING_OFF;

	#elif defined( MODE_GENERIC )

    SerialUSB.print( "\nRunning in GENERIC mode\n\r");
    modulationParams.PacketType                       = PACKET_TYPE_GENERIC;
    modulationParams.Params.Generic.BitrateBandwidth  = GEN_BLE_BR_0_125_BW_0_3;
    modulationParams.Params.Generic.ModulationIndex   = GEN_BLE_MOD_IND_1_00;
    modulationParams.Params.Generic.ModulationShaping = RADIO_MOD_SHAPING_BT_1_0;

    packetParams.PacketType                    = PACKET_TYPE_GENERIC;
    packetParams.Params.Generic.PreambleLength = PREAMBLE_LENGTH_32_BITS;
    packetParams.Params.Generic.SyncWordLength = GEN_SYNCWORD_LENGTH_5_BYTE;
    packetParams.Params.Generic.SyncWordMatch  = RADIO_RX_MATCH_SYNCWORD_1;
    packetParams.Params.Generic.HeaderType     = RADIO_PACKET_VARIABLE_LENGTH;
    packetParams.Params.Generic.PayloadLength  = MAX_PAYLOAD_LENGTH;
    packetParams.Params.Generic.CrcLength      = RADIO_CRC_3_BYTES;
    packetParams.Params.Generic.Whitening      = RADIO_WHITENING_ON;

	#elif defined( MODE_LORA )
	
    Serial.println( "Running in LORA mode");
	Serial.println( "Spreading factor set to 7");
	Serial.println( "Bandwidth set to 400kHz");
	Serial.println( "CR encoding set to 4/5");
    modulationParams.PacketType                  = PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.SpreadingFactor = LORA_SF7;
    modulationParams.Params.LoRa.Bandwidth       = LORA_BW_0400;
    modulationParams.Params.LoRa.CodingRate      = LORA_CR_4_5;

    PacketParams.PacketType                 = PACKET_TYPE_LORA;
    PacketParams.Params.LoRa.PreambleLength = 0x08;
    PacketParams.Params.LoRa.HeaderType     = LORA_PACKET_VARIABLE_LENGTH;
    PacketParams.Params.LoRa.PayloadLength  = MAX_PAYLOAD_LENGTH;
    PacketParams.Params.LoRa.CrcMode        = LORA_CRC_ON;
    PacketParams.Params.LoRa.InvertIQ       = LORA_IQ_INVERTED;

	#elif defined( MODE_FLRC )

    SerialUSB.print( "\nRunning in FLRC mode\n\r");
    modulationParams.PacketType                    = PACKET_TYPE_FLRC;
    modulationParams.Params.Flrc.BitrateBandwidth  = FLRC_BR_0_260_BW_0_3;
    modulationParams.Params.Flrc.CodingRate        = FLRC_CR_1_2;
    modulationParams.Params.Flrc.ModulationShaping = RADIO_MOD_SHAPING_BT_1_0;

    packetParams.PacketType                 = PACKET_TYPE_FLRC;
    packetParams.Params.Flrc.PreambleLength = PREAMBLE_LENGTH_32_BITS;
    packetParams.Params.Flrc.SyncWordLength = FLRC_SYNCWORD_LENGTH_4_BYTE;
    packetParams.Params.Flrc.SyncWordMatch  = RADIO_RX_MATCH_SYNCWORD_1;
    packetParams.Params.Flrc.HeaderType     = RADIO_PACKET_VARIABLE_LENGTH;
    packetParams.Params.Flrc.PayloadLength  = MAX_PAYLOAD_LENGTH;
    packetParams.Params.Flrc.CrcLength      = RADIO_CRC_3_BYTES;
    packetParams.Params.Flrc.Whitening      = RADIO_WHITENING_OFF;

	#else
	#error "Please select the mode of operation for the RC Transponder"
	#endif

	Radio->SetStandby( STDBY_RC );
    Radio->SetPacketType( modulationParams.PacketType );
    Radio->SetModulationParams( &modulationParams );
    Radio->SetPacketParams( &PacketParams );

    Radio->SetRfFrequency( rf_frequency );
    Radio->SetBufferBaseAddresses( 0x00, 0x00 );
    Radio->SetTxParams( tx_power, RADIO_RAMP_20_US ); 
	Serial.println( "Frequency set to " + String(rf_frequency/1000000) + "MHz");
	Serial.println( "TX Power set to " + String(tx_power) + "dBm");
}


void E28_2G4M20S::OnTxDone( void )
{
	// switch PA to RX.
	SetRxModeActive();
	SetRXMode(false); // Set to RX with no timout.
	radioIdle = true;
	digitalWrite(21, LOW);
}

void E28_2G4M20S::OnRxDone( void )
{
	if(BufferReady==true){
		// If buffer us ready, means application is not done reading the data.
		return;
	}
	memset(&Buffer, 0x00, MAX_PAYLOAD_LENGTH);
	BufferSize=0;
	if(Radio->GetPayload(Buffer, &BufferSize, MAX_PAYLOAD_LENGTH-2)){
		// If return 1, then package size is larger than MAX_PAYLOAD_LENGTH
		Serial.println("Oops! - New package is to big. Length="+String(BufferSize)+". Max Size="+String(MAX_PAYLOAD_LENGTH-2));
	}else{
		// New data has been copied to buffer.
		Radio->GetPacketStatus(&PacketStatus);
		switch( PacketStatus.packetType )
		{
			case PACKET_TYPE_GFSK:
			Buffer[BufferSize]	 = PacketStatus.Gfsk.RssiSync;
			BufferSize=BufferSize+1;
			break;

			case PACKET_TYPE_LORA:
			case PACKET_TYPE_RANGING:
			Buffer[BufferSize] = PacketStatus.LoRa.RssiPkt;
			Buffer[BufferSize+1] = PacketStatus.LoRa.SnrPkt;
			BufferSize=BufferSize+2;
			break;

			case PACKET_TYPE_FLRC:
			Buffer[BufferSize] = PacketStatus.Flrc.RssiSync;
			BufferSize=BufferSize+1;
			break;

			case PACKET_TYPE_BLE:
			Buffer[BufferSize] = PacketStatus.Ble.RssiSync;
			BufferSize=BufferSize+1;
			break;

			case PACKET_TYPE_NONE:
			Buffer[BufferSize] = 0;
			break;
		}
		
		//Serial.println("New package received. Length="+String(BufferSize));
		BufferReady=true;
	}
}

/*
void E28_2G4M20S::OnRxDone( void )
{
	// Get radio telegram
	RadioTelegram_t telegram;
	
	memset(&Buffer, 0x00, PAYLOAD_LENGTH);
	uint8_t size=PAYLOAD_LENGTH;
	Radio->GetPayload(Buffer, &size, PAYLOAD_LENGTH);
	Radio->GetPacketStatus(&PacketStatus);
	switch( PacketStatus.packetType )
	{
		case PACKET_TYPE_GFSK:
			telegram.rssi = PacketStatus.Gfsk.RssiSync;
		break;

		case PACKET_TYPE_LORA:
		case PACKET_TYPE_RANGING:
			telegram.rssi = PacketStatus.LoRa.RssiPkt;
		break;

		case PACKET_TYPE_FLRC:
			telegram.rssi = PacketStatus.Flrc.RssiSync;		
		break;

		case PACKET_TYPE_BLE:
			telegram.rssi = PacketStatus.Ble.RssiSync;
		break;

		case PACKET_TYPE_NONE:
			telegram.rssi = 0;
		break;
	 }

	telegram.receiver_ID = Buffer[0];
	telegram.transmitter_ID = Buffer[1];
	telegram.UTCTime =   (uint32_t)((Buffer[2] << 16) + (Buffer[3] << 8) + Buffer[4]);
	telegram.Latitude =  (uint32_t)((Buffer[5] << 24) + (Buffer[6] << 16) + (Buffer[7] << 8) + Buffer[8]);
	telegram.Longitude = (uint32_t)((Buffer[9] << 24) + (Buffer[10] << 16) + (Buffer[11] << 8) + Buffer[12]);
	telegram.Fix = (Buffer[13] >> 2) & 0b00000011;
	telegram.PCBVersion = ((Buffer[13] >> 4) & 0b00001111) + 10;
	telegram.FirmwwareVersion = ((float)Buffer[14]) + ((float)Buffer[15])/(float)100;
	telegram.NumberOfSatellites = Buffer[16];
	telegram.BatteryVoltage = (((float)Buffer[17])+(float)200)/(float)100;
	
	int16_t dummy = (int16_t)((Buffer[18] << 8) + Buffer[19]);
	telegram.Altitude = ((float)dummy)/10;
	
	
	// If message ok, then make i available.
	this->LastMsg = telegram;
	this->telegramValid = true;
	
}
*/

// Only call this when interrupt has occured.
void E28_2G4M20S::HandleIRQ( void )
{
	Radio->ProcessIrqs();	
//	Serial.println("E28 Radio Interrupt!");
	
	if(Radio->RadioPacketStatus.txDone == true){
//		Serial.println("TX Done!");
		this->OnTxDone();
	}    
	if(Radio->RadioPacketStatus.rxDone == true){
//		Serial.println("RX Done!");
		this->OnRxDone();
		// read the message, check CRC if ok, make available.

	}
	if(Radio->RadioPacketStatus.rxSyncWordDone == true){
		Serial.println("rxSyncWordDone!");
	}	
	if(Radio->RadioPacketStatus.rxHeaderDone == true){
		Serial.println("rxHeaderDone!");
	}
	if(Radio->RadioPacketStatus.txTimeout == true){
		Serial.println("TX Timeout!");
		radioIdle = true;
	}
	if(Radio->RadioPacketStatus.rxTimeout == true){
		Serial.println("RX Timeout!");
		this->SetRXMode(true); // Do it again!
	}	
	if(Radio->RadioPacketStatus.rxError > 0x00){
		Serial.println("IRQ Error! " + String(Radio->RadioPacketStatus.rxError));
	}
	if(Radio->RadioPacketStatus.rangingDone > 0x00){
		Serial.println("IRQ Ranging Code! " + String(Radio->RadioPacketStatus.rangingDone));
	}
	if(Radio->RadioPacketStatus.cadDone == true){
		Serial.println("CAD Done!");
	}
	
}

uint8_t E28_2G4M20S::GetPackage(uint8_t *payload, uint8_t maxSize){
	if(this->BufferReady){
		if(this->BufferSize < maxSize){
			return 0;
		}else{
			memcpy(&this->Buffer, payload, BufferSize+2);	
			this->BufferReady = false;
			return this->BufferSize+2; // Payload+1 byte of RSSI and 1 byte of SNR
		}
	}
}

void E28_2G4M20S::SendPackage(uint8_t *payload, uint8_t payloadLength)
{
   this->SetTxModeActive(); // Switch the hardware amplifier to TX mode.
   Radio->SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE ); // Set module to interrupt on TX complete on DI01.
   this->PacketParams.Params.LoRa.PayloadLength = payloadLength;
   Radio->SetPacketParams( &PacketParams );
   Radio->SendPayload(payload, payloadLength, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
   radioIdle = false;
   digitalWrite(21, HIGH);
}

void E28_2G4M20S::Debug(void){
		Serial.println("Radio Firmware version: " + String(Radio->GetFirmwareVersion()));
}

void E28_2G4M20S::SetRXMode(bool useTimeout)
{
   this->SetRxModeActive(); // Switch the hardware amplifier to RX mode.
   Radio->SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE ); // Set module to interrupt on RX complete on DI01 or Timeout.
   if(useTimeout){
		Radio->SetRx( ( TickTime_t ) { RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE } );
   }else{
		Radio->SetRx(RX_TX_CONTINUOUS);
   }
}

/*
void E28_2G4M20S::SendTelegram(RadioTelegram_t *telegram)
{
	memset(&Buffer, 0x00, MAX_PAYLOAD_LENGTH);
	Buffer[0] = telegram->receiver_ID;
	Buffer[1] = telegram->transmitter_ID;
	
	// Time transformed to 24bits.
	Buffer[2] = (uint8_t)((telegram->UTCTime >> 16) & 0xFF);
	Buffer[3] = (uint8_t)((telegram->UTCTime >> 8) & 0xFF);
	Buffer[4] = (uint8_t)(telegram->UTCTime & 0xFF);
		
	// Latitude transformed to 32bits.
	Buffer[5] = (uint8_t)((telegram->Latitude >> 24) & 0xFF);
	Buffer[6] = (uint8_t)((telegram->Latitude >> 16) & 0xFF);
	Buffer[7] = (uint8_t)((telegram->Latitude >>  8) & 0xFF);
	Buffer[8] = (uint8_t)(telegram->Latitude & 0xFF);
	
	// Longitude transformed to 32bits.
	Buffer[9]  = (uint8_t)((telegram->Longitude >> 24) & 0xFF);
	Buffer[10] = (uint8_t)((telegram->Longitude >> 16) & 0xFF);
	Buffer[11] = (uint8_t)((telegram->Longitude >>  8) & 0xFF);
	Buffer[12] = (uint8_t)(telegram->Longitude & 0xFF);
	
	// GPS bits 0b11111111
	// bit 0 -> 1= N, 1=S			  	
	// bit 1 -> 1= W, 1=E			  	
	// bit 2-3 -> Fix type, 0, 1 or 2	
	// bit 4-7 -> PCB version	

//	Serial.println("Fix :" + String(telegram->Fix));	  	
	Buffer[13] |= ((telegram->Fix & 0b00000011) << 2);
//	Serial.println("buffer :" + String(Buffer[13]));	  	
//	Serial.println("PCBVersion :" + String(telegram->PCBVersion));	  	
	Buffer[13] |= ((telegram->PCBVersion-10) << 4);
//	Serial.println("buffer :" + String(Buffer[13]));	  	
	
	Buffer[14] = (uint8_t)(telegram->FirmwwareVersion); // 1.90 -> 1
	Buffer[15] = (uint8_t)(telegram->FirmwwareVersion * 100) - (uint8_t)Buffer[14]; // 1.90 -> 90
			
	Buffer[16] = telegram->NumberOfSatellites;
	Buffer[17] = (uint8_t)(telegram->BatteryVoltage*100)-200; // 4.20 -> 420-200 = 220 // voltage will always be betweem 3.00V or 4.20V.
	
	int16_t dummy = (int16_t)(telegram->Altitude*10);
	
	Buffer[18] = (uint8_t)((dummy >> 8) & 0xFF);
	Buffer[19] = (uint8_t)(dummy & 0xFF); 
	
	// DEBUG:
	/*
	 Serial.println("");
	 Serial.println("RX Payload:");
	 
	 Serial.print("Receiver ID :");
	 Serial.print(String(Buffer[0]));
	 Serial.println(":");
	 
	 Serial.print("Transmitter ID :");
	 Serial.print(String(Buffer[1]));
	 Serial.println(":");
	 
	 
	 Serial.print("UTC Time :");
	 Serial.print(String( (uint32_t)((Buffer[2] << 16) + (Buffer[3] << 8) + Buffer[4]) ) );
	 Serial.println(":");
	 
	 Serial.print("GPS Latitude :");
	 Serial.print(String((uint32_t)((Buffer[5] << 24) + (Buffer[6] << 16) + (Buffer[7] << 8) + Buffer[8])));
	 Serial.println(":");
	 
	 Serial.print("GPS Longitude :");
	 Serial.print(String((uint32_t)((Buffer[9] << 24) + (Buffer[10] << 16) + (Buffer[11] << 8) + Buffer[12])));
	 Serial.println(":");
	 
	 Serial.print("GPS Fix :");
	 Serial.print(String((Buffer[13] >> 2) & 0b00000011));
	 Serial.println(":");
		 
	 Serial.print("PCB Version :");
	 Serial.print(String(((Buffer[13] >> 4) & 0b00001111) + 10));
	 Serial.println(":");
	 
	 Serial.print("Firmware Version :");
	 Serial.print(String(Buffer[14]));
	 Serial.print(".");
	 Serial.print(String(Buffer[15]));
	 Serial.println(":");
	
	 
	 Serial.print("Number Of Satellites :");
	 Serial.print(String(Buffer[16]));
	 Serial.println(":");
	 
	 Serial.print("Battery Voltage :");
	 Serial.print(String((((float)Buffer[17])+200)/100));
	 Serial.println(":");
	 
	 Serial.print("Altitude :");
	 Serial.print(String(Buffer[18]));
	 Serial.println(":");
	 */
	 /*
	this->SendPackage(Buffer, MAX_PAYLOAD_LENGTH);		
}
*/

/*
RadioTelegram_t *E28_2G4M20S::GetTelegram(void){
	if(telegramValid){
		this->telegramValid=false;
		return &LastMsg;	  
	}else
		return NULL;
}
*/

void E28_2G4M20S::Sleep(void){
	SleepParams_t SleepParameters;
	
	SleepParameters.WakeUpRTC=0;
	SleepParameters.InstructionRamRetention=1;
	SleepParameters.DataBufferRetention=1;
	SleepParameters.DataRamRetention=1;
	
//	Serial.println("Radio Firmware version: " + String(Radio->GetFirmwareVersion()));
	
	digitalWrite(_txEnablePin, LOW);
	digitalWrite(_rxEnablePin, LOW);
		
	Radio->SetSaveContext();
	Radio->SetSleep(SleepParameters);	
	digitalWrite(_chipSelectPin, HIGH);
}


bool E28_2G4M20S::IsIdle(void){
	return radioIdle;
}

void E28_2G4M20S::test(void){
	RadioStatus_t status = Radio->GetStatus();
	
	Serial.println("Status Chipmode" + String(status.Fields.ChipMode));
	Serial.println("Status CmdStatus" + String(status.Fields.CmdStatus));
	Serial.println("Status CpuBusy" + String(status.Fields.CpuBusy));
	Serial.println("Status DmaBusy" + String(status.Fields.DmaBusy));
	Serial.println("----  -----");
}

void E28_2G4M20S::WakeUp(void){
	Radio->SetWakeup();
	
	//Serial.println("BUSY=" + String(digitalRead(_busyPin)));
//	if(digitalRead(busyPin) == 1){
//		Serial.println("BUSY!IRQ1 status: " + String(Radio->GetIrqStatus()));
		
//	}


	//Serial.println("IRQ1 status: " + String(Radio->GetIrqStatus()));
	//Radio->ClearIrqStatus( IRQ_RADIO_ALL );			
	//Serial.println("IRQ2 status: " + String(Radio->GetIrqStatus()));

}

bool E28_2G4M20S::NewPackageReady(void){
	if(BufferReady){
		return true;
	}else{
		return false;
	}
}

uint8_t * E28_2G4M20S::GetPayload(uint8_t &len){
	len=BufferSize;
	if(BufferSize == 0){
		return NULL;
	}	
		
	return Buffer;
}

void E28_2G4M20S::SetBufferReady(bool _set){
	this->BufferReady = _set;
}

