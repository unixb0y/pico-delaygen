#include <stdio.h>
#include "pico/stdlib.h"
#include <tusb.h>
#include "hardware/pio.h"
#include "square.pio.h"

// Define for debugging; will have to handle all the extra serial prints, though
//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) printf(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

// Set to 1 if using Pico Debug 'n Dump PCB
#define USE_PDND 0
#if USE_PDND
    #define PIN_IN 18
    #define PIN_OUT 19
#else
    #define PIN_IN 9
    #define PIN_OUT 6
#endif

int oc(uint clk_interval);

/* --- pin mappings --- */
/* GPIO2 = reset        */
/* GPIO6 = glitch out   */
/* GPIO9 = trigger in   */
static const uint reset_output_pin = 2;
static const uint trigger_input_pin = PIN_IN;
static const uint trigger_output_pin = PIN_OUT;
static const uint led_pin = PICO_DEFAULT_LED_PIN;
bool led_on = true;

// number of clock cycles to delay glitch
uint delay_length = 100;
// number of clock cycles to pulse glitch
uint glitch_length = 100;
// avoids loading PIO assembly multiple times
static uint offset = 0xFFFFFFFF;

// global variable for PIO instance
// Choose PIO instance (0 or 1)
PIO pio = pio0;
// global variable for SM instance
uint sm = -1;

// variable is true if device
// waiting for an input pulse
bool waiting_for_pulse = false;
void irq0_callback() {
    waiting_for_pulse = true;
}

void irq1_callback() {
    waiting_for_pulse = false;
    // glitching finished, turn off LED
    led_on = false;
    gpio_put(led_pin, false);
}

inline uint32_t Reverse32(uint32_t value) {
    return (((value & 0x000000FF) << 24) |
            ((value & 0x0000FF00) <<  8) |
            ((value & 0x00FF0000) >>  8) |
            ((value & 0xFF000000) >> 24));
}

void reset_glitcher() {
    led_on = false;
    gpio_put(led_pin, false);
    waiting_for_pulse = false;
    pio_sm_set_enabled(pio, sm, false);
}

void toggle_led() {
    led_on = !led_on;
    gpio_put(led_pin, led_on);
}

// u32 command, i.e. read 4 more bytes
void set_glitch_pulse() {
    uint32_t glitch_len = 0;
    fread(&glitch_len, sizeof(char), 4, stdin);
    glitch_len = Reverse32(glitch_len);
    DEBUG_PRINT("Got pulse length: %lu\n", glitch_len); 
    glitch_length = glitch_len;
}

// u32 command, i.e. read 4 more bytes
void set_delay() {
    uint32_t delay_len = 0;
    fread(&delay_len, sizeof(char), 4, stdin);
    delay_len = Reverse32(delay_len);
    DEBUG_PRINT("Got delay length: %lu\n", delay_len); 
    delay_length = delay_len;
}

// u8 command; just go ahead and arm the PIO
void glitch() {
    // glitching starting, turn on LED
    led_on = true;
    gpio_put(led_pin, true);
    waiting_for_pulse = false;

    // Get first free state machine in PIO 0
    sm = pio_claim_unused_sm(pio, true);

    // Add PIO program to PIO instruction memory. SDK will find location and
    // return with the memory offset of the program.
    if (offset == 0xFFFFFFFF) { // Only load the program once
        offset = pio_add_program(pio, &square_program);
    }

    // Initialize the program using the helper function in our .pio file
    square_program_init(pio, sm, offset, trigger_input_pin, trigger_output_pin);

    // Pass the glitch length through FIFO; deduct 2 lost cycles
    pio_sm_put_blocking(pio, sm, delay_length-8);
    pio_sm_put_blocking(pio, sm, glitch_length-2);

    // Unclaim state machine after the run
    pio_sm_unclaim(pio, sm);
}

// read u8 command; need to return 1 byte
uint8_t get_status() {
    // armed          = 0b0001
    // pin in charged = 0b0010
    // timeout active = 0b0100
    // hvp internal   = 0b1000
    DEBUG_PRINT("Waiting for pulse? %d\n", waiting_for_pulse);
    if (waiting_for_pulse) {
        return 2;
    }
    else {
        return 0;
    }
}

int read_cmd() {
    char c = 0;
    while(1) {
        c = getchar();
        if (c == 0) continue;
        DEBUG_PRINT("Got char: %d\n", c);
        switch(c) {
            case 64:
                reset_glitcher();
            case 65:
                toggle_led();
                break;
            case 67:
                set_glitch_pulse();
                break;
            case 68:
                set_delay();
                break;
            case 70:
                glitch();
                break;
            case 73:
                putchar(get_status());
                break;
        }
    }
}

int main() {
    gpio_init(led_pin);
    gpio_init(reset_output_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_set_dir(reset_output_pin, GPIO_OUT);
    gpio_put(reset_output_pin, true);

    stdio_init_all();
    while (!tud_cdc_connected()) { sleep_ms(100);  }
    DEBUG_PRINT("USB connected.\n");

    // Uncomment for overclocking to 400MHz
    //oc(2); 

    irq_set_exclusive_handler(PIO0_IRQ_0, irq0_callback);
    irq_set_exclusive_handler(PIO0_IRQ_1, irq1_callback);
    irq_set_enabled(PIO0_IRQ_0, true);
    irq_set_enabled(PIO0_IRQ_1, true);
    pio0->inte0 = PIO_IRQ0_INTE_SM0_BITS;
    pio0->inte1 = PIO_IRQ0_INTE_SM1_BITS;

    DEBUG_PRINT("Starting program now...\n");
    read_cmd();
}
