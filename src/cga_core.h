#ifndef CORE_H
#define CORE_H

#define null ((void*) 0)
#define true 1
#define false 0

typedef char boolean;

char* cgaFormatString(int maxlen, char* format, ...);

#endif // CORE_H