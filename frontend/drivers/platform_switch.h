#ifndef _PLATFORM_SWITCH_H
#define _PLATFORM_SWITCH_H

char* SWITCH_GPU_PROFILES[] = {
   "docked-overclock-3",
   "docked-overclock-2",
   "docked-overclock-1",
   "docked",
   "non-docked-overclock-5",
   "non-docked-overclock-4",
   "non-docked-overclock-3",
   "non-docked-overclock-2",
   "non-docked-overclock-1",
   "non-docked",
   "non-docked-underclock-1",
   "non-docked-underclock-2",
   "non-docked-underclock-3",
};
   
char* SWITCH_GPU_SPEEDS[] = {
   "998 Mhz",
   "921 Mhz",
   "844 Mhz",
   "768 Mhz",
   "691 Mhz",
   "614 Mhz",
   "537 Mhz",
   "460 Mhz",
   "384 Mhz",
   "307 Mhz",
   "230 Mhz",
   "153 Mhz",
   "76 Mhz"   
};

int SWITCH_BRIGHTNESS[] = {
   10,
   20,
   30,
   40,
   50,
   60,
   70,
   80,
   90,
   100
};

char* SWITCH_CPU_PROFILES[] = {
   "overclock-4",
   "overclock-3",
   "overclock-2",
   "overclock-1",
   "default",
};

#define SWITCH_DEFAULT_CPU_PROFILE        4 /* default */
#define LIBNX_MAX_CPU_PROFILE             1 /* overclock-3 */
   
char* SWITCH_CPU_SPEEDS[] = {
   "1912 MHz",
   "1734 MHz",
   "1530 MHz",
   "1224 MHz",
   "1020 MHz"
};

unsigned SWITCH_CPU_SPEEDS_VALUES[] = {
   1912000000,
   1734000000,
   1530000000,
   1224000000,
   1020000000
};

#endif