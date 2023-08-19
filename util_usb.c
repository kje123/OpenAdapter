#include "util_usb.h"

usb_mode_t adapter_mode         = USB_MODE_GC;
uint16_t usb_timeout_time       = 0;

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

tusb_desc_strarray_device_t global_string_descriptor = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},                // 0: is supported language is English (0x0409)
    "Keith Ellingwood", // 1: Manufacturer
    "OpenAdapter (name tbd)",      // 2: Product
    CONFIG_TINYUSB_DESC_SERIAL_STRING,       // 3: Serials, should use chip ID
};

/** GAMECUBE HID MODE **/
// 1. Device Descriptor
// 2. HID Report Descriptor
// 3. Configuration Descriptor
// 4. TinyUSB Config
/**--------------------------**/

/**** GameCube Adapter Device Descriptor ****/
static const tusb_desc_device_t gc_descriptor_dev = {
    .bLength = 18,
    .bDescriptorType = 0x01,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = 64,
    .idVendor = 0x057E,
    .idProduct = 0x0337,

    .bcdDevice = CONFIG_TINYUSB_DESC_BCD_DEVICE,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

/**** GameCube Adapter HID Report Descriptor ****/
const uint8_t gc_hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x11,        //     Report ID (17)
    0x19, 0x00,        //     Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //     Usage Maximum (0xFF)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x05,        //     Report Count (5)
    0x91, 0x00,        //     Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x21,        //     Report ID (33)
    0x05, 0x00,        //     Usage Page (Undefined)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0xFF,        //     Logical Maximum (-1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x08,        //     Usage Maximum (0x08)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x32,        //     Usage (Z)
    0x09, 0x33,        //     Usage (Rx)
    0x09, 0x34,        //     Usage (Ry)
    0x09, 0x35,        //     Usage (Rz)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x06,        //     Report Count (6)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x13,        //     Report ID (19)
    0x19, 0x00,        //     Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //     Usage Maximum (0xFF)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x00,        //     Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
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

/**** GameCube Adapter TinyUSB Config ****/
static const tinyusb_config_t gc_cfg = {
    .device_descriptor          = &gc_descriptor_dev,
    .string_descriptor          = global_string_descriptor,
    .external_phy               = false,
    .configuration_descriptor   = gc_hid_configuration_descriptor,
};

/**--------------------------**/
/**--------------------------**/


/********* TinyUSB HID callbacks ***************/
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) reqlen;

  return 0;
}


void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
    if (report[0] == 0x21){
        usb_process_data();
    }
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    if (!report_id && !report_type)
    {
        if (buffer[0] == 0x11)
        {
            rx_vibrate = (buffer[1] > 0) ? true : false;
        }
        else if (buffer[0] == 0x13)
        {
            ESP_LOGI("INIT", "Rx");
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

uint8_t scale_axis(int input)
{
    int res = input;

    if (input > 255)
    {
        input = 255;
    }
    if (input < 0)
    {
        input = 0;
    }

    if (input > 127)
    {
        float tmp = (float) input - 127;
        tmp = tmp * analog_scaler_f;
        res = (int) tmp + 127;
    }
    else if (input < 127)
    {
        float tmp = 127 - (float) input;
        tmp = tmp * analog_scaler_f;
        res = 127 - (int) tmp;    
    }
    else
    {
        res = 127;
    }
    
    if (res > 255)
    {
        res = 255;
    }
    if (res < 0)
    {
        res = 0;
    }
    return (uint8_t) res;
}

#define SIGNED_SCALER (float) 
short sign_axis(int input)
{
    uint8_t scaled = scale_axis(input);

    int start = (int) scaled - 127;
    if ((start * 256) > 32765)
    {
        start = 32765;
    }
    else if ((start * 256) < -32765)
    {
        start = -32765;
    }
    else
    {
        start *= 256;
    }
    return (short) start;
}

uint8_t scale_trigger(int input)
{
    if (input < 0)
    {
        return (uint8_t) 0;
    }
    else if ( input <= 255)
    {
        return (uint8_t) input;
    }
    else
    {
        return (uint8_t) 255;
    }
}

int gc_origin_adjust(uint8_t value, int origin, bool invert)
{
    int out = 0;

    if(invert)
    {
        out = 255 - ((int) value - origin);
    }
    else
    {
        out = (int) value - origin;
    }

    if (out < 0)
    {
        out = 0;
    }
    else if (out > 255)
    {
        out = 255;
    }

    return out;
    
}

void gcusb_start(usb_mode_t mode)
{

    while(!tud_mounted())
    {
        vTaskDelay(8/portTICK_PERIOD_MS);
    }
    while (!tud_hid_ready())
    {
        vTaskDelay(8/portTICK_PERIOD_MS);
    }
    }
    vTaskDelay(250/portTICK_PERIOD_MS);
    usb_send_data();
}

int adj_x;
int adj_y;
int adj_cx;
int adj_cy;
int adj_tl;
int adj_tr;

bool gc_first = false;
void gc_send_data(void)
{
    gc_buffer[0] = 0x21;

    if (cmd_phase != CMD_PHASE_POLL)
    {
        gc_input.buttons_1 = 0x00;
        gc_input.buttons_2 = 0x00;
        gc_input.stick_x    = 127;
        gc_input.stick_y    = 127;
        gc_input.cstick_x   = 127;
        gc_input.cstick_y   = 127;
        gc_input.trigger_l = 0;
        gc_input.trigger_r = 0;
    }
    else
    {

        // Generate the USB Data for GameCube native mode
        gc_input.button_a   = gc_poll_response.button_a;
        gc_input.button_b   = gc_poll_response.button_b;

        gc_input.dpad_down  = gc_poll_response.dpad_down;
        gc_input.dpad_up    = gc_poll_response.dpad_up;
        gc_input.dpad_left  = gc_poll_response.dpad_left;
        gc_input.dpad_right = gc_poll_response.dpad_right;

        gc_input.button_start   = gc_poll_response.button_start;

        // Defaults
        gc_input.button_y       = gc_poll_response.button_y;
        gc_input.button_x       = gc_poll_response.button_x;
        gc_input.button_z       = gc_poll_response.button_z;

        if (adapter_settings.gc_zjump == 1)
        {
            gc_input.button_x       = gc_poll_response.button_z;
            gc_input.button_z       = gc_poll_response.button_x;
        }
        else if (adapter_settings.gc_zjump == 2)
        {
            gc_input.button_y       = gc_poll_response.button_z;
            gc_input.button_z       = gc_poll_response.button_y;
        }

        adj_tl  = gc_origin_adjust(gc_poll_response.trigger_l, gc_origin_data.trigger_l,  false);
        adj_tr  = gc_origin_adjust(gc_poll_response.trigger_r, gc_origin_data.trigger_r,  false);


        gc_input.trigger_l          = scale_trigger(adj_tl);
        gc_input.button_l           = gc_poll_response.button_l;

        gc_input.trigger_r      = scale_trigger(adj_tr);
        gc_input.button_r       = gc_poll_response.button_r;

        adj_x   = gc_origin_adjust(gc_poll_response.stick_x,  gc_origin_data.stick_x,     false);
        adj_y   = gc_origin_adjust(gc_poll_response.stick_y,  gc_origin_data.stick_y,     false);
        adj_cx  = gc_origin_adjust(gc_poll_response.cstick_x, gc_origin_data.cstick_x,    false);
        adj_cy  = gc_origin_adjust(gc_poll_response.cstick_y, gc_origin_data.cstick_y,    false);

        gc_input.stick_x        = (uint8_t) adj_x;
        gc_input.stick_y        = (uint8_t) adj_y;
        gc_input.cstick_x       = (uint8_t) adj_cx;
        gc_input.cstick_y       = (uint8_t) adj_cy;
    }
    

    if (!gc_first)
    {
        /*GC adapter notes for new data
        
        with only black USB plugged in
        - no controller, byte 1 is 0
        - controller plugged in to port 1, byte 1 is 0x10
        - controller plugged in port 2, byte 10 is 0x10

        with both USB plugged in
        - no controller, byte 1 is 0x04
        - controller plugged in to port 1, byte is 0x14 */
        gc_buffer[1] = 0x14;
        gc_buffer[10] = 0x04;
        gc_buffer[19] = 0x04;
        gc_buffer[28] = 0x04;
        gc_first = true;
    }
    else
    {
        memcpy(&gc_buffer[2], &gc_input, 8);
    }

    tud_hid_report(0, &gc_buffer, GC_HID_LEN);
    
}

void usb_send_data(void)
{
    if (!tud_ready())
    {
        return;
    }
    gc_send_data();
}

// Some definitions for USB Timing
#define TIME_USB_US 22
#define TIME_GC_POLL 410/2
#define TIMEOUT_GC_US 500
#define TIMEOUT_COUNTS 10

#define TIME_ENDCAP_MAX 550

// The philosophy behind dynamic HID polling alignment
/* 
You have T1, which is the timestamp on which the RMT tx is started
You have T2, which is the time it took for the USB packet to send

We want to calculate the exact center of the minimum polling cycle
in a given scenario.
*/

// This is our time counter that we can use
// for calculations
uint64_t usb_delay_time = 0;

// This is the calculated delay we add
// We only add this when we enter POLLING
uint64_t usb_time_offset = 50;

uint64_t rmt_poll_time = 0;

void rmt_reset()
{
    JB_RX_MEMOWNER  = 1;
    JB_RX_RDRST     = 1;
    JB_RX_RDRST     = 0;
    JB_RX_CLEARISR  = 1;
    JB_RX_BEGIN     = 0;
    JB_RX_SYNC      = 1;
    JB_RX_SYNC      = 0;
    JB_RX_BEGIN     = 1;
    JB_TX_RDRST     = 1;
    JB_TX_WRRST     = 1;
    JB_TX_CLEARISR  = 1;
}

// This is called after each successful USB report send.
void usb_process_data(void)
{
    usb_timeout_time = 0;
    // Check if we have config data to send out
    if(cmd_flagged)
    {
        gc_timer_stop();
        gc_timer_reset();
        command_queue_process();
        rmt_reset();
        return;
    }
    
    if (cmd_phase == CMD_PHASE_POLL)
    {
        if (active_gc_type == GC_TYPE_WIRED)
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = (rx_vibrate == true) ? JB_HIGH : JB_LOW;
        }
        else if (active_gc_type == GC_TYPE_WAVEBIRD)
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = JB_LOW;
        }

        if (gc_timer_status == GC_TIMER_IDLE)
        {
            gc_timer_start();
        }
        else if (gc_timer_status == GC_TIMER_STARTED)
        {
            gptimer_get_raw_count(gc_timer, &usb_delay_time);
            gc_timer_reset();

            // Calculate new time delay that we use during polling for
            // perfectly centered polls (Only valid for above 2ms refresh)
            if (usb_delay_time >= 2500)
            {
                usb_time_offset = (usb_delay_time/2) - TIME_GC_POLL - TIME_USB_US;
            }
            // Otherwise we know we're polling at 1ms and where we need to place the poll
            else
            {   
                usb_time_offset = 500-TIME_GC_POLL;
            }

            ets_delay_us(usb_time_offset);
        }
    }
    else
    {
        ets_delay_us(50);
    }
    
    // Start RMT transaction
    // Set mem owner
    JB_RX_MEMOWNER  = 1;
    // Set RX to begin so it starts when sync bit is set.
    JB_RX_BEGIN     = 1;
    // Start next transaction.
    JB_TX_BEGIN     = 1;
    
    ets_delay_us(TIMEOUT_GC_US);

    // If we timed out, just reset for next phase
    if (!rx_recieved)
    {
        
        rmt_reset();

        rx_timeout_counts += 1;
        if (rx_timeout_counts >= TIMEOUT_COUNTS)
        {
            gc_timer_stop();
            gc_timer_reset();

            rx_timeout_counts = 0;
            cmd_phase = CMD_PHASE_PROBE;
            rgb_animate_to(COLOR_RED);

            memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GCMD_PROBE_LEN);
        }
    }
    else if (rx_recieved)
    {
        rx_recieved = false;
        rx_timeout_counts = 0;
        // Process our data if we received something
        // from the gamecube controller
        gamecube_rmt_process();
        rmt_reset();
    }

    usb_send_data();
}