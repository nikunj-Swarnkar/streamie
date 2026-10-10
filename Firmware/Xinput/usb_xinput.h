#pragma once
#include <stdint.h>
#include <stdbool.h>

// Button bits, byte 2
#define XI_DPAD_UP    0x01
#define XI_DPAD_DOWN  0x02
#define XI_DPAD_LEFT  0x04
#define XI_DPAD_RIGHT 0x08
#define XI_START      0x10
#define XI_BACK       0x20
#define XI_L3         0x40
#define XI_R3         0x80
// Button bits, byte 3
#define XI_LB         0x01
#define XI_RB         0x02
#define XI_GUIDE      0x04
#define XI_A          0x10
#define XI_B          0x20
#define XI_X          0x40
#define XI_Y          0x80

typedef struct __attribute__((packed)) {
    uint8_t  msg_type;     // always 0x00
    uint8_t  size;         // always 0x14 (20)
    uint8_t  buttons1;
    uint8_t  buttons2;
    uint8_t  lt, rt;       // 0..255
    int16_t  lx, ly, rx, ry;
    uint8_t  reserved[6];
} xinput_report_t;

_Static_assert(sizeof(xinput_report_t) == 20, "XInput report must be 20 bytes");

typedef void (*xinput_rumble_cb_t)(uint8_t left, uint8_t right);

void xinput_set_rumble_cb(xinput_rumble_cb_t cb);
bool xinput_send(const xinput_report_t *report);   // false if USB busy / not ready
