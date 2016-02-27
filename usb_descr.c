/*
 *
 */

#include "usb_descr.h"

#include "usb_private.h"

#include <avr/io.h>
#include <string.h>

static const int16_t langIDs_string[] = {0x0409}; /* english */
static const struct usb_descriptor_string usb_descr_langIDs =
{
    4,  /* length */
    3,  /* type */
    langIDs_string
};

static const int16_t str_product[] = L"ClubSport Wheel";
static const struct usb_descriptor_string usb_descr_str_product =
{
    sizeof(str_product),    /* size of string - 2(null term) + 2(descr header) */
    3,                      /* descr type */
    str_product
};

static const int16_t str_manuf[] = L"Fanatec";
static const struct usb_descriptor_string usb_descr_str_manuf =
{
    sizeof(str_manuf),  /* size of string - 2(null term) + 2(descr header) */
    3,      /* descr type */
    str_manuf
};

const struct usb_descriptor_string *usb_descr_strings[] = {
    &usb_descr_langIDs,
    &usb_descr_str_product,
    &usb_descr_str_manuf
};

const int usb_descr_str_max = (sizeof(usb_descr_strings)/sizeof(struct usb_descriptor_string *)) - 1;

static inline uint8_t
getMinOf3(const uint16_t a, const uint8_t b, const uint8_t c)
{
    uint8_t r = c;
    r = (r < b) ? r : b;
    r = (r < a) ? r : a;
    return r;
}

static void
usb_descr_ctl_send_buf(uint8_t *b, uint16_t len)
{
    uint8_t i;
    uint8_t n;

    do {
        //wait for host ready for IN packet
        do {
            i = UEINTX;
        } while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
        if (i & (1<<RXOUTI)) return;    // abort
        // send IN packet
        n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
        for (i = n; i; i--) {
            UEDATX = *b;
            b++;
        }
        len -= n;
        UEINTX = ~(1<<TXINI);
    } while (len || n == ENDPOINT0_SIZE);
    return;
}

void
usb_descr_string_send(int pos, uint16_t wLength)
{
    const struct usb_descriptor_string *dp = usb_descr_strings[pos];
    uint8_t len = (*dp).bLength;
    uint8_t buf[64];

    buf[0] = len;
    buf[1] = (*dp).bDescriptorType;
    memcpy(&buf[2], (uint8_t *)(*dp).wString, len-2);
    usb_descr_ctl_send_buf(buf, len);
    return;
}
