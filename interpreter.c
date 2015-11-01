/* interpreter.c - Program for interpreting Scheme code using C              */
/* By Tore Banta & Charlie Sarano                                            */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "linkedlist.h"
#include "talloc.h"
#include "interpreter.h"
#include "value.h"
#include "parser.h"

void printVal(Value *input) {
    switch (input->type) {
        case CONS_TYPE:
            printf("(");
            printVal(car(input));
            printf(", ");
            printVal(cdr(input));
            printf(")");
            break;
        case BOOL_TYPE:
            if (input->i == 0) {
                printf("#f:boolean");
            }
            else {
                printf("#t:boolean");
            }
            break;
        case INT_TYPE:
            printf("%i:integer", input->i);
            break;
        case DOUBLE_TYPE:
            printf("%f:float", input->d);
            break;
        case STR_TYPE:
            printf("\"%s\":string", input->s);
            break;
        case SYMBOL_TYPE:
            printf("%s:symbol", input->s);
            break;
        case PTR_TYPE:
            printf("%i:pointer", input->p);
            break;
        case NULL_TYPE:
            printf("null");
            break;
    }
}

void interpret(Value *tree) {
    Frame *global = talloc(sizeof(Frame));
    while ((*tree).type != NULL_TYPE) {
        assert((*tree).type == CONS_TYPE);
        Value *expression = car(tree);
        Value *result = eval(expression, global);
        
        switch ((*result).type) {
            case BOOL_TYPE:
                if ((*result).i == 0) {
                    printf("#f\n");
                }
                else {
                    printf("#t\n");
                }
                break;
            case INT_TYPE:
                printf("%i\n", (*result).i);
                break;
            case DOUBLE_TYPE:
                printf("%f\n", (*result).d);
                break;
            case STR_TYPE:
                printf("\"%s\"\n", (*result).s);
                break;
            case SYMBOL_TYPE:
                printf("%s\n", (*result).s);
                break;
        }
        tree = cdr(tree);
    }
}

void evaluationError(int error) {
    printf("Evaluation error: ");
    if (error == 0) {
        printf("Test condition for if statement doesn't resolve solve to a boolean\n");
    }
    else if (error == 1) {
        printf("If statement doesn't contain enough arguments\n");
    }
    else if (error == 2) {
        printf("Parameter for Let must be nested list\n");
    }
    else if (error == 3) {
        printf("Symbol being evaluated doesn't have a bound value\n");
    }
    else if (error == 4) {
        printf("Not a recognized special form\n");
    }
    else if (error == 5) {
        printf("Not a symbol\n");
    }
    texit(1);
}

Value *lookUpSymbol(Value *symbol, Frame *frame) {
    while (frame != NULL) {
        Value *bindings = frame->bindings;
        while (bindings == NULL) {
            if (frame->parent == NULL) {
                evaluationError(3);
            }
            frame = frame->parent;
            bindings = frame->bindings;
        }
        while ((*bindings).type != NULL_TYPE) {
            Value *symbol1_cons = car(bindings);
            Value *symbol1 = car(symbol1_cons);
            assert(symbol1->type == SYMBOL_TYPE);
            
            if (strcmp((*symbol1).s, (*symbol).s) == 0) {
                return car(cdr(symbol1_cons));
            }
            else {
                bindings = cdr(bindings);
            }
        }
        frame = frame->parent;
    }
    evaluationError(3);
    return symbol;
}

Value *evalIf(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE){
        evaluationError(1);
    }
    else {
        if (cdr(args)->type != CONS_TYPE) {
            evaluationError(1);
        }
        else if (cdr(cdr(args))->type != CONS_TYPE) {
            evaluationError(1);
        }
    }
    
    Value *bool_exp = eval(car(args), frame);
    Value *true_result = car(cdr(args));
    Value *false_result = car(cdr(cdr(args)));
    if (bool_exp->type == BOOL_TYPE) {
        if (bool_exp->i == 1) {
            return eval(true_result, frame);
        }
        else {
            return eval(false_result, frame);
        }
    }
    else {
        evaluationError(0);
    }
    return args;
}

Value *evalLet(Value *args, Frame *frame) {
    Frame *new_frame = talloc(sizeof(Frame));
    new_frame->parent = frame;
    Value *new_bindings = makeNull();
    
    Value *cur_node = car(args);
    
    if (car(cur_node)->type != CONS_TYPE) {
        evaluationError(2);
    }
    
    while (cur_node->type != NULL_TYPE) {
        Value *node_to_eval = car(cdr(car(cur_node)));
        Value *pointer = eval(node_to_eval, frame);
 
        Value *val = cons(pointer,makeNull());
        Value *symbol_val = car(car(cur_node));
        if (symbol_val->type == SYMBOL_TYPE) {
            val = cons(symbol_val, val);
        }
        else {
            texit(1);
        }
        new_bindings = cons(val, new_bindings);
        
        cur_node = cdr(cur_node);
    }
    
    
    new_frame->bindings = new_bindings;

    return eval(car(cdr(args)), new_frame);
}

Value *eval(Value *tree, Frame *frame) {
    Value *result;
    switch (tree->type) {
        case INT_TYPE:
            result = tree;
            break;
        case DOUBLE_TYPE:
            result = tree;
            break;
        case STR_TYPE:
            result = tree;
            break;
        case BOOL_TYPE:
            result = tree;
            break;
        case SYMBOL_TYPE:
            result = lookUpSymbol(tree, frame);
            break;
        case CONS_TYPE:
            {
            Value *first_arg = car(tree);
            Value *args = cdr(tree);
            
            // Sanity and error checking on first...
            
            if ((*first_arg).type != SYMBOL_TYPE) {
                evaluationError(5);
            }
            
            if (strcmp(first_arg->s,"if") == 0) {
                result = evalIf(args, frame);
            }
            
            else if (strcmp(first_arg->s, "let") == 0) {
                result = evalLet(args, frame);
            }
            
            else {
                // not a recognized special form
                evaluationError(4);
            }
            break;
            }
        // more case statements?
    }
    return result;
}