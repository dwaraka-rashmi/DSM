#ifdef _DSM_LIB_H
  #define EXTERN
#else
  #define EXTERN extern
#endif

// http://stackoverflow.com/questions/3658490/undefined-reference-when-using-extern

// Mutexes and locks
extern pthread_condattr_t cond_attrs[MAX_SHARED_PAGES];
extern pthread_cond_t conds[MAX_SHARED_PAGES];
extern pthread_mutex_t mutexes[MAX_SHARED_PAGES];