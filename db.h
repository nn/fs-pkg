/*
 * Database abstration layer
 *
 * Provides support for sqlite and berkeley DB
 */
enum db_query_res_type {
   QUERY_NULL = 0,                     /* no result */
   QUERY_INT,                          /* integer result */
   QUERY_CHAR,                         /* char result */
   QUERY_INODE,                        /* pkgfs_inode result */
};

struct db_connector {
   /*
    * internal properties 
    */
   void       *hndl;                   /* database handle */
   pthread_mutex_t mutex;              /* mutex */
   u_int16_t   error;                  /* error return */

   /*
    * semi-private methods 
    */
   void        (*begin) (void);
   void        (*commit) (void);
   void        (*rollback) (void);

   /*
    * public methods 
    */
   void       *(*db_query) (enum db_query_res_type type, const char *fmt, ...);
   int         (*db_open) (const char *path);
   void        (*db_destroy) (void);
};

#include "db_sqlite.h"
