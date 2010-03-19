

#ifndef _SCHED_H
#  define _SCHED_H

/* Scheduler state flags. */
extern volatile unsigned int sched_flags; /**< Scheduler flags. */
#define SCHED_HEARTBEAT             (1<<0) /**< HEARTBEAT Task. */
#define SCHED_MOTOR                 (1<<1) /**< Motor control task. */


#endif /* _SCHED_H */

