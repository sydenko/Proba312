/*******************************************************
CodeWizardAVR V3.12 Advanced http://www.hpinfotech.com
Project :Version :Date    : 23.03.2017 Author  :Company :Comments:
Chip type               : ATmega328P
AVR Core Clock frequency: 16,000000 MHz 
*******************************************************/
#include <mega328p.h>
#include <delay.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
// Библиотека stdio.h, содержит функцию sprint() // void sprintf(char *str, char flash *fmtstr,...);
// Первым параметром мы передаем переменную, в которую будет записан результат выполнения функции.
// Второй параметр – форматированная строка. //Остальные – переменные.
// Вот пример использования:
// char day=22, month=12,  year=2008;
// sprint(text,“Data: %d.%d.%d”,day,month,year);
#include <stdio.h>
// 1 Wire Bus interface functions
#include <1wire.h>
// DS1820 Temperature Sensor functions //#include <ds1820.h>
#include "my18b20.h"
// Alphanumeric LCD functions
#include <alcd.h>   // Подключаем библиотеку дисплея 
#include <stdlib.h>
#define pause delay_ms(1) // Объявляем переменную pause, которая будет осуществлять задержку
int bFdrawTermoU = 0; // Флаг резрешения рисовать температуры
#define ROZ_1	281
#define ROZ_2	562
#define ROZ_3	843
#define ROZ_4	1124
#define ROZ_5	1405
#define ROZ_6	1686
#define ROZ_7	1967
#define ROZ_8	2248
#define ROZ_9	2529
#define ROZ_10	2810
#define ROZ_11	3091
#define ROZ_12	3372
#define ROZ_13	3653
#define ROZ_14	3934
#define ROZ_15	4215
#define ROZ_16	4496
#define ROZ_17	4777
#define ROZ_18	5058  
// Константа, определяющая максимальное число датчиков, подключенных к МК #define MAXDEVICES 10
#define MAX_DS1820 3
unsigned char ds1820_devices;	// Переменная, в которую записывается число найденных датчиков.
// Переменная, в которой хранятся идентификационные коды найденных датчиков.
// для каждого устройства используются 9 байт (см. Описание функции w1_search в справке)
unsigned char ds1820_rom_codes[MAX_DS1820][9];

char i; // Переменная для счетчика для анимации Используется в цикле конфигурирования датчиков
///////////////////////////////////////////////////////////////////////////////////////
typedef unsigned char byte;    // Объявляем новый тип переменной 
// Определяем непосредственно сам символ (имеет 5 точек в ширину и 7 в высоту) // 223 - градус
flash byte char0[8]={ 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000};
flash byte char1[8]={ 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000};
flash byte char2[8]={ 0b00011100, 0b00011100, 0b00011100, 0b00011100, 0b00011100, 0b00011100, 0b00011100, 0b00011100};
flash byte char3[8]={ 0b00011110, 0b00011110, 0b00011110, 0b00011110, 0b00011110, 0b00011110, 0b00011110, 0b00011110}; 
flash byte charM[8]={ 0b10000111, 0b11000111, 0b10000010, 0b10010111, 0b10010101, 0b10011111, 0b10010101, 0b10010111}; // мотор
flash byte charR[8]={ 0b10000011, 0b10000100, 0b10011111, 0b10010001, 0b10010001, 0b10011111, 0b10000100, 0b11011000}; // редуктор
flash byte charS[8]={ 0b10000100, 0b10010001, 0b10001110, 0b10011011, 0b10001110, 0b10010001, 0b10000100, 0b11000000}; // солнышко
flash byte charU[8]={ 0b10001010, 0b10011111, 0b10011011, 0b10010001, 0b10011011, 0b10011111, 0b10010001, 0b11011111}; // Батарейка
///////////////////////////////////////////////////////////////////////////////////////
int v=0; // Переменная с цикла для отладки прогрес бара
void drawProgresBar(int valueProgres); // Объявление фунции для рисования прогресс-бара
void drawTermoU(void); // Объявление фунции для рисования прогресс-бара
void fnTest(int v_max); // функция для тестирования и отладки параметр - количество итераций
///////////////////////////////////////////////////////////////////////////////////////
volatile int i_count_TIM1_OVF =0;
char textTIM1_OVF[17];
int cFds18b20 = 50;  // кол-во пропусков замеров температуры
volatile unsigned int tachFltr = 0;	// результат после сокращения
unsigned int tachFltr_Old = 0; // для определения разрешения перерисовки температур
///////////////////////////////////////////////////////////////////////////////////////
// Объявляем глобальные переменные
 volatile unsigned char xL=0; //,yL=0,zL=0; // Для захвата таймера
 volatile unsigned char xH=0; //,yH=0,zH=0; // Для захвата таймера
//volatile unsigned int x=0, uiKilkistZamiriv=0, uiTestStop=0; // Для захвата таймера y=0,z=0,
//unsigned long int uiSumaZamiriv=0;
// char str_tLH[7]; // использовался внутри while
//unsigned char count = 0;			// Считаем количество замеров
//volatile unsigned long int tachBuf = 0;			// буфер для суммы замеров
//#define KILKIST_ZAMIRIV 8
//volatile unsigned char tachBufL = 0; // буфер , tachBufH = 0

//volatile int i_count_TIM1_OVF =0;
//char textTIM1_OVF[17];    int cFds18b20 = 5;  // кол-во пропусков замеров температуры
// volatile unsigned int tachFltr = 0;	// результат после сокращения
///////////////////////////////////////////////////////////////////////////
int calc_drawTermoU = 0;
char cFVectorA=0;		// Флаг для контроля направления ускорения
char cFVectorA_old=0;	// Флаг для контроля направления ускорения и перерисовки температур в момент изменения направления вектора ускорения
///////////////////////////////////////////////////////////////////////////

// Voltage Reference: AVCC pin
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))
// Voltage Reference: AREF pin
//#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))
unsigned int read_adc(unsigned char adc_input) // Возвращаем результат преобразования
{ // попробовать вставить запрет на выполнение вдруг конфликты исчезнут...
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
////////////////////////delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

////////////////
//const char backslash[8] = {0x00 , 0x00, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00};
//	unsigned int ii=0;
//unsigned int A0;
// unsigned int jj=0;
char strU[5];

//////////////////////////////////// 
//char text[16];
// Строка с примера DS18B20
char lcd_buffer[33];
/* maximum number of DS18B20 connected to the 1 Wire bus */
#define MAX_DEVICES 3 
/* DS18B20 devices ROM code storage area */
unsigned char rom_code[MAX_DEVICES][9];

unsigned char devices;

interrupt [TIM1_CAPT] void timer1_capt_isr(void) // Таймер1 вход захвата обработки прерывания
{   xL = ICR1L; // ВНИМАНИЕ ВАЖНА ПОСЛЕДОВАТЕЛЬНОСТЬ | если поменять местами то один из них вообще 0
    xH = ICR1H; // ВНИМАНИЕ ВАЖНА ПОСЛЕДОВАТЕЛЬНОСТЬ | если поменять местами то один из них вообще 0
    TCNT1H=0;   // ВНИМАНИЕ ВАЖНА ПОСЛЕДОВАТЕЛЬНОСТЬ | Обнуляем счётный регистр TCNT1 = 0;
    TCNT1L=0;   // ВНИМАНИЕ ВАЖНА ПОСЛЕДОВАТЕЛЬНОСТЬ | Обнуляем счётный регистр 
    tachFltr = (xH << 8) | xL;   //#asm("cli")    //#asm("sei")
    i_count_TIM1_OVF=0; //    my_while();
}

interrupt [TIM1_OVF] void timer1_ovf_isr(void) // Таймер1 обработка прерывания по переполнению
{   // Timer Period: 0,26214 s. Поэтому 20 циклов = 5,2428 секунд
    if(i_count_TIM1_OVF < 20 ){ // Продолжаем считать пока <20, потом  Выключаем газовое реле 
        i_count_TIM1_OVF++; // считаем количество срабатываний по переполнению
    }
    else
    {   
        PORTC.3=0;   // Выключаем газовое реле 
        i_count_TIM1_OVF=1; // !!! ВСПОМНИТЬ ПОЧЕМУ устанавливаем в 1 после выключения реле
        tachFltr = 17000;
    }

    if(i_count_TIM1_OVF == 2) { // Не будем долго дожидаться перед показом температур пока стоим   
      bFdrawTermoU = 1;         // Возможно потом вообще уберу это условие
    }
}

// Описываем функцию записи символа в flash память экрана
void define_char(byte flash *pc,byte char_code)
{   byte i,a;   a=(char_code<<3)|0x40;  for (i=0; i<8; i++) lcd_write_byte(a++,*pc++);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main(void)
{   // Declare your local variables here  //unsigned char i,j,devices;
// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Инициализация портов
// Порт B
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
DDRB=(0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) | (0<<DDB1) | (0<<DDB0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T 
PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

// Порт C
// Function: Bit6=In Bit5=In Bit4=In Bit3=Out Bit2=In Bit1=In Bit0=In 
DDRC=(0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (1<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
// State: Bit6=T Bit5=T Bit4=T Bit3=1 Bit2=T Bit1=T Bit0=T 
PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (1<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);

// Порт D
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
DDRD=(0<<DDD7) | (0<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T 
PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);

//// Timer/Counter 0 initialization
//// Clock source: System Clock
//// Clock value: 15,625 kHz
//// Mode: Normal top=0xFF
//// OC0A output: Disconnected
//// OC0B output: Disconnected
//// Timer Period: 16,384 ms
//TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
//TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
//TCNT0=0x00;
//OCR0A=0x00;
//OCR0B=0x00;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
// Mode: Normal top=0xFF
// OC0A output: Disconnected
// OC0B output: Disconnected
TCCR0A=0x00;
TCCR0B=0x00;
TCNT0=0x00;
OCR0A=0x00;
OCR0B=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 250,000 kHz
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Rising Edge
// Timer Period: 0,26214 s
// Timer1 Overflow Interrupt: On
// Input Capture Interrupt: On
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (1<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2A output: Disconnected
// OC2B output: Disconnected
ASSR=(0<<EXCLK) | (0<<AS2);
TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
TCCR2B=(0<<WGM22) | (0<<CS22) | (0<<CS21) | (0<<CS20);
TCNT2=0x00;
OCR2A=0x00;
OCR2B=0x00;

// Timer/Counter 0 Interrupt(s) initialization
//TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (0<<TOIE0);
// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (0<<TOIE0);

// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=(1<<ICIE1) | (0<<OCIE1B) | (0<<OCIE1A) | (1<<TOIE1);

// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (0<<TOIE2);

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// Interrupt on any change on pins PCINT0-7: Off
// Interrupt on any change on pins PCINT8-14: Off
// Interrupt on any change on pins PCINT16-23: Off
EICRA=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
EIMSK=(0<<INT1) | (0<<INT0);
PCICR=(0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);

// USART initialization
// USART disabled
UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (0<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);

// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin
ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
// Digital input buffer on AIN0: On
// Digital input buffer on AIN1: On
DIDR1=(0<<AIN0D) | (0<<AIN1D);

// ADC initialization
// ADC Clock frequency: 1000,000 kHz
// ADC Voltage Reference: AREF pin
// ADC Auto Trigger Source: Free Running
// Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
// ADC4: On, ADC5: On
DIDR0=(0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D);
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (1<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

// SPI initialization
// SPI disabled
SPCR=(0<<SPIE) | (0<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);

// TWI initialization
// TWI disabled
TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

// 1 Wire Bus initialization
// 1 Wire Data port: PORTB
// 1 Wire Data bit: 1
// Note: 1 Wire port settings are specified in the
// Project|Configure|C Compiler|Libraries|1 Wire menu.
w1_init();

// Determine the number of DS1820 devices
// connected to the 1 Wire bus
// Запуск функции поиска 1wire устройств и запись их количества в переменную ds1820_devices.
ds1820_devices=w1_search(0xf0,ds1820_rom_codes);

// Alphanumeric LCD initialization  // Connections are specified in the
// Project|Configure|C Compiler|Libraries|Alphanumeric LCD menu:
// RS - PORTD Bit 2 // RD - PORTD Bit 1 // EN - PORTD Bit 3 // D4 - PORTD Bit 4 
// D5 - PORTD Bit 5 // D6 - PORTD Bit 6 // D7 - PORTD Bit 7 // Characters/line: 16
lcd_init(16);
//lcd_putsf(__DATE__);
lcd_putsf("   Hello Ivan 3"); 
lcd_putsf("\n    ");
lcd_putsf(__TIME__);

 // Запись символов char0 - char4 по адресам 0х00 - 0х04 соответственноdefine_char(char0,0);
 define_char(char0,0);
 define_char(char1,1);
 define_char(char2,2);
 define_char(char3,3);
 define_char(charM,4); // Заносим в знакогенератор значёк Мотора
 define_char(charR,5); // Заносим в знакогенератор значёк редуктора 
 define_char(charS,6); // Заносим в знакогенератор значёк Солнышко
 define_char(charU,7); // Заносим в знакогенератор значёк аккумалятора-U
delay_ms(2800);
lcd_clear();

/* Определить, сколько устройств DS18B20 подключено к шине 1 Wire */
devices=w1_search(0xf0,rom_code);
// тут был тест номеров датчиков. Теперь он в fnTest()
for (i=0;i<devices;) // Конфигурируем все датчики DS18B20_12BIT_RES 3 // 12 bit thermometer resolution
    if (!ds18b20_init_MY(&rom_code[i++][0],20,30,DS18B20_9BIT_RES))
    {
       sprintf(lcd_buffer,"Gluk datchika #%u\nperezagruzi #%u",i);
       lcd_clear();
       lcd_puts(lcd_buffer);
       while (1); /* stop here if init error */
    };
//////////////////////////////////////////////////////////////////////////////////////////////////////
// fnTest(5000); // 1024 TEST TEST TEST
//lcd_clear(); 
//drawTermoU(); // Делаем ознакомительный замер температуры
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Разрешаем глобальные прерывания
#asm("sei")

//delay_ms(15000);
   
   
   
while (1){
	//////////////////////////////////// 
	// В постоянном цикле Выводим задержку тахометра безоговорочно
	// 
	// lcd_puts(str_tLH);	//	lcd_puts("_");
	// 
	//  	if(calc_drawTermoU++ >15)
  	if(bFdrawTermoU) // Возможно современем уберу это условие
    {
        drawTermoU();		// calc_drawTermoU = 0;
        bFdrawTermoU = 0;	// ЗАПРЕТИТЬ повторное перечитывание температур и U бортовой сети (после выполнения)
    }  ////////////////////////////////////fgfg fg fyh dy erh ye
	

    if(i_count_TIM1_OVF == 0 ){
//		drawProgresBar(5000 - tachFltr * 0.3); // пореводим в обороты за минуту (ltoa(5000 - tachFltr * 0.3, str_tLH);)
		drawProgresBar(5000 - tachFltr * 0.6); // пореводим в обороты за минуту (ltoa(5000 - tachFltr * 0.3, str_tLH);)

		if(tachFltr_Old != tachFltr){

			if(tachFltr_Old + 3 < tachFltr) // Когда обороты уменьшаются на 5 от последнего сохранения
			{
				lcd_gotoxy(10,0);
				lcd_puts("+ ");//  lcd_putchar('+');          i_count_TIM1_OVF =0 ;
				cFVectorA = 1;
			// Флаг для контроля направления ускорения
			bFdrawTermoU = 1; // Разрешить перечитывание температур и U бортовой сети
			}
			else
			{
				lcd_gotoxy(10,0);
				lcd_puts(" -");//            i_count_TIM1_OVF =0 ;
				cFVectorA = -1;
			} 

		}
//		else
//		{
//			lcd_gotoxy(10,0);
//			lcd_puts("__");//            i_count_TIM1_OVF =0 ;
//		}
		tachFltr_Old = tachFltr ;//+ 50
		
		if(cFVectorA_old <= cFVectorA)  // + 3
		{
				lcd_gotoxy(10,0);
				lcd_puts("><");//  lcd_putchar('+');          i_count_TIM1_OVF =0 ;
		}
			cFVectorA_old = cFVectorA;	// Флаг для контроля направления ускорения и перерисовки температур в момент изменения направления вектора ускорения

		


	}
	else
	{
		itoa(i_count_TIM1_OVF, textTIM1_OVF);
        lcd_gotoxy(4,0);
        lcd_puts(textTIM1_OVF);
        lcd_gotoxy(0,1);
        lcd_puts(" <=Stop=> ");//            i_count_TIM1_OVF =0 ;      
    } 
    
    
 }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
float fTermM_old = 0; // для функции drawTermoU защита от сбоев термодатчиков
float fTermR_old = 0; // для функции drawTermoU защита от сбоев термодатчиков
float fTermS_old = 0; // для функции drawTermoU защита от сбоев термодатчиков
void drawTermoU(void) { // Выводим температур и U бортовой сети // #asm("cli") // lcd_clear();
    float fRead_adc = 0;
    float fTermM = 0; // для функции drawTermoU защита от сбоев термодатчиков
    float fTermR = 0; // для функции drawTermoU защита от сбоев термодатчиков
    float fTermS = 0; // для функции drawTermoU защита от сбоев термодатчиков
    //////////////////////////////////////////////////////////////////
//    if(cFds18b20 <= 0)
//    {   //cFds18b20--;
        lcd_gotoxy(11,1);
        lcd_putchar(7);
        fRead_adc = read_adc(0)  *  0.01857585 ; //  
        ftoa(fRead_adc , 1, strU); // 0.018575851393188854489164086687307
        lcd_gotoxy(12, 1);
        lcd_puts(strU);  // #asm("sei")
        calc_drawTermoU = 0;
         fTermM = ds18b20_temperature_MY(&rom_code[0][0]);
         if(fTermM != -9999) fTermM_old = fTermM;
         fTermR = ds18b20_temperature_MY(&rom_code[1][0]);
         if(fTermR != -9999) fTermR_old = fTermR;
         fTermS = ds18b20_temperature_MY(&rom_code[2][0]);
         if(fTermS != -9999) fTermS_old = fTermS;
         sprintf(lcd_buffer,"%c%.0f%c  %c%.0f%c  %c%.0f%c", 
                        4,fTermM_old, 223,
                        5,fTermR_old, 223,
                        6,fTermS_old, 223);
        lcd_gotoxy(0,0);
        lcd_puts(lcd_buffer);  //
 //       cFds18b20 = 4; // 400 кол-во пропусков замеров температуры 
 //   }
 //   else
 //   {
 //       cFds18b20--;
 //   }
}

//int valueProgresOld = 0;
char strTEMP[16];
int i_raznostProgres;
void fnDrav5element(void){ // Показіваем-рисуем один из 5-и символов 
    int kkk = i_raznostProgres * 0.017793594306049; //  i_raznostProgres/56 == 0.0178571428571429
    if(kkk<4) lcd_putchar(kkk); else lcd_putchar(255);
}

void fnDrav5text(int *valueProgres){  // Показіваем числовое значение тахометра //// if(valueProgres<1685){  // valueProgres>500 && 
	itoa(*valueProgres, strTEMP);
	lcd_gotoxy(6, 1);
	lcd_puts(strTEMP);
	lcd_puts(" ");     //}
}

void drawProgresBar(int valueProgres){
lcd_gotoxy(0, 1);
if (valueProgres > ROZ_16) { lcd_puts("яяяяяяяяяяяяяя^"); i_raznostProgres = valueProgres - ROZ_16; fnDrav5element(); lcd_puts("");  goto metka;}
if (valueProgres > ROZ_15) { lcd_puts("яяяяяяяяяяяяяя"); i_raznostProgres = valueProgres - ROZ_15; fnDrav5element(); lcd_puts(" ");  goto metka;}
if (valueProgres > ROZ_14) { lcd_puts("яяяяяяяяяяяяя"); i_raznostProgres = valueProgres - ROZ_14; fnDrav5element(); lcd_puts(" ");  goto metka;}   // goto metka;
if (valueProgres > ROZ_13) { lcd_puts("яяяяяяяяяяяя"); i_raznostProgres = valueProgres - ROZ_13; fnDrav5element(); lcd_puts(" ");  goto metka;} // return;
if (valueProgres > ROZ_12) { lcd_puts("яяяяяяяяяяя"); i_raznostProgres = valueProgres - ROZ_12; fnDrav5element(); lcd_puts(" ");  goto metka;}
if (valueProgres > ROZ_11) { lcd_puts("яяяяяяяяяя"); i_raznostProgres = valueProgres - ROZ_11; fnDrav5element(); lcd_puts(" ");  goto metka;}
if (valueProgres > ROZ_10) { lcd_puts("яяяяяяяяя"); i_raznostProgres = valueProgres - ROZ_10; fnDrav5element(); lcd_puts(" ");  goto metka;}
if (valueProgres > ROZ_9 ) { lcd_puts("яяяяяяяя"); i_raznostProgres = valueProgres - ROZ_9;  fnDrav5element(); lcd_puts("  ");  goto metka;}
if (valueProgres > ROZ_8 ) { lcd_puts("яяяяяяя"); i_raznostProgres = valueProgres - ROZ_8;  fnDrav5element(); lcd_puts("   ");  goto metka;}
if (valueProgres > ROZ_7 ) { lcd_puts("яяяяяя"); i_raznostProgres = valueProgres - ROZ_7;  fnDrav5element(); lcd_puts("    ");  goto metka;}
if (valueProgres > ROZ_6 ) { lcd_puts("яяяяя"); i_raznostProgres = valueProgres - ROZ_6;  fnDrav5element(); lcd_puts("     ");  goto metka;}
if (valueProgres > ROZ_5 ) { lcd_puts("яяяя"); i_raznostProgres = valueProgres - ROZ_5;  fnDrav5element(); lcd_puts(" "); fnDrav5text(&valueProgres); goto metka;}  // "_"
if (valueProgres > ROZ_4 ) { lcd_puts("яяя"); i_raznostProgres = valueProgres - ROZ_4;  fnDrav5element(); lcd_puts("  "); fnDrav5text(&valueProgres); goto metka;}  // "--"
if (valueProgres > ROZ_3 ) { lcd_puts("яя"); i_raznostProgres = valueProgres - ROZ_3;  fnDrav5element(); lcd_puts("   "); fnDrav5text(&valueProgres); goto metka;}  // "+++"
if (valueProgres > ROZ_2 ) { lcd_puts("я"); i_raznostProgres = valueProgres - ROZ_2;  fnDrav5element(); lcd_puts("    "); fnDrav5text(&valueProgres); goto metka;}  // "===="
if (valueProgres > ROZ_1 ) { lcd_puts(""); i_raznostProgres = valueProgres - ROZ_1;  fnDrav5element(); lcd_puts("          "); goto metka;}  // "&&&&&"
metka:
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void fnTest(int v_max){
    lcd_clear();
    /* detect how many DS18B20 devices are connected to the 1 Wire bus 
    //devices=w1_search(0xf0,rom_code);
    sprintf(lcd_buffer,"   Obnarugeno\n    %u DS18B20",devices);
    lcd_puts(lcd_buffer);
    delay_ms(800);          */
    /* display the ROM codes for each device 
    if (devices)
    {
        for (i=0;i<devices;i++)
        {
            sprintf(lcd_buffer,"DS18B20 #%u ROM\nimeet kod:",i+1);
            lcd_clear();
            lcd_puts(lcd_buffer);
            delay_ms(1000);
            lcd_clear();
            for (j=0;j<8;j++)
            {
                sprintf(lcd_buffer,"%02X ",rom_code[i][j]);
                lcd_puts(lcd_buffer);
                if (j==3) lcd_gotoxy(0,1);
            };
            delay_ms(1500);  // ==========================================================
       };
    }
    else
    while (1); */ /* stop here if no devices were found */
    
    lcd_clear();
    for (v=0; v<v_max; v++)
    {
        drawProgresBar(v); //ltoa(long int n, char *str) // lcd_gotoxy(0,1); ltoa(x,str_tLH); ltoa(xH, str_tLH); lcd_puts(str_tLH); lcd_gotoxy(4,1); ltoa(x,str_tLH); ltoa(xL, str_tLH); lcd_puts(str_tLH);
		// Маленькое примечание 3003 24028 //		lcd_gotoxy(0,1);//		lcd_puts("_____");
        pause;
    }
}

// см. https://datagor.ru/microcontrollers/682-gryzem-mikrokontrollery.-urok-4..html

