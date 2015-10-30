/* parser.c - Parser program for use in interpreter project.                 */
/* By Tore Banta                                                             */

#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Adds a token to a parse tree
Value *addToParseTree(Value *tree, int *depth, Value *token) {
    // If token is an open paren, increments depth
    if ((*token).type == OPEN_TYPE) {
        tree = cons(token, tree);
        (*depth)++;
        return tree;
    }
    else {
        tree = cons(token, tree);
        return tree;
    }
}

// Prints error messages and exits for the two syntax error cases
void syntaxError(int case_num) {
    if (case_num == 1) {
        printf("Syntax error: too many close parentheses.\n");
        texit(1);
    }
    else if (case_num == 2) {
        printf("Syntax error: not enough close parentheses.\n");
        texit(1);
    }
}

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
    // Makes an empty linked list to contain parse trees
    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    while ((*current).type != NULL_TYPE) {
        // Adds tokens to parse tree until reach a close paren
        if ((*car(current)).type != CLOSE_TYPE) {
            Value *token = car(current);
            tree = addToParseTree(tree, &depth, token);
            current = cdr(current);
        }
        else {
            // In which case a list is make to add to tree, and iterates
            // through the tree, which behaves like a stack
            Value *cur_node = tree;
            Value *list_to_add = makeNull();
            // Adds tokens in tree to list being added to tree until an open
            // paren is reached
            while ((*cur_node).type != NULL_TYPE &&
                   (*car(cur_node)).type != OPEN_TYPE) {
                Value *token = car(cur_node);
                list_to_add = cons(token, list_to_add);
                cur_node = cdr(cur_node);
            }
            // If it reaches bottom of tree stack without hitting an open paren
            // throws appropriate syntax error
            if ((*cur_node).type == NULL_TYPE) {
                syntaxError(1);
            }
            // Updates tree to get rid of tokens that have been listified, and
            // adds list to tree, decrementing depth
            tree = cdr(cur_node);
            tree = cons(list_to_add, tree);
            depth--;
            current = cdr(current);
        }
    }
    
    // Depth should always be at 0 when returning
    if (depth != 0) {
        syntaxError(2); // error case 2
    }
    
    return reverse(tree);
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree(Value *tree) {
    Value *cur_node = tree;
    while ((*cur_node).type != NULL_TYPE) {
        // If statement to print nested parse trees
        if ((*car(cur_node)).type == CONS_TYPE) {
            printf("(");
            printTree(car(cur_node));
            printf(")");
        }
        // If statement to print empty lists
        else if ((*car(cur_node)).type == NULL_TYPE) {
            printf("()");
        }
        // Two sets of switch statements for printing with or without a space
        else if ((*cdr(cur_node)).type == NULL_TYPE) {
            Value *car_val = car(cur_node);
            int car_type = (*car_val).type;
            
            switch (car_type) {
                case BOOL_TYPE:
                    if ((*car_val).i == 0) {
                        printf("#f");
                    }
                    else {
                        printf("#t");
                    }
                    break;
                case INT_TYPE:
                    printf("%i", (*car_val).i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f", (*car_val).d);
                    break;
                case STR_TYPE:
                    printf("\"%s\"", (*car_val).s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s", (*car_val).s);
                    break;
            }
        }
        else {
            Value *car_val = car(cur_node);
            int car_type = (*car_val).type;
            
            switch (car_type) {
                case BOOL_TYPE:
                    if ((*car_val).i == 0) {
                        printf("#f ");
                    }
                    else {
                        printf("#t ");
                    }
                    break;
                case INT_TYPE:
                    printf("%i ", (*car_val).i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f ", (*car_val).d);
                    break;
                case STR_TYPE:
                    printf("\"%s\" ", (*car_val).s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s ", (*car_val).s);
                    break;
            }
        }
        cur_node = cdr(cur_node);
    }
}