/* 
 * This file is derived from source code for the Pintos
 * instructional operating system which is itself derived
 * from the Nachos instructional operating system. The 
 * Nachos copyright notice is reproduced in full below. 
 *
 * Copyright (C) 1992-1996 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose, without fee, and
 * without written agreement is hereby granted, provided that the
 * above copyright notice and the following two paragraphs appear
 * in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE
 * AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 *
 * Modifications Copyright (C) 2017 David C. Harrison. All rights reserved.
 */

#include <stdio.h>
#include <string.h>

#include "threads/semaphore.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* 
 * Initializes semaphore SEMA to VALUE.  A semaphore is a
 * nonnegative integer along with two atomic operators for
 * manipulating it:
 *
 * - down or Dijkstra's "P": wait for the value to become positive, 
 * 	then decrement it.
 * - up or Dijkstra's "V": increment the value(and wake up one waiting 
 * 	thread, if any). 
 */
void
semaphore_init(struct semaphore *sema, unsigned value)
{
    ASSERT(sema != NULL);

    sema->value = value;
    list_init(&sema->waiters);
}

/*
 * typedef bool list_less_func for list_insert_ordered()
 * returns true if list_elem a's priority > b's priority
 *
 * Resources used: 
 * https://jeason.gitbooks.io/pintos-reference-guide-sysu/content/list.html
 */
bool
sema_less_func(struct list_elem *a, struct list_elem *b, void *aux) {

    bool result = list_entry(a,struct thread,elem)->priority >
        list_entry(b,struct thread,elem)->priority;
    
    return result;
}

/* 
 * Down or Dijkstra's "P" operation on a semaphore.  Waits for SEMA's 
 * value to become positive and then atomically decrements it.
 *
 * This function may sleep, so it must not be called within an
 * interrupt handler.  This function may be called with
 * interrupts disabled, but if it sleeps then the next scheduled
 * thread will probably turn interrupts back on. 
 */
void
semaphore_down(struct semaphore *sema)
{
    ASSERT(sema != NULL);
    ASSERT(!intr_context());

    enum intr_level old_level = intr_disable();
    while (sema->value == 0) {
        // list_push_back(&sema->waiters, &thread_current()->elem);
        // priority-sema: insert into waiters list ordered by priority
        list_insert_ordered(&sema->waiters, &thread_current()->elem, sema_less_func, NULL);
        thread_block();
    }
    sema->value--;
    intr_set_level(old_level);
}

/* 
 * Down or Dijkstra's "P" operation on a semaphore, but only if the
 * semaphore is not already 0.  Returns true if the semaphore is
 * decremented, false otherwise.
 *
 * This function may be called from an interrupt handler. 
 */
bool
semaphore_try_down(struct semaphore *semaphore)
{
    ASSERT(semaphore != NULL);

    bool success = false;
    enum intr_level old_level = intr_disable();
    if (semaphore->value > 0) {
        semaphore->value--;
        success = true;
    } else {
        success = false;
    }
    intr_set_level(old_level);

    return success;
}

/* 
 * Up or Dijkstra's "V" operation on a semaphore.  Increments SEMA's value
 * and wakes up one thread of those waiting for SEMA, if any.
 *
 * This function may be called from an interrupt handler. 
 */
void
semaphore_up(struct semaphore *semaphore)
{
    enum intr_level old_level;

    ASSERT(semaphore != NULL);

    old_level = intr_disable();
    if (!list_empty(&semaphore->waiters)) {
        thread_unblock(list_entry(
            list_pop_front(&semaphore->waiters), struct thread, elem));
    }
    semaphore->value++;
    // priority-sema: yield thread after incrementing semaphore
    thread_yield();
    intr_set_level(old_level);
}
