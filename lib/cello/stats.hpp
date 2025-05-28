#ifndef CELLO_STATS_HPP
#define CELLO_STATS_HPP
#include <util/statics.hpp>

#define CELLO_GATHER_STATISTICS
#ifdef  CELLO_GATHER_STATISTICS
#ifndef HOST
/**
 * start of cello statistics declaration
 * does nothing on device code
 */
#define CELLO_STAT_DECL_START()
/**
 * stop of cello statistics declaration
 * does nothing on device code
 */
#define CELLO_STAT_DECL_STOP()    
/**
 * declare a cello statistic
 */
#define CELLO_STAT_DECL(stat_name)             \
    extern "C" int stat_name;
/**
 * declare a cello statistic
 */
#define CELLO_STAT_DEF(stat_name)              \
    DMEM(int) stat_name;
/**
 * increment a cello statistic
 */
#define CELLO_STAT_ADD(stat_name)               \
    do { stat_name++; } while (0)
#else
/**
 * start of cello statistics declaration
 */
#define CELLO_STAT_DECL_START()                 \
    __attribute__((weak))                       \
    const char * cello_statistics [] = {        \
/**
 * stop of cello statistics declaration
 * does nothing on device code
 */
#define CELLO_STAT_DECL_STOP()                  \
    };
/**
 * declare a cello statistic
 */
#define CELLO_STAT_DECL(stat_name)              \
    # stat_name,
/**
 * declare a cello statistic
 * does nothing on host
 */
#define CELLO_STAT_DEF(stat_name)
/**
 * increment a cello statistic
 * does nothing on host
 */
#define CELLO_STAT_ADD(stat_name)               \
    do { stat_name++; } while (0)
#endif
#else
/**
 * start of cello statistics declaration
 * does nothing on device code
 */
#define CELLO_STAT_DECL_START()
/**
 * stop of cello statistics declaration
 * does nothing on device code
 */
#define CELLO_STAT_DECL_STOP()    

/**
 * declare a cello statistic
 */
#define CELLO_STAT_DECL(stat_name)             \
    ;
/**
 * declare a cello statistic
 */
#define CELLO_STAT_DEF(stat_name)              \
    ;
/**
 * increment a cello statistic
 */
#define CELLO_STAT_ADD(stat_name)               \
    ;
#endif

CELLO_STAT_DECL_START()
CELLO_STAT_DECL(cello_steals)
CELLO_STAT_DECL(cello_task_push)
CELLO_STAT_DECL(cello_task_execute)
CELLO_STAT_DECL(cello_owner_lock_acquire_fail)
CELLO_STAT_DECL_STOP()

#endif
