#ifndef _COMMON_H_
#define _COMMON_H

#define STDOUT stdout
#define STDERR stderr

#ifndef LOGGER_FACILITY
#define LOGGER_FACILITY STDERR
#endif

#ifndef DEBUG_PRINTS
#define DEBUG_PRINTS 0
#endif

#if DEBUG_PRINTS == 1
#define LOG(fmt, ...) \
	fprintf(LOGGER_FACILITY, "[%s] %s:%d: %s(): " fmt "\n",\
			__TIME__, __FILE__, __LINE__, __func__, ##__VA_ARGS__);
#else
#define LOG(fmt, ...)
#endif

#endif
