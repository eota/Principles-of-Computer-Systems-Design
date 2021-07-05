/************************************************************************
 *
 * CSE130 Winter 2021 
 *
 * Copyright (C) 2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 * 
 ************************************************************************/

/*********************************************************
 **               DO NOT MODIFY THIS FILE               ** 
 *********************************************************/

#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "simulator.h"

/**
 * Default is First Come First Served, only Round Robin has a quantum.
 */
enum algorithm {
  FIRST_COME_FIRST_SERVED,
  ROUND_ROBIN,
  NON_PREEMPTIVE_PRIORITY,
  PREEMPTIVE_PRIORITY,
  NON_PREEMPTIVE_SHORTEST_JOB_FIRST,
  PREEMPTIVE_SHORTEST_JOB_FIRST,
  NON_PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST,
  PREEMPTIVE_SHORTEST_REMAINING_TIME_FIRST
};

/**
 * Stats of the scheduler simulator run, airthmentic means at the top level, 
 * individual threads at the second level. 
 * 
 * For example, in a mildly JSON like syntax, stats for a run with two threads:
 * 
 *  {
 *    Mean waiting time  
 *    Mean turnaround time
 *    thread count = 2
 *    [  
 *      { 
 *        tid of T1
 *        Waiting time for T1
 *        Turnaround time for T1
 *      },
 *      { 
 *        tid of T2
 *        Waiting time for T2
 *        Turnaround time for T2
 *      }
 *    ]
 *  }
 * 
 * Assuming a linked list of thread data your scheduler added to and maintained
 * whilst the simulation was running, you'd create a stats object like so:
 * 
 *   ( Note that 'list' and 'size_of_list()' are yours to implement )
 * 
 *   int thread_count = size_of_list(list); 
 *   stats_t *stats = malloc(sizeof(stats_t));
 *   stats->tstats = malloc(sizeof(stats_t)*thread_count);
 * 
 * Then you'd fill in the individual stats->tstats[0..thread_count-1] records 
 * for each thread and set the mean waiting and turnaround times on stats itself.
 */
typedef struct __stats_t {
  unsigned int tid;           // Only meaningful at second level
  unsigned int waiting_time; 
  unsigned int turnaround_time;
  unsigned int thread_count;  // Number of elements in TSTATS
  struct __stats_t *tstats;   // Address of the first of THREAD_COUNT stats_t 'objects'
} stats_t;

/**
 * Initalise a scheduler implemeting the requested ALGORITHM. QUANTUM is only
 * meaningful for ROUND_ROBIN.
 */
void scheduler(enum algorithm algorithm, unsigned int quantum);

/**
 * Tread T is ready to be scheduled for the first time.
 */
void sys_exec(thread_t *t);

/**
 * Thread T has completed execution and should never again be scheduled.
 */
void sys_exit(thread_t *t);

/**
 * Thread T has requested a read operation and is now in an I/O wait queue.
 * When the read operation starts, io_starting(T) will be called, when the
 * read operation completes, io_complete(T) will be called.
 */
void sys_read(thread_t *t);

/**
 * Thread T has requested a write operation and is now in an I/O wait queue.
 * When the write operation starts, io_starting(T) will be called, when the
 * write operation completes, io_complete(T) will be called.
 */
void sys_write(thread_t *t);

/**
 * An I/O operation requested by T has completed; T is now ready to be 
 * scheduled again.
 */
void io_complete(thread_t *t);

/**
 * An I/O operation requested by T is starting; T will not be ready for
 * scheduling until the operation completes.
 */
void io_starting(thread_t *t);

/**
 * Return stats for the scheduler simulation, see type definition above.
 */
stats_t *stats();

#endif // __SCHEDULER_H