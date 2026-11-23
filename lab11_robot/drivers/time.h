// time.h - Millisecond/microsecond timekeeping on Timer2
#ifndef TIME_H
#define TIME_H
#include <stdint.h>

void time_init(void);
uint32_t millis(void);
uint32_t micros(void);

// Pause/resume the tick interrupt (used around sleep mode so the
// 1ms interrupt does not keep waking the CPU).
void time_pause(void);
void time_resume(void);

#endif
