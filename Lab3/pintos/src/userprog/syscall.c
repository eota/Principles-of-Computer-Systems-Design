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
 * Modifications Copyright (C) 2017-2018 David C. Harrison. 
 * All rights reserved.
 */

#include <stdio.h>
#include <syscall-nr.h>
#include <list.h>

#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "userprog/umem.h"

static void syscall_handler(struct intr_frame *);

static void write_handler(struct intr_frame *);
static void exit_handler(struct intr_frame *);

static void read_handler(struct intr_frame *);
static void create_handler(struct intr_frame *);
static void open_handler(struct intr_frame *);
static void close_handler(struct intr_frame *);
static void exec_handler(struct intr_frame *);
static void wait_handler(struct intr_frame *);
static void filesize_handler(struct intr_frame *);

int openFds[1024];
int fdIndex = 0;
void *file_array[1024];

// Resources: Lecture Secret Sauce

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler(struct intr_frame *f)
{
  int syscall;
  ASSERT( sizeof(syscall) == 4 ); // assuming x86

  // The system call number is in the 32-bit word at the caller's stack pointer.
  umem_read(f->esp, &syscall, sizeof(syscall));

  // Store the stack pointer esp, which is needed in the page fault handler.
  // Do NOT remove this line
  thread_current()->current_esp = f->esp;

  switch (syscall) {
  case SYS_HALT: 
    shutdown_power_off();
    break;

  case SYS_EXIT: 
    exit_handler(f);
    break;
      
  case SYS_WRITE: 
    write_handler(f);
    break;

  case SYS_CREATE:
    create_handler(f);
    break;

  case SYS_READ:
    read_handler(f);
    break;
  
  case SYS_OPEN:
    open_handler(f);
    break;
  
  case SYS_CLOSE:
    close_handler(f);
    break;
  
  case SYS_EXEC:
    exec_handler(f);
    break;

  case SYS_WAIT:
    wait_handler(f);
    break;

  case SYS_FILESIZE:
    filesize_handler(f);
    break;

  default:
    printf("[ERROR] system call %d is unimplemented!\n", syscall);
    thread_exit();
    break;
  }
}

/****************** System Call Implementations ********************/

void sys_exit(int status) 
{
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}

static void exit_handler(struct intr_frame *f) 
{
  int exitcode;
  umem_read(f->esp + 4, &exitcode, sizeof(exitcode));

  sys_exit(exitcode);
}

/*
 * BUFFER+0 and BUFFER+size should be valid user adresses
 */
static uint32_t sys_write(int fd, const void *buffer, unsigned size)
{
  umem_check((const uint8_t*) buffer);
  umem_check((const uint8_t*) buffer + size - 1);

  int ret = -1;

  if (fd == 1) { // write to stdout
    putbuf(buffer, size);
    ret = size;
  } else {
    // struct file *f = filesys_open("test.txt");
    struct file *f = file_array[fd];
    if (f == NULL) {
      return 0;
    }
    ret = file_write(f, buffer, size);
  }

  // ret = 0; // write zero

  return (uint32_t) ret;
}

static void write_handler(struct intr_frame *f)
{
    int fd;
    const void *buffer;
    unsigned size;

    umem_read(f->esp + 4, &fd, sizeof(fd));
    umem_read(f->esp + 8, &buffer, sizeof(buffer));
    umem_read(f->esp + 12, &size, sizeof(size));

    f->eax = sys_write(fd, buffer, size);
}

// READ
static uint32_t sys_read(int fd, void *buffer, unsigned size)
{
  umem_check((const uint8_t*) buffer);
  umem_check((const uint8_t*) buffer + size - 1);

  int ret = -1;
  
  // struct file *f = filesys_open("sample.txt");
  struct file *f = file_array[fd];
  if (f == NULL) {
    return 0;
  }
  ret = file_read(f, buffer, size);

  file_close(f);

  return (uint32_t) ret;
}

static void read_handler(struct intr_frame *f)
{
    int fd;
    void *buffer;
    unsigned size;

    umem_read(f->esp + 4, &fd, sizeof(fd));
    umem_read(f->esp + 8, &buffer, sizeof(buffer));
    umem_read(f->esp + 12, &size, sizeof(size));

    f->eax = sys_read(fd, buffer, size);
}

// CREATE
static uint32_t sys_create(char* fname, int isize)
{
  bool success = filesys_create(fname, isize, false);

  return success;
}

static void create_handler(struct intr_frame *f)
{
  char* fname;
  int isize;

  umem_read(f->esp + 4, &fname, sizeof(fname));
  umem_read(f->esp + 8, &isize, sizeof(isize));

  f->eax = sys_create(fname, isize);
}

// OPEN
static uint32_t sys_open(char* fname)
{
  struct file *f = filesys_open(fname);
  if (f == NULL) {
    return -1;
  }
  int ret = openFds[fdIndex] = fdIndex + 3;
  fdIndex++;
  file_array[ret] = f;
  return ret;
}

static void open_handler(struct intr_frame *f)
{
    char* fname;

    umem_read(f->esp + 4, &fname, sizeof(fname));

    f->eax = sys_open(fname);
}

// CLOSE
static uint32_t sys_close(int fd)
{
  //struct file *f = filesys_open("sample.txt");
  struct file *f = file_array[fd];
  if (f == NULL) {
    return -1;
  }
  file_close(f);
  return 0;
}

static void close_handler(struct intr_frame *f)
{
    int fd;

    umem_read(f->esp + 4, &fd, sizeof(fd));

    f->eax = sys_close(fd);
}

// EXEC
static uint32_t sys_exec(char* name)
{
  printf("\nSYS_EXEC(): name = %s\n", name);
  // tid_t tid = process_execute(name);

  // printf("\nTID = %d\n", tid);

  return 0; // tid;
}

static void exec_handler(struct intr_frame *f)
{
    char* name;

    umem_read(f->esp + 4, &name, sizeof(name));

    f->eax = sys_exec(name);
}

// WAIT
static uint32_t sys_wait(tid_t tid)
{
  printf("\nSYS_WAIT(): tid = %d\n", tid);
  // int ret = process_wait(tid);

  return 0; // ret;
}

static void wait_handler(struct intr_frame *f)
{
    tid_t tid;

    umem_read(f->esp + 4, &tid, sizeof(tid));

    f->eax = sys_wait(tid);
}

// FILESIZE
static uint32_t sys_filesize(int fd)
{
  struct file *f = file_array[fd];
  int ret = file_length(f);
  // printf("\nfd = %d\n", fd);

  return ret;
}

static void filesize_handler(struct intr_frame *f)
{
    int fd;

    umem_read(f->esp + 4, &fd, sizeof(fd));

    f->eax = sys_filesize(fd);
}

