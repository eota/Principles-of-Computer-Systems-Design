#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <list.h>
#include <stdbool.h>

/* Semaphore */
struct semaphore {
    unsigned value; /* Current value */
    struct list waiters; /* List of waiting threads */
    struct list_elem elem; /* condvar waiters list elem  */
};

void semaphore_init(struct semaphore *, unsigned value);
void semaphore_down(struct semaphore *);
bool semaphore_try_down(struct semaphore *);
void semaphore_up(struct semaphore *);
void semaphore_self_test(void);

bool sema_less_func(struct list_elem *a, struct list_elem *b, void *aux);

#endif /* UCSC CSE130 */
