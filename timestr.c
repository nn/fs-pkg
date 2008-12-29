#define	_BSD_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "timestr.h"
/* these two are used to convert from/to a formatted time string.  the string
 * should be of the format [Xd][Xh][Xm][X][s].  e.g. 3600 is 3600s, or 1h.
 * either lower or upper case is acceptable. */
time_t nn2_timestr_to_time(const char *str, const time_t def) {
   char       *s;
   char       *s2;
   time_t      ret = 0;

   if (str == NULL)
      return def;

   s = s2 = strdup(str);
   while (*s2 != '\0') {
      *s2 = tolower(*s2);
      s2++;
   }

   s2 = strchr(s, 'd');
   if (s2 != NULL) {
      *s2 = '\0';
      ret += strtol(s, NULL, 0) * 86400;  /* days */
      s = s2 + 1;
   }
   s2 = strchr(s, 'h');
   if (s2 != NULL) {
      *s2 = '\0';
      ret += strtol(s, NULL, 0) * 3600;   /* hours */
      s = s2 + 1;
   }
   s2 = strchr(s, 'm');
   if (s2 != NULL) {
      *s2 = '\0';
      ret += strtol(s, NULL, 0) * 60;  /* minutes */
      s = s2 + 1;
   }
   ret += strtol(s, NULL, 0);          /* seconds */

   return ret;
}

char       *nn2_time_to_timestr(time_t itime) {
   int         d, h, m, s;
   static char rbuf[64];
   int         rlen = 0;

   d = itime / 86400;                  /* days */
   itime %= 86400;
   h = itime / 3600;                   /* hours */
   itime %= 3600;
   m = itime / 60;                     /* minutes */
   itime %= 60;
   s = itime;

   if (d)
      rlen += snprintf(rbuf + rlen, 64, "%dd", d);
   if (h)
      rlen += snprintf(rbuf + rlen, 64, "%dh", h);
   if (m)
      rlen += snprintf(rbuf + rlen, 64, "%dm", m);
   if (s)
      rlen += snprintf(rbuf + rlen, 64, "%ds", s);
   if (!rlen) {                        /* if we haven't added anything, it's 0s */
      rbuf[0] = '0';
      rbuf[1] = 's';
      rbuf[2] = '\0';
   }

   return rbuf;
}
