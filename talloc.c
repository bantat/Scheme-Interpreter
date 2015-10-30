/* talloc.c - Alternate malloc implementation for use in interpreter project */
/* By Tore Banta & Charlie Sarano                                            */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "value.h"

// Uninitialized pointers to front and back of list of talloc'd pointers
Value *pointers = NULL;
Value *pointers_copy;

// Create a new CONS_TYPE value node.
Value *cons_cell(Value *car, Value *cdr) {
    Value *cons_node = malloc(sizeof(Value));
    (*cons_node).type = CONS_TYPE;
    struct ConsCell cons_cell;
    cons_cell.car = car;
    cons_cell.cdr = cdr;
    (*cons_node).c = cons_cell;
    
    return cons_node;
}

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size) {
    if (pointers == NULL) {
        // If pointers list uninitialized, initializes...
        pointers = malloc(sizeof(Value));
        // Copy points to back of linked list for purpose of freeing later
        pointers_copy = pointers;
        
        (*pointers).type = CONS_TYPE;
        
        Value *null_val = malloc(sizeof(Value));
        (*null_val).type = NULL_TYPE;
        
        // Value created to store pointer member p
        Value *ptr_storage = malloc(sizeof(Value));
        void *ptr = malloc(size);
        (*ptr_storage).p = ptr;
        
        // Cons with null Value node to start list
        pointers = cons_cell(ptr_storage, null_val);
        
        return ptr;
    }
    else {
        // Otherwise, just creates pointer storage Value node to cons with list
        Value *ptr_storage = malloc(sizeof(Value));
        void *ptr = malloc(size);
        struct ConsCell pointer_cell;
        (*ptr_storage).type = PTR_TYPE;
        (*ptr_storage).p = ptr;
        
        pointers = cons_cell(ptr_storage, pointers);

        return ptr;
    }
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree() {
    Value *cur_node = pointers;
    // Iterate through linked list...
    while ((*cur_node).type != NULL_TYPE) {
        struct ConsCell cons_cell = (*cur_node).c;
        Value *car_val = cons_cell.car;
        Value *cdr_val = cons_cell.cdr;

        // First freeing pointer member vars, and car Value storing it
        void *ptr = (*car_val).p;
        free(ptr);
        free(car_val);
        // and then using a temp pointer to free Value containing ConsCell
        Value *temp_ptr = cur_node;
        cur_node = cdr_val;
        free(temp_ptr);
    }
    // Free last NULL_TYPE Value in linked list
    free(cur_node);
    // Reset list of pointers to be null, as it is now empty
    pointers = NULL;
    // Frees pointer to back of list that would have been inaccessible
    free(pointers_copy);
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status) {
    tfree();
    exit(status);
}