#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN 25

int main() {
    stdio_init_all();

    // Initialize onboard LED (GPIO 25)
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Wait 2 seconds for USB serial to establish
    sleep_ms(2000);

    // Print a hello message
    printf("Hello from blink on RP2040!\n");

    // Blink the onboard LED
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
    return 0;
}
