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
#include "clean-list.h"

typedef struct times {
  thread_t thread;
  int arival;
  int finish;
  int first_cpu_burst_start;
  int second_cpu_burst_start;
  int first_cpu_burst_end;
  int io_burst_start;
  int io_burst_end;
}times;

list_node* ready_list;
list_node* all_threads_list;
// list_node* back_node;
thread_t *current_thread;
times t1;
int num_threads;
List clean_list;

void scheduler(enum algorithm algorithm, unsigned int quantum) {
  // initialize lists
  // ready_list = list_create(NULL);
  all_threads_list = list_create(NULL);

  num_threads = 0;
}

void sim_tick() { }

void sys_exec(thread_t *t) {

  // printf("\nsys_exec\n");

  // Times to remember: Arival

  t1.arival = sim_time();
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
  } else {
    ready_list = list_insert_beginning(ready_list, t);
  }

    // Updating times for stats
    list_node* node = list_insert_end(all_threads_list, t);
    node->arival = arival;
    if (first_cpu_burst_start != -1) {
      node->first_cpu_burst_start = first_cpu_burst_start;
    }
    num_threads++;
}

void sys_read(thread_t *t) {

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

void sys_write(thread_t *t) {

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

void sys_exit(thread_t *t) {

  // printf("\nsys_exit\n");

  // Times to remember: Finish
  t1.finish = sim_time();
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

void io_complete(thread_t *t) {

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
  // int thread_count = 1;
  // stats_t *stats = malloc(sizeof(stats_t));
  // stats->tstats = malloc(sizeof(stats_t)*thread_count);

  // // stats for thread 1
  // stats->tstats[0].tid = 1;
  // stats->tstats[0].turnaround_time = t1.finish - t1.arival + 1;
  // // stats->tstats[0].waiting_time = ;

  // // means for all threads
  // stats->thread_count = thread_count;
  // stats->turnaround_time = t1.finish - t1.arival + 1;
  // // stats->waiting_time = ;

  // NEW IMPLEMENTATION
  int thread_count = list_size(all_threads_list) - 1;
  stats_t *stats = malloc(sizeof(stats_t));
  stats->tstats = malloc(sizeof(stats_t)*thread_count);

  // printf("\nTHREAD COUNT = %d\n", thread_count);

  for (int i = 0; i < thread_count; i++) {
    list_node * l = list_front(all_threads_list);
    stats->tstats[i].tid = l->thread->tid;
    stats->tstats[i].turnaround_time = l->finish - l->arival + 1;
    int first_cpu_wait = l->first_cpu_burst_start - l->arival;
    int first_io_wait = l->io_burst_start - l->first_cpu_burst_end - 1;
    int second_cpu_wait = l->second_cpu_burst_start - l->io_burst_end - 1;
    if (l->io_burst_start == 0) {
      first_io_wait = 0;
      second_cpu_wait = 0;
    }
    stats->tstats[i].waiting_time = first_cpu_wait + first_io_wait + second_cpu_wait;
    // printf("\nTHREAD ID= %d\n", l->thread->tid);
    // printf("\nl->finish = %d\n", l->finish);
    // printf("\nl->arival = %d\n", l->arival);
    // printf("\nl->first_cpu_burst_start = %d\n", l->first_cpu_burst_start);
    // printf("\nl->second_cpu_burst_start = %d\n", l->second_cpu_burst_start);
    // printf("\nl->first_cpu_burst_end = %d\n", l->first_cpu_burst_end);
    // printf("\nl->io_burst_start = %d\n", l->io_burst_start);
    // printf("\nl->io_burst_end = %d\n", l->io_burst_end);
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
