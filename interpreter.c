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
    else if (error == 10) {
        printf("Invalid arguments for primitive function\n");
    }
    else if (error == 11) {
        printf("Car and Cdr require a list as an argument\n");
    }
    else if (error == 12) {
        printf("Invalid arguments for let statement\n");
    }
    texit(1);
}

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

Value *trueVal() {
    Value *true_val = talloc(sizeof(Value));
    true_val->type = BOOL_TYPE;
    true_val->i = 1;
    return true_val;
}

Value *falseVal() {
    Value *false_val = talloc(sizeof(Value));
    false_val->type = BOOL_TYPE;
    false_val->i = 0;
    return false_val;
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

Value *primitiveAdd(Value *args) {
    float result = 0;
    while (args->type != NULL_TYPE) {
        Value *cur_node = car(args);
        if (cur_node->type != INT_TYPE) {
            if (cur_node->type != DOUBLE_TYPE) {
                evaluationError(10);
            }
            result = result + cur_node->d;
            args = cdr(args);
        }
        else {
            result = result + cur_node->i;
            args = cdr(args);
        }
    }
    Value *result_val = talloc(sizeof(Value));
    result_val->type = DOUBLE_TYPE;
    result_val->d = result;
    return result_val;
}

Value *primitiveMultiply(Value *args) {
    float result = 1;
    while (args->type != NULL_TYPE) {
        Value *cur_node = car(args);
        if (cur_node->type != INT_TYPE) {
            if (cur_node->type != DOUBLE_TYPE) {
                evaluationError(10);
            }
            result = result * cur_node->d;
            args = cdr(args);
        }
        else {
            result = result * cur_node->i;
            args = cdr(args);
        }
    }
    Value *result_val = talloc(sizeof(Value));
    result_val->type = DOUBLE_TYPE;
    result_val->d = result;
    return result_val;
}


Value *primitiveNull(Value *args) {
    // The argument passed should not be empty
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    assert(args->type == CONS_TYPE);
    // If the cdr isn't empty, then they have passed more than one argument
    if (cdr(args)->type != NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        // Calls method that returns true bool val
        return trueVal();
    }
    else {
        // Equivalent method for false bool val
        return falseVal();
    }
}

Value *primitiveCar(Value *args) {
    // Cannot call car of a nonexistent argument
    if (args->type == NULL_TYPE){
        evaluationError(10);
    }
    assert(args->type == CONS_TYPE);
    // Cannot have more than one argument
    if (cdr(args)->type != NULL_TYPE) {
        evaluationError(10);
    }
    // takes the first argument, and returns its car
    Value *argument = car(args);
    return car(argument);
}

Value *primitiveCdr(Value *args) {
    // Cannot call car of a nonexistent argument
    if (args->type == NULL_TYPE){
        evaluationError(10);
    }
    assert(args->type == CONS_TYPE);
    // Cannot have more than one argument
    if (cdr(args)->type != NULL_TYPE) {
        evaluationError(10);
    }
    Value *lst = car(args);
    if (lst->type != CONS_TYPE) {
        evaluationError(11);
    }
    // Returns the cdr of the first argument
    return cdr(lst);
}

Value *primitiveCons(Value *args) {
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    
    // If there exists a third argument, throw an Error
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    //The first argument to put in cons cell will always be car(args)
    Value *arg1 = car(args);
    Value *arg2;
    
    
    if (cdr(args)->type == CONS_TYPE) {
        arg2 = car(cdr(args));
    }
    // If the cdr of args isn't a Cons type, we simply set arg2 as the cdr
    else {
        arg2 = cdr(args);
    }
    Value *result_val = cons(arg1, arg2);
    return result_val;
}

Value *primitiveEquals(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    
    float arg1;
    float arg2;
    
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (float) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        evaluationError(10);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (float) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        evaluationError(10);
    }
    
    if (arg1 == arg2) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

Value *primitiveGreaterThan(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    
    float arg1;
    float arg2;
    
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (float) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        evaluationError(10);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (float) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        evaluationError(10);
    }
    
    if (arg1 > arg2) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

Value *primitiveLessThan(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    
    float arg1;
    float arg2;
    
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (float) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        evaluationError(10);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (float) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        evaluationError(10);
    }
    
    if (arg1 < arg2) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

Value *primitiveDivide(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    
    float arg1;
    int arg_1 = NULL;
    
    float arg2;
    int arg_2 = NULL;
    
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (float) argument->i;
        arg_1 = argument->i; 
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        evaluationError(10);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (float) argument1->i;
        arg_2 = argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        evaluationError(10);
    }
    if (arg_1 != NULL && arg_2 != NULL) {
        int result = arg_1/arg_2;
        
        Value *result_val = talloc(sizeof(Value));
        result_val->type = INT_TYPE;
        result_val->i = result;
    
        return result_val;
    }
        
    float result = arg1/arg2;
    
    Value *result_val = talloc(sizeof(Value));
    result_val->type = DOUBLE_TYPE;
    result_val->d = result;
    
    return result_val;
}

Value *primitiveModulo(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    
    int arg1;
    int arg2;
    
    if (car(args)->type == INT_TYPE) {
        if (car(cdr(args))->type == INT_TYPE) {
            Value *argument = car(args);
            arg1 = argument->i; 
            Value *argument1 = car(cdr(args));
            arg2 = argument1->i;
            
            int result = arg1 % arg2;
        
            Value *result_val = talloc(sizeof(Value));
            result_val->type = INT_TYPE;
            result_val->i = result;
    
            return result_val;  
        }
        else {
            evaluationError(10);
        }
    }

    else {
        evaluationError(10);
    }
    
    return falseVal();
}

Value *primitiveGreaterOrEqual(Value *args) {
    Value *bool_val = primitiveLessThan(args);
    if (bool_val->i == 0) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

Value *primitiveLessOrEqual(Value *args) {
    Value *bool_val = primitiveGreaterThan(args);
    if (bool_val->i == 0) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

Value *primitiveSubtract(Value *args) {
    float result = 0;
    
    Value *cur_node = car(args);
    if (cur_node->type != INT_TYPE) {
        if (cur_node->type != DOUBLE_TYPE) {
            evaluationError(10);
        }
        result = result + cur_node->d;
        args = cdr(args);
    }
    else {
        result = result + cur_node->i;
        args = cdr(args);
    }
    
    while (args->type != NULL_TYPE) {
        Value *cur_node = car(args);
        if (cur_node->type != INT_TYPE) {
            if (cur_node->type != DOUBLE_TYPE) {
                evaluationError(10);
            }
            result = result - cur_node->d;
            args = cdr(args);
        }
        else {
            result = result - cur_node->i;
            args = cdr(args);
        }
    }
    Value *result_val = talloc(sizeof(Value));
    result_val->type = DOUBLE_TYPE;
    result_val->d = result;
    return result_val;
}

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Add primitive functions to top-level bindings list
    Value *fun_val = talloc(sizeof(Value));
    fun_val->type = PRIMITIVE_TYPE;
    fun_val->pf = function;
    // Add binding of name to value
    Value *symbol = talloc(sizeof(Value));
    symbol->type = SYMBOL_TYPE;
    symbol->s = name;
    
    Value *binding = makeNull();
    binding = cons(fun_val, binding);
    binding = cons(symbol, binding);
    
    frame->bindings = cons(binding, frame->bindings);
}

// Interprets input scheme code and prints results to command line
void interpret(Value *tree) {
    // Create a global frame in function call
    Frame *global = talloc(sizeof(Frame));
    global->bindings = makeNull();
    
    bind("+", primitiveAdd, global);
    bind("-", primitiveSubtract, global);
    bind("*", primitiveMultiply, global);
    bind("/", primitiveDivide, global);
    bind(">", primitiveGreaterThan, global);
    bind("<", primitiveLessThan, global);
    bind(">=", primitiveGreaterOrEqual, global);
    bind("<=", primitiveLessOrEqual, global);
    bind("=", primitiveEquals, global);
    bind("modulo", primitiveModulo, global);
    bind("null?", primitiveNull, global);
    bind("car", primitiveCar, global);
    bind("cdr", primitiveCdr, global);
    bind("cons", primitiveCons, global);
    
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
                printf("(");
                printTree(result);
                printf(")\n");
                break;
            case CLOSURE_TYPE:
                printf("#<procedure>\n");
                break;
            case PRIMITIVE_TYPE:
                printf("#<procedure>\n");
                break;
            case NULL_TYPE:
                printf("()\n");
                break;
        }
        tree = cdr(tree);
    }
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
            evaluationError(12);
        }
        new_bindings = cons(val, new_bindings);
        
        // Iterative step to set up next var and value pair
        cur_node = cdr(cur_node);
    }
    
    new_frame->bindings = new_bindings;
    
    // Eval in new frame with new bindings
    return eval(car(cdr(args)), new_frame);
}

Value *evalLetStar(Value *args, Frame *frame) {
    Frame *cur_frame = frame;
    
    Value *cur_node = car(args);
    // Check valid input structure
    if (car(cur_node)->type != CONS_TYPE) {
        evaluationError(2);
    }
    // Iterate through arguments...
    while (cur_node->type != NULL_TYPE) {
        Value *new_binding = makeNull();
        // Sets up singular var and value pair
        Value *node_to_eval = car(cdr(car(cur_node)));
        Value *pointer = eval(node_to_eval, cur_frame);
 
        Value *val = cons(pointer, makeNull());
        Value *symbol_val = car(car(cur_node));
        if (symbol_val->type == SYMBOL_TYPE) {
            val = cons(symbol_val, val);
        }
        else {
            evaluationError(12);
        }
        new_binding = cons(val, new_binding);
        
        Frame *new_frame = talloc(sizeof(Frame));
        new_frame->parent = cur_frame;
        new_frame->bindings = new_binding;
        
        // Iterative step to set up next var and value pair
        cur_frame = new_frame;
        cur_node = cdr(cur_node);
    }
    
    // Eval in new frame with new bindings
    return eval(car(cdr(args)), cur_frame);
}

Value *evalLetRec(Value *args, Frame *frame) {
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
        Value *pointer = eval(node_to_eval, new_frame);
 
        Value *val = cons(pointer,makeNull());
        Value *symbol_val = car(car(cur_node));
        if (symbol_val->type == SYMBOL_TYPE) {
            val = cons(symbol_val, val);
        }
        else {
            evaluationError(12);
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
    Value *var = car(args);
    Value *expr = car(cdr(args));
    
    if (cdr(cdr(args))->type != NULL_TYPE) {
        // If too many arguments for define...
        evaluationError(6);
    }
    
    // Evaluates expression and sets up in global frame
    Value *eval_expr = eval(expr, frame);
    Value *new_bindings = makeNull();
    new_bindings = cons(eval_expr, new_bindings);
    new_bindings = cons(var, new_bindings);
    
    frame->bindings = cons(new_bindings, frame->bindings);
    
    // Returns void Value for interpreter to ignore
    Value *void_val = talloc(sizeof(Value));
    void_val->type = VOID_TYPE;
    
    return void_val;
}

Value *apply(Value *function, Value *args) {
    // Applies given function to multiple arguments
    assert(function->type == CLOSURE_TYPE || function->type == PRIMITIVE_TYPE);
    
    if (function->type == PRIMITIVE_TYPE) {
        return function->pf(args);
    }
    
    struct Closure closure = function->cl;
    
    // Sets up new frame for execution of body of code in closure
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = closure.frame;
    
    Value *new_bindings = makeNull();
    Value *cur_node = args;
    Value *params = closure.paramNames;
    Value *cur_param = params;
    
    // Sets up list of bindings based on parameters
    while (cur_node->type != NULL_TYPE) {
        if (cur_param->type == NULL_TYPE) {
            // If too many parameters are passed into function
            evaluationError(8);
        }
        
        Value *list = makeNull();
        list = cons(car(cur_node), list);
        list = cons(car(cur_param), list);
        
        new_bindings = cons(list, new_bindings);
        
        cur_node = cdr(cur_node);
        cur_param = cdr(cur_param);
    }
    // If there are less parameters passed than what function needs
    if (cur_param->type != NULL_TYPE) {
        evaluationError(9);
    }
    
    frame->bindings = new_bindings;
    Value *body = closure.functionCode;
    
    return eval(body, frame);
}

Value *evalLambda(Value *args, Frame *frame) {
    // Sets up a closure and returns the closure type Value
    if (args->type != CONS_TYPE) {
        evaluationError(7);
    }
    // Extracts body and parameters from input arguments
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
            // If the first argument is a string...
            if ((*first_arg).type == SYMBOL_TYPE) {
                if (strcmp(first_arg->s, "if") == 0) {
                    result = evalIf(args, frame);
                }

                else if (strcmp(first_arg->s, "let") == 0) {
                    result = evalLet(args, frame);
                }
                
                else if (strcmp(first_arg->s, "let*") == 0) {
                    result = evalLetStar(args, frame);
                }
                
                else if (strcmp(first_arg->s, "letrec") == 0) {
                    result = evalLetRec(args, frame);
                }

                else if (strcmp(first_arg->s, "quote") == 0) {
                    return car(args);
                }

                else if (strcmp(first_arg->s, "define") == 0) {
                    result = evalDefine(args, frame);
                }

                else if (strcmp(first_arg->s, "lambda") == 0) {
                    result = evalLambda(args, frame);
                }
                
                else {
                    // If not a special form, evaluate the first
                    // evaluate the args, then apply the first to the args.
                    Value *evaledOperator = eval(first_arg, frame);
                    Value *evaledArgs = evalEach(args, frame);
                    return apply(evaledOperator, evaledArgs);
                }
                
            }
            // Otherwise it might be another ConsCell
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