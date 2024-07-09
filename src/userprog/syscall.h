#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <debug.h>
#include <stdbool.h>
#include "threads/synch.h"
#include "plist.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define MAX_SIZE_FILENAME 14

void syscall_init (void);
void halt (void) NO_RETURN;
void exit (int) NO_RETURN;
pid_t exec (const char *);
int wait (pid_t);
void sleep(int);
void plist (void);
bool create (const char *, unsigned);
bool remove (const char *);
int open (const char *);
int filesize(int);
int read (int, void *, unsigned);
int write (int, const void *, unsigned);
void seek (int, unsigned);
unsigned tell (int);
void close (int);

#endif /* userprog/syscall.h */
