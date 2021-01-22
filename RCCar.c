/*
 * File:   RCCar.c
 * Author: John
 * PIC12HV752
 * Created on August 4, 2019, 5:01 PM
 */

#define _XTAL_FREQ 8000000
#include <xc.h>
#include <stdlib.h>

// CONFIG
#pragma config FOSC0 = INT, WDTE = OFF, PWRTE = OFF, MCLRE = OFF, CP = OFF,BOREN = EN, WRT = OFF, CLKOUTEN = OFF

unsigned char packet[4];
int count, hcount, servo_t, horn = 0;

void __interrupt() timer_isr(void)
{  
    if(INTCONbits.T0IF==1) // Timer has overflown
    {
        count++;
        TMR0 = 140;
    }
    
    if (servo_t >= count) RA4 = 1;
    
    if (count > servo_t) RA4 = 0;
    
    if(count == 160){
        count = 0;
        if(horn == 1){
            if(hcount < 25){
                RA0 = 1;
                hcount++;
            }
            else {
                RA0 = 0;
                hcount = 0;
                horn = 0;
            }
        }
    }
    INTCONbits.T0IF=0;
}

// Receives 2 byte message containing an ID and value
void receive_rf(void) {
    unsigned char data, bits, bytes = 0;
    unsigned char rxdat[4];
    int i, period;
     
    while(1) {
        while(!RA3);    // wait for input / edge
        TMR1 = 0;
        while(RA3);     // wait for input \ edge
        period = TMR1;
        if(period > 2030 || period < 1970) bytes = 0;   // 1ms = 2004
        else break;
    }

    while(bytes < 4) {
        bits = 8;
        while(bits) {
            while(!RA3);
            TMR1 = 0;
            while(RA3);
            period = TMR1;
            if(period > 2000) break;                     // unexpected start pulse

            if(period < 900) data &= ~(1UL << 0);     // bit is 0
            else            data |= 1UL << 0;       // bit is 1
            bits--;
            if(bits == 0) break;
            data = (data << 1);
        }
        
        if(bits) bytes = 0;
        else {                                      // byte is good
            rxdat[bytes] = data;
            bytes++;
        }
    }
    
    for(i = 0; i < 4; i++) {
          packet[i] = rxdat[i];
    }
}

void main(void){
    OSCCONbits.IRCF = 0b11;
    OPTION_REGbits.T0CS = 0b0;
    OPTION_REGbits.PSA = 0b0;
    OPTION_REGbits.PS = 0b000;
    INTCON = 0b11100000;
    T1CON |= 0b00000001;
    PR2 = 200;
    CCP1CONbits.CCP1M = 0b1100;
    T2CONbits.T2CKPS = 0b01;
    T2CONbits.TMR2ON = 0b1;
    TRISA = 0b00000000;
    ANSELA = 0;
    PORTA = 0;
 
    int pwm, i, FB, LR = 0;  

    while(1){
        
        receive_rf();
        
        for(i = 0; i < 3; i++) {        // horn on
            if(packet[i] == 104) {
                if(packet[i+1] == 110) {
                    horn = 1;
                    i += 1;
                    break;
                }
            }
            
            if(packet[i] == 108) {       // lights on/off
                if(packet[i+1] == 116) {
                    if(RA5 == 1) RA5 = 0;
                    else         RA5 = 1;
                    i += 1;
                    break;
                }
            }
            
            if(packet[i] == 120) {       // drive
                FB = packet[i+1]-49;     // 804 pwm = 100% duty cycle 
                if(FB >= 0) {            // foward
                    RA1 = 0;
                    pwm = FB*16;
                    CCPR1L = (pwm>>2);
                    CCP1CONbits.DC1B = (pwm&0b0011);
                }
                else if(FB < 0) {          // reverse
                    RA1 = 1;
                    pwm = 804-(FB*-16);
                    CCPR1L = (pwm>>2);
                    CCP1CONbits.DC1B = (pwm&0b0011);
                }
                i += 1;
                break;
            }
            
            if(packet[i] == 121) {        // turn servo left or right
                LR = packet[i+1]-50;
                if(LR == 0) {
                    servo_t = 12;
                }
                if(LR < 0) {
                    if(LR < -25) servo_t = 14;
                    else         servo_t = 13;
                }
                if(LR > 0) {
                    if(LR > 25) servo_t = 10;
                    else        servo_t = 11;
                }
                i += 1;
                break;
            }
        }
    }
    return;
}

