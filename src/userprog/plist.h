#include <stdbool.h>

#ifndef _PLIST_H_
#define _PLIST_H_

#include "threads/synch.h"

/* Place functions to handle a running process here (process list).

   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.

   - A function that print the entire content of the list in a nice,
     clean, readable format.

 */

#define PLIST_MAX_LENGTH 255  
#define NAME_LENGTH 15

typedef struct process *process_t;
typedef int pid_t;


struct process
{                                   
  struct semaphore sema_p;
  struct semaphore sema_p_wait;     
  char name[NAME_LENGTH + 1];       
  unsigned tid;                     
  int exit_status;                  
  unsigned char pid_parent;         
  unsigned is_alive     :1;         
  unsigned parent_alive :1;         
};


struct process_list
{
  process_t content[PLIST_MAX_LENGTH];
  pid_t num_open_spaces;
  pid_t open_slot;
};

/* Global process list and rw lock for process list */
extern struct process_list p_list;
extern struct rw_lock lock_plist_rw;


/* Process functions */
process_t process_create (unsigned, char *, pid_t); //, pid_t);
void process_destroy (process_t);

/* Process list functions */
void plist_init (void);
pid_t plist_insert (process_t);
process_t plist_find (pid_t);
process_t plist_remove (pid_t);
void plist_print (void);
pid_t plist_get_proc_from_tid (unsigned);
pid_t plist_open_slot (void);

#endif
