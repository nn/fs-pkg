/*
 * Dictionary based configuration lookup
 * 	Used for single-entry configurations
 *	See lconf.[ch] for list-based configuration
 *	suitable for ACLs and similar options.
 *
 * Copyright (C) 2008 N2 Networks LLC
 *
 * This code wouldn't be possible without N. Devillard's dictionary.[ch]
 * from the iniparser package. Thanks!!
 */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <strings.h>
#include "conf.h"
#include "dictionary.h"
#include "logger.h"
#include "str.h"

void dconf_init(const char *file) {
   FILE       *fp;
   char       *p;
   char        buf[512];
   int         in_comment = 0;
   int         line = 0;
   char       *key, *val;
   NN2_DCONF_DICT = dictionary_new(0);

   if (!(fp = fopen(file, "r"))) {
      Log(LOG_FATAL, "unable to open config file %s", file);
      raise(SIGTERM);
   }

   while (!feof(fp)) {
      line++;
      memset(buf, 0, 512);
      fgets(buf, sizeof(buf), fp);

      /*
       * did we get a line? 
       */
      if (buf == NULL)
         continue;

      p = buf;

      str_strip(p);
      p = str_whitespace_skip(p);

      /*
       * did we eat the whole line? 
       */
      if (strlen(p) == 0)
         continue;

      /*
       * fairly flexible comment handling
       */
      if (p[0] == '*' && p[1] == '/') {
         in_comment = 0;               /* end of block comment */
         continue;
      } else if (p[0] == ';' || p[0] == '#' || (p[0] == '/' && p[1] == '/')) {
         continue;                     /* line comment */
      } else if (p[0] == '/' && p[1] == '*')
         in_comment = 1;               /* start of block comment */

      if (in_comment)
         continue;                     /* ignored line, in block comment */

      key = p;

      if ((val = strchr(p, '=')) != NULL) {
         *val = '\0';
         val++;
      }

      val = str_unquote(val);
      dconf_set(key, val);
   }
}

void dconf_fini(void) {
   dictionary_del(NN2_DCONF_DICT);
   NN2_DCONF_DICT = NULL;
}

int dconf_get_bool(const char *key, const int def) {
   char       *tmp;
   int         rv = 0;

   if ((tmp = dconf_get_str(key, NULL)) == NULL)
      return def;
   else if (strcasecmp(tmp, "true") == 0 || strcasecmp(tmp, "on") == 0 ||
            strcasecmp(tmp, "yes") == 0 || (int)strtol(tmp, NULL, 0) == 1)
      rv = 1;
   else if (strcasecmp(tmp, "false") == 0 || strcasecmp(tmp, "off") == 0 ||
            strcasecmp(tmp, "no") == 0 || (int)strtol(tmp, NULL, 0) == 0)
      rv = 0;

   return rv;
}

double dconf_get_double(const char *key, const double def) {
   char       *tmp;

   if ((tmp = dconf_get_str(key, NULL)) == NULL)
      return def;

   return atof(tmp);
}

int dconf_get_int(const char *key, const int def) {
   char       *tmp;

   if ((tmp = dconf_get_str(key, NULL)) == NULL)
      return def;

   return (int)strtol(tmp, NULL, 0);
}

char       *dconf_get_str(const char *key, const char *def) {
   if (NN2_DCONF_DICT == NULL || key == NULL)
      return NULL;

   return dictionary_get(NN2_DCONF_DICT, key, def);
}

time_t dconf_get_time(const char *key, const time_t def) {
   return (nn2_timestr_to_time(dconf_get_str(key, NULL), def));
}

int dconf_set(const char *key, const char *val) {
   return dictionary_set(NN2_DCONF_DICT, key, val);
}

void dconf_unset(const char *key) {
   dictionary_unset(NN2_DCONF_DICT, key);
}
