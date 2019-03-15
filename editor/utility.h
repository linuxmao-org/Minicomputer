#ifndef UTILITY_H_
#define UTILITY_H_

#include <string>
#include <stdio.h>
#include <stdarg.h>

inline std::string vastrprintf(const char *fmt, va_list ap)
{
    char *strp;
    if (vasprintf(&strp, fmt, ap) == -1)
        throw std::bad_alloc();
    std::string str(strp);
    free(strp);
    return str;
}

__attribute__((format(printf, 1, 2)))
inline std::string astrprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string string = vastrprintf(fmt, ap);
    va_end(ap);
    return string;
}

#endif  // UTILITY_H_
