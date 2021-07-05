/************************************************************************
 * 
 * CSE130 Winter 2021 Assignment 1
 * 
 * UNIX Shared Memory Multi-Process Merge Sort
 * 
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ************************************************************************/

#include "merge.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* LEFT index and RIGHT index of the sub-array of ARR[] to be sorted */
void singleProcessMergeSort(int arr[], int left, int right) 
{
  if (left < right) {
    int middle = (left+right)/2;
    singleProcessMergeSort(arr, left, middle); 
    singleProcessMergeSort(arr, middle+1, right); 
    merge(arr, left, middle, right); 
  } 
}

/* 
 * This function stub needs to be completed
 */
void multiProcessMergeSort(int arr[], int left, int right) 
{
  // Your code goes here
  int middle = (left+right)/2;
  // Create shared memory
  // Attach to shared memory
  int shmid = shmget(IPC_PRIVATE, 1024, 0666|IPC_CREAT);
  int *shm = (int *)shmat(shmid,(void*)0,0);

  // Copy local memory (the array) into shared memory
  // void *memcpy(void *dest, const void * src, size_t n)
  // from https://www.tutorialspoint.com/c_standard_library/c_function_memcpy.htm
  memcpy(shm, arr, (right+1)*sizeof(int));

  // for (int i = 0; i < (right+1); i++) {
  //   printf("shm[i] = ");
  //   printf("%d ", shm[i]);
  //   printf("\n");
  // }

  // Fork
  switch (fork()) {
    case -1:
      exit(-1);
    case 0:
      // if child
      // attach to shared memory
      shm = (int*) shmat(shmid,(void*)0,0);
      //sort one side of shared memory
      singleProcessMergeSort(shm, left, middle);
      //detach from shared memory
      shmdt(shm);
      exit(0);
    default:
      // sort other side of shared memory
      singleProcessMergeSort(shm, middle+1, right);
      // wait for child to finish
      wait(NULL);
      // merge shared memory
      merge(shm, left, middle, right);
      // copy shared memory to local memory (ie back into given array)
      for (int j = 0; j < (right+1); j++) {
        arr[j] = shm[j];
        // printf("Value in arr[j] = ");
        // printf("%d", arr[j]);
        // printf("\n");
      }
      // detach from shared memory
      // destroy shared memory
      shmdt(shm);
      shmctl(shmid, IPC_RMID,NULL);
  }  
}
