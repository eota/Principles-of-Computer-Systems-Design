
/*********************************************************************
 *
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

#include "manpage.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t sems[7]; // Semaphores for each thread

/*
 * This function is run by each of the threads.
 * 
 * 1. Get paragraph id
 * 2. if (pid > 0)
 *     wait for semaphore for paragraph i - 1 to go up
 * 3. Show paragraph
 * 4. if (pid < 6)
 *     put semaphore for paragraph i up
 */
void *thread_func(void *param) {
  int pid = getParagraphId();
  if (pid > 0) {
    sem_wait(&sems[pid-1]);
    // printf("sem_wait for %d\n", pid);
    // printf("sem[%d] = %d", pid, sems[pid]);
  }
  showParagraph();
  if (pid < 6) {
    sem_post(&sems[pid]);
    // printf("sem_post for %d\n", pid);
  }
  pthread_exit(NULL);
}

/*
 * See manpage.h for details.
 *
 * As supplied, shows random single messages.
 */
void manpage() 
{
  pthread_t threadsList[7];

  // initialize semaphores
  for (long i = 0; i < 7; i++) {
    sem_init(&sems[i], 0, 0);
  }

  // create threads
  for (long i = 0; i < 7; i++) {
    int rc = pthread_create(&threadsList[i], NULL, thread_func, NULL);

    if (rc) {
      fprintf(stderr, "ERROR: pthread_create() returned %d\n", rc);
      exit(EXIT_FAILURE);
    }
  }

  // wait for threads to finish, destroy semaphores
  for (long i = 0; i < 7; i++) {
    pthread_join(threadsList[i], NULL);
    sem_destroy(&sems[i]);
  }

}
