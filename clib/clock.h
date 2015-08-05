#ifndef _CLOCK_H_
#define _CLOCK_H_

volatile extern uint32_t ticks;  // 1/125 sec resolution
void gettime(char*);

void Minute_Task(void);

#endif
