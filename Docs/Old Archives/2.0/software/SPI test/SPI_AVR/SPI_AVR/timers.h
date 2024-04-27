#ifndef TIMERS_H_
#define TIMERS_H_

#define MIN_US	10
#define MAX_US	0x7FFF
#define MIN_MS	1
#define MAX_MS	0x006E

void interruptInit(void);

void delayInit(void);

void delayus(fourbyte delayValue);

void delayms(fourbyte delayValue);

#endif /* TIMER_H_ */