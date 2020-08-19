/*
 * all definitions of data structures used in assembly process.
 */

/* holds a word in machine code, in data image or code image */
typedef struct machine_word{
    unsigned int w:24;
}machine_word;

/* Matching between a name to a number, to store information about types os data instructions or extern appearances */
typedef struct{
    int num;
    char *name;
} definition;

/* stores information about an operation in assembly */
typedef struct operation{
    int opcode; /* operation code */
    int func; /* operation fucntion number */
    char *name; /* operation name */
    int source_addressing[3]; /*operation source operand addressing types accepted */
    int dest_addressing[3]; /*operation destination operand addressing types accepted */
} operation;

/* symbol table node definition */
typedef struct node *ptr;
typedef struct node{ /* represents a symbol in a linked list */
    char* name; /* name of the symbol */
    int address; /* index of the symbol in the data or code image */
    int type; /* The type of the symbol - extern, code or data */
    int is_entry; /* flag showing if this symbol is marked 'entry' */
    ptr next; /*  pointer to the next symbol in the list */
}node;
