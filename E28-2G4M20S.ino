
#include <Arduino.h>
#include <SPI.h>
#include "E28-2G4M20S.h" // Ebyte E28-2G4M20S module which use Semtech SX1280 radio chip.

bool IS_MASTER=true; // set false for slave(rx module) module.

// Object for Ebyte E28-2G420S module using the SX1280 radio chip
E28_2G4M20S *Radio = NULL;
uint8_t Payload[MAX_PAYLOAD_LENGTH]; // Message Payload (set in E28-2G4M20S.h.
uint8_t PayloadLength; // Length of the payload.
  
// Define Pins for Radio
const int chipSelectPin = 1;  // Pin D1  on MKRZero Board - Chip pin is 32 or PA23
const int misoPin       = 10; // Pin D10 on MKRZero Board - Chip pin is 28 or PA19
const int mosiPin       = 8;  // Pin D8  on MKRZero Board - Chip pin is 25 or PA16
const int sckPin        = 9;  // Pin D9  on MKRZero Board - Chip pin is 26 or PA17
const int rxEnPin       = 17; // Pin A2  on MKRZero Board - Chip pin is 48 or PB03
const int txEnPin       = 18; // Pin A3  on MKRZero Board - Chip pin is  9 or PA04
const int resetPin      = 4;  // Pin D4  on MKRZero Board - Chip pin is 19 or PB10
const int busyPin       = 5;  // Pin D5  on MKRZero Board - Chip pin is 20 or PB11
const int dio1Pin       = 6;  // Pin D6  on MKRZero Board - Chip pin is 29 or PA20

//ISR (Interrupt Service Routine) for radio module. This routine is called every time DIO1 is set by the SX1280 chip.
// Interrupt could be from:
// RX complete (new incomming messages)
// TX complete (finished sending message)
// RX/TX timoeout.
void Radio_isr(void){
  // The radio module has something for us.
  Radio->HandleIRQ(); 
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting Hello World program");

  // Init E28 Radio module (SX1280 chip): When in sleep mode (all data rtained.
  // Connect the E28-2G4M20S module to Arduino MKRZERO as shown:
  // SX1280 -> Arduino MKRZERO
  // MISO   -> D10 (SPI MISO Pin PA19_S1_MISO)
  // MOSI   -> D8  (SPI MOSI Pin PA16_S1_MOSI)
  // SCK    -> D9  (SPI SCK  Pin PA17_S1_SCK)
  // NSS    -> D1  (chipselect, any GPIO pin can be used)
  // RESET  -> D4  (Any GPIO can be used)
  // BUSY   -> D5  (Any GPIO can be used) 
  // DIO1   -> D6  (Pin with interrupt!) Arduino UNO must use pin 2 if I remeber correctly.
  //
  // Pins for the E28-2G4M20S output switches:
  // RX_EN  -> A2  Receive enable - Any GPIO can be used. 
  // TX_EN  -> A3  Transmit-enable - Any GPIO can be used. 
  
  // Make a new instants of the Radio module:
  Radio = new E28_2G4M20S(chipSelectPin,resetPin,busyPin,dio1Pin,0,0,txEnPin,rxEnPin);
  Radio->Init(); // Init the radio module defualt is: LoRa mode, SF7, BW 400KHz, CR4/5 - freqency 2.400.000Hz
  attachInterrupt(dio1Pin, Radio_isr, RISING); 

  // TX message
  Payload[0]='H';
  Payload[1]='e';
  Payload[2]='l';
  Payload[3]='l';
  Payload[4]='0';
  Payload[5]=' ';
  Payload[6]='W';
  Payload[7]='o';
  Payload[8]='r';
  Payload[9]='l';
  Payload[10]='d';
  PayloadLength=11;
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(IS_MASTER){
    Radio->SendPackage(Payload, PayloadLength);
  }else{
    if( Radio->NewPackageReady()){
      // Enter here when radio module has a message for us (this is set via ISR).    
      
      uint8_t size;
      uint8_t *data = Radio->GetPayload(size);
      Serial.print("New message received!:");
      for(int n=0;n<size;n++){
        Serial.print(String(*data));
        data++;
      }
      Serial.println(":");
    }
  }
  delay(1000);
}
