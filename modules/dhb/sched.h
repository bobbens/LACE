

#ifndef _SCHED_H
#  define _SCHED_H

/* Scheduler state flags. */
extern uint8_t sched_flags; /**< Scheduler flags. */
#define SCHED_HEARTBEAT             (1<<0) /**< HEARTBEAT Task. */
#define SCHED_MOTOR                 (1<<1) /**< Motor control task. */
#define SCHED_SPIS_PREP_MOTORGET    (1<<2)
#define SCHED_SPIS_PREP_CURRENT     (1<<3)

inline void heartbeat_set( /*uint8_t rate0,*/ uint8_t rate1 );


#endif /* _SCHED_H */

