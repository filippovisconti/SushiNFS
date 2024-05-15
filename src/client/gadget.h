
// SPDX-License-Identifier: Apache-2.0
//
// Emulates a USB device that aims to pass usbtest driver tests.
// Part of the USB Raw Gadget test suite.
// See https://github.com/xairy/raw-gadget for details.
//
// Andrey Konovalov <andreyknvl@gmail.com>

#include <linux/usb/ch9.h>

int usb_raw_open();
void usb_raw_init(int fd, enum usb_device_speed speed, const char *driver, const char *device);
void usb_raw_run(int fd);

void ep0_loop(int fd);
