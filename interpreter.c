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

void evaluationError() {
    printf("Evaluation error\n");
    texit(1);
}

Value *lookUpSymbol(Value *symbol, Frame *frame) {
    while (frame != NULL) {
        Value *bindings = frame->bindings;
        /*printf("\n");
        printTree(bindings);*/
        while ((*bindings).type != NULL_TYPE) {
            Value *symbol1_cons = car(bindings);
            Value *symbol1 = car(symbol1_cons);
            assert(symbol1->type == SYMBOL_TYPE);
            if ((*symbol1).s == (*symbol).s) {
                assert (car(cdr(symbol1_cons))->type == PTR_TYPE);
                return (*car(cdr(symbol1_cons))).p;
            }
            else {
                bindings = cdr(bindings);
            }
        }
        frame = frame->parent;
    }
    evaluationError();
    return symbol;
}

Value *evalIf(Value *args, Frame *frame) {
    assert(args->type == CONS_TYPE);
    assert(cdr(args)->type == CONS_TYPE);
    assert(cdr(cdr(args))->type == CONS_TYPE);
    
    Value *bool_exp = eval(car(args),frame);
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
        evaluationError();
    }
    return args;
}

Value *evalLet(Value *args, Frame *frame) {
    /*printf("\n");
    printTree(args);
    printf("\n");*/
    Frame *new_frame = talloc(sizeof(Frame));
    new_frame->parent = frame;
    Value *new_bindings = car(args);
    
    new_frame->bindings = new_bindings;
    printTree(new_bindings);
    printf("\nWeird error above\n\n\n");
    //assert(new_bindings->type != NULL_TYPE);
    return eval(car(cdr(args)),new_frame);
}

Value *eval(Value *tree, Frame *frame) {
    Value *result;
    switch (tree->type)  {
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
            result =  lookUpSymbol(tree, frame);
            break;
        case CONS_TYPE:
            {
            Value *first_arg = car(tree);
            Value *args = cdr(tree);
            
            // Sanity and error checking on first...
            
            if ((*first_arg).type != SYMBOL_TYPE) {
                evaluationError();
            }
            
            if (strcmp(first_arg->s,"if")==0) {
                result = evalIf(args, frame);
            }
            
            else if (strcmp(first_arg->s, "let")==0) {
                result = evalLet(args, frame);
            }
            
            else {
                // not a recognized special form
                evaluationError();
            }
            break;
            }
        // more case statements?
    }
    // do something here
    return result;
}