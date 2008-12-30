#if	!defined(inode_h)
#define	inode_h

#include "balloc.h"
struct pkg_inode {
   char        dummy[1];
};

typedef struct pkg_inode pkg_inode_t;

BlockHeap  *inode_heap;

extern void inode_init(void);
extern void inode_fini(void);

#endif                                 /* !defined(inode_h) */
