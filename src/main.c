#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>

#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>

#include <timing/timing.h>

#include <math.h>

#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(main);

#define LED0_NODE DT_ALIAS(led0)

#if !DT_NODE_EXISTS(LED0_NODE)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {});

static const uint8_t hid_report_desc[] = HID_MOUSE_REPORT_DESC(2);

// static const uint8_t hid_gamepad_report_desc[] =
// {
//     HID_COLLECTION_PHYSICAL()
// };


static const uint8_t gamepad_report_desc[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //   Usage (0x20)
    0x75, 0x06,        //   Report Size (6)
    0x95, 0x01,        //   Report Count (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x21,        //   Usage (0x21)
    0x95, 0x36,        //   Report Count (54)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x05,        //   Report ID (5)
    0x09, 0x22,        //   Usage (0x22)
    0x95, 0x1F,        //   Report Count (31)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x04,        //   Report ID (4)
    0x09, 0x23,        //   Usage (0x23)
    0x95, 0x24,        //   Report Count (36)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x02,        //   Report ID (2)
    0x09, 0x24,        //   Usage (0x24)
    0x95, 0x24,        //   Report Count (36)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x08,        //   Report ID (8)
    0x09, 0x25,        //   Usage (0x25)
    0x95, 0x03,        //   Report Count (3)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x10,        //   Report ID (16)
    0x09, 0x26,        //   Usage (0x26)
    0x95, 0x04,        //   Report Count (4)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x11,        //   Report ID (17)
    0x09, 0x27,        //   Usage (0x27)
    0x95, 0x02,        //   Report Count (2)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x12,        //   Report ID (18)
    0x06, 0x02, 0xFF,  //   Usage Page (Vendor Defined 0xFF02)
    0x09, 0x21,        //   Usage (0x21)
    0x95, 0x0F,        //   Report Count (15)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x13,        //   Report ID (19)
    0x09, 0x22,        //   Usage (0x22)
    0x95, 0x16,        //   Report Count (22)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x14,        //   Report ID (20)
    0x06, 0x05, 0xFF,  //   Usage Page (Vendor Defined 0xFF05)
    0x09, 0x20,        //   Usage (0x20)
    0x95, 0x10,        //   Report Count (16)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x15,        //   Report ID (21)
    0x09, 0x21,        //   Usage (0x21)
    0x95, 0x2C,        //   Report Count (44)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x06, 0x80, 0xFF,  //   Usage Page (Vendor Defined 0xFF80)
    0x85, 0x80,        //   Report ID (-128)
    0x09, 0x20,        //   Usage (0x20)
    0x95, 0x06,        //   Report Count (6)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x81,        //   Report ID (-127)
    0x09, 0x21,        //   Usage (0x21)
    0x95, 0x06,        //   Report Count (6)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x82,        //   Report ID (-126)
    0x09, 0x22,        //   Usage (0x22)
    0x95, 0x05,        //   Report Count (5)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x83,        //   Report ID (-125)
    0x09, 0x23,        //   Usage (0x23)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x84,        //   Report ID (-124)
    0x09, 0x24,        //   Usage (0x24)
    0x95, 0x04,        //   Report Count (4)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x85,        //   Report ID (-123)
    0x09, 0x25,        //   Usage (0x25)
    0x95, 0x06,        //   Report Count (6)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x86,        //   Report ID (-122)
    0x09, 0x26,        //   Usage (0x26)
    0x95, 0x06,        //   Report Count (6)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x87,        //   Report ID (-121)
    0x09, 0x27,        //   Usage (0x27)
    0x95, 0x23,        //   Report Count (35)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x88,        //   Report ID (-120)
    0x09, 0x28,        //   Usage (0x28)
    0x95, 0x22,        //   Report Count (34)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x89,        //   Report ID (-119)
    0x09, 0x29,        //   Usage (0x29)
    0x95, 0x02,        //   Report Count (2)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x90,        //   Report ID (-112)
    0x09, 0x30,        //   Usage (0x30)
    0x95, 0x05,        //   Report Count (5)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x91,        //   Report ID (-111)
    0x09, 0x31,        //   Usage (0x31)
    0x95, 0x03,        //   Report Count (3)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x92,        //   Report ID (-110)
    0x09, 0x32,        //   Usage (0x32)
    0x95, 0x03,        //   Report Count (3)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x93,        //   Report ID (-109)
    0x09, 0x33,        //   Usage (0x33)
    0x95, 0x0C,        //   Report Count (12)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA0,        //   Report ID (-96)
    0x09, 0x40,        //   Usage (0x40)
    0x95, 0x06,        //   Report Count (6)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA1,        //   Report ID (-95)
    0x09, 0x41,        //   Usage (0x41)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA2,        //   Report ID (-94)
    0x09, 0x42,        //   Usage (0x42)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA3,        //   Report ID (-93)
    0x09, 0x43,        //   Usage (0x43)
    0x95, 0x30,        //   Report Count (48)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA4,        //   Report ID (-92)
    0x09, 0x44,        //   Usage (0x44)
    0x95, 0x0D,        //   Report Count (13)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA5,        //   Report ID (-91)
    0x09, 0x45,        //   Usage (0x45)
    0x95, 0x15,        //   Report Count (21)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA6,        //   Report ID (-90)
    0x09, 0x46,        //   Usage (0x46)
    0x95, 0x15,        //   Report Count (21)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF0,        //   Report ID (-16)
    0x09, 0x47,        //   Usage (0x47)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF1,        //   Report ID (-15)
    0x09, 0x48,        //   Usage (0x48)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF2,        //   Report ID (-14)
    0x09, 0x49,        //   Usage (0x49)
    0x95, 0x0F,        //   Report Count (15)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA7,        //   Report ID (-89)
    0x09, 0x4A,        //   Usage (0x4A)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA8,        //   Report ID (-88)
    0x09, 0x4B,        //   Usage (0x4B)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA9,        //   Report ID (-87)
    0x09, 0x4C,        //   Usage (0x4C)
    0x95, 0x08,        //   Report Count (8)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAA,        //   Report ID (-86)
    0x09, 0x4E,        //   Usage (0x4E)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAB,        //   Report ID (-85)
    0x09, 0x4F,        //   Usage (0x4F)
    0x95, 0x39,        //   Report Count (57)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAC,        //   Report ID (-84)
    0x09, 0x50,        //   Usage (0x50)
    0x95, 0x39,        //   Report Count (57)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAD,        //   Report ID (-83)
    0x09, 0x51,        //   Usage (0x51)
    0x95, 0x0B,        //   Report Count (11)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAE,        //   Report ID (-82)
    0x09, 0x52,        //   Usage (0x52)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xAF,        //   Report ID (-81)
    0x09, 0x53,        //   Usage (0x53)
    0x95, 0x02,        //   Report Count (2)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xB0,        //   Report ID (-80)
    0x09, 0x54,        //   Usage (0x54)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};

#define GAMEPAD_REPORT_LEN 64

static enum usb_dc_status_code usb_status;

void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
    usb_status = status;
}

void fill_report(uint8_t report[GAMEPAD_REPORT_LEN], uint8_t rhorz, uint8_t rvert)
{
    report[0] = 1; // report id
    report[1] = 128; // lhorz
    report[2] = 128; // lvert
    report[3] = rhorz; // rhorz
    report[4] = rvert; // rvert
    uint8_t dpad = 0xf;
    uint8_t buttons1 = 0; // x, a, b, y
    report[5] = dpad | (buttons1 << 4); // dpad
    uint8_t buttons2 = 0; // LB, RB, LT, RT, Select, Start, LSB, RSB
    report[6] = buttons2;
    uint8_t buttons3 = 0; // PS4, touchpad, 6 bit padding
    report[7] = buttons3;
    report[8] = 0; // lt
    report[9] = 0; // rt
}

const struct device* hid_dev;
struct k_timer gamepad_timer;

void send_gamepad_report()
{
    int ret;
    static int index = 0;
    const float ANGLE_INCREMENT = 3.1415926535f / 128.0f;
    float angle = index * ANGLE_INCREMENT;
    ++index;
    if (index == 256)
    {
        index = 0;
    }
    uint8_t x = (cosf(angle) + 1.0f) * 127.5f + 0.5f;
    uint8_t y = (sinf(angle) + 1.0f) * 127.5f + 0.5f;

    uint8_t gamepadReport[GAMEPAD_REPORT_LEN] = {};
    fill_report(gamepadReport, x, y);

    angle += ANGLE_INCREMENT;

    ret = hid_int_ep_write(hid_dev, gamepadReport, sizeof(gamepadReport), NULL);
    if (ret) {
        LOG_ERR("HID write error, %d", ret);
    }

    ret = gpio_pin_toggle(led0.port, led0.pin);
    if (ret < 0) {
        LOG_ERR("Failed to toggle the LED pin, error: %d", ret);
    }
}

K_WORK_DEFINE(gamepad_work, send_gamepad_report);

static void gamepad_timer_cb(struct k_timer* timer_id)
{
    k_work_submit(&gamepad_work);
}

static void write_i16le(uint8_t* data, int16_t val)
{
    uint16_t uval = (uint16_t)val;
    data[0] = (uval & 0xFF);
    data[1] = uval >> 8;
}

static int handle_get_report(
    const struct device *dev,
    struct usb_setup_packet *setup,
    int32_t *len,
    uint8_t **data)
{
    static uint8_t MAC_ADDRESS[] = { 0x06, 0x96, 0xAA, 0xE3, 0x88, 0x9E };

    LOG_INF("get report: %u %u %u", (setup->wValue >> 8), (setup->wValue & 0xFF), setup->wLength);
    if ((setup->wValue >> 8) == 0x03 && (setup->wValue & 0xFF) == 0x81 && setup->wLength > 0)
    {
        uint8_t* output = *data;
        output[0] = 0x81;
        memcpy(output + 1, MAC_ADDRESS, 6);
        *len = 7;
        LOG_INF("get report 0x81");
        return 0;
    }
    else if ((setup->wValue >> 8) == 0x03 && (setup->wValue & 0xFF) == 0x12 && setup->wLength >= 7)
    {
        uint8_t* output = *data;
        output[0] = 0x12;
        memcpy(output + 1, MAC_ADDRESS, 6);
        memset(output + 7, 0, 9);
        *len = 16;
        LOG_INF("get report 0x12");
        return 0;
    }
    else if ((setup->wValue >> 8) == 0x03 && (setup->wValue & 0xFF) == 0x02 && setup->wLength >= 37)
    {
        uint8_t* output = *data;
        memset(output, 0, 37);
        output[0] = 0x02;
        write_i16le(output + 1, 1);
        write_i16le(output + 3, 1);
        write_i16le(output + 5, 1);
        write_i16le(output + 7, 1);
        write_i16le(output + 9, -1);
        write_i16le(output + 11, 1);
        write_i16le(output + 13, -1);
        write_i16le(output + 15, 1);
        write_i16le(output + 17, -1);
        write_i16le(output + 19, 1);
        write_i16le(output + 21, -1);
        write_i16le(output + 23, 1);
        write_i16le(output + 25, -1);
        write_i16le(output + 27, 1);
        write_i16le(output + 29, -1);
        write_i16le(output + 31, 1);
        write_i16le(output + 33, -1);
        *len = 37;
        LOG_INF("get report 0x02");
        return 0;
    }
    else if ((setup->wValue >> 8) == 0x03 && (setup->wValue & 0xFF) == 0xA3 && setup->wLength >= 49)
    {
        uint8_t* output = *data;
        memset(output, 0, 49);
        output[0] = 0xA3;
        output[35] = 0x70;
        output[36] = 0x01;
        output[41] = 0x70;
        output[42] = 0x01;
        *len = 49;
        LOG_INF("get report 0x02");
        return 0;
    }
    return -1;
}

void setup_logging(void)
{
    const struct device *console_dev = device_get_binding(
            CONFIG_UART_CONSOLE_ON_DEV_NAME);
    uint32_t dtr = 0;

    if (usb_enable(NULL)) {
        return;
    }

    while (!dtr) {
        uart_line_ctrl_get(console_dev, UART_LINE_CTRL_DTR, &dtr);
    }

    if (strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME) !=
        strlen("CDC_ACM_0") ||
        strncmp(CONFIG_UART_CONSOLE_ON_DEV_NAME, "CDC_ACM_0",
            strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME))) {
        printk("Error: Console device name is not USB ACM\n");
    }
}

void main(void)
{
    int ret;

    if (!device_is_ready(led0.port)) {
        LOG_ERR("LED device %s is not ready", led0.port->name);
        return;
    }

    hid_dev = device_get_binding("HID_0");
    if (hid_dev == NULL) {
        LOG_ERR("Cannot get USB HID Device");
        return;
    }

    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure the LED pin, error: %d", ret);
        return;
    }

    struct hid_ops ops = {};
    ops.get_report = handle_get_report;

    usb_hid_register_device(
        hid_dev,
        gamepad_report_desc,
        sizeof(gamepad_report_desc),
        &ops);

    usb_hid_init(hid_dev);

    // setup_logging();

    ret = usb_enable(status_cb);
    if (ret != 0) {
        LOG_ERR("Failed to enable USB");
        return;
    }

    k_timer_init(&gamepad_timer, gamepad_timer_cb, NULL);
    k_timer_start(&gamepad_timer, K_MSEC(4000), K_MSEC(4));

    while (true)
    {
        k_sleep(K_FOREVER);
    }
}
