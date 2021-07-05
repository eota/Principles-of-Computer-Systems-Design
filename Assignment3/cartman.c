
#include "cartman.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t sems[5]; // Semaphores for each thread

typedef struct data {
  unsigned int cart;
  enum junction junction;
  enum track track;
} data;

/*
 * You need to implement this function, see cartman.h for details 
 */
void *cartThreadFunc(void *param){
  data* ptr = (data*)param;
  unsigned int cart = ptr->cart;
  enum track track = ptr->track;
  enum junction junction = ptr->junction;
  
  // printf("\ncart = %d\n", cart);
  // printf("\ntrack = %d\n", track);
  // printf("\njunction = %d\n", junction);
  // printf("IN cartThreadFunc for CART: %d\n", cart);
  // printf("junction = %d\n", junction);
  // printf(" AND junction = %d\n", junction+1);
  // printf("\ntrack = %d\n", track);

  if (junction < 4) {
    // printf("IN JUNCTION < 4 PRE WAIT\n");
    sem_wait(&sems[junction]);
    sem_wait(&sems[junction+1]);
    // printf("IN JUNCTION < 4 POST WAIT\n");

    reserve(cart, junction);
    reserve(cart, junction+1);
    // printf("IN JUNCTION < 4 POST RESERVE\n");
    // sem_wait(&sems[junction]);
    // sem_wait(&sems[junction+1]);
  } else {
    sem_wait(&sems[junction]);
    sem_wait(&sems[0]);

    reserve(cart, junction);
    reserve(cart, 0);

    // printf("IN ELSE\n");
    // sem_wait(&sems[junction]);
    // sem_wait(&sems[0]);
  }
  cross(cart, track, junction);
  pthread_exit(NULL);
}

/*
 * You need to implement this function, see cartman.h for details 
 */
void arrive(unsigned int cart, enum track track, enum junction junction) 
{
  // Called when CART arived at a junction
  // 1. Create a thread for the CART
  // 2. Reserve the junctions you need for exclusive access to the
  //    critical section.
  // 3. But must make sure you don't call reserve when the junctions
  //    you need are already reserved by another CART/thread
  // 4. Once you have exclusive access to both junctions,
  //    call cross().

  pthread_t cartThread;

  struct data *cartData = malloc(sizeof *cartData);
  // data cartData;
  cartData->cart = cart;
  cartData->track = track;
  cartData->junction = junction;

  // struct data *dataPtr = malloc(sizeof(data));

  // printf("\ncart = %d\n", cart);
  // printf("\ntrack = %d\n", track);
  // printf("\njunction = %d\n", junction);
  // printf("\ncartData.track = %d\n", cartData->track);


  int rc = pthread_create(&cartThread, NULL, cartThreadFunc, cartData);
  if (rc) {
    fprintf(stderr, "ERROR: pthread_create() returned %d\n", rc);
    exit(EXIT_FAILURE);
  }
  pthread_join(cartThread, NULL);
}

/*
 * You need to implement this function, see cartman.h for details 
 */
void depart(unsigned int cart, enum track track, enum junction junction) 
{
  // Called when CART has reached the other side of a critical section
  // 1. Release the two junctions it reserved

  // printf("Depart(): Junction = %d\n", junction);
  
  // sem_post(&sems[junction]);

  if (junction > 0) {
    // printf("Depart(): junction > 0\n");
    release(cart, junction);
    release(cart, junction-1);

    sem_post(&sems[junction]);
    sem_post(&sems[junction-1]);
  } else {
    // printf("Depart(): junction = 0\n");
    release(cart, junction);
    release(cart, 4);

    sem_post(&sems[junction]);
    sem_post(&sems[4]);
  }
}

/*
 * You need to implement this function, see cartman.h for details 
 */
void cartman() 
{
  // initialize anything before system runs ie. semaphores, locks
    // initialize semaphores
  for (long i = 0; i < 5; i++) {
    sem_init(&sems[i], 0, 1);
  }
}

