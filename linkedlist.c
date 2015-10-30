/* linkedlist.c - Linked list implementation for use in interpreter project */
/* By Tore Banta & Charlie Sarano                                           */

#include <stdbool.h>
#include "talloc.h"
#include "value.h"
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"

// Create a new NULL_TYPE value node.
Value *makeNull() {
    Value *null_node = talloc(sizeof(Value));
    (*null_node).type = NULL_TYPE;
    
    return null_node;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr) {
    Value *cons_node = talloc(sizeof(Value));
    (*cons_node).type = CONS_TYPE;
    struct ConsCell cons_cell;
    cons_cell.car = car;
    cons_cell.cdr = cdr;
    (*cons_node).c = cons_cell;
    
    return cons_node;
}

// Display the contents of the linked list to the screen in some kind of readable format
void display(Value *list) {
    if ((*list).type == NULL_TYPE) {
        printf("The list is empty.\n");
    }
    
    else {
        // Create cur_node pointer to iterate through linked list
        Value *cur_node = list;
        printf("{");
        while ((*cur_node).type != NULL_TYPE) {
            // Creates ConsCell in stack to determine car type
            struct ConsCell cons_cell = (*cur_node).c;
            Value *car_val = cons_cell.car;
            Value *cdr_val = cons_cell.cdr;
            
            int car_type = (*car_val).type;
            
            // Performs appropriate print statement based on car's type
            switch (car_type) {
                case INT_TYPE:
                    printf("%i, ", (*car_val).i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f, ", (*car_val).d);
                    break;
                case STR_TYPE:
                    printf("%s, ", (*car_val).s);
                    break;
                case CONS_TYPE:
                    printf("\nInvalid LinkedList structure. Exiting.\n");
                    exit(1);
                case NULL_TYPE:
                    break;
            }
            
            cur_node = cdr_val;
        }
        printf("}\n");
    }
}

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list) {
    // Returns the same empty list if only entry is null
    if ((*list).type == NULL_TYPE) {
        return list;
    }
    
    else {
        // Makes an empty node to be end of reversed version
        Value *reversed_list = makeNull();
        Value *cur_node = list;
        
        // Iterates through linked list...
        while ((*cur_node).type != NULL_TYPE) {
            // Creating ConsCells to access Value pointers for car and cdr
            struct ConsCell cons_cell = (*cur_node).c;
            Value *car_val = cons_cell.car;
            Value *cdr_val = cons_cell.cdr;
            
            // Cons new Value in order of iteration, effectively reversing order
            reversed_list = cons(car_val, reversed_list);
            cur_node = cdr_val;
            
        }
        return reversed_list;
    }
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list) {
    // Confirms appropriate structure for list
    assert(list != NULL);
    assert((*list).type == CONS_TYPE);
    struct ConsCell cons_cell = (*list).c;
    Value *car_val = cons_cell.car;
    
    return car_val;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list) {
    // Confirms appropriate structure for list
    assert(list != NULL);
    assert((*list).type == CONS_TYPE);
    struct ConsCell cons_cell = (*list).c;
    Value *cdr_val = cons_cell.cdr;
    
    return cdr_val;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
    // Confirms appropriate structure for list
    assert(value != NULL);
    
    if ((*value).type == NULL_TYPE) {
        return 1;
    }
    
    else {
        return 0;
    }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value) {
    // Confirms appropriate structure for list
    assert(value != NULL);
    
    // Checks front of list and returns 0 if the list is NULL
    if ((*value).type == NULL_TYPE) {
        return 0;
    }
    
    else {
        int length = 0;
        Value *cur_node = value;
        
        // Iterates through list and adds to length for every car that isn't empty
        while ((*cur_node).type != NULL_TYPE) {
            struct ConsCell cons_cell = (*cur_node).c;
            Value *cdr_val = cons_cell.cdr;
            
            cur_node = cdr_val;
            length++;
        }
        
        return length;
    }
}