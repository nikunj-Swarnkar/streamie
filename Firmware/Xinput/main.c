#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "tusb.h"
#include "usb_xinput.h"

// ---- pins ----
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
#define PIN_HOME 11
#define PIN_SELECT 28
#define PIN_L3 29

#define PIN_MISO 12
#define PIN_CS   13
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
#define CH_R3 6

// ---- tuning ----
#define STICK_DEADZONE   40      // 10-bit counts
#define INVERT_LY        false   // flip if up/down is backwards
#define INVERT_RY        false
#define TRIGGER_NOISE    12      // counts ignored near the rest position
#define TRIGGER_MIN_SPAN 100     // assumed travel until real travel is measured
#define R3_PRESSED_BELOW 300

static int center_lx = 512, center_ly = 512, center_rx = 512, center_ry = 512;
static int lt_rest, rt_rest;               // trigger rest readings (taken at boot)
static int lt_span = TRIGGER_MIN_SPAN, rt_span = TRIGGER_MIN_SPAN;

static const uint8_t button_pins[] = {
    PIN_A, PIN_B, PIN_X, PIN_Y, PIN_UP, PIN_DOWN, PIN_LEFT, PIN_RIGHT,
    PIN_L1, PIN_R1, PIN_START, PIN_HOME, PIN_SELECT, PIN_L3
};

static inline bool pressed(uint pin) { return !gpio_get(pin); }

static uint16_t read_mcp(uint8_t ch) {
    uint8_t tx[3] = { 0x01, (uint8_t)(0x80 | (ch << 4)), 0x00 };
    uint8_t rx[3];
    gpio_put(PIN_CS, 0);
    spi_write_read_blocking(spi1, tx, rx, 3);
    gpio_put(PIN_CS, 1);
    return (uint16_t)(((rx[1] & 0x03) << 8) | rx[2]);
}

static int16_t shape_stick(int raw, int center, bool invert) {
    int d = raw - center;
    if (abs(d) < STICK_DEADZONE) return 0;
    d += (d > 0) ? -STICK_DEADZONE : STICK_DEADZONE;
    long out = (long)d * 32767 / (512 - STICK_DEADZONE);
    if (out > 32767) out = 32767;
    if (out < -32767) out = -32767;
    return (int16_t)(invert ? -out : out);
}

// Direction-agnostic: whichever way the pot turns from its boot position counts as "pressed".
static uint8_t shape_trigger(int raw, int rest, int *span) {
    int ex = abs(raw - rest);
    if (ex > *span) *span = ex;            // learn full travel as you use it
    if (ex < TRIGGER_NOISE) return 0;
    long out = (long)(ex - TRIGGER_NOISE) * 255 / (*span - TRIGGER_NOISE);
    return (uint8_t)(out > 255 ? 255 : out);
}

static void on_rumble(uint8_t left, uint8_t right) {
    pwm_set_gpio_level(PIN_RUMBLE_L, left);
    pwm_set_gpio_level(PIN_RUMBLE_R, right);
}

static void rumble_init(void) {
    gpio_set_function(PIN_RUMBLE_L, GPIO_FUNC_PWM);
    gpio_set_function(PIN_RUMBLE_R, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_RUMBLE_L);   // GP26 & GP27 share slice 5
    pwm_set_clkdiv(slice, 24.0f);                        // ~20 kHz
    pwm_set_wrap(slice, 255);
    pwm_set_gpio_level(PIN_RUMBLE_L, 0);
    pwm_set_gpio_level(PIN_RUMBLE_R, 0);
    pwm_set_enabled(slice, true);
}

static void calibrate(void) {
    long lx = 0, ly = 0, rx = 0, ry = 0;
    const int N = 32;
    for (int i = 0; i < N; i++) {
        lx += read_mcp(CH_LX); ly += read_mcp(CH_LY);
        rx += read_mcp(CH_RX); ry += read_mcp(CH_RY);
        sleep_ms(2);
    }
    center_lx = lx / N; center_ly = ly / N;
    center_rx = rx / N; center_ry = ry / N;

    lt_rest = read_mcp(CH_LT);   // triggers must be released at power-up
    rt_rest = read_mcp(CH_RT);
}

int main(void) {
    for (unsigned i = 0; i < sizeof(button_pins); i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);
    }

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    spi_init(spi1, 1000 * 1000);          // MCP3008 @3.3V: stay <= 1.35 MHz
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    rumble_init();
    xinput_set_rumble_cb(on_rumble);

    tusb_init();
    calibrate();   // sticks should be untouched at power-up

    uint32_t last = 0;
    while (true) {
        tud_task();

        uint32_t now = time_us_32();
        if (now - last < 1000) continue;   // ~1 kHz
        last = now;

        xinput_report_t r = {0};
        r.msg_type = 0x00;
        r.size = 0x14;

        if (pressed(PIN_UP))     r.buttons1 |= XI_DPAD_UP;
        if (pressed(PIN_DOWN))   r.buttons1 |= XI_DPAD_DOWN;
        if (pressed(PIN_LEFT))   r.buttons1 |= XI_DPAD_LEFT;
        if (pressed(PIN_RIGHT))  r.buttons1 |= XI_DPAD_RIGHT;
        if (pressed(PIN_START))  r.buttons1 |= XI_START;
        if (pressed(PIN_SELECT)) r.buttons1 |= XI_BACK;
        if (pressed(PIN_L3))     r.buttons1 |= XI_L3;
        if (read_mcp(CH_R3) < R3_PRESSED_BELOW) r.buttons1 |= XI_R3;

        if (pressed(PIN_L1))     r.buttons2 |= XI_LB;
        if (pressed(PIN_R1))     r.buttons2 |= XI_RB;
        if (pressed(PIN_HOME))   r.buttons2 |= XI_GUIDE;
        if (pressed(PIN_A))      r.buttons2 |= XI_A;
        if (pressed(PIN_B))      r.buttons2 |= XI_B;
        if (pressed(PIN_X))      r.buttons2 |= XI_X;
        if (pressed(PIN_Y))      r.buttons2 |= XI_Y;

        r.lx = shape_stick(read_mcp(CH_LX), center_lx, false);
        r.ly = shape_stick(read_mcp(CH_LY), center_ly, INVERT_LY);
        r.rx = shape_stick(read_mcp(CH_RX), center_rx, false);
        r.ry = shape_stick(read_mcp(CH_RY), center_ry, INVERT_RY);
        r.lt = shape_trigger(read_mcp(CH_LT), lt_rest, &lt_span);
        r.rt = shape_trigger(read_mcp(CH_RT), rt_rest, &rt_span);

        xinput_send(&r);
    }
}
