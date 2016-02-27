/*
 *
 */

#ifndef USB_DESCR_H
#define USB_DESCR_H

#include <stdint.h>

struct usb_descriptor_string {
    uint8_t bLength;    /* length of descriptor in bytes */
    uint8_t bDescriptorType;    /* Allways 0x03 */
    const int16_t *wString;          /* pointer to UTF-16 string */
};

void
usb_descr_string_send(int pos, uint16_t wLength);

extern const int usb_descr_str_max;
#endif /* USB_DESCR_H */
