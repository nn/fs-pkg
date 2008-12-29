/* Sentinel - IRC Statistical and Operator Services
** s_string.h - String management functions
**
** Copyright W. Campbell and others.  See README for more details
** Some code Copyright: Jonathan George, Kai Seidler, ircd-hybrid Team,
**                      IRCnet IRCD developers.
**
** $Id: s_string.h,v 1.3 2003/11/06 20:41:53 wcampbel Exp $
*/
#ifndef S_STRING_H
#define S_STRING_H

#define MAXPARA	15                   /* The maximum number of parameters in an IRC message */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

extern char *str_ctime(time_t);
extern int  str_tokenize(char *, char **);
extern int  str_tokenize_generic(char, int, char *, char **);
extern void str_parv_expand(char *, int, int, int, char **);
extern int  str_islower(char *);
extern void str_strip(char *);
extern char *str_dup(const char *str);
extern void str_tohex(char *dst, const char *src, unsigned int length);
extern time_t dhms_to_sec(const char *str, int def);
extern char *sec_to_dhms(time_t itime);
extern char *str_unquote(char *str);
extern char *str_whitespace_skip(char *str);
extern char *str_tolower(char *s);
#endif
