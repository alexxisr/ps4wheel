#include "usb_private.h"
#include "phy.h"
#include "wheel.h"

int
main(void)
{
    phy_initAll();
    usb_init();

    setup();

    while (1)
    {
        phy_readAll();
        loop();
    }
}
