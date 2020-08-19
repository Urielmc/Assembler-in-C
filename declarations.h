
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "data_structures.h"

/*constants definitions*/
#define OPS_NUM 16 /* Number of operations in the assembly language */
#define DEFS_NUM 4 /* Number of definitions in the assembly language */
#define MAX_LABEL_SIZE 31 /*max length of a label */
#define MAX_LINE_SIZE 81/* max length of an argument in assembly code file (including '\n') */
#define START_OF_CODE 100 /* the beginning address of instructions in code_image */

/*macros*/
/* used as index for line_parsed array, returns the word in index x (skipping over label definition, if exists) */
#define index(x) (SYMBOL_FLAG+x)
/*perform check if memory allocation succeeded */
#define memory_check(x)  {if(!x) return print_error("memory allocation failed");}

/*enum's storing information and representing values of flags*/
enum signs_in_ASCII{SPACE=20,QUOTE=34,SHARP,AND=38,PLUS=43,COMMA,MINUS,ZERO=48,SEVEN=55,COLON=58};
enum addressing_type{IMMEDIATE, DIRECT, RELATIVE, DIRECT_REGISTER}; /* Numbers of addressing methods, represented by their name */
enum defs{DATA, STRING, EXTERN, ENTRY, CODE};
enum are{E=1,R=2,A=4}; /* constants for the ARE section in a machine_code word */
enum ERROR{ERROR,OK, NOT_FOUND=-1}; /* constants to represent if an error occured during runtime */

/* global variables used all across the program */
extern int SYMBOL_FLAG; /* if flag is on then there is a label at the beginning of the current line read from the file */
extern operation operations[];/* store each operation name and information about it */
extern definition definitions[]; /*definitions of data instructions */
extern definition **externs; /*list of extern symbols appearances */
extern ptr symbol_table_head; /* head of symbol table */

extern int IC, /* Instruction counter */
    ICF, /* final Instruction counter */
    DC, /* Data counter */
    DCF, /* final Data counter */
    extern_count; /* Extern variables appearances counter */

extern char **line_parsed /* Holds current line read from the file, parsed by function parse_line*/;
extern int line_parsed_len /* The length of line_parsed array */;

extern machine_word **data_image; /* Holds the data instructions in the current file, translated to machine code */
extern machine_word **code_image;/* Holds the operations instructions  in the current file, translated to machine code */

/**
 * prints an error, given by the parameter of the character pointer, to stderr
 * @param - error - description of the error to print
 * @return ERROR
 */
int print_error(char *); /*in assembler.c*/

