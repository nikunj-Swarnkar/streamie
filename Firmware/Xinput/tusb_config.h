#pragma once
#ifndef CFG_TUSB_OS 
#define CFG_TUSB_OS  OPT_OS_NONE
#endif
#define CFG_TUD_ENABLED           1
#define CFG_TUSB_RHPORT0_MODE     (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUD_ENDPOINT0_SIZE    64

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif
#define CFG_TUSB_MEM_ALIGN        __attribute__((aligned(4)))

#define CFG_TUD_CDC               0
#define CFG_TUD_MSC               0
#define CFG_TUD_HID               0
#define CFG_TUD_MIDI              0
#define CFG_TUD_VENDOR            0
