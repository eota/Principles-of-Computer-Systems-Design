/**
 * See scheduler.h for function details. All are callbacks; i.e. the simulator 
 * calls you when something interesting happens.
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "simulator.h"
#include "scheduler.h"
#include "List.h"
#include "queue.h"

typedef struct {
  thread_t *t;
  // Timings
  int arival;
} timings_t;

void *queue;
list_node* ready_list;
list_node* all_threads_list;
thread_t *current_thread;
enum algorithm algo;
int quan;
int ticks_on_cpu;

static int thread_comparator(void *a, void *b) {

  return ((timings_t*)a)->t->priority - ((timings_t*)b)->t->priority;
}

void scheduler(enum algorithm algorithm, unsigned int quantum) {
  queue = queue_create();
  all_threads_list = list_create(NULL);
  algo = algorithm;
  quan = quantum;
  ticks_on_cpu = 0;
}

void sim_tick() { }

void sim_ready() {
  // printf("\nSIM_READY()\n");
  if (algo == ROUND_ROBIN) {
    if (is_empty(ready_list)) {
      // if (at the end of quantum)
      // reset ticks on cpu count to 0 ie. let it run for another quantum
      // printf("\nsim_ready(): QUEUE EMPTY, ticks_on_cpu = %d\n", ticks_on_cpu);
      if (ticks_on_cpu >= quan) {
        ticks_on_cpu = 0;
      }
    } else {
      // if (current thread has spent quantum ticks on cpu)
      // put current thread on back of ready queue
      // dispatch head of ready queue and set its tick count to 0
      // printf("\nsim_ready(): QUEUE NOT EMPTY, ticks_on_cpu = %d\n", ticks_on_cpu);
      if (ticks_on_cpu >= quan) {
        ready_list = list_insert_beginning(ready_list, current_thread);
        current_thread = list_front(ready_list)->thread;
        sim_dispatch(list_front(ready_list)->thread);// dispatch top element of ready_list
        list_remove(&ready_list, list_front(ready_list));
        ticks_on_cpu = 0;
      }
    }
    // Increment tick counter
    if (current_thread != NULL) {
      ticks_on_cpu++;
    }
  }

  if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY) {
    if (current_thread == NULL && queue_size(queue) > 0) {
      queue_sort(queue, thread_comparator);
      timings_t *head = queue_head(queue);
      current_thread = head->t;
      sim_dispatch(head->t);
      queue_dequeue(queue);
    }
  }
}

void sys_exec(thread_t *t) {

  if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY) {

    int arival = sim_time();
    int first_cpu_burst_start = -1;

    if (algo == PREEMPTIVE_PRIORITY) {
      if (current_thread != NULL) {
        int current_prio = current_thread->priority;
        int this_prio = t->priority;
        if (this_prio < current_prio) {
          timings_t *tim = malloc(sizeof(timings_t*));
          tim->t = current_thread;
          tim->arival = sim_time();
          queue_enqueue(queue, tim);
          current_thread = NULL;
        }
      }
    }

    if (false) { //current_thread == NULL
      current_thread = t;
      first_cpu_burst_start = sim_time();
      sim_dispatch(t);
    } else {
      timings_t *tim = malloc(sizeof(timings_t*));
      tim->t = t;
      queue_enqueue(queue, tim);
      if (queue_size(queue) > 0) {
        queue_sort(queue, thread_comparator);
      }
      // timings_t *head = queue_head(queue);
      // printf("\nSYS_EXEC(): queue_head(queue)->t->tid = %d\n", head->t->tid);
    }
    // Updating times for stats
    list_node* node = list_insert_end(all_threads_list, t);
    node->arival = arival;
    if (first_cpu_burst_start != -1) {
      node->first_cpu_burst_start = first_cpu_burst_start;
    }

  } else {

  // printf("\nsys_exec\n");
  int arival = sim_time();
  int first_cpu_burst_start = -1;

  // if no thread running
  //    dispatch thread
  // else
  //    put thread on ready queue
  if (current_thread == NULL) {
    current_thread = t;
    first_cpu_burst_start = sim_time();
    sim_dispatch(t);
    ticks_on_cpu = 0;
  } else {
    ready_list = list_insert_beginning(ready_list, t);
  }

    // Updating times for stats
    list_node* node = list_insert_end(all_threads_list, t);
    node->arival = arival;
    if (first_cpu_burst_start != -1) {
      node->first_cpu_burst_start = first_cpu_burst_start;
    }

  }
}

void sys_read(thread_t *t) {

  if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY) {
    int first_cpu_burst_end = sim_time();
    current_thread = NULL;
    if (current_thread == NULL) {
      if (queue_size(queue) > 0) {
        queue_sort(queue, thread_comparator);
        timings_t *head = queue_head(queue);
        if (queue_size(queue) > 0 && head->t != NULL) {

          // Update times
          int first_cpu_burst_start = sim_time() + 1;
          int second_cpu_burst_start = sim_time() + 1;
          list_node* node = list_find_by_data(all_threads_list, head->t);
          if (node) {
            if (node->first_cpu_burst_start == 0 && node->first_cpu_burst_end == 0) {
              node->first_cpu_burst_start = first_cpu_burst_start;
            }
            node->second_cpu_burst_start = second_cpu_burst_start;
          } else {
            // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
          }

          current_thread = head->t;
          sim_dispatch(head->t);
          queue_dequeue(queue);
        }
      }
    }
    // Updating times for stats
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->first_cpu_burst_end = first_cpu_burst_end;
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }
  } else {

    // printf("\nsys_read\n");

    // Times to remember: First CPU burst end, IO requested
    int first_cpu_burst_end = sim_time();
    current_thread = NULL;

    // dispatch next thread from ready queue (if any)
    if (current_thread == NULL) {
      if (is_empty(ready_list) == false) {
        if (list_front(ready_list)->thread != NULL) {
          // Updating times for stats
          int first_cpu_burst_start = sim_time() + 1;
          int second_cpu_burst_start = sim_time() + 1;
          list_node* node = list_find_by_data(all_threads_list, list_front(ready_list)->thread);
          if (node) {
            if (node->first_cpu_burst_start == 0 && node->first_cpu_burst_end == 0) {
              node->first_cpu_burst_start = first_cpu_burst_start;
            }
            node->second_cpu_burst_start = second_cpu_burst_start;
          } else {
            // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
          }
          current_thread = list_front(ready_list)->thread;
          sim_dispatch(list_front(ready_list)->thread);// dispatch top element of ready_list
          list_remove(&ready_list, list_front(ready_list));
          ticks_on_cpu = 0;
        }
      }
    }

    // Updating times for stats
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->first_cpu_burst_end = first_cpu_burst_end;
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }

  }
}

void sys_write(thread_t *t) {

  if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY) {
    int first_cpu_burst_end = sim_time();
    current_thread = NULL;
    if (current_thread == NULL) {
      if (queue_size(queue) > 0) {
        queue_sort(queue, thread_comparator);
        timings_t *head = queue_head(queue);
        if (queue_size(queue) > 0 && head->t != NULL) {
          // Updating times for stats
          int first_cpu_burst_start = sim_time() + 1;
          int second_cpu_burst_start = sim_time() + 1;
          list_node* node = list_find_by_data(all_threads_list, head->t);
          if (node) {
            if (node->first_cpu_burst_start == 0 && node->first_cpu_burst_end == 0) {
              node->first_cpu_burst_start = first_cpu_burst_start;
            }
            node->second_cpu_burst_start = second_cpu_burst_start;
          } else {
            // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
          }

          current_thread = head->t;
          sim_dispatch(head->t);
          queue_dequeue(queue);
        }
      }
    }
    // timings
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->first_cpu_burst_end = first_cpu_burst_end;
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }

  } else {

    // printf("\nsys_write\n");

    // Times to remember: First CPU burst end, IO requested

    int first_cpu_burst_end = sim_time();
    current_thread = NULL;

    // dispatch next thread from ready queue (if any)
    if (current_thread == NULL) {
      if (is_empty(ready_list) == false) {
        if (list_front(ready_list)->thread != NULL) {
          // Updating times for stats
          int first_cpu_burst_start = sim_time() + 1;
          int second_cpu_burst_start = sim_time() + 1;
          list_node* node = list_find_by_data(all_threads_list, list_front(ready_list)->thread);
          if (node) {
            if (node->first_cpu_burst_start == 0 && node->first_cpu_burst_end == 0) {
              node->first_cpu_burst_start = first_cpu_burst_start;
            }
            node->second_cpu_burst_start = second_cpu_burst_start;
          } else {
            // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
          }

          current_thread = list_front(ready_list)->thread;
          sim_dispatch(list_front(ready_list)->thread);// dispatch top element of ready_list
          list_remove(&ready_list, list_front(ready_list));
          ticks_on_cpu = 0;
        }
      }
    }

    // Updating times for stats
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->first_cpu_burst_end = first_cpu_burst_end;
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }
  }
}

void sys_exit(thread_t *t) {

  if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY) {
    // Times to remember: Finish
    int finish = sim_time();
    current_thread = NULL;
    if (current_thread == NULL) {
      if (queue_size(queue) > 0) {
        queue_sort(queue, thread_comparator);
        timings_t *head = queue_head(queue);
        // printf("\nSYS_EXIT(): queue_head(queue)->t->tid = %d\n", head->t->tid);
        if (queue_size(queue) > 0 && head->t != NULL) {
          // Update times
          int first_cpu_burst_start = sim_time() + 1;
          int second_cpu_burst_start = sim_time() + 1;
          list_node* node = list_find_by_data(all_threads_list, head->t);
          if (node) {
            if (node->first_cpu_burst_start == 0 && node->first_cpu_burst_end == 0) {
              node->first_cpu_burst_start = first_cpu_burst_start;
            }
            node->second_cpu_burst_start = second_cpu_burst_start;
          } else {
            // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
          }

          current_thread = head->t;
          // printf("head->t->tid = %d\n",head->t->tid);
          sim_dispatch(head->t);
          // printf("\nSYS_EXIT() SIM_DISPATCH(head->t)\n");
          queue_dequeue(queue);
        }
      }
    }
    // timings
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->finish = finish;
      if (node->io_burst_start == 0) {
        node->first_cpu_burst_end = finish;
      }
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }

  } else {

    // printf("\nsys_exit\n");

    // Times to remember: Finish
    int finish = sim_time();

    current_thread = NULL;

    // dispatch next thread from ready queue (if any)
    if (current_thread == NULL) {
      if (is_empty(ready_list) == false) {
        if (list_front(ready_list)->thread != NULL) {

          // Updating times for stats
          int first_cpu_burst_start = sim_time() + 1;
          int second_cpu_burst_start = sim_time() + 1;
          list_node* node = list_find_by_data(all_threads_list, list_front(ready_list)->thread);
          if (node) {
            if (node->first_cpu_burst_start == 0 && node->first_cpu_burst_end == 0) {
              node->first_cpu_burst_start = first_cpu_burst_start;
            }
            node->second_cpu_burst_start = second_cpu_burst_start;
          } else {
            // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
          }
          current_thread = list_front(ready_list)->thread;
          sim_dispatch(list_front(ready_list)->thread);// dispatch top element of ready_list
          list_remove(&ready_list, list_front(ready_list));
          ticks_on_cpu = 0;
        }
      }
    }

    // Updating times for stats
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->finish = finish;
      if (node->io_burst_start == 0) {
        node->first_cpu_burst_end = finish;
      }
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }
  }
}

void io_complete(thread_t *t) {

  if (algo == NON_PREEMPTIVE_PRIORITY || algo == PREEMPTIVE_PRIORITY) {
    int io_burst_end = sim_time();
    int second_cpu_burst_start = -1;
    // Preempt:
    // if a t->priority is greater than current_thread->priority
    //     preempt (dispatch t)
    if (algo == PREEMPTIVE_PRIORITY) {
      if (current_thread != NULL) {
        int current_prio = current_thread->priority;
        int this_prio = t->priority;
        if (this_prio < current_prio) {
          timings_t *tim = malloc(sizeof(timings_t*));
          tim->t = current_thread;
          queue_enqueue(queue, tim);
          current_thread = NULL;
        }
      }
    }
    if (current_thread == NULL) {
      current_thread = t;
      second_cpu_burst_start = sim_time() + 1;
      sim_dispatch(t);
    } else {
      timings_t *tim = malloc(sizeof(timings_t*));
      tim->t = t;
      queue_enqueue(queue, tim);
      queue_sort(queue, thread_comparator);
    }
    // Updating times for stats
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->io_burst_end = io_burst_end;
      if (second_cpu_burst_start != -1) {
        node->second_cpu_burst_start = second_cpu_burst_start;
      }
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }
  } else {

    // printf("\nio_complete\n");

    int io_burst_end = sim_time();
    int second_cpu_burst_start = -1;

    // if no thread running
    //    dispatch thread
    // else
    //    put thread on back of ready queue
    if (current_thread == NULL) {
      current_thread = t;
      second_cpu_burst_start = sim_time() + 1;
      sim_dispatch(t);
    } else {
      ready_list = list_insert_beginning(ready_list, t);
    }

    // Updating times for stats
    list_node* node = list_find_by_data(all_threads_list, t);
    if (node) {
      node->io_burst_end = io_burst_end;
      if (second_cpu_burst_start != -1) {
        node->second_cpu_burst_start = second_cpu_burst_start;
      }
    } else {
      // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
    }

  }
}

void io_starting(thread_t *t) {
  // Times to remember: IO burst start

  int io_burst_start = sim_time();

  list_node* node = list_find_by_data(all_threads_list, t);
  if (node) {
    node->io_burst_start = io_burst_start;
  } else {
    // printf("\nCOULDNT FIND NODE FOR THREAD T\n");
  }
}

stats_t *stats() {
  // Overall wait_time = turnaround_time - (cpu_time + io_time)
  // io time = io_end - io_start + 1
  if (false) { //algo == NON_PREEMPTIVE_PRIORITY
    return NULL;
  } else {

    // NEW IMPLEMENTATION
    int thread_count = list_size(all_threads_list) - 1;
    stats_t *stats = malloc(sizeof(stats_t));
    stats->tstats = malloc(sizeof(stats_t)*thread_count);

    // printf("\nTHREAD COUNT = %d\n", thread_count);

    for (int i = 0; i < thread_count; i++) {
      list_node * l = list_front(all_threads_list);
      stats->tstats[i].tid = l->thread->tid;
      int turnaround = l->finish - l->arival + 1;
      int io_time = l->io_burst_end - l->io_burst_start + 1;
      // int first_cpu_wait = l->first_cpu_burst_start - l->arival;
      // int first_io_wait = l->io_burst_start - l->first_cpu_burst_end - 1;
      // int second_cpu_wait = l->second_cpu_burst_start - l->io_burst_end - 1;
      if (l->io_burst_start == 0) {
        // first_io_wait = 0;
        // second_cpu_wait = 0;
        io_time = 0;
      }
      stats->tstats[i].turnaround_time = turnaround; // l->finish - l->arival + 1;
      // stats->tstats[i].waiting_time = first_cpu_wait + first_io_wait + second_cpu_wait;
      stats->tstats[i].waiting_time = turnaround - (l->thread->length + io_time);
      // printf("\nTHREAD ID= %d\n", l->thread->tid);
      // printf("\nl->finish = %d\n", l->finish);
      // printf("\nl->arival = %d\n", l->arival);
      // printf("\nl->first_cpu_burst_start = %d\n", l->first_cpu_burst_start);
      // printf("\nl->second_cpu_burst_start = %d\n", l->second_cpu_burst_start);
      // printf("\nl->first_cpu_burst_end = %d\n", l->first_cpu_burst_end);
      // printf("\nl->io_burst_start = %d\n", l->io_burst_start);
      // printf("\nl->io_burst_end = %d\n", l->io_burst_end);
      // printf("\nl->thread->length = %d\n", l->thread->length);
      list_remove(&all_threads_list, l);
    }

    //means for all threads
    stats->thread_count = thread_count;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    for (int i = 0; i < thread_count; i++) {
      turnaround_time_sum += stats->tstats[i].turnaround_time;
      waiting_time_sum += stats->tstats[i].waiting_time;
    }
    stats->turnaround_time = (turnaround_time_sum / thread_count);
    stats->waiting_time = (waiting_time_sum / thread_count);

    return stats;
  }
}
