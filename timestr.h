#if	!defined(NN2_TIMESTRING)
#define	NN2_TIMESTRING
#include <time.h>
extern time_t nn2_timestr_to_time(const char *str, const time_t def);
extern char *nn2_time_to_timestr(time_t itime);
#endif	/* !defined(NN2_TIMESTRING) */
