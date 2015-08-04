#ifndef __WS2812_H
#define __WS2812_H

void ws2812_init(void);
void ws2812_task(uint32_t currentTicks);
void ws2812_register_remote_button(uint8_t button);
void ws2812_shift(void);


#endif /* end __WS2812_H */
