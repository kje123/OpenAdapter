#include "GamecubeController.hpp"
#include "gamecube_definitions.h"

#include <hardware/pio.h>
#include <pico/stdlib.h>
#include "tusb.h"
#include "tusb_types.h"
#include "bsp/board.h"
#include "class/hid/hid_device.h"
#include "class/vendor/vendor_device.h"

// device descriptor
static const tusb_desc_device_t gc_descriptor_dev = {
    .bLength = 18,
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x057E,
    .idProduct = 0x0337,

    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

/**** GameCube Adapter HID Report Descriptor ****/
const uint8_t gc_hid_report_descriptor[] = {
    0x05, 0x05,        // Usage Page (Game Ctrls)
    0x09, 0x00,        // Usage (Undefined)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x11,        //   Report ID (17) (rumble)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x05,        //   Report Count (5)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x21,        //   Report ID (33) (inputs)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x25,        //   Report Count (37)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x12,        //   Report ID (18)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x22,        //   Report ID (34)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x19,        //   Report Count (25)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x13,        //   Report ID (19)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x23,        //   Report ID (35)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x14,        //   Report ID (20)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x24,        //   Report ID (36)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x15,        //   Report ID (21)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x25,        //   Report ID (37)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection

    // 214 bytes
};

char *string_desc_arr[] = {
    (char[]){0x09, 0x04},                // 0: is supported language is English (0x0409)
    "Keith Ellingwood", // 1: Manufacturer
    "OpenAdapter (name tbd)",      // 2: Product
    "2040",       // 3: Serials, should use chip ID
};

/**** GameCube Adapter Configuration Descriptor ****/
#define GC_CGCDES_LEN   9 + 9 + 9 + 7 + 7
static const uint8_t gc_hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, 41, TUSB_DESC_CONFIG_ATT_SELF_POWERED, 500),
    // Interface
    9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_HID, 0x00, 0x00, 0x00,
    // HID Descriptor
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(sizeof(gc_hid_report_descriptor)),
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x81, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(37), 1,
    // Endpoint Descriptor
    7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(6), 1,
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return gc_hid_configuration_descriptor;
}

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &gc_descriptor_dev;
}

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if ( index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }else
  {
    // Convert ASCII string into UTF-16

    if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

  return _desc_str;
}

GamecubeController *gcc;
gc_report_t gc_report = default_gc_report;
uint8_t buffer[37];

typedef struct {
    union {
        struct {
            uint8_t a : 1;
            uint8_t b : 1;
            uint8_t x : 1;
            uint8_t y : 1;
            uint8_t dl : 1;
            uint8_t dr : 1;
            uint8_t dd : 1;
            uint8_t du : 1;
        };
        uint8_t b1;
    };

    union
    {
        struct
        {
            uint8_t start: 1;
            uint8_t z    : 1;
            uint8_t r    : 1;
            uint8_t l    : 1;
            uint8_t blank1      : 4;
        };
        uint8_t b2;
    };

    uint8_t a_x;
    uint8_t a_y;
    uint8_t c_x;
    uint8_t c_y;
    uint8_t a_l;
    uint8_t a_r;

} __attribute__ ((packed)) gc_usb_report;

// needed to check if port is being used
// index 0 is p1, 1 is p2
bool isActive [2] = { 0, 0 };

void gc_reset_data(void) {
    memset(buffer, 0, 37);
    isActive[0] = false;
}

void send_data(void) {
    if(tud_ready()) {
        gc_usb_report curr;

        // set first bit to id
        buffer[0] = 0x21;

        // set unused ports to inactive bytecode
        buffer[10] = 0x04;
        buffer[19] = 0x04;
        buffer[28] = 0x04;

        // digital
        curr.a = gc_report.a;
        curr.b = gc_report.b;
        curr.x = gc_report.x;
        curr.y = gc_report.y;
        curr.z = gc_report.z;
        curr.l = gc_report.l;
        curr.r = gc_report.r;
        curr.start = gc_report.start;
        curr.dl = gc_report.dpad_left;
        curr.dr = gc_report.dpad_right;
        curr.dd = gc_report.dpad_down;
        curr.du = gc_report.dpad_up;

        // analog
        curr.a_x = gc_report.stick_x;
        curr.a_y = gc_report.stick_y;
        curr.c_x = gc_report.cstick_x;
        curr.c_y = gc_report.cstick_y;
        curr.a_l = gc_report.l_analog;
        curr.a_r = gc_report.r_analog;

        if(!isActive[0]) {
            buffer[1] = 0x14;
            isActive[0] = true;
        } else {
            // copy reports into buffer
            memcpy(&buffer[2], &curr, 8);
        }

        tud_hid_report(0, &buffer, 37);
    }
}

bool rumbleToggle = 0;

int main(void) {
    set_sys_clock_khz(130'000, true);

    uint joybus_pin = 1;

    gcc = new GamecubeController(joybus_pin, 1000, pio0);

    board_init();
    tusb_init();

    uint16_t usb_time = 0;

    while (true) {
        usb_time++;
        if(usb_time > 200) {
            usb_time = 0;
            gcc->Poll(&gc_report, rumbleToggle);
            gc_reset_data();
            tud_task();
            send_data();
        }
    }
}


//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    if (!report_id && !report_type) {
        if (buffer[0] == 0x11) {
            rumbleToggle = (bool)buffer[1];
        }
    }
}

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void) instance;
    return gc_hid_report_descriptor;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len) {
    if(report[0] == 0x21) { // if report id is 33, process inputs
        send_data();
    }
}

