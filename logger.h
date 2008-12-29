#if	!defined(NN2_LOGGER)
#define NN2_LOGGER
#include <stdio.h>
#define	__logger_h

enum log_priority {
   LOG_DEBUG = 0,
   LOG_INFO,
   LOG_WARNING,
   LOG_ERROR,
   LOG_FATAL,
   LOG_HACK
};

#if	!defined(NN2_LOGGER_FP)
extern FILE *log_fp;
#endif

extern FILE *log_open(const char *path);
extern void log_close(FILE * fp);
extern void Log(enum log_priority priority, const char *fmt, ...);
#endif	/* !defined(NN2_LOGGER) */
