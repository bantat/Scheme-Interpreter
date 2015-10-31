/* interpreter.c - Program for interpreting Scheme code using C              */
/* By Tore Banta & Charlie Sarano                                            */

void interpret(Value *tree) {
    Frame *global = talloc(sizeof(Frame));
    while ((*tree).type != NULL_TYPE) {
        assert((*tree).type == CONS_TYPE);
        Value *expression = car(tree);
        Value *result = eval(expression, global);
        
        switch ((*result).type) {
            case BOOL_TYPE:
                if ((*car_val).i == 0) {
                    printf("#f\n");
                }
                else {
                    printf("#t\n");
                }
                break;
            case INT_TYPE:
                printf("%i\n", (*car_val).i);
                break;
            case DOUBLE_TYPE:
                printf("%f\n", (*car_val).d);
                break;
            case STR_TYPE:
                printf("\"%s\"\n", (*car_val).s);
                break;
            case SYMBOL_TYPE:
                printf("%s\n", (*car_val).s);
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
        Value *bindings = (*frame).bindings;
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
}

Value *evalIf(Value *args, Frame *frame) {
    assert(args->type == CONS_TYPE);
    assert(cdr(args)->type == CONS_TYPE);
    assert(cdr(cdr(args))->type == CONS_TYPE);
    
    Value *bool_exp = eval(car(args),frame);
    true_result = car(cdr(args));
    false_result = car(cdr(cdr(args)));
    if (bool_exp->type == BOOL_TYPE) {
        if (bool_exp->i = 1) {
            return eval(true_result, frame);
        }
        else {
            return eval(false_result, frame);
        }
    }
    else {
        evaluationError();
    }          
}

Value *evalLet(Value *args, Frame *frame) {
    Frame *new_frame;
    new_frame->parent = frame;
    Value *new_bindings = car(args);
    
    new_frame->bindings = new_bindings;
    
    return eval(car(cdr(args)),new_frame);
}

Value *eval(Value *tree, Frame *frame) {
    switch (tree->type)  {
        case INT_TYPE:
            return tree;
            break;
        case DOUBLE_TYPE:
            return tree;
            break;
        case STR_TYPE:
            return tree;
            break;
        case BOOL_TYPE:
            return tree;
            break;
        case SYMBOL_TYPE:
            return lookUpSymbol(tree, frame);
            break;
        case CONS_TYPE:  
            Value *first = car(expr);
            Value *args = cdr(expr);
            
            // Sanity and error checking on first...
            
            if ((*first).type != SYMBOL_TYPE) {
                evaluationError();
            }
            
            if (!strcmp(first->s,"if")) {
                result = evalIf(args, frame);
            }
            
            if (!strcmp(first->s, "let")) {
                result = evalLet(args, frame);
            }
            
            else {
                // not a recognized special form
                evaluationError();
            }
            break;
        // more case statements?
    }
    // do something here
}