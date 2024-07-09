#include <stddef.h>

#include "flist.h"
#include "syscall.h"


/**
 * Initialises array elements to NULL and 
 * initialises num_open_places member 
 */
void flist_init(struct flist *fl)
{
    /* Set each pointer to NULL */
    for (unsigned i = 0; i < FLIST_MAX_LENGTH; i++)
        fl->content[i] = NULL;

    /* Set num_open_places to size of the array - 2
       Reserve the first 2 indices for STDIN, STDOUT. */
    fl->num_open_places = FLIST_MAX_LENGTH - 2;
}


/**
 * Inserts a value into an available location in the map.
 * Returns the key (index) if insertion was successful
 * or -1 if nothing was done.
 */
key_t flist_insert (struct flist *fl, value_t v)
{
    /* If there are no open spaces, return -1 and get out */
    if (0 == fl->num_open_places)
      return -1;

    /* Initialize an insert index */ 
    key_t insert_at = FLIST_MAX_LENGTH - fl->num_open_places;
    
    /* Make sure the index to insert at is NULL */
    while (fl->content[insert_at] != NULL) 
    {
        insert_at++;

        /* Don't wander off the array */
        if (FLIST_MAX_LENGTH == insert_at)
            insert_at = 2;
    }
    
    /* Insert the value, decrease the number of open spaces */
    fl->content[insert_at] = v;
    fl->num_open_places--;
    /* Return the key where the value was inserted */ 
    return insert_at;
}


/**
 * Find the value stored at the given key.
 * Return the value if found, return NULL if not found 
 * or key was out of bounds.
 * */
value_t flist_find (struct flist *fl, key_t k)
{
    if (k >= FLIST_MAX_LENGTH || k < 0)
        return NULL;
    
    value_t value = fl->content[k]; 
    return value; 
}


/**
 * Find and remove a value stored at a given key.
 * Returns the value if it was removed, NULL if
 * not.
 * */
value_t flist_remove (struct flist *fl, key_t k)
{
  value_t value = flist_find(fl, k);

  if (value != NULL)
  {
      fl->content[k] = NULL;
      fl->num_open_places++;
  }

  return value;
}


/**
 * Apply the function exec to each element in the array.
 * */
void flist_for_each (struct flist *fl, void (*exec)(key_t k, value_t v, int aux), int aux)
{
  for (unsigned i = 2; i < FLIST_MAX_LENGTH; i++)
      exec(i, fl->content[i], aux);
}


/**
 * For each element of the array, if pred evaluates to true, remove the element
 * from the array.
 * */
void flist_remove_if(struct flist *fl, bool (*pred)(key_t k, value_t v, int aux), int aux)
{
  for (unsigned i = 2; i < FLIST_MAX_LENGTH; i++)
  {
      if ( fl->content[i] != NULL && pred(i, fl->content[i], aux))
      {
        fl->content[i] = NULL;
        ++fl->num_open_places;
      }          
  }
}


/**
 * Close all files in the file table
 */
void flist_close_all (void)
{
  for (unsigned i = 2; i < FLIST_MAX_LENGTH; i++)
    close (i);
}
