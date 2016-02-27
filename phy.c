
/*
 *
 */

#include "phy.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint16_t phy_wheelPos = 0x8000U;

uint8_t phy_shifterButs[16];

uint8_t phy_wheelButs[8];

uint16_t phy_pedalAccel;

uint16_t phy_pedalBrake;

uint16_t phy_pedalClutch;

uint16_t phy_shifterX;

uint16_t phy_shifterY;

void
phy_initAll(void)
{
    /*
     * set main clock
     */
    cli();
    CLKPR = (1 << CLKPCE);      /* Enable CLK prescaler change */
    CLKPR = 0x00;               /* no divider - 16 MHz for teensy */
    sei();

    /*
     * configure ADC
     */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* Enable and set prescaler to 64 */
    /*
     * we need freq about 200 kHz 
     */
    ADCSRB = (1 << ADHSM);      /* enable fast ADC mode */

    DIDR0 = 0;                  /*enable digital input on ADC pins */
    DIDR2 = 0;                  /*enable digital input on ADC pins */

    /*
     * LED pin 
     */
    DDRD |= (1 << DDD6);        /* output */

    phy_offLED();               /* turn off during setup */

    /*
     * Shifter stick analog pins 
     */
    DDRF &= ~((1 << DDF5) | (1 << DDF6));       /* input */
    PORTF |= ((1 << DDF5) | (1 << DDF6));       /* pullup */
    DIDR0 |= ((1 << 5) | (1 << 6));     /* disable digital input on ADC0 ADC1 */

    /*
     * Pedals analog pins 
     */
    DDRF &= ~((1 << DDF0) | (1 << DDF1) | (1 << DDF4)); /* input */
    PORTF |= ((1 << PORTF0) | (1 << PORTF1) | (1 << PORTF4));   /* pullup */
    DIDR0 |= ((1 << 0) | (1 << 1) | (1 << 2));  /* disable digital input */

    /*
     * Wheel encoder pins 
     */
    DDRB &= ~((1 << DDB4) | (1 << DDB5) | (1 << DDB6)); /* input */
    PORTB |= ((1 << PORTB4) | (1 << PORTB5) | (1 << PORTB6));   /* pullup */

    /*
     * Wheel buttons pins 
     */
    DDRD |= (1 << DDD1);        /* output SH/LD */
    DDRD |= (1 << DDD0);        /* output CLK */
    DDRB &= ~(1 << DDB7);       /* input QH */
    PORTB &= ~(1 << PORTB7);    /* no pullup QH */

    /*
     * Wheel buttons init 
     */
    PORTD |= (1 << PORTD1);     /* SH/LD to high */
    PORTD |= (1 << PORTD0);     /* CLK to high */

    /*
     * pin change interrupt 
     */
    PCMSK0 = (1 << 5) | (1 << 6);       /* ChA + ChI */
    PCICR = 0x01;               /* enable interrupt */

    /*
     * Shifter buttons pins 
     */
    DDRD |= (1 << DDD7);        /* output SH/LD */
    DDRC |= (1 << DDC6);        /* output CLK */
    DDRC &= ~(1 << DDC7);       /* input QH */
    PORTC &= ~(1 << PORTC7);    /* no pullup QH */

    /*
     * Shifter buttons init 
     */
    PORTD |= (1 << PORTD7);     /* SH/LD to high */
    PORTC |= (1 << PORTC6);     /* CLK to high */

    /*
     * enable high speed ADC 
     */
    ADCSRB = 0x80;
}

void
phy_readShifterButs(void)
{
    /*
     * load buttons to registers 
     */
    PORTD &= ~(1 << PORTD7);    /* set parallel mode */
    _delay_us(2);               /* wait for stabile signals */
    PORTD |= (1 << PORTD7);     /* return to serial mode */

    /*
     * shift bits 
     */
    for (int i = 0; i < 16; i++)
    {
        PORTC &= ~(1 << PORTC6);        /* next CLK cycle */
        _delay_us(2);           /* minimal period */
        phy_shifterButs[i] = PINC & (1 << PINC7);
        PORTC |= (1 << PORTC6); /* CLK tick */
        _delay_us(2);
    }
}

void
phy_readWheelButs(void)
{
    /*
     * load buttons to registers 
     */
    PORTD &= ~(1 << PORTD1);    /* set parallel mode */
    _delay_us(2);               /* wait for stabile signals */
    PORTD |= (1 << PORTD1);     /* return to serial mode */

    /*
     * shift bits 
     */
    for (int i = 0; i < 8; i++)
    {
        PORTD &= ~(1 << PORTD0);        /* next CLK cycle */
        _delay_us(2);           /* minimal period */
        phy_wheelButs[i] = PINB & (1 << PINB7);
        PORTD |= (1 << PORTD0); /* CLK tick */
        _delay_us(2);
    }
}

static inline void
phy_ADC_selectPin(uint8_t pin)
{
    const uint8_t voltRef = 0x40;

    ADMUX = voltRef | pin;
}

static inline void
phy_ADC_start(void)
{
    ADCSRA |= (1 << ADSC);
}

static inline void
phy_ADC_wait(void)
{
    while (ADCSRA & (1 << ADSC));
}

static inline uint16_t
phy_ADC_readData(void)
{
    uint8_t low, high;

    low = ADCL;
    high = ADCH;
    return (high << 8) | low;
}

static inline uint16_t
phy_read_ADC(uint8_t pin)
{
    phy_ADC_selectPin(pin);
    phy_ADC_start();
    phy_ADC_wait();
    return phy_ADC_readData();
}

void
phy_readAll(void)
{
    phy_readShifterButs();
    phy_readWheelButs();

    const uint8_t pinADCAccel = 0;

    const uint8_t pinADCBrake = 1;

    const uint8_t pinADCClutch = 4;

    const uint8_t pinADCShiftX = 5;

    const uint8_t pinADCShiftY = 6;

    phy_pedalAccel = phy_read_ADC(pinADCAccel);
    phy_pedalBrake = phy_read_ADC(pinADCBrake);
    phy_pedalClutch = phy_read_ADC(pinADCClutch);

    phy_shifterX = phy_read_ADC(pinADCShiftX);
    phy_shifterY = phy_read_ADC(pinADCShiftY);
}

void
phy_offLED(void)
{
    PORTD |= (1 << 6);
}

void
phy_onLED(void)
{
    PORTD &= ~(1 << 6);
}

ISR(PCINT0_vect)
{
    int chi = PINB & (1 << 6);

    const uint16_t centr = 32800;
    const uint16_t i_intr = 1000;
    if (chi)
    {
        phy_offLED();
        if(phy_wheelPos < (centr - i_intr/2))
        {
            if(phy_wheelPos < (centr - i_intr - i_intr/2))
            {

            }
            else
            {
                phy_wheelPos = centr - i_intr;
            }
        }
        else if(phy_wheelPos > (centr + i_intr/2))
        {
            if(phy_wheelPos < (centr + i_intr + i_intr/2))
            {
                phy_wheelPos = centr + i_intr;
            }
        }
        else
        {
            phy_wheelPos = centr;
        }
    } else
    {
        phy_onLED();
        int chab = PINB & ((1<<4) | (1<<5));

        if ((chab == ((1<<4) | (1<<5))) || (chab == 0x00))
        {
            phy_wheelPos++;
        } else
        {
            phy_wheelPos--;
        }
    }
}
