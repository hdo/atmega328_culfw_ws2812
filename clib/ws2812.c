#include <stdint.h>
#include "ws2812.h"
#include "display.h"

#define NO_BUTTON 0xFF

uint8_t last_button = NO_BUTTON;

void ws2812_task(void) {
	if (last_button != NO_BUTTON) {
		DH2(last_button);
		last_button = NO_BUTTON;
	}
	return;
}

void ws2812_register_remote_button(uint8_t button) {
	last_button = button;
	return;
}
