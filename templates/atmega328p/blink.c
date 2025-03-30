#include <avr/io.h>
#include <util/delay.h>

// ----------------------------------------------------------------------------
// UART Setup for 9600 baud (assuming a 16MHz clock)
// UBRR value for 9600 baud = 103 (approx)
// ----------------------------------------------------------------------------
void uart_init(unsigned int ubrr) {
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    // Enable transmitter (and receiver if needed)
    UCSR0B = (1 << TXEN0);
    // Set frame format: 8 data bits, no parity, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(unsigned char data) {
    // Wait until the transmit buffer is empty
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_print(const char *str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

int main(void) {
    // Initialize UART for 9600 baud (for 16MHz clock, ubrr = 103)
    uart_init(103);

    // Wait 2000 ms for any connection to be established
    _delay_ms(2000);

    // Print a hello message over UART
    uart_print("Hello! (from blink on AVR)\n");

    // Configure LED pin: Using PORTB, pin 5 (Arduino Uno onboard LED)
    DDRB |= (1 << DDB5); // Set PB5 as output

    // Blink the LED in an infinite loop
    while (1) {
        PORTB |= (1 << PORTB5);   // LED on
        _delay_ms(500);           // Delay 500 ms
        PORTB &= ~(1 << PORTB5);  // LED off
        _delay_ms(500);           // Delay 500 ms
    }

    return 0;
}
