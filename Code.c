#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define degree_sysmbol 0xdf

//LCD PORT Define
#define lcd_D7_port     PORTD                   // LCD D7 connection
#define lcd_D7_bit      PORTD7
#define lcd_D7_ddr      DDRD

#define lcd_D6_port     PORTD                   // LCD D6 connection
#define lcd_D6_bit      PORTD6
#define lcd_D6_ddr      DDRD

#define lcd_D5_port     PORTD                   // LCD D5 connection
#define lcd_D5_bit      PORTD5
#define lcd_D5_ddr      DDRD

#define lcd_D4_port     PORTD                   // LCD D4 connection
#define lcd_D4_bit      PORTD4
#define lcd_D4_ddr      DDRD

#define lcd_E_port      PORTD                   // LCD Enable pin
#define lcd_E_bit       PORTD3
#define lcd_E_ddr       DDRD

#define lcd_RS_port     PORTD                   // LCD Register Select pin
#define lcd_RS_bit      PORTD2
#define lcd_RS_ddr      DDRD


// LCD module information
#define lcd_LineOne     0x00                    // start of line 1  0000 0000
#define lcd_LineTwo     0x40                    // start of line 2  0100 0000

// LCD instructions
#define lcd_Clear           0b00000001          // replace all characters with ASCII 'space'
#define lcd_Home            0b00000010          // return cursor to first position on first line
#define lcd_EntryMode       0b00000110          // shift cursor from left to right on read/write
#define lcd_DisplayOff      0b00001000          // turn display off
#define lcd_DisplayOn       0b00001100          // display on, cursor off, don't blink character
#define lcd_FunctionReset   0b00110000          // reset the LCD
#define lcd_FunctionSet4bit 0b00101000          // 4-bit data, 2-line display, 5 x 7 font
#define lcd_SetCursor       0b10000000          // set cursor position

// Function Prototypes

void lcd_write_4(uint8_t);
void lcd_write_instruction_4d(uint8_t);
void lcd_write_character_4d(uint8_t);
void lcd_write_string_4d(uint8_t *);
void lcd_init_4d(void);
void ADC_Init();
int ADC_Read(char channel);
void ADC_Init2();
int ADC_Read2(char channel);


/******************************* Main Program Code *************************/
int main(void)
{
	DDRB|=(1<<0); //AC port
	DDRB|=(1<<1); //venti port
	PORTB|=(1<<1); // venti high
		
// configure the microprocessor pins for the data lines
    lcd_D7_ddr |= (1<<lcd_D7_bit);                  // 4 data lines - output
    lcd_D6_ddr |= (1<<lcd_D6_bit);
    lcd_D5_ddr |= (1<<lcd_D5_bit);
    lcd_D4_ddr |= (1<<lcd_D4_bit);

// configure the microprocessor pins for the control lines
    lcd_E_ddr |= (1<<lcd_E_bit);                    // E line - output
    lcd_RS_ddr |= (1<<lcd_RS_bit);                  // RS line - output

// initialize the LCD controller as determined by the defines (LCD instructions)
    lcd_init_4d();                                  // initialize the LCD display for a 4-bit interface
	
	char Temperature[10];
	char Temperature2[10];
	char DIF[10];
	char BEST[10];
	float celsius;
	float celsius2;

	ADC_Init();                 /* initialize ADC*/
	ADC_Init2();                 /* initialize ADC2*/
	
	while(1){
	
		celsius = (ADC_Read(0)*4.03);
		celsius = (celsius/10.00);
		sprintf(Temperature,"%d%cC  ", (int)celsius, degree_sysmbol);/* convert integer value to ASCII string */
		
		celsius2 = (ADC_Read2(1)*4.03);
		celsius2 = (celsius2/10.00);
		sprintf(Temperature2,"%d%cC  ", (int)celsius2, degree_sysmbol);/* convert integer value to ASCII string */
		
		lcd_write_instruction_4d(lcd_SetCursor | lcd_LineOne);    // set cursor to start of first line
		_delay_us(80);
		
		lcd_write_string_4d("IN-");   // display the first line of information
		lcd_write_string_4d(Temperature);
		_delay_ms(2000);
		
		lcd_write_string_4d("OUT-");   
		lcd_write_string_4d(Temperature2);
		_delay_ms(2000);
		
		
		if (celsius2+1> celsius)
		{
				lcd_write_instruction_4d(lcd_SetCursor | lcd_LineTwo);  // set cursor to start of second line
				_delay_us(80);
				
				float diff = celsius - celsius2;   // Getting the Difference between 2 temperature
				sprintf(DIF,"%d%cC  ", (int)diff, degree_sysmbol);/* convert integer value to ASCII string */
				_delay_us(80);
				
				float best_t = celsius2 + diff/2;  //Best temperature Calculation
				sprintf(BEST,"%d%cC  ", (int)best_t, degree_sysmbol);/* convert integer value to ASCII string */
				
				lcd_write_string_4d("BEST TEMP - ");  //best temp suggesion
				lcd_write_string_4d(BEST);
				_delay_ms(1000);
				
				PORTB&=~(1<<0);  // ac low
				PORTB|=(1<<1); //venti high					
		}
		
		else {
			do 
			{
				lcd_write_instruction_4d(lcd_Clear);             // clear display RAM
				_delay_ms(4);                                   // 1.64 mS delay (min)
				
				lcd_write_instruction_4d(lcd_SetCursor | lcd_LineOne);    // set cursor to start of first line
				_delay_us(80);
				
				PORTB&=~(1<<1);  // venti low
				PORTB|=(1<<0); // ac high
							
				lcd_write_string_4d("A/C is Turn off");
				_delay_ms(10000);
				
				lcd_write_instruction_4d(lcd_Clear);             // clear display RAM
				_delay_ms(40);                                   // 1.64 mS delay (min)
				
			} while (celsius2 > celsius);   // indoor <outdoor		  
		}	
		memset(Temperature2,0,10);
		memset(Temperature,0,10);
	}
// endless loop
    while(2);
    return 0;
}

/******************************* End of Main Program Code ******************/

/*==============================  ADC Conversation   ======================*/

void ADC_Init(){
	DDRC = 0x00;	        /* Make ADC port as input */
	ADCSRA = 0x87;          /* Enable ADC, with freq/128  */
	ADMUX = 0x40;           /* Vref: Avcc, ADC channel: 0 */
}

int ADC_Read(char channel)
{
	ADMUX = 0x40 | (channel & 0x07);   /* set input channel to read */
	ADCSRA |= (1<<ADSC);               /* Start ADC conversion */
	while (!(ADCSRA & (1<<ADIF)));     /* Wait until end of conversion by polling ADC interrupt flag */
	ADCSRA |= (1<<ADIF);               /* Clear interrupt flag */
	_delay_ms(1);                      /* Wait a little bit */
	return ADCW;                       /* Return ADC word */
}

void ADC_Init2(){
	DDRC = 0x00;	        /* Make ADC port as input */
	ADCSRA = 0x87;          /* Enable ADC, with freq/128  */
	ADMUX = 0x41;           /* Vref: Avcc, ADC channel: 1 */
}

int ADC_Read2(char channel)
{
	ADMUX = 0x41 | (channel & 0x07);   /* set input channel to read */
	ADCSRA |= (1<<ADSC);               /* Start ADC conversion */
	while (!(ADCSRA & (1<<ADIF)));     /* Wait until end of conversion by polling ADC interrupt flag */
	ADCSRA |= (1<<ADIF);               /* Clear interrupt flag */
	_delay_ms(1);                      /* Wait a little bit */
	return ADCW;                       /* Return ADC word */
}

/*============================== End of ADC Conversation ======================*/


/*==============================   4-bit LCD Functions  ======================*/

void lcd_init_4d(void)
{
// Power-up delay
    _delay_ms(100);                                 // initial 40 mSec delay

// Set up the RS and E lines for the 'lcd_write_4' subroutine.
    lcd_RS_port &= ~(1<<lcd_RS_bit);                // select the Instruction Register (RS low)
    lcd_E_port &= ~(1<<lcd_E_bit);                  // make sure E is initially low

// Reset the LCD controller
    lcd_write_4(lcd_FunctionReset);                 // first part of reset sequence
    _delay_ms(10);                                  // 4.1 mS delay (min)

    lcd_write_4(lcd_FunctionReset);                 // second part of reset sequence
    _delay_us(200);                                 // 100uS delay (min)

    lcd_write_4(lcd_FunctionReset);                 // third part of reset sequence
    _delay_us(200);                                 // this delay is omitted in the data sheet
 
    lcd_write_4(lcd_FunctionSet4bit);               // set 4-bit mode
    _delay_us(80);                                  // 40uS delay (min)

// Function Set instruction
    lcd_write_instruction_4d(lcd_FunctionSet4bit);   // set mode, lines, and font
    _delay_us(80);                                  // 40uS delay (min)

// Display On/Off Control instruction
    lcd_write_instruction_4d(lcd_DisplayOff);        // turn display OFF
    _delay_us(80);                                  // 40uS delay (min)

// Clear Display instruction
    lcd_write_instruction_4d(lcd_Clear);             // clear display RAM
    _delay_ms(4);                                   // 1.64 mS delay (min)

// ; Entry Mode Set instruction
    lcd_write_instruction_4d(lcd_EntryMode);         // set desired shift characteristics
    _delay_us(80);                                  // 40uS delay (min)
 
// Display On/Off Control instruction
    lcd_write_instruction_4d(lcd_DisplayOn);         // turn the display ON
    _delay_us(80);                                  // 40uS delay (min)
}


/*-------------------display a string of characters on the LCD------------------*/

void lcd_write_string_4d(uint8_t theString[])
{
    volatile int i = 0;                             // character counter*/
    while (theString[i] != 0)
    {
        lcd_write_character_4d(theString[i]);
        i++;
        _delay_us(80);                              // 40 uS delay (min)
    }
}

/*--------------------Send a byte of information to the LCD data register--------------*/

void lcd_write_character_4d(uint8_t theData)
{
    lcd_RS_port |= (1<<lcd_RS_bit);                 // select the Data Register (RS high)
    lcd_E_port &= ~(1<<lcd_E_bit);                  // make sure E is initially low
    lcd_write_4(theData);                           // write the upper 4-bits of the data
    lcd_write_4(theData << 4);                      // write the lower 4-bits of the data
}

/*---------Send a byte of information to the LCD instruction register----------*/

void lcd_write_instruction_4d(uint8_t theInstruction)
{
    lcd_RS_port &= ~(1<<lcd_RS_bit);                // select the Instruction Register (RS low)
    lcd_E_port &= ~(1<<lcd_E_bit);                  // make sure E is initially low
    lcd_write_4(theInstruction);                    // write the upper 4-bits of the data
    lcd_write_4(theInstruction << 4);               // write the lower 4-bits of the data
}


/*---------------Send a byte of information to the LCD module---------------------*/

void lcd_write_4(uint8_t theByte)
{
    lcd_D7_port &= ~(1<<lcd_D7_bit);                        // assume that data is '0'
    if (theByte & 1<<7) lcd_D7_port |= (1<<lcd_D7_bit);     // make data = '1' if necessary

    lcd_D6_port &= ~(1<<lcd_D6_bit);                        // repeat for each data bit
    if (theByte & 1<<6) lcd_D6_port |= (1<<lcd_D6_bit);

    lcd_D5_port &= ~(1<<lcd_D5_bit);
    if (theByte & 1<<5) lcd_D5_port |= (1<<lcd_D5_bit);

    lcd_D4_port &= ~(1<<lcd_D4_bit);
    if (theByte & 1<<4) lcd_D4_port |= (1<<lcd_D4_bit);

// write the data
                                                    // 'Address set-up time' (40 nS)
    lcd_E_port |= (1<<lcd_E_bit);                   // Enable pin high
    _delay_us(1);                                   // implement 'Data set-up time' (80 nS) and 'Enable pulse width' (230 nS)
    lcd_E_port &= ~(1<<lcd_E_bit);                  // Enable pin low
    _delay_us(1);                                   // implement 'Data hold time' (10 nS) and 'Enable cycle time' (500 nS)
}

/*============================== End of 4-bit LCD Functions ======================*/

