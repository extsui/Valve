#ifndef UTILS_H
#define UTILS_H

#include "main.h" // LED1_*
#include "Console.h" // Console::Log()

static inline void ABORT_NO_MESSAGE() { while (1); }

#define LOG(format, ...) Console::Log(format, __VA_ARGS__)

#define ABORT() \
    LOG("Abort: file %s, line %d\n", __FILE__, __LINE__); \
    ABORT_NO_MESSAGE()

#define __ASSERT(expr, file, line)                  \
	LOG("Assertion failed: %s, file %s, line %d\n", \
		expr, file, line),                          \
	ABORT_NO_MESSAGE()

#define ASSERT(expr)                              \
    ((expr) ? ((void)0) :                         \
    (void)(__ASSERT(#expr, __FILE__, __LINE__)))

static inline void LED_DEBUG(int count)
{
    for (int i = 0; i < count; i++) {
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    }
}

#endif // UTILS_H
