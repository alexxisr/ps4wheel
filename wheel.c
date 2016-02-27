#include "wheel.h"

#include "usb_api.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>

#include "phy.h"

#define G_135 780
#define G_246 160
#define G_12  380
#define G_56  690

#define DI_REVERSE         1

int old_gear;

void
setup()
{
    _delay_ms(500);

    old_gear = 0;
}

inline int
normPedAcc(int pa)
{
    int r = pa + (pa >> 2) - 139;

    r = r > 1020 ? 1023 : r;
    r = r < 25 ? 0 : r;
    return r;
}

inline int
normPedBr(int pa)
{
    int r = pa + (pa >> 2) + (pa >> 3) - 257;

    r = r > 1000 ? 1023 : r;
    r = r < 20 ? 0 : r;
    return r;
}

inline int
normPedCl(int pa)
{
    int r = pa + (pa >> 2) + (pa >> 3) - 267;

    r = r > 1020 ? 1023 : r;
    r = r < 20 ? 0 : r;
    return r;
}

void
loop()
{
    uint16_t pc = normPedCl(phy_pedalClutch);

    uint16_t pb = normPedBr(phy_pedalBrake);

    uint16_t pa = normPedAcc(phy_pedalAccel);



    joy_nullButtons();

    int calCenter = 0;

    if (phy_wheelButs[7])
    {
        joy_button(6, 1);       //L1 gear down
        calCenter++;
    }
    if (phy_wheelButs[6])
    {
        joy_button(5, 1);       //R1 gear up
        calCenter++;
    }

    if (calCenter == 2)
    {
        phy_wheelPos = 0x8000;
    }

    if (phy_shifterButs[8])
    {
        joy_button(4, 1);
    }                           // triangle but
    if (phy_shifterButs[9])
    {
        joy_button(3, 1);
    }                           // round but
    if (phy_shifterButs[10])
    {
        joy_button(1, 1);
    }                           // quad but
    if (phy_shifterButs[11])
    {
        joy_button(2, 1);
    }                           // X but

    if (phy_shifterButs[7])
    {
        joy_button(9, 1);
    }                           //  
    if (phy_shifterButs[5])
    {
        joy_button(12, 1);
    }                           // L3
    if (phy_shifterButs[4])
    {
        joy_button(11, 1);
    }                           // R3
    if (phy_shifterButs[6])
    {
        joy_button(10, 1);
    }                           // options


    joy_hat(-1);
    if (phy_shifterButs[12])
    {
        joy_hat(90);
    }
    if (phy_shifterButs[13])
    {
        joy_hat(270);
    }
    if (phy_shifterButs[14])
    {
        joy_hat(180);
    }
    if (phy_shifterButs[15])
    {
        joy_hat(0);
    }

    for (int i = 0; i < 6; i++)
    {
        joy_button(30 + i, phy_wheelButs[i]);
    }

    int gear = 0;

    if (phy_shifterY < G_246)
    {
        if (phy_shifterX < G_12)
        {
            gear = 2;
        } else if (phy_shifterX > G_56)
        {
            gear = 6;
        } else
        {
            gear = 4;
        }
    } else if (phy_shifterY > G_135)
    {
        if (phy_shifterX < G_12)
        {
            gear = 1;
        } else if (phy_shifterX > G_56)
        {
            gear = 5;
        } else
        {
            gear = 3;
        }
    } else
    {
        gear = 0;
    }

    if (phy_shifterButs[DI_REVERSE])
    {
        if ((gear == 6) && ((old_gear == -1) || (old_gear == -2)))
        {
            gear = -2;          //reverse activated
        } else if (gear == 0)
        {
            gear = -1;          //reverse unblocked on neutral
        }
    }

    switch (gear)
    {
    case 1:
        joy_button(14, 1);
        break;
    case 2:
        joy_button(15, 1);
        break;
    case 3:
        joy_button(16, 1);
        break;
    case 4:
        joy_button(17, 1);
        break;
    case 5:
        joy_button(18, 1);
        break;
    case 6:
        joy_button(19, 1);
        break;
    case -1:
        joy_button(21, 1);
        break;
    case -2:
        joy_button(13, 1);
        break;
    }
    old_gear = gear;

//joy_button(17,1);
    joy_axis(joy_axisX, phy_wheelPos);
    joy_axis(joy_axisZr, pc << 6);      /*clutch */
    joy_axis(joy_axisY, pa << 6);       /*acc */
    joy_axis(joy_axisZ, pb << 6);       /*brake */


//    joy_send_now();
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        memcpy(joystick_report_data_usb, joystick_report_data, 33);
    }
}

