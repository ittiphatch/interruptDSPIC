/*
 * File:   newmainXC16.c
 * Author: Stang
 *
 * Created on November 16, 2018, 9:02 AM
 */
//
// LCD display program for dsPIC30F4011
// Written by Ted Burke - Last updated 16-4-2013
//
 
#include <xc.h>
#include <libpic30.h>
#include <timer.h>


// Configuration settings
_FOSC(CSW_FSCM_OFF & FRC_PLL16); // Fosc=16x7.5MHz, Fcy=30MHz
_FWDT(WDT_OFF);                  // Watchdog timer off
_FBORPOR(MCLR_DIS);              // Disable reset pin
 
#define RS_PIN _LATC13
#define RW_PIN _LATC14
#define E_PIN _LATC15
#define CLOCK_FREQ 8000000ULL
void delay_ms(unsigned int n);
void send_nibble(unsigned char nibble);
void send_command_byte(unsigned char byte);
void send_data_byte(unsigned char byte);

/*const char line1[] = {'S','t','a','r','t','.','.','.'};
const char line2[] = {'F','i','n','n','i','s','h'};
const char line3[] = {"Time Out"};
int n;*/

/*void __attribute__((__interrupt__)) _T1Interrupt(void)
{
    int n;
    const char line[8] = "Time Out";
    if(IFS0bits.INT0IF == 1){
          PORTAbits.RA0 = 0; 
        while(1){
            send_command_byte(0b00000001);
            send_command_byte(0x02);
             // Clear display
            for (n=0 ; n<8 ; ++n){ 
                send_data_byte(line[n]);
            }
            if (PORTAbits.RA2 == 1)
                break;
        }
    }   
    
    IFS0bits.INT0IF = 0;
    IFS0bits.T1IF = 0; 
}*/ 
void __attribute__((__interrupt__)) _INT0Interrupt(void)
{
    //unsigned int match_value = 0xFFF;   
    /*OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_STOP &
               T1_PS_1_1 & T1_SYNC_EXT_OFF &
               T1_SOURCE_INT, match_value);*/
    WriteTimer1(0);
    char line2[] = "Start...";
    int n;
    WriteTimer1(0);
    while(PORTEbits.RE2 == 0){// && ReadTimer1() < 62500){
        LATEbits.LATE0 = 1; 
        
        send_command_byte(0b00000001); // Clear display
        send_command_byte(0x02);
        for (n=0 ; n<8 ; ++n){ 
            send_data_byte(line2[n]);
        }
    }
    IFS0bits.INT0IF = 0;       
} 
int main()
{   
    INTCON2bits.INT0EP = 1;
    //IEC0bits.T1IE = 1;
    IEC0bits.INT0IE = 1;
    IPC0 = 0x0001;   
    unsigned int match_value = 0xFFFF;
    //ConfigIntTimer1(T1_INT_PRIOR_1 & T1_INT_ON);
    OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_STOP &
               T1_PS_1_256 & T1_SYNC_EXT_OFF &
               T1_SOURCE_INT, match_value);
    //CloseTimer1();
    __builtin_write_OSCCONH((uint8_t) ((0x0700 >> _OSCCON_NOSC_POSITION) & 0x00FF));
    __builtin_write_OSCCONL((uint8_t) (0x0700 & 0x00FF));
    TRISEbits.TRISE0 = 0; //PIN A1 = OUTPUT
    TRISEbits.TRISE2 = 1; //PIN A2 = INPUT
    TRISC = 0; // RC13-15 as digital output
    TRISB = 0xFFF0; // RB0-3 as digital outputs
    
    _PCFG0 = 1; // AN0 is digital
    _PCFG1 = 1; // AN1 is digital
    _PCFG2 = 1; // AN2 is digital
    _PCFG3 = 1; // AN3 is digital
    RW_PIN = 0;
    RS_PIN = 0;
    E_PIN = 1;   
    // Initialisation
    delay_ms(16); // must be more than 15ms
    send_nibble(0b0011);
    delay_ms(5); // must be more than 4.1ms
    send_nibble(0b0011);
    delay_ms(1); // must be more than 100us
    send_nibble(0b0011);
    delay_ms(5); // must be more than 4.1ms
    send_nibble(0b0010); // select 4-bit mode
     
    // Display settings
    send_command_byte(0b00101000); // N=0 : 2 lines (half lines!), F=0 : 5x7 font
    send_command_byte(0b00001000); // Display: display off, cursor off, blink off
    send_command_byte(0b00000001); // Clear display
    send_command_byte(0b00000110); // Set entry mode: ID=1, S=0
    send_command_byte(0b00001111);
    //send_command_byte(0b00001111); // Display: display on, cursor on, blink on
    // Define two 8 character strings
    while(1){
        unsigned int n;
        const char line[8] = "Empty...";
        LATEbits.LATE0 = 0;
        send_command_byte(0b00000001); 
        send_command_byte(0x02);
        // Clear display
        for (n=0 ; n<8 ; ++n){ 
            send_data_byte(line[n]);
        }
    }
 
}
// Delay by specified number of milliseconds
void delay_ms(unsigned int n)
{
    while(n--) __delay32(3000);
}
 
void send_nibble(unsigned char nibble)
{
    // Note: data is latched on falling edge of pin E
    LATB = nibble;
    delay_ms(1);
    E_PIN = 0;
    delay_ms(1);
    E_PIN = 1;
    delay_ms(2); // Enough time even for slowest command
}
 
// Send a command byte (i.e. with pin RS low)
void send_command_byte(unsigned char byte)
{
    RS_PIN = 0;
    send_nibble(byte >> 4);
    send_nibble(byte & 0xF);
}
 
// Send a data byte (i.e. with pin RS high)
void send_data_byte(unsigned char byte)
{
    RS_PIN = 1;
    send_nibble(byte >> 4);
    send_nibble(byte & 0xF);
}