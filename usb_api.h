#ifndef USBserial_h_
#define USBserial_h_

#include <inttypes.h>

extern uint8_t  joystick_report_data[33];
extern uint8_t  joystick_report_data_usb[33];

static inline void
joy_nullButtons(void)
{
    int             i;

    joystick_report_data[0] &= 0x0F;
    //Hat switch
        for (i = 1; i < 16; i++)
    {
        joystick_report_data[i] = 0x00;
    }
    for (i = 1; i < 33; i++)
    {
        joystick_report_data[i] = 0x00;
    }
}

static inline void
joy_button(uint8_t button, uint8_t val)
{
    button--;
    uint8_t         mask = (1 << (button & 7));
    uint8_t         bi;

    if (val)
    {
        if (button < 4)
        {
            mask = (1 << (button + 4));
            joystick_report_data[0] |= mask;
        } else
        {
            button -= 4;
            bi = button >> 3;
            //byte index
                mask = (1 << (button & 0x07));
            //bit index
                joystick_report_data[bi + 1] |= mask;
        }
    } else
    {
        if (button < 4)
        {
            mask = ~(1 << (button + 4));
            joystick_report_data[0] &= mask;
        } else
        {
            button -= 4;
            bi = button >> 3;
            //byte index
                mask = ~(1 << (button & 0x07));
            //bit index
                joystick_report_data[bi + 1] &= mask;
        }
    }
}

static inline void
joy_hat(int16_t dir)
{
    uint8_t         val= 15;

    if (dir < 0)
        val = 15;
    else if (dir < 23)
        val = 0;
    else if (dir < 68)
        val = 1;
    else if (dir < 113)
        val = 2;
    else if (dir < 158)
        val = 3;
    else if (dir < 203)
        val = 4;
    else if (dir < 245)
        val = 5;
    else if (dir < 293)
        val = 6;
    else if (dir < 338)
        val = 7;
    joystick_report_data[0] = (joystick_report_data[0] & 0xF0) | (val << 0);
}

static inline void
joy_axis(uint8_t axis, uint16_t val)
{
    int n = 16 + (axis<<1); /*offset in data report*/
    joystick_report_data[n] = ((val & 0x00FF) >> 0);    /*LSB*/
    joystick_report_data[n+1] = ((val & 0xFF00) >> 8);  /*MSB*/
}

#define joy_axisX  0
#define joy_axisY  1
#define joy_axisZ  2
#define joy_axisZr 3

extern void joy_send_now(void);

#endif
