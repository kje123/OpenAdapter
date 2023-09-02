#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <time.h>

#include "GamecubeController.hpp"
#include "gamecube_definitions.h"
#include "usb_definitions.h"
#include "tusb.h"
#include "tusb_types.h"
#include "bsp/board.h"

GamecubeController *gcc;
gc_report_t gc_report = default_gc_report;
uint8_t buffer[37];
bool rumbleToggle = 0;

int phase = 0;
bool isReady = 0;
int timer = 0;
#define LED_PIN 25
#define GC_AXIS_CENTER 128

// needed to check if port is being used
// index 0 is p1, 1 is p2
bool isActive [2] = { 0, 0 };

void send_data() {
    if(!tud_ready()) {
        return;
    }

    gc_usb_report curr;

    // set first bit to id
    buffer[0] = 0x21;

    if(!(gcc->_initialized)) {
        curr.b1 = 0x00;
        curr.b2 = 0x00;
        curr.a_x = GC_AXIS_CENTER;
        curr.a_y = GC_AXIS_CENTER;
        curr.c_x = GC_AXIS_CENTER;
        curr.c_y = GC_AXIS_CENTER;
        curr.a_l = 0;
        curr.a_r = 0;
    } else {
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
    }

    if(!isActive[0]) {
        // set unused ports to inactive bytecode
        buffer[1] = 0x14;
        buffer[10] = 0x04;
        buffer[19] = 0x04;
        buffer[28] = 0x04;
        isActive[0] = true;
    } else {
        memcpy(&buffer[2], &curr, 8);
    }

    tud_hid_report(0, &buffer, 37);
}

void led_task(void) {
    if(phase) {
        gpio_put(LED_PIN, 1);
    } else {
        gpio_put(LED_PIN, 0);
    }
}

void joybus_main(void) {
    uint joybus_pin = 1;
    gcc = new GamecubeController(joybus_pin, 1000, pio1);
    while(true) {
        gcc->Poll(&gc_report, rumbleToggle);
    }
}

int main(void) {
    set_sys_clock_khz(130'000, true);
    board_init();
    tusb_init();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    multicore_launch_core1(joybus_main);

    send_data();

    while (true) {
        send_data();
        tud_task();
        led_task();
    }
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    if (!report_id && !report_type) {
        if (buffer[0] == 0x11) {
            rumbleToggle = (bool)buffer[1];
        }
    }
}

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return gc_hid_report_descriptor;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
    if (report[0] == 0x21) {
        
    }
}