#include <avr/io.h>
#include <util/delay.h>

#define DATA_PIN PD3  
#define CLOCK_PIN PD4 
#define LATCH_PIN PD2 

#define BUTTON_PIN PD5 
#define TILT_PIN PD6   

#define RED_PIN PD7   
#define GREEN_PIN PB0 
#define BLUE_PIN PB1  

#define BUZZER_PIN PB2 

void shift_out(uint8_t data) {
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i))
            PORTD |= (1 << DATA_PIN);
        else
            PORTD &= ~(1 << DATA_PIN);

        PORTD |= (1 << CLOCK_PIN);
        PORTD &= ~(1 << CLOCK_PIN);
    }
}

void display(uint8_t pattern) {
    shift_out(pattern);

    PORTD |= (1 << LATCH_PIN);
    PORTD &= ~(1 << LATCH_PIN);
}

void display_digit(uint8_t digit)
{
    switch (digit) {
    case 0:
        display(0xA0);
        break;

    case 1:
        display(0xF9);
        break;

    case 2:
        display(0xC4);
        break;

    case 3:
        display(0xD0);
        break;

    case 4:
        display(0x99);
        break;

    case 5:
        display(0x92);
        break;

    case 6:
        display(0x82);
        break;

    case 7:
        display(0xF8);
        break;

    case 8:
        display(0x00);
        break;

    case 9:
        display(0x90);
        break;
    }
}

void rgb_off(void) {
    PORTD &= ~(1 << RED_PIN);
    PORTB &= ~(1 << GREEN_PIN);
    PORTB &= ~(1 << BLUE_PIN);
}

void rgb_red(void) {
    rgb_off();

    PORTD |= (1 << RED_PIN);
}

void rgb_green(void) {
    rgb_off();

    PORTB |= (1 << GREEN_PIN);
}

void rgb_blue(void) {
    rgb_off();

    PORTB |= (1 << BLUE_PIN);
}

void rgb_yellow(void)
{
    rgb_off();

    PORTD |= (1 << RED_PIN);
    PORTB |= (1 << GREEN_PIN);
}

void rgb_cyan(void) {
    rgb_off();

    PORTB |= (1 << GREEN_PIN);
    PORTB |= (1 << BLUE_PIN);
}

void rgb_magenta(void) {
    rgb_off();

    PORTD |= (1 << RED_PIN);
    PORTB |= (1 << BLUE_PIN);
}

void rgb_white(void) {
    PORTD |= (1 << RED_PIN);
    PORTB |= (1 << GREEN_PIN);
    PORTB |= (1 << BLUE_PIN);
}

void rgb_finished_animation(void) {
    rgb_red();
    _delay_ms(500);

    rgb_green();
    _delay_ms(500);

    rgb_blue();
    _delay_ms(500);

    rgb_yellow();
    _delay_ms(500);

    rgb_cyan();
    _delay_ms(500);

    rgb_magenta();
    _delay_ms(500);

    rgb_white();
    _delay_ms(1000);
}

void buzzer_off(void) {
    PORTB &= ~(1 << BUZZER_PIN);
}

void buzzer_on(void) {
    PORTB |= (1 << BUZZER_PIN);
}

void buzzer_finished(void) {
    buzzer_on();
    _delay_ms(1000);
    buzzer_off();
}

int main(void) {
    DDRD |= (1 << LATCH_PIN);
    DDRD |= (1 << DATA_PIN);
    DDRD |= (1 << CLOCK_PIN);
    DDRD &= ~(1 << BUTTON_PIN);
    DDRD &= ~(1 << TILT_PIN);
    PORTD |= (1 << BUTTON_PIN);
    PORTD |= (1 << TILT_PIN);
    DDRD |= (1 << RED_PIN);
    DDRB |= (1 << GREEN_PIN);
    DDRB |= (1 << BLUE_PIN);
    DDRB |= (1 << BUZZER_PIN);

    rgb_off();

    buzzer_off();

    display(0xFF);

    while (1) {
        if (!(PIND & (1 << BUTTON_PIN))) {
            _delay_ms(30);

            if (!(PIND & (1 << BUTTON_PIN))) {
                uint8_t digit = 0;

                rgb_off();

                buzzer_off();

                display_digit(0);

                while (digit < 10) {
                    if (PIND & (1 << TILT_PIN)) {
                        digit = 0;

                        display_digit(0);

                        _delay_ms(50);

                        while (PIND & (1 << TILT_PIN)) {
                        }
                    }

                    for (uint16_t i = 0; i < 100; i++) {
                        _delay_ms(10);

                        if (PIND & (1 << TILT_PIN)) {
                            digit = 0;

                            display_digit(0);

                            _delay_ms(50);

                            while (PIND & (1 << TILT_PIN)) {
                            }

                            break;
                        }
                    }

                    if (digit < 9) {
                        digit++;
                        display_digit(digit);
                    } else {
                        break;
                    }
                }

                display(0xFF);

                buzzer_finished();

                rgb_finished_animation();

                rgb_off();

                buzzer_off();

                while (!(PIND & (1 << BUTTON_PIN))){
                }

                _delay_ms(30);
            }
        }
    }
}