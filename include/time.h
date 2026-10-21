#ifndef __C1_TIME_H
#define __C1_TIME_H
#ifndef __C1_TIME_T_DEFINED
#define __C1_TIME_T_DEFINED
typedef long time_t;
#endif
typedef long clock_t;
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long tm_gmtoff;
    const char *tm_zone;
};
time_t time(time_t *result);
clock_t clock(void);
struct tm *localtime(const time_t *value);
struct tm *gmtime(const time_t *value);
unsigned long strftime(char *text, unsigned long size, const char *format, const struct tm *value);
#define CLOCKS_PER_SEC 1000000L
#endif
