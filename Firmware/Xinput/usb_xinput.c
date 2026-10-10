#include "usb_xinput.h"

#define EP_IN_ADDR 0x81
#define EP_OUT_ADDR 0x02

static uint8_t ep_in = EP_IN_ADDR;
static uint8_t ep_out = EP_OUT_ADDR;
static uint8_t in_buf[32] __attribute__((aligned(4)));
static uint8_t out_buf[32] __attribute__((aligned(4)));
static xinput_rumble_cb_t rumble_cb = NULL;

void xinput_set_rumble_cb(xinput_rumble_cb_t cb)
{
    rumble_cb = cb;
}
static const tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0xFF,
    .bDeviceSubClass = 0xFF,
    .bDeviceProtocol = 0xFF,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x045E,
    .idProduct = 0x028E,
    .bcdDevice = 0x0114,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1
};

static const uint8_t desc_config[] = {
    0x09, 0x04, 0x00 , 0x00 , 0x02, 0xFF, 0x5D, 0x01, 0x00 ,
    0x10, 0x21, 0x10 , 0x01 , 0x01, 0x24, EP_IN_ADDR, 0x14 ,
    0x03, 0x00, 0x03 , 0x13 , EP_OUT_ADDR, 0x00 , 0x03, 0x00 ,
    0x07, 0x05, EP_IN_ADDR , 0x03 , 0x40, 0x00, 0x01,
    0x07, 0x05, EP_OUT_ADDR , 0x03 , 0x20, 0x00, 0x08 

};

static const char *const strings[] = { "", "Generic", "XInput Gamepad", "000001" };
static uint16_t str_buf[32];

uint8_t const *tud_descriptor_device_cb(void) { return (uint8_t const *)&desc_device; }
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) { (void)index; return desc_config; }

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    uint8_t n;
    if (index == 0) {
        str_buf[1] = 0x0409;
        n = 1;
    } else {
        if (index >= TU_ARRAY_SIZE(strings)) return NULL;
        const char *s = strings[index];
        n = (uint8_t)strlen(s);
        if (n > 31) n = 31;
        for (uint8_t i = 0; i < n; i++) str_buf[1 + i] = s[i];
    }
    str_buf[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * n + 2));
    return str_buf;
}


static void xi_init(void) {}
static void xi_reset(uint8_t rhport){(void)rhport;}

static uint16_t xi_open(uint8_t rhport, tusb_desc_interface_t const *itf , uint16_t max_len)
{
  TU_VERIFY(itf->bInterfaceClass == 0xFF && 
            itf->bInterfaceSubClass == 0x5D && 
            itf->bInterfaceProtocol == 0x01 ,0);
    const uint16_t drv_len = 9 + 16 + 7 + 7;
    TU_VERIFY(max_len >= drv_len, 0);
    
    uint8_t const *p = tu_desc_next(itf);
    p = tu_desc_next(p); 
    TU_ASSERT(usbd_open_edpt_pair(rhport, p, 2, TUSB_XFER_INTERRUPT, &ep_in, &ep_out), 0);
    
    usbd_edpt_xfer(rhport, ep_out, out_buf, sizeof(out_buf));
    return drv_len;
}

static bool xi_control_xfer_cd(uint8_t rhport, uint8_t stage , tusb_control_request_t const *request)
{
  (void)rhport;
  (void)stage;
  (void)request;
  return false;
}

static bool xi_xfer_cd(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t len)
{
    if (ep_addr == ep_out && result == XFER_RESULT_SUCCESS)
    {
        if (len >= 5 && out_buf[0] == 0x00 && out_buf[1] == 0x08 && rumble_cd)
        {
            rumble_cd(out_buf[3], out_buf[4]);
        }
        usbd_edpt_xfer(rhport, ep_out, out_buf, sizeof(out_buf));

    }
    return true;

}

static const usbd_class_driver_t xinput_driver = { 
    #if CFG_TUSB_DEBUG >= 2
    .name = "XInput",
    #endif
    .init = xi_init,
    .reset = xi_reset,
    .open = xi_open,
    .control_xfer_cb = xi_control_xfer_cd,
    .xfer_cb = xi_xfer_cd
    .sof = NULL
};

usbd_class_driver_t const *usbd_class_driver_get_cd(uint8_t *driver_count)
{
    *driver_count = 1;
    return &xinput_driver;
}

bool xinput_send(const xinput_report_t *report)
{
    if (!tud_ready()|| usbd_edpt_busy(0, ep_in)) return false;
    if (!usbd_edpt_claim(0, ep_in)) return false;
    memcpy(in_buf, report, sizeof(*report));
    return usbd_edpt_xfer(0, ep_in, in_buf, sizeof(*report));
}
