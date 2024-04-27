#ifndef TIMERS_H_
#define TIMERS_H_

#define MIN_US	10
#define MAX_US	0x7FFF

void interruptInit(uint8_t prescaler, uint8_t compare);

void delayInit(void);

void delayus(uint16_t delayValue);

void delayms(uint16_t delayValue);

#endif /* TIMER_H_ */