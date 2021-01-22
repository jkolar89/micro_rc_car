
/*
 * File:   Transmitter.c
 * Author: John
 * PIC12HV752
 * Created on July 24, 2019, 11:57 AM
 */

#define _XTAL_FREQ 1000000
#include <xc.h>
#include <stdlib.h>

// CONFIG
#pragma config FOSC0 = INT, WDTE = OFF, PWRTE = OFF, MCLRE = OFF, CP = OFF, BOREN = DIS, WRT = OFF, CLKOUTEN = OFF

void send_data(unsigned char dat, unsigned char val){
    unsigned char tbit;
    int i;
    for(i = 0; i < 4; i++) {
        RA5 = 1;
        __delay_us(300);
        RA5 = 0;
        __delay_us(300);
    }
    
    RA5 = 1;
    __delay_ms(1);
    RA5 = 0;
    __delay_us(450);

    for(i = 0; i < 2; i++) {
        for(tbit=0; tbit<8; tbit++){
            RA5 = 1;
            __delay_us(300);
            if(dat & 0x80) __delay_us(300);
            RA5 = 0;
            __delay_us(450);
            dat <<= 1;
        }
        for(tbit=0; tbit<8; tbit++){
            RA5 = 1;
            __delay_us(300);
            if(val & 0x80) __delay_us(300);
            RA5 = 0;
            __delay_us(450);
            val <<= 1;
        }
    }
    __delay_us(450);
}

void main(void){
    OSCCON |= 0b00010000;
    INTCON = 0b11000000;
    PIE1 = 0b00000000;
    PIE2 = 0b00000000;
    OPTION_REG = 0b11010110;
    T1CON |= 0b00110001;
    TRISA = 0b00001110;
    ANSELA = 0b00000110;
    ADCON0 = 0b00001001;
    ADCON1 = 0b01000000;
    PORTA = 0;
    
    unsigned char x, y = 0;
    int t, c1, c2 = 0;
    
    while(1){  
        ADCON0bits.CHS = 0b0001;
        __delay_us(5);
        ADCON0bits.GO_nDONE = 1;
        while(ADCON0bits.GO_nDONE == 1){}
        x = ((ADRESH<<2)+(ADRESL>>6))/10;  // max value ~102+
        if(x == 49) {
            if(c1 < 50) {
                send_data('x', x);
                c1++;
            }
        }
        else {
            send_data('x', x);
            c1 = 0;
        }
        
        ADCON0bits.CHS = 0b0010;
        __delay_us(5);
        ADCON0bits.GO_nDONE = 1;
        while(ADCON0bits.GO_nDONE == 1){}
        y = ((ADRESH<<2)+(ADRESL>>6))/10;
        if(y == 50) {
            if(c2 < 50) {
                send_data('y', y);
                c2++;
            }
        }
        else {
            send_data('y', y);
            c2 = 0;
        }
        
        if(RA3 == 1){
            __delay_ms(40);
            TMR1 = 0;
            while(RA3 == 1);
            t = TMR1;
            if(t < 15000) send_data('h', 'n');
            else          send_data('l','t');
        }
    }
    
    return;
}