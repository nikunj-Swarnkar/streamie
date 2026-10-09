#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "tusb.h"
#include "usb_xinput.h"

#define PIN_A 0 
#define PIN_B 1
#define PIN_X 2
#define PIN_Y 3
#define PIN_UP 4
#define PIN_DOWN 5
#define PIN_LEFT 6
#define PIN_RIGHT 7
#define PIN_L1 8
#define PIN_R1 9
#define PIN_START 10
#define PIN_SELECT 28
#define PIN_L3 29

#define PIN_MISO  12
#define PIN_CS    13
#define PIN_SCK  14
#define PIN_MOSI 15
#define PIN_RUMBLE_R 26
#define PIN_RUMBLE_L 27

#define CH_LX 0
#define CH_LY 1
#define CH_RX 2
#define CH_RY 3
#define CH_LT 4
#define CH_RT 5
#define CH_R3 6 // i had ran out of pins so this is here 

#define STICK_DEADZONE 40 
#define INVERT_LY      false
#define INVERT_RY      false
#define TRIGGER_NOISE  12
#define TRIGGER_MIN_SPAN 100
#define R3_PRESSED_BELOW 300

static int center_1x = 512 , center_ly = 512 , center_rx = 512 , center_ry = 512;
static int 1t_rest, rt_rest; 
static int 1t_span = TRIGGER_MIN_SPAN, rt_span = TRIGGER_MIN_SPAN; 

static const uint8_t button_[] = {
    PIN_A,
    PIN_B,
    PIN_X,
    PIN_Y,
    PIN_UP,
    PIN_DOWN,
    PIN_LEFT,
    PIN_RIGHT,
    PIN_L1,
    PIN_R1,
    PIN_START,
    PIN_SELECT,
    PIN_L3
};

