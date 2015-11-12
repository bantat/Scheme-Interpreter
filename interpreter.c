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


// Helper function to print Value for troubleshooting purposes
void printVal(Value *input) {
    switch (input->type) {
        case CONS_TYPE:
            printf("(");
            printVal(car(input));
            printf(", ");
            printVal(cdr(input));
            printf(")");
            break;
        case BOOL_TYPE:// more case statements?
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

Value *evalEach(Value *args, Frame *frame) {
    Value *cur_node = args;
    Value *evaled_args = makeNull();
    
    while (cur_node->type != NULL_TYPE) {
        Value *arg = car(cur_node);
        
        Value *evaled_arg = eval(arg, frame);
        
        evaled_args = cons(evaled_arg, evaled_args);
        
        cur_node = cdr(cur_node);
    }
    
    evaled_args = reverse(evaled_args);
    
    return evaled_args;
}

// Interprets input scheme code and prints results to command line
void interpret(Value *tree) {
    // Create a global frame in function call
    Frame *global = talloc(sizeof(Frame));
    global->bindings = makeNull();
    // Iterates through input parse tree, evaluating S-expressions and
    // printing results
    while ((*tree).type != NULL_TYPE) {
        assert((*tree).type == CONS_TYPE);
        // Evaluate individual expression...
        Value *expression = car(tree);
        Value *result = eval(expression, global);
        // And print resulting Value appropriately
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
            case CONS_TYPE:
                printTree(result);
                printf("\n");
                break;
            case CLOSURE_TYPE:
                printf("#<procedure>\n");
                break;
        }
        tree = cdr(tree);
    }
}

// Helper function to print appropriate evaluation error message and cleanup
// memory on exit
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
    else if (error == 6) {
        printf("Too many arguments for define\n");
    }
    else if (error == 7) {
        printf("Lambda needs multiple parameters\n");
    }
    else if (error == 8) {
        printf("Function call has too many parameters\n");
    }
    else if (error == 9) {
        printf("Function call needs more parameters\n");
    }
    texit(1);
}

// Finds and returns Value bound to argument symbol in argument Frame or
// Frame's parents
Value *lookUpSymbol(Value *symbol, Frame *frame) {
    // Searches for symbol in each frame above input frame
    while (frame != NULL) {
        Value *bindings = frame->bindings;
        // While loop in case bindings member of frame not assigned
        while (bindings == NULL) {
            if (frame->parent == NULL) {
                evaluationError(3);
            }
            frame = frame->parent;
            bindings = frame->bindings;
        }
        // Iterates through bindings...
        while ((*bindings).type != NULL_TYPE) {
            Value *symbol1_cons = car(bindings);
            Value *symbol1 = car(symbol1_cons);
            assert(symbol1->type == SYMBOL_TYPE);
            // Checks if string member of binding matches that of input symbol
            if (strcmp((*symbol1).s, (*symbol).s) == 0) {
                return car(cdr(symbol1_cons));
            }
            // Otherwise continues search
            else {
                bindings = cdr(bindings);
            }
        }
        frame = frame->parent;
    }
    // If symbol not found, print evaluation error and exit
    evaluationError(3);
    return symbol;
}

// Evaluates argumnets of an if statement and returns resulting Value
Value *evalIf(Value *args, Frame *frame) {
    // Checks if valid input structure, if not throws evaluation error
    if (args->type != CONS_TYPE) {
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
    // Creates Values for expression and both possible results to be returned
    Value *bool_exp = eval(car(args), frame);
    Value *true_result = car(cdr(args));
    Value *false_result = car(cdr(cdr(args)));
    // Checks valid input structure and returns appropriate result
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

// Evaluates arguments of a let statement and returns resulting Value
Value *evalLet(Value *args, Frame *frame) {
    // Create new frame and set input frame to be parent frame
    Frame *new_frame = talloc(sizeof(Frame));
    new_frame->parent = frame;
    // Create new linked list to store bindings created in let statement
    Value *new_bindings = makeNull();
    
    Value *cur_node = car(args);
    // Check valid input structure
    if (car(cur_node)->type != CONS_TYPE) {
        evaluationError(2);
    }
    // Iterate through arguments...
    while (cur_node->type != NULL_TYPE) {
        // Sets up singular var and value pair
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
        
        // Iterative step to set up next var and value pair
        cur_node = cdr(cur_node);
    }
    
    new_frame->bindings = new_bindings;
    
    // Eval in new frame with new bindings
    return eval(car(cdr(args)), new_frame);
}

Value *evalDefine(Value *args, Frame *frame) {
    //printTree(args);
    Value *var = car(args);
    Value *expr = car(cdr(args));
    //printVal(cdr(cdr(args)));
    
    if (cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(6);
    }
    
    Value *eval_expr = eval(expr, frame);
    Value *new_bindings = makeNull();
    new_bindings = cons(eval_expr, new_bindings);
    new_bindings = cons(var, new_bindings);
    
    frame->bindings = cons(new_bindings, frame->bindings);
    
    Value *void_val = talloc(sizeof(Value));
    void_val->type = VOID_TYPE;
    
    return void_val;
}

Value *apply(Value *function, Value *args) {
    assert(function->type == CLOSURE_TYPE);
    struct Closure closure = function->cl;
    
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = closure.frame;
    
    Value *new_bindings = makeNull();
    Value *cur_node = args;
    Value *params = closure.paramNames;
    Value *cur_param = params;
    
    while (cur_node->type != NULL_TYPE) {
        if (cur_param->type == NULL_TYPE) {
            evaluationError(8);
        }
        
        Value *list = makeNull();
        list = cons(car(cur_node), list);
        list = cons(car(cur_param), list);
        
        new_bindings = cons(list, new_bindings);
        
        cur_node = cdr(cur_node);
        cur_param = cdr(cur_param);
    }
    
    if (cur_param->type != NULL_TYPE) {
        evaluationError(9);
    }
    
    frame->bindings = new_bindings;
    Value *body = closure.functionCode;
    
    return eval(body, frame);
}

Value *evalLambda(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE) {
        evaluationError(7);
    }
    Value *params = car(args);
    Value *body = car(cdr(args));
    
    struct Closure cl;
    cl.paramNames = params;
    cl.functionCode = body;
    cl.frame = frame;
    
    Value* closure = talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    closure->cl = cl;
    
    return closure;
}

// Eval block
Value *eval(Value *tree, Frame *frame) {
    Value *result;
    switch (tree->type) {
        // For int, bool, double, and string type, we simply return tree
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
        // Looks for symbol in frames
        case SYMBOL_TYPE:
            result = lookUpSymbol(tree, frame);
            break;
        case CONS_TYPE:
            {
            Value *first_arg = car(tree);
            Value *args = cdr(tree);
            
            // Checking first argument...
            
            if ((*first_arg).type == SYMBOL_TYPE) {
                if (strcmp(first_arg->s, "if") == 0) {
                    result = evalIf(args, frame);
                }

                else if (strcmp(first_arg->s, "let") == 0) {
                    result = evalLet(args, frame);
                }

                else if (strcmp(first_arg->s, "quote") == 0) {
                    return args;
                }

                else if (strcmp(first_arg->s, "define") == 0) {
                    result = evalDefine(args, frame);
                }

                else if (strcmp(first_arg->s, "lambda") == 0) {
                    result = evalLambda(args, frame);
                }
                
                else {
                    // If not a special form, evaluate the first, evaluate the args, then
                    // apply the first to the args.
                    Value *evaledOperator = eval(first_arg, frame);
                    Value *evaledArgs = evalEach(args, frame);
                    return apply(evaledOperator, evaledArgs);
                }
                
            }
            
            else {
                // If not a special form, evaluate the first, evaluate the args, then
                // apply the first to the args.
                Value *evaledOperator = eval(first_arg, frame);
                Value *evaledArgs = evalEach(args, frame);
                return apply(evaledOperator, evaledArgs);
            }
            break;
            }
    }
    return result;
}