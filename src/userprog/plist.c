#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "threads/malloc.h"
#include "threads/thread.h"
#include "plist.h"

/* The global process list */
struct process_list p_list;
struct rw_lock lock_plist_rw;


/****************************** process functions *************************************/


/* Allocates and initializes a process struct and returns a pointer to 
 * the process struct
 */
process_t process_create (unsigned tid, char *p_name, pid_t parent) // pid_t me, pid_t parent)
{
  process_t p = malloc (sizeof (struct process));
  
  /* Don't try and initialize if allocation failed */
  if (p != NULL)
  {
    strlcpy_first_word (p->name, p_name, NAME_LENGTH + 1);
    p->pid_parent = parent;
    p->tid = tid;
    p->is_alive = 1;
    p->parent_alive = 1;

    /* Default */
    p->exit_status = -1; 
    
    sema_init (&p->sema_p_wait, 0);
    sema_init (&p->sema_p, 1);
  }
  return p;
}


/**
 * Free the memory for a process 
 */
void
process_destroy (process_t p)
{
  free (p);
}


/******************** plist functions *********************************/

/**
 * Initialises array elements to NULL and 
 * initialises num_open_places member
 */
void
plist_init (void)
{
  for (unsigned i = 0; i< PLIST_MAX_LENGTH; i++)
    p_list.content[i] = NULL;

  p_list.num_open_spaces = PLIST_MAX_LENGTH; 
  p_list.open_slot = 0;
}


/**
 * Inserts a value into an available location in the map.
 * Returns the process id of the inserted process which is the index
 * where the process * was inserted if insertion was successful or -1 
 * if nothing was done.
 */
pid_t
plist_insert (process_t p)
{
  /* If there are no open spaces, return -1 and get out */
  if (0 == p_list.num_open_spaces) 
    return -1;

  /* Initialize an insert index */    
  pid_t insert_at = p_list.open_slot; 
  p_list.content[insert_at] = p; 
  p_list.num_open_spaces--;

  /* If we are now full */
  if (0 == p_list.num_open_spaces)
  {
    p_list.open_slot = -1;
    return insert_at;
  } 

  /* If we are not full, find the next open space */
  while (p_list.content[p_list.open_slot] != NULL) 
  {
    p_list.open_slot++;

    /* Don't wander off the array */
    if (p_list.open_slot == PLIST_MAX_LENGTH)
        p_list.open_slot = 0;
  }
  
  return insert_at;
}


/**
 * Find the process with the given pid.
 * Return the value if found, return NULL if not found 
 * or key was out of bounds.
 * */
process_t 
plist_find (pid_t p)
{
  if (p >= PLIST_MAX_LENGTH || p < 0)
    return NULL;

  process_t process;
  process = p_list.content[p];
  
  return process;
}


/**
 * Find and remove a process with the given pid_t p.
 * Returns the value if it was removed, NULL if
 * not.
 * */
process_t 
plist_remove (pid_t p)
{
  process_t value = plist_find (p);
  
  if (value != NULL)
  {
    p_list.content[p] = NULL;
    p_list.open_slot = p;
    p_list.num_open_spaces++;
  }
  
  return value;
}


/* Nicely prints the process list */
void 
plist_print (void)
{
  printf ("|-----------------------------------------"
          "--Process List-----------------------------------------|\n");
  
  for (unsigned i = 0; i < PLIST_MAX_LENGTH; i++)
  {
    if (p_list.content[i] != NULL)
    {
      printf("| PROC-> name: %-14s | pid: %-5d | parent: %-5d | "
             "alive: %d | p_alive: %d | exit: %-5d |\n",
             p_list.content[i]->name, i,
             p_list.content[i]->pid_parent,
             p_list.content[i]->is_alive,
             p_list.content[i]->parent_alive,
             p_list.content[i]->exit_status);
    }
  }
  
  printf ("|-----------------------------------------"
          "-------------------------------------------------------|\n");
}  


/* Return the pid of the process associated with the given tid */
pid_t
plist_get_proc_from_tid (unsigned tid)
{
  /* If the tid is for the idle thread, return PLIST_MAX_LENGTH */
  if (1 == tid)
    return PLIST_MAX_LENGTH;
  
  for (unsigned i = 0; i < PLIST_MAX_LENGTH; i++)
  {
    if (p_list.content[i] != NULL && 
        p_list.content[i]->tid == tid)
      return i;
  }
  
  /* Did not find any process with the given thread id */
  return -1; 
}


/* Return a current open_slot in p_list */
pid_t
plist_open_slot (void)
{
  return p_list.open_slot;
}
