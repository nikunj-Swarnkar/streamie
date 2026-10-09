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
static int lt_rest, rt_rest; 
static int lt_span = TRIGGER_MIN_SPAN, rt_span = TRIGGER_MIN_SPAN; 

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

static inline bool pressed(uint pin) { return !gpio_get(pin); }

static uint16_t read_mcp(uint8_t ch)
{
    uint8_t tx[3] = { 0x01, (uint8_t)(0x80 | (ch << 4)), 0x00 }; 
    uint8_t rx[3];
    gpio_put(PIN_CS, 0);
    spi_write_read_blocking(spi0, tx, rx, 3);
    gpio_put(PIN_CS, 1);
    return (uint16_t)((rx[1] & 0x03) << 8| rx[2]);

}

static int16_t shape_stick(int raw, int center, bool invert)
{
    int d=raw - center;
    if (abs(d) < STICK_DEADZONE) return 0;
    d += (d > 0) ? -STICK_DEADZONE : STICK_DEADZONE;
    long out = (long)d * 32767 / (512 - STICK_DEADZONE);
    if (out > 32767) out = 32767;
    if (out < -32768) out = -32768;
    return (int16_t)(invert ? -out : out);
}

static uint8_t shape_trigger(int raw, int rest, int *span)
{
    int ex = abs(raw - rest);
    if (ex < TRIGGER_NOISE) return 0;
    if (ex > *span) *span = ex;
    long out = (long)(ex - TRIGGER_NOISE) * 255 / (*span - TRIGGER_NOISE);
    return (uint8_t)(out > 255 ? 255 : out);
}

static void on_rumble(uint8_t left, uint8_t right)

{
    pwm_set_gpio_level(PIN_RUMBLE_L, left * 16);
    pwm_set_gpio_level(PIN_RUMBLE_R, right * 16);
}

static void rumble_init(void)
{
    gpio_set_function(PIN_RUMBLE_L, GPIO_FUNC_PWM);
    gpio_set_function(PIN_RUMBLE_R, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_RUMBLE_L);
    pwm_set_clkdiv(slice, 23.0f);
    pwm_set_wrap(slice, 255);
    pwm_set_gpio_level(PIN_RUMBLE_L, 0);
    pwm_set_gpio_level(PIN_RUMBLE_R, 0);
    pwm_set_enabled(slice, true);
}

static void calibrate(void)
{
    long lx = 0, ly = 0, rx = 0, ry = 0;
    const int N = 32;
    for (int i = 0; i < N; i++) {
        lx += read_mcp(CH_LX);
        ly += read_mcp(CH_LY);
        rx += read_mcp(CH_RX);
        ry += read_mcp(CH_RY);
        sleep_ms(10);
    }
    center_1x = lx / N;
    center_ly = ly / N;
    center_rx = rx / N;
    center_ry = ry / N;
    lt_rest = read_mcp(CH_LT);
    rt_rest = read_mcp(CH_RT);
}

int main(void)
{
    for (unsigned i = 0; i < sizeof(button_); i++) {
        gpio_init(button_[i]);
        gpio_set_dir(button_[i], GPIO_IN);
        gpio_pull_up(button_[i]);
    }

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    spi_init(spi0, 1000 * 1000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI); 

    rumble_init();
    xinput_set_rumble_cb(on_rumble);
    tusb_init();
    calibrate();

    uint32_t last = 0;
    while (true) {
        tud_task();
        uint32_t now = time_us_32();
        if (now - last < 1000) continue;
        last = now;

        xinput_report_t report = {0};
        report.msg_type = 0x00 ;
        report.size = 0x14;

        if(pressed(PIN_UP)) report.buttons1 |= XI_DPAD_UP;
        if(pressed(PIN_DOWN)) report.buttons1 |= XI_DPAD_DOWN;
        if(pressed(PIN_LEFT)) report.buttons1 |= XI_DPAD_LEFT;
        if(pressed(PIN_RIGHT)) report.buttons1 |= XI_DPAD_RIGHT;
        if(pressed(PIN_START)) report.buttons1 |= XI_START;
        if(pressed(PIN_SELECT)) report.buttons1 |= XI_BACK;
        if(pressed(PIN_L3)) report.buttons1 |= XI_L3;
        if(read_mcp(CH_R3) < R3_PRESSED_BELOW) report.buttons1 |= XI_R3;
        
        if(pressed(PIN_L1)) report.buttons2 |= XI_LB;
        if(pressed(PIN_R1)) report.buttons2 |= XI_RB;
        if(pressed(PIN_HOME)) report.buttons2 |= XI_GUIDE;
        if(pressed(PIN_A)) report.buttons2 |= XI_A;
        if(pressed(PIN_B)) report.buttons2 |= XI_B;
        if(pressed(PIN_X)) report.buttons2 |= XI_X;
        if(pressed(PIN_Y)) report.buttons2 |= XI_Y;

        report.lx = shape_stick(read_mcp(CH_LX), center_1x, false);
        report.ly = shape_stick(read_mcp(CH_LY), center_ly, INVERT_LY);
        report.rx = shape_stick(read_mcp(CH_RX), center_rx, false);
        report.ry = shape_stick(read_mcp(CH_RY), center_ry, INVERT_RY);
        report.lt = shape_trigger(read_mcp(CH_LT), lt_rest, &lt_span);
        report.rt = shape_trigger(read_mcp(CH_RT), rt_rest, &rt_span);
       
        xinput_send(&report);
    }
}
