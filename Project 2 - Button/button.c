#define F_CPU 16000000UL
#include <avr/io.h>

int main(void)
{
    DDRB |= (1 << DDB5);

    DDRD &= ~(1 << DDD2);

    while (1)
    {
        if (PIND & (1 << PIND2))
        {
            PORTB |= (1 << PORTB5);   
        }
        else
        {
            PORTB &= ~(1 << PORTB5);  
        }
    }
}