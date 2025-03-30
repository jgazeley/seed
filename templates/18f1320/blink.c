#include <xc.h>

#pragma config OSC = INTIO2
#pragma config WDT = OFF
#pragma config MCLRE = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000

void main(void) {
    OSCCON = 0b01110000;       // 8 MHz internal oscillator

    TRISAbits.TRISA1 = 0;      // Set RA1 as output

    while (1) {
        LATAbits.LATA1 = 1;    // LED ON
        __delay_ms(30);
        LATAbits.LATA1 = 0;    // LED OFF
        __delay_ms(30);
    }
}
