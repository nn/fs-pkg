#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "logger.h"
#include "str.h"
#include "support.h"

char       *str_dup(const char *str) {
   register char *ptr = NULL;

   if (str == NULL)
      return NULL;

   if ((ptr = strdup(str)) == NULL)
      Log(LOG_ERROR, "%s:strdup %d:%s", __FUNCTION__, errno, strerror(errno));

   return ptr;
}

void str_tohex(char *dst, const char *src, unsigned int length) {
   char        hex[17] = "0123456789abcdef";

   unsigned int i;

   for (i = 0; i < length; i++) {
      dst[2 * i] = hex[(src[i] & 0xf0) >> 4];
      dst[2 * i + 1] = hex[(src[i] & 0x0f)];
   }

   dst[2 * i] = 0;
}

/* Sentinel - IRC Statistical and Operator Services
** s_string.c - String manipulation functions
**
** Copyright W. Campbell and others.  See README for more details
** Some code Copyright: Jonathan George, Kai Seidler, ircd-hybrid Team,
**                      IRCnet IRCD developers.
**
** $Id: s_string.c,v 1.4 2003/11/06 20:41:53 wcampbel Exp $
*/

/* Our own ctime function */
/* Modified for l10n - 12 f\xe9v 2002 -Hwy */
char       *str_ctime(time_t stuff) {
   static char buf[32];

   struct tm  *ltm = localtime(&stuff);

   static char *gcc_sucks = "%c";

   /*
    * %c is the POSIX localized replacement for ctime() 
    */

   /*
    * gcc complains about some non-BSD implementations not using
    * ** a 4 digit year in some locales.  The warning can be ignored.
    * **
    * ** The lameness in this function is the only way I know of that
    * ** the warning can be ignored.
    * **
    * ** Some versions of gcc support -Wno-y2k, but apparently it's
    * ** only gcc3 and higher.  Using a custom -W parameter will cause
    * ** issues when run with another compiler, so for now, this lameness
    * ** stays.
    */
   strftime(buf, 32, gcc_sucks, ltm);
   return buf;
}

/* tokenize splits apart a message (in our case with the origin and command
** picked off already...
*/
int str_tokenize(char *message, char **parv) {
   char       *pos = NULL;

   char       *next;

   int         count = 0;

   if (!message) {
      /*
       * Something is seriously wrong...bail 
       */
      return -1;
   }

   /*
    * First we find out if there's a : in the message, save that string
    * ** somewhere so we can set it to the last param in parv
    * ** Also make sure there's a space before it...if not, then we're
    * ** screwed
    */
   pos = message;
   while (TRUE) {
      if ((pos = strchr(pos, ':'))) {
         pos--;
         if (*pos != ' ') {
            /*
             * There SHOULDN'T be a problem with this... 
             */
            pos += 2;
            continue;
         }
         *pos = '\0';
         /*
          * This is BAD - get rid of the space before the : 
          */
         pos++;
         *pos = '\0';
         /*
          * VERY BAD...over now though 
          */
         pos++;
         break;
      } else {
         break;
      }
   }

   /*
    * Now we take the beginning of message and find all the spaces...
    * ** set them to \0 and use 'next' to go through the string
    */

   next = message;
   parv[0] = message;
   count = 1;

   while (*next) {
      if (count == MAXPARA - 1) {
         /*
          * We've reached one less than our max limit
          * ** to handle the parameter we already ripped off
          */
         Log(LOG_DEBUG, "DEBUG: Reached the MAXPARA limit!");
         return count;
      }
      if (*next == ' ') {
         *next = '\0';
         next++;
         /*
          * Eat any additional spaces 
          */
         while (*next == ' ')
            next++;
         /*
          * If it's the end of the string, it's simply
          * ** an extra space before the :parameter.  Here we
          * ** break.
          */
         if (*next == '\0')
            break;
         parv[count] = next;
         count++;
      } else {
         next++;
      }
   }

   if (pos) {
      parv[count] = pos;
      count++;
   }

   return count;
}

/* A generic tokenizer, tokenizes based on a specified character */
int str_tokenize_generic(char delim, int size, char *message, char **parv) {
   char       *next;

   int         count;

   if (!message) {
      /*
       * Something is seriously wrong...bail 
       */
      parv[0] = NULL;
      return 0;
   }

   /*
    * Now we take the beginning of message and find all the spaces...
    * ** set them to \0 and use 'next' to go through the string
    */
   next = message;
   parv[0] = next;
   count = 1;

   while (*next) {
      /*
       * This is fine here, since we don't have a :delimited
       * ** parameter like tokenize
       */
      if (count == size) {
         /*
          * We've reached our limit 
          */
         Log(LOG_DEBUG, "Reached the size limit!");
         return count;
      }
      if (*next == delim) {
         *next = '\0';
         next++;
         /*
          * Eat any additional delimiters 
          */
         while (*next == delim)
            next++;
         /*
          * If it's the end of the string, it's simply
          * ** an extra space at the end.  Here we break.
          */
         if (*next == '\0')
            break;
         parv[count] = next;
         count++;
      } else {
         next++;
      }
   }

   return count;
}

void str_parv_expand(char *dest, int max, int initial, int parc, char *parv[]) {
   int         i;

   if (parv[initial])
      strlcpy(dest, parv[initial], max);
   for (i = initial + 1; i < parc; i++) {
      strlcat(dest, " ", max);
      strlcat(dest, parv[i], max);
   }
}

char       *str_tolower(char *s) {
   char       *t;

   for (t = s; *t != '\0'; t++)
      *t = tolower(*t);

   return s;
}

/* Return 1 if the string s is all lower case, return 0 if not */
int stris_lower(char *s) {
   char       *t;

   if (s == NULL)
      return 1;
   for (t = s; *t != '\0'; t++) {
      if (islower((int)*t) == 0) {
         return 0;
      }
   }
   return 1;
}

/* Remove CR and LF from the line input.  ASSUME that the line
** does NOT contain CR or LF in the middle
*/
void str_strip(char *line) {
   char       *c;

   if ((c = strchr(line, '\r'))) {
      /*
       * If we receive a CR, then we can just terminate the
       * ** call here
       */
      *c = '\0';
      return;
   }

   if ((c = strchr(line, '\n'))) {
      *c = '\0';
      return;
   }
}

char       *str_unquote(char *str) {
   char       *tmp;

   if (str && *str == '"')
      str++;
   if ((tmp = index(str, '"')))
      *tmp = '\0';

   return str;
}

char       *str_whitespace_skip(char *str) {
   /*
    * Skip over white space 
    */
   while (*str && (*str == ' ' || *str == '\t'))
      str++;
   return str;
}
