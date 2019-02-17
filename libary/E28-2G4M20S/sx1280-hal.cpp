/*
Original File:"
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Handling of the node configuration protocol

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian"

Modified by Lagoni 12022019
(c) Lagoni 2019
Not for commercial use

I downloaded the source code from Semtech website: "https://os.mbed.com/teams/Semtech/code/SX1280Lib/" But no LICENSE.TXT was included in the project.
*/
#include "sx1280-hal.h"

/*!
 * \brief Helper macro to avoid duplicating code for setting dio pins parameters
 */
#if defined( TARGET_NUCLEO_L476RG )
#define DioAssignCallback( dio, pinMode, callback )                    \
            if( dio != NULL )                                          \
            {                                                          \
                dio->mode( pinMode );                                  \
                dio->rise( this, static_cast <Trigger>( callback ) );  \
            }
#else
#define DioAssignCallback( dio, pinMode, callback )                    \
            if( dio != NULL )                                          \
            {                                                          \
                dio->rise( this, static_cast <Trigger>( callback ) );  \
            }
#endif
/*!
 * \brief Used to block execution waiting for low state on radio busy pin.
 *        Essentially used in SPI communications
 */
#define WaitOnBusy( )          while( digitalRead(BUSY) == 1 ){ }


// This code handles cases where assert_param is undefined
#ifndef assert_param
#define assert_param( ... )
#endif

SX1280Hal::SX1280Hal( int nss, int busy, int dio1, int dio2, int dio3, int rst)
        :   SX1280( )
{
	RadioNss = nss;
    RadioReset = rst;
    BUSY = busy;
    
	pinMode(RadioNss, OUTPUT);
    digitalWrite(RadioNss, HIGH);
	pinMode(RadioReset, OUTPUT);
    digitalWrite(RadioReset, HIGH);

    SPI.begin();
}

SX1280Hal::~SX1280Hal( void )
{
 
};
/*
void SX1280Hal::IoIrqInit( DioIrqHandler irqHandler )
{
    assert_param( RadioSpi != NULL || RadioUart != NULL );
    if( RadioSpi != NULL )
    {
        SpiInit( );
    }
    if( RadioUart != NULL )
    {
        UartInit( );
    }

    BUSY.mode( PullNone );

    DioAssignCallback( DIO1, PullNone, irqHandler );
    DioAssignCallback( DIO2, PullNone, irqHandler );
    DioAssignCallback( DIO3, PullNone, irqHandler );
}
*/
void SX1280Hal::Reset( void )
{
//    noInterrupts();
    delay( 20 );
    pinMode(RadioReset, OUTPUT);
    digitalWrite(RadioReset, LOW);
    delay( 50 );
    digitalWrite(RadioReset, HIGH);
    pinMode(RadioReset, INPUT); // Using the internal pull-up
    delay( 20 );
//    interrupts();
}

void SX1280Hal::Wakeup( void )
{
//    noInterrupts();

    //Don't wait for BUSY	here
	
	digitalWrite(RadioNss, LOW);
	delayMicroseconds(150);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
//	digitalWrite(21, LOW);	 debug
    SPI.transfer( RADIO_GET_STATUS );
    SPI.transfer( 0 );
    digitalWrite(RadioNss, HIGH);
	SPI.endTransaction();
		
    // Wait for chip to be ready.
    WaitOnBusy( );
//    interrupts();
}

void SX1280Hal::WriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    WaitOnBusy( );

   
    digitalWrite(RadioNss, LOW);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    SPI.transfer( ( uint8_t )command );
    for( uint16_t i = 0; i < size; i++ )
    {
        SPI.transfer( buffer[i] );
    }
    digitalWrite(RadioNss, HIGH);
	SPI.endTransaction();
   
    if( command != RADIO_SET_SLEEP )
    {
        WaitOnBusy( );
    }
}

void SX1280Hal::ReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    WaitOnBusy( );

    digitalWrite(RadioNss, LOW);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	
	 if( command == RADIO_GET_STATUS )
     {
		buffer[0] = SPI.transfer( ( uint8_t )command );
		SPI.transfer( 0 );
		SPI.transfer( 0 );
	
	 }else
	 {
		 SPI.transfer( ( uint8_t )command );
		 SPI.transfer( 0 );
		 for( uint16_t i = 0; i < size; i++ )
         {
             buffer[i] = SPI.transfer( 0 );
         }
	 }
	digitalWrite(RadioNss, HIGH);
	SPI.endTransaction();
	
    WaitOnBusy( );
}

void SX1280Hal::WriteRegister( uint16_t address, uint8_t *buffer, uint16_t size )
{
    WaitOnBusy( );

	digitalWrite(RadioNss, LOW);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	
	SPI.transfer( RADIO_WRITE_REGISTER );
	SPI.transfer( ( address & 0xFF00 ) >> 8  );
	SPI.transfer( address & 0x00FF  );
		 
	for( uint16_t i = 0; i < size; i++ )
    {
            SPI.transfer( buffer[i] );
    }
		
	digitalWrite(RadioNss, HIGH);
	SPI.endTransaction();
   
    WaitOnBusy( );
}

void SX1280Hal::WriteRegister( uint16_t address, uint8_t value )
{
    WriteRegister( address, &value, 1 );
}

void SX1280Hal::ReadRegister( uint16_t address, uint8_t *buffer, uint16_t size )
{
    WaitOnBusy( );

    digitalWrite(RadioNss, LOW);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	
	SPI.transfer( RADIO_READ_REGISTER );
	SPI.transfer( ( address & 0xFF00 ) >> 8  );
	SPI.transfer( address & 0x00FF  );
	SPI.transfer( 0 );	 
	for( uint16_t i = 0; i < size; i++ )
    {
            buffer[i] = SPI.transfer( 0 );
    }
		
	digitalWrite(RadioNss, HIGH);
	SPI.endTransaction();

    WaitOnBusy( );
}

uint8_t SX1280Hal::ReadRegister( uint16_t address )
{
    uint8_t data;

    ReadRegister( address, &data, 1 );
    return data;
}

void SX1280Hal::WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    WaitOnBusy( );

	digitalWrite(RadioNss, LOW);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	
	SPI.transfer( RADIO_WRITE_BUFFER );
	SPI.transfer( offset  );
	for( uint16_t i = 0; i < size; i++ )
    {
            SPI.transfer( buffer[i] );
    }
		
	digitalWrite(RadioNss, HIGH);
	SPI.endTransaction();
	   
    WaitOnBusy( );
}

void SX1280Hal::ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    WaitOnBusy( );

    digitalWrite(RadioNss, LOW);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    SPI.transfer( RADIO_READ_BUFFER );
    SPI.transfer( offset );
    SPI.transfer( 0 );
    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = SPI.transfer( 0 );
    }
    digitalWrite(RadioNss, HIGH);
	SPI.endTransaction();
 
    WaitOnBusy( );
}
/*
uint8_t SX1280Hal::GetDioStatus( void )
{
    return ( *DIO3 << 3 ) | ( *DIO2 << 2 ) | ( *DIO1 << 1 ) | ( BUSY << 0 );
}*/
