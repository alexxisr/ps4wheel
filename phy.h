
/*
 * physical layer
 */

#ifndef PHY_H
#define PHY_H

#include <stdint.h>

void phy_initAll(void);

void phy_readAll(void);

void phy_onLED(void);

void phy_offLED(void);

/* Wheel position*/
extern uint16_t phy_wheelPos;

/* wheel buttons */
extern uint8_t phy_wheelButs[8];

/* Pedals */
extern uint16_t phy_pedalAccel;

extern uint16_t phy_pedalBrake;

extern uint16_t phy_pedalClutch;

/* Shifter */
extern uint16_t phy_shifterX;

extern uint16_t phy_shifterY;

extern uint8_t phy_shifterButs[16];

#endif
