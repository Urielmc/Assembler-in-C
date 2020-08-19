
#include <ctype.h>
#include <math.h>
#include "declarations.h"

#define BIT(x) (1u<<x) /* macro to get bit of index x in a number */

/* operations names in the assembly language and their opcode */
enum op_num{MOV, CMP, ADD, SUB=2, LEA=4, CLR, NOT=5, INC=5,
    DEC=5, JMP=9, BNE=9, JSR=9, RED=12, PRN, RTS, STOP};
/* The length of each section in the machine code word */
enum length{ARE_LEN = 3, FUNC_LEN=5, OPERAND_LEN = 3, ADDRESSING_LEN=2, OPCODE_LEN = 6, VAL_LEN = 21};
/* The first_bit of each section in the machine code word */
enum first_bit{ARE_FIRST_BIT, FUNC_FIRST_BIT=3,DEST_FIRST_BIT = 8, DEST_ADDRESSING_FIRST_BIT = 11,
    SRC_FIRST_BIT= 13,SRC_ADDRESSING_FIRST_BIT= 16, OPCODE_FIRST_BIT = 18,VAL_FIRST_BIT = 3};

int add_to_symbol_table(char *, int, int);

machine_word **code_image;/* Holds the operations instructions in the current file, translated to machine code */
/* all operations and information about addressings of operands and identifiers */
operation operations[]=
        {{MOV, 0,"mov", {IMMEDIATE,DIRECT,DIRECT_REGISTER},{DIRECT,DIRECT_REGISTER,NOT_FOUND} },
         {CMP, 0,"cmp",{IMMEDIATE,DIRECT,DIRECT_REGISTER},{IMMEDIATE,DIRECT,DIRECT_REGISTER}},
         {ADD,1, "add",{IMMEDIATE,DIRECT,DIRECT_REGISTER},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {SUB,2, "sub",{IMMEDIATE,DIRECT,DIRECT_REGISTER},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {LEA,0 ,"lea",{DIRECT,NOT_FOUND},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {CLR,1, "clr",{NOT_FOUND},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {NOT,2, "not",{NOT_FOUND},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {INC,3, "inc",{NOT_FOUND},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {DEC,4, "dec",{NOT_FOUND},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {JMP,1, "jmp",{NOT_FOUND},{DIRECT,RELATIVE,NOT_FOUND}},
         {BNE,2, "bne",{NOT_FOUND},{DIRECT,RELATIVE,NOT_FOUND}},
         {JSR,3, "jsr", {NOT_FOUND},{DIRECT,RELATIVE,NOT_FOUND}},
         {RED,0, "red",{NOT_FOUND},{DIRECT,DIRECT_REGISTER,NOT_FOUND}},
         {PRN,0, "prn",{NOT_FOUND},{IMMEDIATE,DIRECT,DIRECT_REGISTER}},
         {RTS,0, "rts",{NOT_FOUND},{NOT_FOUND}},
         {STOP,0, "stop",{NOT_FOUND},{NOT_FOUND}}};

/**
 * sets the desired bits in machine_code word to the desired value
 * @param word - pointer to the word to modify
 * @param num - the number to insert
 * @param first bit - the first bit of the section to change
 * @param length - the length of the section to change
 */
void set(machine_word *word, int num, unsigned int first_bit, unsigned int length)
{
    word->w &= ~((BIT(length) - 1) << first_bit); /* Using bitwise operations to change the specific bits */
    word->w += num * (BIT(first_bit));
}

/**
 * set the ARE bits
 * @param w - the pointer to the word to change
 * @param num - the number of ARE
 */
void set_ARE(machine_word *w, int num)
{
    set(w, num, ARE_FIRST_BIT, ARE_LEN);
}

/**
 * set the function bits
 * @param w - the pointer to the word to change
 * @param num - the number of function
 */
void set_FUNC(machine_word *w, int num)
{
    set(w, num, FUNC_FIRST_BIT, FUNC_LEN);
}

/**
 * Sets the destination operand bits
 * @param w - the pointer to the word to change
 * @param num - value of destination
 */
void set_dest(machine_word *w, int num)
{
    set(w, num, DEST_FIRST_BIT, OPERAND_LEN);
}

/**
 * Sets the destination operand addressing method bits
 * @param w - the pointer to the word to change
 * @param num - value of addressing
 */
void set_dest_addressing(machine_word *w, int num)
{
    set(w, num, DEST_ADDRESSING_FIRST_BIT, ADDRESSING_LEN);
}

/**
 * Sets the source operand bits
 * @param w - the pointer to the word to change
 * @param num - value of source
 */
void set_src(machine_word *w, int num)
{
    set(w, num, SRC_FIRST_BIT, OPERAND_LEN);
}

/**
 * Sets the source operand addressing method bits
 * @param w - the pointer to the word to change
 * @param num - value of addressing
 */
void set_src_addressing(machine_word *w, int num)
{
    set(w, num, SRC_ADDRESSING_FIRST_BIT, ADDRESSING_LEN);
}

/**
 * Sets opcode according to the number
 * @param w - the pointer to the word to change
 * @param num - opcode value
 */
void set_opcode(machine_word *w, int num)
{
    set(w, num, OPCODE_FIRST_BIT, OPCODE_LEN);
}

/**
 * sets the value in the word according to the number. used to set immediate number value, address value or distance to label value
 * @param w - the pointer to the word to change
 * @param num - value to set
 */
void set_val(machine_word *w, int num)
{
    set(w, num, VAL_FIRST_BIT, VAL_LEN);
}

/**
 * check if addressing type of operand is legal
 * @param addressings - array of legal addressing types
 * @param addressing_type - the type of the opernad found
 */
int addressing_legal(int *addressings, int addressing_type)
{
    int i=0;
    while(i<3&&addressings[i]!=NOT_FOUND)
    {
        if(addressings[i]==addressing_type)
            return OK;
        i++;
    }
    return ERROR;
}

/**
 * analysing operand, determines type of addressing and whether its addressing is legal
 * @param operation - operation number
 * @param operand_ind - index of operand in line_parsed array
 * @return NOT_FOUND (-1) if syntax addressing is illegal to operation, ADDRESSING TYPE if legal
 */
int get_address_method(int operation, int operand_ind)
{
    int i;
    /* store addressing types in array */
    int *addressings;
    if (operand_ind == line_parsed_len - 1) /*if its a destination operand */
        addressings=operations[operation].dest_addressing;
    else /*if its a source operand */
        addressings=operations[operation].source_addressing;
    /* determine addressing methods by first character */
    if(line_parsed[operand_ind][0] == SHARP) { /*if starts with #, its a number */
        /*check if addressing type is legal */
        if (!addressing_legal(addressings, IMMEDIATE)) {
            print_error("illegal addressing type");
            return NOT_FOUND;
        }
        /*check if operand syntax is legal */
        if (!isdigit(line_parsed[operand_ind][1])&&line_parsed[operand_ind][1]!=PLUS&&line_parsed[operand_ind][1]!=MINUS) {
            print_error("immidiate addressing type requires a number");
            return NOT_FOUND;
        }
        i=2;
        while (line_parsed[operand_ind][i]) {
            if (!isdigit(line_parsed[operand_ind][i])) {
                print_error("immidiate addressing type requires a number");
                return NOT_FOUND;
            }
            i++;
        }
        return IMMEDIATE;
    }
    else if(isalpha(line_parsed[operand_ind][0])) { /*if first character is a letter than it is a label or a register */
        if(line_parsed[operand_ind][0]=='r'&&line_parsed[operand_ind][1]>=ZERO&&line_parsed[operand_ind][1]<=SEVEN&&strlen(line_parsed[operand_ind])==2) { /* if its a register */
            if (!addressing_legal(addressings, DIRECT_REGISTER)) {
                print_error("illegal addressing type");
                return NOT_FOUND;
            }
            return DIRECT_REGISTER;
        }
        /* if its not a register and its starts with a letter it must be a label */
        else if (!addressing_legal(addressings, DIRECT)) {
                print_error("illegal addressing type");
                return NOT_FOUND;
            }
            return DIRECT;
    }
    else if(line_parsed[operand_ind][0]==AND) { /* if first character is '&' its relative */
        if (!addressing_legal(addressings, RELATIVE)) {
            print_error("illegal addressing type");
            return NOT_FOUND;
        }
        return RELATIVE;
    }
    print_error("illegal operand"); /* if its not any of above, its illegal */
    return NOT_FOUND;
}
/**
 * check if number of commas in the line fits the type of the operation
 * @param line - the original line from the file, before parse
 * @param num_of_operands - number of operands expected according to operation
 * @return ok if number of commas fits operation, error if not
 */
int wrong_number_of_commas(char *line, int num_of_operands)
{
    int commas=0,i=0;
    for (; i < strlen(line);i++)  /*count number of commas*/
            if(line[i]==COMMA)
                commas++;
    if((num_of_operands==2&&commas!=1)||(num_of_operands<2&&commas>0))
        return print_error("wrong placement of commas");
    return OK;
}

/*
 * analysing operation of two operands.
 * for each operand, determine its type and value and find if its legal.
 * convert the whole line to machine code.
 * param operation - the number of operation
 * return OK if line is legal and convertions succeeded
 */
int two_operands(int operation)
{
    int src_addressing_method,dest_addressing_method, L=0 /*number of extra words needed */, src_value=0, dest_value=0;
    src_addressing_method=get_address_method(operation, index(1));
    dest_addressing_method=get_address_method(operation, index(2));
    if(src_addressing_method==NOT_FOUND||dest_addressing_method==NOT_FOUND)
        return ERROR;
    set_ARE(code_image[IC],A); /* turn on A on ARE section */
    set_FUNC(code_image[IC],operations[operation].func); /* set function according to operation */
    /* convert source operand to machine code and insert to code_image */
    if(src_addressing_method==IMMEDIATE) {
        src_value = atoi(line_parsed[index(1)]+1);
        code_image=(machine_word **)realloc(code_image,(IC+1+(++L))*sizeof(machine_word*));
        memory_check(code_image)
        code_image[IC+L] = (machine_word *)calloc(1, sizeof(machine_word));
        memory_check(code_image[IC+L])
        set_src(code_image[IC],0);
        set_ARE(code_image[IC+L],A);
        set_val(code_image[IC+L],src_value);
    }
    else if(src_addressing_method==DIRECT_REGISTER) {
        src_value = atoi(line_parsed[index(1)]+1);
        set_src(code_image[IC], src_value);
    }
    else { /*operand addressing method is relative or direct - allocate memory and space for distance or address in next word */
        code_image=(machine_word **)realloc(code_image,(IC+1+(++L))*sizeof(machine_word*));
        memory_check(code_image)
        code_image[IC + L] = (machine_word *) calloc(1, sizeof(machine_word));
        memory_check(code_image[IC + L])
    }
    /* convert destination operand to machine code and insert to code_image */
    if(dest_addressing_method==IMMEDIATE) {
        code_image=(machine_word **)realloc(code_image,(IC+1+(++L))*sizeof(machine_word*));
        memory_check(code_image)
        dest_value = atoi(line_parsed[index(2)]+1);
        code_image[IC+L] = (machine_word *)calloc(1, sizeof(machine_word));
        memory_check(code_image[IC+L])
        set_dest(code_image[IC],0);
        set_ARE(code_image[IC+L],A);
        set_val(code_image[IC+L],dest_value);
    }
    else if(dest_addressing_method==DIRECT_REGISTER) {
        dest_value = atoi(line_parsed[index(2)]+1);
        set_dest(code_image[IC], dest_value);
    }
    else { /*operand addressing method is relative or direct - allocate memory and space for distance or address in next word */
        code_image=(machine_word **)realloc(code_image,(IC+1+(++L))*sizeof(machine_word*));
        memory_check(code_image)
        code_image[IC + L] = (machine_word *) calloc(1, sizeof(machine_word));
        memory_check(code_image[IC+L])
    }
    /* finally, set addressings sections and opcode */
    set_dest_addressing(code_image[IC],dest_addressing_method);
    set_src_addressing(code_image[IC],src_addressing_method);
    set_opcode(code_image[IC],operations[operation].opcode);
    L++;
    IC+=L; /* update IC accordingly */
    return OK;
}

/*
 * analysing operation of one operand.
 * for operand, determine its type and value and find if its legal.
 * convert the whole line to machine code.
 * param operation - the number of operation
 * return OK if line is legal and convertions succeeded
 */
int one_operand(int operation)
{
    int dest_addressing_method, L=0,dest_value=0;
    dest_addressing_method=get_address_method(operation, index(1));
    if(dest_addressing_method==NOT_FOUND) /*return ERROR if addressing type is illegal */
        return ERROR;
    set_ARE(code_image[IC],A);/* turn on A on ARE section */
    set_FUNC(code_image[IC],operations[operation].func); /* set function according to operation */
    /* convert source operand to machine code and insert to code_image */
    if(dest_addressing_method==IMMEDIATE) {
        dest_value = atoi(line_parsed[line_parsed_len-1]+1);
        code_image=(machine_word **)realloc(code_image,(IC+1+(++L))*sizeof(machine_word*));
        memory_check(code_image)
        code_image[IC+L] = (machine_word *)calloc(1, sizeof(machine_word));
        memory_check(code_image[IC+L])
        set_dest(code_image[IC],0);
        set_ARE(code_image[IC+L],A);
        set_val(code_image[IC+L],dest_value);
    }
    else if(dest_addressing_method==DIRECT_REGISTER) {
        dest_value = atoi(line_parsed[index(1)]+1);
        set_dest(code_image[IC], dest_value);
    }
    else { /*operand addressing method is relative or direct - allocate memory and space for distance or address in next word */
        code_image=(machine_word **)realloc(code_image,(IC+1+(++L))*sizeof(machine_word*));
        memory_check(code_image)
        code_image[IC +L] = (machine_word *) calloc(1, sizeof(machine_word));
        memory_check(code_image[IC+L])
    }
    /* finally, set addressings sections and opcode */
    set_dest_addressing(code_image[IC],dest_addressing_method);
    set_src(code_image[IC],0);
    set_src_addressing(code_image[IC],0);
    set_opcode(code_image[IC],operations[operation].opcode);
    L++;
    IC+=L;
    return OK;
}

/*
 * analysing operation of zero operands.
 * convert the whole line to machine code.
 * param operation - the number of operation
 * return OK if convertions succeeded
 */
int zero_operands(int op_num)
{
    set_ARE(code_image[IC],A);
    set_FUNC(code_image[IC],0);
    set_dest(code_image[IC],0);
    set_dest_addressing(code_image[IC],0);
    set_src(code_image[IC],0);
    set_src_addressing(code_image[IC],0);
    set_opcode(code_image[IC],operations[op_num].opcode);
    IC++;
    return OK;
}

/**
 * find the number of operation, according to the first word in line_parsed array.
 * allocate memory for code_image array
 * call analysing function according to number of operands.
 * check if number of commas is legal.
 * @param line - original line from file, to check number of commas
 * @return ERROR if the operation is undefined or an error found in addressing and convertions
 */
int get_operation(char *line)
{
    int operation,i=0;
    while(i<OPS_NUM) { /* look for operation name in operations array */
        if (!strcmp(line_parsed[index(0)], operations[i].name)) {
            operation = i;
            break;
        }
        i++;
    }
    /* if operation name not found, return error */
    if(i==OPS_NUM)
        return print_error("operation not exists");
    /* insert label to symbol_table if exists */
    if(SYMBOL_FLAG)
        add_to_symbol_table(line_parsed[0],CODE,START_OF_CODE+IC);
    /* if number of operands doesnt fit operation, return error */
    if((i<5&&line_parsed_len-index(0)!=3)||(i>4&&i<14&&line_parsed_len-index(0)!=2)||(i>13&&line_parsed_len-index(0)!=1))
        return print_error("illegal number of operands for operation");
    code_image=(machine_word **)realloc(code_image,(IC+1)*sizeof(machine_word*));
    memory_check(code_image)
    code_image[IC] = (machine_word *)calloc(1, sizeof(machine_word));
    memory_check(code_image[IC])
    /* call analysing function according to number of operands */
    if(i<5)
        return (wrong_number_of_commas(line, 2)&&two_operands(operation));
    else if(i<14)
        return (wrong_number_of_commas(line, 1)&&one_operand(operation));
    else
        return (wrong_number_of_commas(line, 0)&&zero_operands(operation));
}

/**
 * retrieves the desired bits in machine_code word
 * @param first - the first bit
 * @param length - length of section desired
 * @return number in desired section in the word
 */
unsigned int get_bits(unsigned int first, unsigned int length)
{
    unsigned int x=(((unsigned int)pow(2.0,length))-1u)<<first;
    unsigned int num=(code_image[IC]->w&x)>>first;
    return num;
}