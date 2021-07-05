#include "merge.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct data{
  int thread_Id;
  int* arr;
  int left;
  int right;
} data;


/* LEFT index and RIGHT index of the sub-array of ARR[] to be sorted */
void singleThreadedMergeSort(int arr[], int left, int right) 
{
  if (left < right) {
    int middle = (left+right)/2;
    singleThreadedMergeSort(arr, left, middle); 
    singleThreadedMergeSort(arr, middle+1, right); 
    merge(arr, left, middle, right); 
  } 
}

// P splits array in two
// P creates two threads to sort half of the array each
// T1 and T2 each create two threads to sort half of their half
// T1 waits for T3 and T4 to finish
//     Then merges left-left and left-right
// T2 waits for T5 and T6 to finish
//     Then merges right-left and right-right
// P waits for T1 and T2 to finish
// P merges left and right

/* 
 * Called by thread_create
 */
void *sortThread(void *param){
  data* ptr = (data*)param;
  int threadId = ptr->thread_Id;
  srand(threadId);
  int* arr = ptr->arr;
  int left = ptr->left;
  int right = ptr->right;

  // printf("\n");
  // printf("In sortThread: ");
  // printf("%d", threadId);
  // printf("\n");

  // printf("Before sort:");
  // for (int j = 0; j < (right-left)+1; j++) {
  //   printf("%d", arr[left+j]);
  //   printf(", ");
  // }
  // printf("\n");

  //Sort part of array
  singleThreadedMergeSort(arr, left, right);

  // printf("After sort: ");
  // for (int j = 0; j < (right-left)+1; j++) {
  //   printf("%d", arr[left+j]);
  //   printf(", ");
  // }
  // printf("\n");

  //Exit
  pthread_exit(NULL);
}

/* 
 * Called by thread_create
 */
void *mergeThread(void *param){
  data* ptr = (data*)param;
  int threadId = ptr->thread_Id;
  srand(threadId);
  int middle = (ptr->left+ptr->right)/2;
  int* arr = ptr->arr;
  int left = ptr->left;
  int right = ptr->right;

  pthread_t threadsList[2];

  // printf("\n");
  // printf("In mergeThread: ");
  // printf("%d", threadId);
  // printf("\n");

  data thread_data[2];
  thread_data[0].thread_Id = ptr->thread_Id+2;
  thread_data[0].arr = ptr->arr;
  thread_data[0].left = ptr->left;
  thread_data[0].right = middle;
  
  thread_data[1].thread_Id = ptr->thread_Id+4;
  thread_data[1].arr = ptr->arr;
  thread_data[1].left = middle+1;
  thread_data[1].right = ptr->right;
  
  // create two threads to sort the half of the half array each
  for (long i = 0; i < 2; i++) {

    int rc = pthread_create(&threadsList[i], NULL, sortThread, &thread_data[i]);

    if (rc) {
      fprintf(stderr, "ERROR: pthread_create() returned %d\n", rc);
      exit(EXIT_FAILURE);
    }
  }

  //Merge threads
  pthread_join(threadsList[0],NULL);
  pthread_join(threadsList[1],NULL);

  merge(arr, left, middle, right);

  // printf("\nHalf merged\n");
  // for (int j = 0; j < (right-left)+1; j++) {
  //   printf("\n");
  //   printf("%d", arr[left+j]);
  // }

  //Exit
  pthread_exit(NULL);
}

/* 
 * This function stub needs to be completed
 */
void multiThreadedMergeSort(int arr[], int left, int right) 
{
  // Your code goes here
  int middle = (left+right)/2;
  pthread_t threads[6];

  // printf("\n");
  // printf("In multiThreadedMergeSort: ");
  // printf("\n");
  // printf("Left: ");
  // printf("%d", left);
  // printf("\n");
  // printf("Right: ");
  // printf("%d", right);
  // printf("\n");

  data thread_data[2];
  thread_data[0].thread_Id = 0;
  thread_data[0].arr = arr;
  thread_data[0].left = left;
  thread_data[0].right = middle;
  
  thread_data[1].thread_Id = 1;
  thread_data[1].arr = arr;
  thread_data[1].left = middle+1;
  thread_data[1].right = right;

  for (long i = 0; i < 2; i++) {

    int rc = pthread_create(&threads[i], NULL, mergeThread, &thread_data[i]);

    if (rc) {
      fprintf(stderr, "ERROR: pthread_create() returned %d\n", rc);
      exit(EXIT_FAILURE);
    }
  }

  pthread_join(threads[0],NULL);
  pthread_join(threads[1],NULL);

  merge(arr, left, middle, right);

}