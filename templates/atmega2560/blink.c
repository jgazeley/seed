#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DELAY_MS 250               // Delay in milliseconds
#define PB7_MASK (1 << PB7)        // Bitmask for PB7

int main(void) {
    // Set PB7 as output
    DDRB |= PB7_MASK;

    while (1) {
        // Toggle PB7
        PORTB ^= PB7_MASK;
        _delay_ms(DELAY_MS);
    }
}
