#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DELAY_MS 250               // Delay in milliseconds
#define PB5_MASK (1 << PB5)        // Bitmask for PB5

int main(void) {
    // Set PB7 as output
    DDRB |= PB5_MASK;

    while (1) {
        // Toggle PB5
        PORTB ^= PB5_MASK;
        _delay_ms(DELAY_MS);
    }
}
