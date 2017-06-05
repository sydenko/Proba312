#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <io.h>

#define true 1
#define false 0

#define MAXDEVICES 10

// Если для эмуляции шины используется USART
//#define UART_AS_OneWire

// Если для эмуляции 1-wire не спольльзуется USART, но используется 2 пина (вход и выход)
//#define OW_TWO_PINS

#ifdef UART_AS_OneWire
	#define USART_BAUDRATE_57600 (((F_CPU / (57600 * 16UL))) - 1)
	#define USART_BAUDRATE_115200 (((F_CPU / (115200 * 16UL))) - 1)
	#define USART_BAUDRATE_9600 (((F_CPU / (9600 * 16UL))) - 1)
#else
	#include <delay.h>
	#define OW_DDR DDRB
	#define OW_PORT PORTB
	#define OW_PIN PINB
	#ifndef OW_TWO_PINS //если используется один пин, укажите его номер
		#define OW_BIT 0
	#else // если используются 2 пина, укажите их номера
		#define OW_BIT_OUT 1
		#define OW_BIT_IN 0
	#endif
#endif

#define OW_CMD_SEARCHROM	0xF0
#define OW_CMD_READROM		0x33
#define OW_CMD_MATCHROM		0x55
#define OW_CMD_SKIPROM		0xCC

#define	OW_SEARCH_FIRST	0xFF		// start new search
#define	OW_PRESENCE_ERR	0xFF
#define	OW_DATA_ERR	    0xFE
#define OW_LAST_DEVICE	0x00		// last device found
//			0x01 ... 0x40: continue searching

#define OW_DS1990_FAMILY_CODE	1
#define OW_DS2405_FAMILY_CODE	5
#define OW_DS2413_FAMILY_CODE	0x3A
#define OW_DS1822_FAMILY_CODE	0x22
#define OW_DS2430_FAMILY_CODE	0x14

#define OW_DS2431_FAMILY_CODE	0x2d
#define OW_DS18S20_FAMILY_CODE	0x10
#define OW_DS18B20_FAMILY_CODE	0x28
#define OW_DS2433_FAMILY_CODE	0x23

// rom-code size including CRC
#define OW_ROMCODE_SIZE	8

unsigned char OW_Reset(void);
void OW_WriteBit(unsigned char bit_);
unsigned char OW_ReadBit(void);
#ifndef UART_AS_OneWire
	unsigned char OW_ReadByte(void);
	void OW_WriteByte(unsigned char byte);
#else
	unsigned char OW_WriteByte(unsigned char byte);
	#define OW_ReadByte() OW_WriteByte(0xFF)
#endif
unsigned char OW_SearchROM( unsigned char diff, unsigned char *id );
void OW_FindROM(unsigned char *diff, unsigned char id[]);
unsigned char OW_ReadROM(unsigned char *buffer);
unsigned char OW_MatchROM(unsigned char *rom);

#endif

///////////////////////////////////////////////////////////////////////////
// #include "onewire.h"

#ifdef UART_AS_OneWire
    #include <interrupt.h>
#endif

#define sbi(reg,bit) reg |= (1<<bit)
#define cbi(reg,bit) reg &= ~(1<<bit)
#define ibi(reg,bit) reg ^= (1<<bit)
#define CheckBit(reg,bit) (reg&(1<<bit))

extern void RunTasks(unsigned char tasks);

void OthersTasks(void){
//    RunTasks(0xFF);
}

#ifndef UART_AS_OneWire
void OW_Set(unsigned char mode)
{
#ifndef OW_TWO_PINS
    if (mode) {
        cbi(OW_PORT, OW_BIT); 
        sbi(OW_DDR, OW_BIT);
    }
    else {
        cbi(OW_PORT, OW_BIT); 
        cbi(OW_DDR, OW_BIT);
    }
#else
    if (mode) cbi(OW_PORT, OW_BIT_OUT);
    else sbi(OW_PORT, OW_BIT_OUT);
#endif
}

unsigned char OW_CheckIn(void)
{
#ifndef OW_TWO_PINS
    return CheckBit(OW_PIN, OW_BIT);
#else
    return CheckBit(OW_PIN, OW_BIT_IN);
#endif
}

#endif

unsigned char OW_Reset(void)
{
#ifdef UART_AS_OneWire
    UCSRB=(1<<RXEN)|(1<<TXEN);
    //9600
    UBRRL = USART_BAUDRATE_9600;
    UBRRH = (USART_BAUDRATE_9600 >> 8);
    UCSRA &= ~(1<<U2X);
    
    while(CheckBit(UCSRA, RXC)) UDR; //Зачистка буферов
    cli();
    UDR = 0xF0;
    UCSRA = (1<<TXC);
    sei();
    //while(!CheckBit(UCSRA, TXC)) OthersTasks();
    while(!CheckBit(UCSRA, RXC)) OthersTasks();
    if (UDR != 0xF0) return 1;
 return 0;
#else
    unsigned char    status;
    OW_Set(1);
    delay_us(480);
    OW_Set(0);
    delay_us(60);
    //Store line value and wait until the completion of 480uS period
    status = OW_CheckIn();
    delay_us(420);
    //Return the value read from the presence pulse (0=OK, 1=WRONG)
 return !status;
#endif
//    return 1 if found
//    return 0 if not found
}

void OW_WriteBit(unsigned char bit_)
{
#ifdef UART_AS_OneWire    
    //115200
    UBRRL = USART_BAUDRATE_115200;
    UBRRH = (USART_BAUDRATE_115200 >> 8);
    UCSRA |= (1<<U2X);    
    
    unsigned char    d = 0x00;    
    while(CheckBit(UCSRA, RXC)) UDR; //Зачистка буферов
    if (bit) d = 0xFF;
    cli();
    UDR = d;
    UCSRA=(1<<TXC);
    sei();
    while(!CheckBit(UCSRA,TXC));
    while(CheckBit(UCSRA, RXC)) UDR; //Зачистка буферов
#else    
    //Pull line low for 1uS
    OW_Set(1);
    delay_us(1);
    //If we want to write 1, release the line (if not will keep low)
    if(bit_) OW_Set(0); 
    //Wait for 60uS and release the line
    delay_us(60);
    OW_Set(0);
#endif    
}

unsigned char OW_ReadBit(void)
{
#ifdef UART_AS_OneWire    
    //115200
    UBRRL = USART_BAUDRATE_115200;
    UBRRH = (USART_BAUDRATE_115200 >> 8);
    UCSRA |= (1<<U2X);
    
    unsigned char    c;
    while(CheckBit(UCSRA, RXC)) UDR; //Зачистка буферов
    cli();        
    UDR = 0xFF;
    UCSRA=(1<<TXC);
    sei();
    while(!CheckBit(UCSRA, TXC));
    while(!CheckBit(UCSRA, RXC));
    c = UDR;
    if (c>0xFE) return 1;
    return 0;
#else    
    unsigned char    bit_=0;
    //Pull line low for 1uS
    OW_Set(1);
    delay_us(1);
    //Release line and wait for 14uS
    OW_Set(0);
    delay_us(14);
    //Read line value
    if(OW_CheckIn()) bit_=1;
    //Wait for 45uS to end and return read value
    delay_us(45);
    return bit_;
#endif    
}

#ifdef UART_AS_OneWire
unsigned char OW_WriteByte(unsigned char byte)
{
    unsigned char    i = 8;
    //115200
    UBRRL = USART_BAUDRATE_115200;
    UBRRH = (USART_BAUDRATE_115200 >> 8);
    UCSRA |= (1<<U2X);
  
    do
    {
        unsigned char    d = 0x00;
        if (byte&1) d = 0xFF;
        cli();
        UDR = d;
        UCSRA=(1<<TXC);
        sei();
        OthersTasks();
        while(!CheckBit(UCSRA,RXC)) OthersTasks();
        byte>>=1;
		if (UDR>0xFE) byte|=128;
	}
	while(--i);
	
	return byte&255;
}
#else
void OW_WriteByte(unsigned char byte_)
{
	for (unsigned char i=0; i<8; i++) OW_WriteBit(CheckBit(byte_, i));
}

unsigned char OW_ReadByte(void)
{
	unsigned char n=0;

	for (unsigned char i=0; i<8; i++) if (OW_ReadBit()) sbi(n, i);
	
	return n;
}
#endif

unsigned char OW_SearchROM( unsigned char diff, unsigned char *id )
{ 	
	unsigned char i, j, next_diff;
	unsigned char b;

	if(!OW_Reset()) 
		return OW_PRESENCE_ERR;       // error, no device found

	OW_WriteByte(OW_CMD_SEARCHROM);     // ROM search command
	next_diff = OW_LAST_DEVICE;      // unchanged on last device
	
	i = OW_ROMCODE_SIZE * 8;         // 8 bytes
	do 
	{	
		j = 8;                        // 8 bits
		do 
		{ 
			b = OW_ReadBit();			// read bit
			if( OW_ReadBit() ) 
			{ // read complement bit
				if( b )                 // 11
				return OW_DATA_ERR;  // data error
			}
			else 
			{ 
				if( !b ) { // 00 = 2 devices
				if( diff > i || ((*id & 1) && diff != i) ) { 
						b = 1;               // now 1
						next_diff = i;       // next pass 0
					}
				}
			}
         OW_WriteBit( b );               // write bit
         *id >>= 1;
         if( b ) *id |= 0x80;			// store bit
         i--;
		} 
		while( --j );
		id++;                            // next byte
    } 
	while( i );
	return next_diff;                  // to continue search
}

void OW_FindROM(unsigned char *diff, unsigned char id[])
{
	while(1)
    {
		*diff = OW_SearchROM( *diff, &id[0] );
    	if ( *diff==OW_PRESENCE_ERR || *diff==OW_DATA_ERR ||
    		*diff == OW_LAST_DEVICE ) return;
    	//if ( id[0] == DS18B20_ID || id[0] == DS18S20_ID ) 
		return;
    }
}

unsigned char OW_ReadROM(unsigned char *buffer)
{
	if (!OW_Reset()) return 0;
	OW_WriteByte(OW_CMD_READROM);
	for (unsigned char i=0; i<8; i++)
	{
		buffer[i] = OW_ReadByte();
	}
 return 1;
}

unsigned char OW_MatchROM(unsigned char *rom)
{
 	if (!OW_Reset()) return 0;
	OW_WriteByte(OW_CMD_MATCHROM);	
	for(unsigned char i=0; i<8; i++)
	{
		OW_WriteByte(rom[i]);
	}
 return 1;
}
