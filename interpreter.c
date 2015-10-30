/* interpreter.c - Program for interpreting Scheme code using C              */
/* By Tore Banta & Charlie Sarano                                            */

void interpret(Value *tree) {
    //...
}

Value *eval(Value *tree, Frame *frame) {
   switch (tree->type)  {
     case INT_TYPE:
        //...
        break;
     case /*some other type*/:
        //...
        break;
     case SYMBOL_TYPE:
        return lookUpSymbol(tree, frame);
        break;
     case CONS_TYPE:  
        Value *first = car(expr);
        Value *args = cdr(expr);

        // Sanity and error checking on first...

        if (!strcmp(first->s,"if")) {
            result = evalIf(args,frame);
        }

        // .. other special forms here...

        else {
           // not a recognized special form
           evalationError();
        }
        break;

      ....
    }    
    ....
}