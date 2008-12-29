#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "conf.h"
#include "logger.h"

FILE       *log_open(const char *path) {
   FILE       *fp;

   if ((fp = fopen(path, "w")) == NULL) {
      Log(LOG_ERROR, "Unable to open log file %s: %d (%s)", path, errno, strerror(errno));
      return NULL;
   }

   return fp;
}

void log_close(FILE * fp) {
   if (fp != NULL) {
      fclose(fp);
      fp = NULL;
   }
}

void Log(enum log_priority priority, const char *fmt, ...) {
   va_list     ap;
   char        timestamp[64];
   time_t      t;
   struct tm  *tm;
   char       *level = NULL;

   if (priority < conf.log_level) {
      return;
   }

   if (!conf.log_fp)
      conf.log_fp = stdout;

   va_start(ap, fmt);
   t = time(NULL);
   tm = localtime(&t);

   switch (priority) {
      case LOG_DEBUG:
         level = "debug";
         break;
      case LOG_INFO:
         level = "info";
         break;
      case LOG_WARNING:
         level = "warning";
         break;
      case LOG_ERROR:
         level = "error";
         break;
      case LOG_FATAL:
         level = "critical";
         break;
      case LOG_HACK:
         level = "hack";
         break;
   default:
         level = "unknown";
         break;
   }

   strftime(timestamp, sizeof(timestamp) - 1, "%Y/%m/%d %H:%M:%S", tm);
   fprintf(conf.log_fp, "[%s] %s: ", timestamp, level);
   vfprintf(conf.log_fp, fmt, ap);
   fprintf(conf.log_fp, "\n");
   fflush(conf.log_fp);

   va_end(ap);
}
