#pragma once

#define GBF_SIMULATE_CONSOLE_UI  0

#if PLATFORM_SWITCH || GBF_SIMULATE_CONSOLE_UI
    #define GBF_CONSOLE_UI 1
#else
#define GBF_CONSOLE_UI 0
#endif
