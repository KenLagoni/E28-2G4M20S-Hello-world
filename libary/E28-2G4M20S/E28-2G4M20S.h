/*
	E28-2G4M20S.h

	Copyright (c) 2019 Lagoni
	Not for commercial use
 */ 
#ifndef __E28_2G4M20S_H__
#define __E28_2G4M20S_H__

#include "Arduino.h" // Needed for Strings and Serial
#include "radio.h"
#include "sx1280-hal.h"

//#define MODE_BLE
#define MODE_LORA
//#define MODE_GENERIC
//#define MODE_FLRC

// Defines the buffer size, i.e. the payload size
#define MAX_PAYLOAD_LENGTH                          25

#define TX_TIMEOUT_VALUE                            50 // ms


// Number of tick size steps for rx timeout
#define RX_TIMEOUT_VALUE                            50 // ms


// Size of ticks (used for Tx and Rx timeout)
#define RX_TIMEOUT_TICK_SIZE                        RADIO_TICK_SIZE_1000_US

/*
struct RadioTelegram_t
{	
	uint8_t receiver_ID;						// ID for the receiver.
	uint8_t transmitter_ID;						// ID for this unit.
	uint32_t UTCTime;  					    	// UTC time Zulu in seconds 130032.000 -> 130032
	int32_t Latitude;							//  5550.0898N -> 55500898 ||  5550.0898S -> -55500898
	int32_t Longitude;							// 01224.0718E -> 12240718 || 01224.0718W -> -12240718
	uint8_t Fix;								// 0 = Invalid, 1=GPS fix, 2=DGPS
	uint8_t NumberOfSatellites;					// Satellites in view.
	float HDOP;				   					// Horizontal Dilution of Precision (HDOP) 
	float Altitude;								// Altitude in meters above sea level.
	
	float BatteryVoltage;						// Battery voltage on transponder battery
	float FirmwwareVersion;
	uint8_t PCBVersion;
	int8_t rssi;
};
*/
class E28_2G4M20S
{
  public:
    E28_2G4M20S(int chipSelectPin, int resetPin, int busyPin, int dio1Pin, int dio2Pin, int dio3Pin, int txEnablePin, int rxEnablePin);
	void Init(); 
	void HandleIRQ();
	void SetRXMode(bool useTimeout); 
	void Debug();
	void Sleep();
	bool IsIdle(void);
	void WakeUp(void);
	void test(void);
	
	// New functions:
	bool NewPackageReady(void);
	void SendPackage(uint8_t *payload, uint8_t payloadLength);
	uint8_t GetPackage(uint8_t *payload, uint8_t maxSize); // returns the payload length
	uint8_t * GetPayload(uint8_t &len);
	void SetBufferReady(bool _set);
		
  private:
    // Pins:
	int _chipSelectPin;
	int _resetPin;
	int _busyPin;
	int _dio1Pin;
	int _dio2Pin;
	int _dio3Pin;
	int _txEnablePin;
	int _rxEnablePin;
	
	// Callback functions
	void OnTxDone( void ); // Function to be executed on Radio Tx Done event
	void OnRxDone( void ); // Function to be executed on Radio Rx Done event
	void OnTxTimeout( void ); //Function executed on Radio Tx Timeout event
	void OnRxTimeout( void ); //Function executed on Radio Rx Timeout event
	void OnRxError( IrqErrorCode_t ); // Function executed on Radio Rx Error event
		    
    void SetTxModeActive( void );
	void SetRxModeActive( void );
    
    uint32_t rf_frequency = 2400000000;
    uint8_t tx_power = 13;
  
    PacketParams_t PacketParams;
    PacketStatus_t PacketStatus;
    ModulationParams_t modulationParams;
  	
    SX1280Hal *Radio = NULL;
	
	////////////////////
	
	// Define the possible message type for this application
	uint8_t BufferSize = MAX_PAYLOAD_LENGTH; //The size of the buffer
	uint8_t Buffer[MAX_PAYLOAD_LENGTH]; //The buffer
	bool BufferReady = false;

	int8_t RssiValue = 0;
	int8_t SnrValue = 0;
	uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT; // Mask of IRQs to listen to in rx mode
	uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT; // Mask of IRQs to listen to in tx mode	
	
	bool radioIdle = true;
};

#endif // __E28_2G4M20S_H__