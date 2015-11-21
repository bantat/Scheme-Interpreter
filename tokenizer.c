/* tokenizer.c - Tokenizer for use in interpreter project.                   */
/* By Tore Banta & Charlie Sarano                                            */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "talloc.h"
#include "linkedlist.h"
#include "value.h"

//Helper functions to determine valid syntax and token type for tokenize()

int is_digit(char input) {
    if (input == '0' || input == '1' || input == '2' || input == '3') {
        return 1;
    }
    else if (input == '4' || input == '5' || input == '6' || input == '7') {
        return 1;
    }
    else if (input == '8' || input == '9') {
        return 1;
    }
    else {
        return 0;
    }
}

int is_udecimal(char *input) {
    int bool_val =  0;
    int decimals = 0;
     for (int i = 0; input[i] != '\0'; i++) {
        if (!is_digit(input[i])) {
            if (input[i] == '.') {
                if (decimals == 1) {
                    return 0;
                }
                bool_val = 1;
                decimals = decimals + 1;
            }
            else {
                return 0;
            }
        }
    }
         
    return bool_val;
}

int is_uinteger(char *input) {
    int i = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (!is_digit(input[i])) {
            return 0;
        }
    }
    return 1;
}

int is_ureal(char *input) {
    if (is_uinteger(input)) {
        return 1;
    }
    else if (is_udecimal(input)) {
        return 1;
    }
    else {
        return 0;
    }
}

int is_sign(char input) {
    if (input == '+') {
        return 1;
    }
    else if (input == '-') {
        return 1;
    }
    else {
        return 0;
    }
}

int is_number(char *input) {
    int size = 0;
    for(int i = 0; input[i] != '\0'; i++) {
        size++;
    }
    if (is_sign(input[0]) && size > 1 && is_ureal(&input[1])) {
        return 1;
    }
    else if (is_ureal(input)) {
        return 1;
    }
    else {
        return 0;
    }
}

int is_letter(char input) {
    if (input >= 'a' && input <= 'z') {
        return 1;
    }
    else if (input >= 'A' && input <= 'Z') {
        return 1;
    }
    else {
        return 0;
    }
}

int is_initial(char input) {
    if (is_letter(input)) {
        return 1;
    }
    else if (input == '!' || input == '$' || input == '%' || input == '&') {
        return 1;
    }
    else if (input == '*' || input == '/' || input == ':' || input == '<') {
        return 1;
    }
    else if (input == '=' || input == '>' || input == '?' || input == '~') {
        return 1;
    }
    else if (input == '_' || input == '^') {
        return 1;
    }
    else {
        return 0;
    }
}

int is_subsequent(char input) {
    if (is_initial(input) || is_digit(input)) {
        return 1;
    }
    else if (input == '.' || input == '+' || input == '-') {
        return 1;
    }
    else {
        return 0;
    }
}

int is_symbol(char *input) {
    if (is_initial(input[0])) {
        for (int i = 0; input[i] != '\0'; i++) {
            if (!is_subsequent(input[i])) {
                return 0;
            }
        }
        return 1;
    }
    else if (input[0] == '+' && input[1] == '\0') {
        return 1;
    }
    else if (input[0] == '-' && input[1] == '\0') {
        return 1;
    }
    else {
        return 0;
    }
}

int is_boolean(char *input) {
    int size = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        size++;
    }
    if (size > 2 || input[0] != '#'){
        return 0;
    }
    else if (input[1] == 'f' || input[1] == 't') {
        return 1;
    }
    else {
        return 0;
    }
}

int is_brace(char input) {
    if (input == '(' || input == ')') {
        return 1;
    }
    return 0;
}

int is_space(char input) {
    if (input == ' ') {
        return 1;
    }
    else if (input == '\n') {
        return 1;
    }
    return 0;
}

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize() {
    char charRead;
    Value *list = makeNull();
    charRead = fgetc(stdin);

    while (charRead != EOF) {
        // Two if statements to catch open and close parens
        if (charRead == '(') {
            char *str_val = "(";
            Value *open = talloc(sizeof(Value));
            (*open).type = OPEN_TYPE;
            (*open).s = str_val;
            list = cons(open, list);
        }
        else if (charRead == ')') {
            char *str_val = ")";
            Value *close = talloc(sizeof(Value));
            (*close).type = CLOSE_TYPE;
            (*close).s = str_val;
            list = cons(close, list);
        }
        // Don't need to address the tabs, newlines or spaces
        else if (is_space(charRead)) {
            //...do nothing
            ;
        }
        else if (charRead == '\n' || charRead == '\t') {
            //...do nothing
            ;
        }
        // Removes characters until the new line character
        else if (charRead == ';') {
            while (charRead != '\n' && charRead != EOF) {
                charRead = fgetc(stdin);
            }
            ;
        }
        // String managing
        else if (charRead == '"') {
            char *test_string = talloc(sizeof(char) * 200);
            test_string[0] = '\0';
            int length = 0; //used to keep track of length of array
            int memSize = 199;
            charRead = fgetc(stdin);
            
            while (charRead != '"') {
                // If end of file is reached before double quote, throw error
                if (charRead == EOF) {
                    printf("String untokenizable, missing quote\n");
                    texit(1);
                }
                // If an escape character is encountered in string,
                // function says charRead is intended character
                else if (charRead == '\\'){
                    char nextChar = fgetc(stdin);
                    if (nextChar == 'n') {
                        charRead = '\n';
                    }
                    else if (nextChar == 't') {
                        charRead = '\t';
                    }
                    else if (nextChar == '\'') {
                        charRead = '\'';
                    }
                    else if (nextChar == '\"') {
                        charRead = '\"';
                    }
                    
                    if (memSize == length) {
                        char *temp = talloc(sizeof(char) * ((memSize * 2) + 1));
                        for(int i = 0; test_string[i] != '\0'; i++) {
                            temp[i] = test_string[i];
                        }
                        memSize = memSize * 2;
                        test_string = temp;
                    }
                    test_string[length] = charRead;
                    length++;
                    test_string[length] = '\0';
                }
                else {
                    // Adds character to test_string
                    if (memSize == length) {
                        char *temp = talloc(sizeof(char) * ((memSize * 2) + 1));
                        for(int i = 0; test_string[i] != '\0'; i++) {
                            temp[i] = test_string[i];
                        }
                        memSize = memSize * 2;
                        test_string = temp;
                    }
                    test_string[length] = charRead;
                    length++;
                    // Adds null terminator for helper function use
                    test_string[length] = '\0';
                    
                }
                charRead = fgetc(stdin);
            }
            // Adds the string to the list of Values
            Value *string_to_add = talloc(sizeof(Value));
            (*string_to_add).type = STR_TYPE;
            (*string_to_add).s = test_string;
            list = cons(string_to_add, list);
        }
        // Else statement that covers numbers, bools, and symbols
        else {
            char *token = talloc(sizeof(char) * 200);
            token[0] = '\0';
            int length2 = 0;
            int memSize2 = 199;
            while ((!is_brace(charRead)) && (!is_space(charRead))) {
                if (charRead == EOF) {
                    printf("Syntax Error: Incomplete Token\n");
                    texit(1);
                }
                else if (charRead == '\n' || charRead == '\t') {
                    //do nothing
                    ;
                }
                else {
                    // Adds the valid character 
                    if (length2 == memSize2) {
                        char *temp2 = talloc(sizeof(char) * ((memSize2 * 2) + 1));
                        for(int i= 0; token[i] != '\0'; i++) {
                            temp2[i] = token[i];
                        }
                        token = temp2;
                        memSize2 = memSize2 * 2;
                    }
                    token[length2] = charRead;
                    length2++;
                    // Null terminator used for helper functions
                    token[length2] = '\0';
                }
                charRead = fgetc(stdin);
            }
            // Ungetting a character that was a brace or a space
            ungetc(charRead, stdin);

            if (is_number(token)) {
                // Determine if integer or float, store in value...
                if (is_sign(token[0])) {
                    char *utoken = talloc(sizeof(char) * strlen(token) - 1);
                    for (int i = 1; token[i] != '\0'; i++) {
                        utoken[i-1] = token[i];
                    }
                    int index = strlen(token) - 2;
                    utoken[index] = '\0';
                    // If the token is a signed int of decimal
                    if (is_uinteger(utoken)) {
                        Value *int_val = talloc(sizeof(Value));
                        (*int_val).type = INT_TYPE;
                        (*int_val).i = atoi(token);
                        list = cons(int_val, list);
                    }
                    else if (is_udecimal(utoken)) {
                        Value *double_val = talloc(sizeof(Value));
                        (*double_val).type = DOUBLE_TYPE;
                        (*double_val).d = atof(token);
                        list = cons(double_val, list);
                    }
                    // If the number isn't an int or decimal, throw error
                    else {
                        printf("Syntax Error: '%s' untokenizable \n", token);
                    }
                }
                // If statements where the first character isn't a sign
                else if (!is_sign(token[0])) {
                    if (is_uinteger(token)) {
                        Value *int_val = talloc(sizeof(Value));
                        (*int_val).type = INT_TYPE;
                        (*int_val).i = atoi(token);
                        list = cons(int_val, list);
                    }
                    else if (is_udecimal(token)) {
                        Value *double_val = talloc(sizeof(Value));
                        (*double_val).type = DOUBLE_TYPE;
                        (*double_val).d = atof(token);
                        list = cons(double_val, list);
                    }
                    else {
                        printf("Syntax Error: '%s' untokenizable \n", token);
                    }
                }
                // If the first character somehow doesn't fit into first two ifs
                else {
                    printf("Syntax Error: '%s' untokenizable \n", token);
                    texit(1);
                }
                    
            }
            // If the token is a symbol
            else if (is_symbol(token)) {
                Value *symbol = talloc(sizeof(Value));
                (*symbol).type = SYMBOL_TYPE;
                (*symbol).s = token;
                list = cons(symbol, list);
            }
            // If the token is a bool
            else if (is_boolean(token)) {
                if (token[1] == 'f') {
                    Value *bool_val = talloc(sizeof(Value));
                    (*bool_val).type = BOOL_TYPE;
                    (*bool_val).i = 0;
                    list = cons(bool_val, list);
                }
                else {
                    Value *bool_val = talloc(sizeof(Value));
                    (*bool_val).type = BOOL_TYPE;
                    (*bool_val).i = 1;
                    list = cons(bool_val, list);
                }
            }
            // Error thrown because the token doesn't fit into syntax category
            else {
                printf("Syntax Error: '%s' untokenizable \n", token);
                texit(1);
            }
        }
        charRead = fgetc(stdin);
    }
    // Reverses list to present tokens in order
    Value *revList = reverse(list);
    
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list) {
    Value *cur_node = list;
    while ((*cur_node).type != NULL_TYPE) {
        struct ConsCell cons_cell = (*cur_node).c;
        Value *car_val = cons_cell.car;
        Value *cdr_val = cons_cell.cdr;
        int car_type = (*car_val).type;
        
        switch (car_type) {
            case BOOL_TYPE:
                if ((*car_val).i == 0) {
                    printf("#f:boolean\n");
                }
                else {
                    printf("#t:boolean\n");
                }
                break;
            case INT_TYPE:
                printf("%i:integer\n", (*car_val).i);
                break;
            case DOUBLE_TYPE:
                printf("%f:float\n", (*car_val).d);
                break;
            case STR_TYPE:
                printf("\"%s\":string\n", (*car_val).s);
                break;
            case SYMBOL_TYPE:
                printf("%s:symbol\n", (*car_val).s);
                break;
            case CLOSE_TYPE:
                printf("%s:close\n", (*car_val).s);
                break;
            case OPEN_TYPE:
                printf("%s:open\n", (*car_val).s);
                break;
        }
        cur_node = cdr_val;
    }
}
