
#include <ctype.h>
#include "declarations.h"

#define BIT(x) (1u<<x)

int extern_count; /* Extern variables appearances counter */
ptr symbol_table_head=NULL; /* The head of the symbol table */
definition **externs;
/* functions */
int get_bits(unsigned int, unsigned int);
void set_val(machine_word *, int );
void set_ARE(machine_word *, int);
int is_name_free(char*);
int add_to_symbol_table(char *, int, int);

/**
 * detects a symbol definition at a start of a line.
 * if symbol found, turn on SYMBOL_FLAG.
 * if in first pass, check if syntax is ok, and if its name is free. store the label in symbol table.
 * if in second pass, return.
 * @param pass_num - flag represents if calling function is first pass or second pass
 * @return OK if a label found and its legal, or if not found, ERROR if found and illegal
 */
int is_label(int pass_num) {
    int i = 1;
    char *word=line_parsed[0];
    if (word[strlen(word) - 1]!=COLON) { /*if not symbol, turn off symbol_flag and return */
        SYMBOL_FLAG = 0;
        return OK;
    }
    if(pass_num==2) { /* if it is symbol and its second pass, turn on symbol_flag and return */
        SYMBOL_FLAG = 1;
        return OK;
    }
    /* if it is symbol and it's first pass, check if symbol name is legal */
    word[strlen(word) - 1] = '\0'; /*removing ':' */
    /* check if label name is legal */
    if (!isalpha(word[0])) /* if first char is not a letter */
        return print_error("label name is illegal");
    while (word[i]) { /*if name contains a char thats not a digit or a letter */
        if (!isalpha(word[i]) && !isdigit(word[i]))
            return print_error("label name is illegal");
        i++;
    }
    if (i > MAX_LABEL_SIZE) /* if label length exceeds 31 chars */
        return print_error("label name is too long");
    if(!is_name_free(word))   /* check if name not already exists or reserved word */
        return ERROR;
    if(line_parsed_len==1) /* if line contains only label */
        return print_error("definition of label without operation or instruction");
    SYMBOL_FLAG = 1;
    return OK;
}

/**
 *  check if name given to a label is legal
 *  @param word - the name of the label
 *  @return OK if free and legal, ERROR if not
 */
int is_name_free(char *word) {
    int i;
    ptr p=symbol_table_head;
    for (i = 0; i < OPS_NUM; i++)
    {   /*if label name equals operation or instruction name, return error */
        if ((!strcmp(word, operations[i].name))||((i<DEFS_NUM)&&!strcmp(word, definitions[i].name)))
            return print_error("reserved words in assembly can't be assigned as label");
    }
    if(strlen(word) == 2 && word[0] == 'r' && word[1] >= '0' && word[1] < '8')
        return print_error("reserved words in assembly can't be assigned as label");
    /*check if name already exists in symbol table */
    while(p) {
        if (!strcmp(word, p->name))
            return print_error("label already exists");
        p=p->next;
    }
    return OK;
}


/**
 * Adding a label to the symbol table
 * @param name - the name of the label
 * @param type - the type of the label, according to the def_num enum
 * @param address - the address of the new symbol in the memory
 * @return ERROR if there's another definition of the label and OK otherwise
 */
int add_to_symbol_table(char* name, int type, int address)
{
    ptr p, new_symbol = (ptr)malloc(sizeof(node)); /* memory for the new symbol */
    memory_check(new_symbol)
    new_symbol->name = (char *)malloc(strlen(name) + 1); /* memory for the symbol's name */
    memory_check(new_symbol->name)
    strcpy(new_symbol->name, name); /* assigning symbols attributes */
    new_symbol->address = address;
    new_symbol->type = type;
    new_symbol->is_entry = 0;
    new_symbol->next = NULL; /* The node is now the tail of the symbol_table */
    if(!symbol_table_head) /* If the symbol_table is empty, the new node will be the head of the symbol_table */
        symbol_table_head = new_symbol;
    else { /* else,  scan the list until next is empty */
            p = symbol_table_head;
            while (p->next)
                p = p->next;
            p->next = new_symbol;
        }
    return OK;
}

/**
 * determines whether current line is an entry or extern type
 * if its extern - if its first pass - insert symbol to symbol table, else - return.
 * if its entry - return OK (in second pass a function called to mark entries)
 * @param pass_num - represents if its first pass or second pass
 * @return OK if found extern or entry, ERROR if its not
 */
int entry_or_extern(int pass_num)
{
    if(!strcmp(line_parsed[index(0)],definitions[3].name)) /* if it is entry */
        return OK;
    else if(!strcmp(line_parsed[index(0)],definitions[2].name)) { /* if it is extern */
        if(pass_num==1) {
            if (line_parsed_len == 1) /* if missing label name, return OK and let first pass find the error */
                return OK;
            return add_to_symbol_table(line_parsed[index(1)], EXTERN, 0);
        }
        if(pass_num==2)
            return OK;
    }
    return ERROR;
}

/**
 * increase address of each symbol of type DATA by IC + 100
 */
void increase_address_to_data()
{
    ptr p = symbol_table_head;
    while(p) /* Scanning the symbol_table */
    {
        if(p->type == DATA) /* If the symbol's type is data, we increase its address by IC + 100 */
            p->address += (IC + START_OF_CODE);
        p = p->next;
    }
}

/**
 * add entry to a given symbol
 * @param label - the name of the desired symbol
 * @return ERROR if the label isn't defined in the code, else OK
 */
int add_entry(char * label)
{
    ptr p = symbol_table_head;
    if(!strcmp(line_parsed[index(0)],definitions[2].name))
        return OK; 
    while(p) /* Scanning the symbol_table */
    {
        if(!strcmp(p->name,label)) {
            p->is_entry = OK;
            return OK;
        }
        p=p->next;
    }
    /* if reached end of symbol table, label is not found */
    return print_error("label not exists");
}

/**
 * if extern used in the code, add to externs list the name and address of use
 * @param name - name of extern label
 * @param index - the address of the appearence
 * @return OK if insertion succeeded, ERROR if not
 */
int add_to_externs(char *name, int index)
{
    /* allocate memory for externs list and cell */
    externs = (definition **)realloc(externs, (1+extern_count) * sizeof(definition *));
    memory_check(externs)
    externs[extern_count] = (definition *)malloc(sizeof(definition));
    memory_check(externs[extern_count])
    externs[extern_count]->name = (char *)malloc(strlen(name)+1);
    memory_check(externs[extern_count]->name)
    /* copy the name and address to the structure */
    strcpy(externs[extern_count]->name, name);
    externs[extern_count++]->num = index;
    return OK;
}

/**
 * called in second pass on operation lines.
 * if symbol appears, adds the address of the symbol from symbol table to code image
 * @return ERROR if label not exists in symbol table, OK if process ended succefully
 */
int add_address_to_symbols()
{
    enum operands {ADDRESSING_LEN=2,DEST_ADDRESSING_FIRST_BIT = 11,SRC_ADDRESSING_FIRST_BIT= 16};
    ptr p = symbol_table_head;
    char *src_value,*dest_value;
    int ERROR_FLAG=OK, L=0, src_distance=0, dest_distance=0;
    /*retrieve source addressing and destination addressing from machine code word in code image */
    unsigned int src_addressing=get_bits(SRC_ADDRESSING_FIRST_BIT, ADDRESSING_LEN);
    unsigned int dest_addressing=get_bits(DEST_ADDRESSING_FIRST_BIT, ADDRESSING_LEN);
    /*if source addressing is direct or relative - find the the symbol */
    if(src_addressing==DIRECT||src_addressing==RELATIVE) {
        L++;
        if (src_addressing == RELATIVE) {
            src_distance = START_OF_CODE + IC;
            src_value = line_parsed[index(1)] + 1;
        }
        else src_value = line_parsed[index(1)];
        while(p) /* scan symbol_table */
        {
            if (!strcmp(p->name,src_value)) {
                /* set address or distance value in the next word and set ARE accordingly */
                set_val(code_image[IC+L],p->address-src_distance);
                set_ARE(code_image[IC+L],src_distance ? A : (p->type==EXTERN ? E : R));
                if(p->type==EXTERN)
                    ERROR_FLAG = add_to_externs(p->name, START_OF_CODE+IC + L);
                break;
            }
            p=p->next;
        }
        if(!p)
            ERROR_FLAG= print_error("label in source operand not exists");
    }
    /* if type of source is IMMEDIATE, increment L and continue */
    else if(src_addressing==IMMEDIATE&&line_parsed_len-SYMBOL_FLAG==3&&line_parsed[index(1)][0]==SHARP)
        L++;

    /*if destination addressing is direct or relative - find the the symbol */
    if(dest_addressing==DIRECT||dest_addressing==RELATIVE) {
        L++;
        p=symbol_table_head;
        if (dest_addressing == RELATIVE) {
            dest_distance = START_OF_CODE + IC;
            dest_value = line_parsed[line_parsed_len - 1] + 1;
        }
        else dest_value = line_parsed[line_parsed_len-1];
        while(p) /* scan symbol_table */
        {
            if (!strcmp(p->name,dest_value)) {
                /* set address or distance value in the next word and set ARE accordingly */
                set_val(code_image[IC+L],p->address-dest_distance);
                set_ARE(code_image[IC+L],dest_distance ? A : p->type==EXTERN ? E : R);
                if(p->type==EXTERN)
                    ERROR_FLAG = add_to_externs(p->name, START_OF_CODE+IC + L);
                break;
            }
            p=p->next;
        }
        if(!p)
            ERROR_FLAG= print_error("label in destination operand not exists");
    }
    /* if type of destination is IMMEDIATE, increment L and continue */
    else if(dest_addressing==IMMEDIATE&&line_parsed[line_parsed_len-1][0]==SHARP)
        L++;
    /* if line doesnt contain operands with immediate, relative or direct addressing, L is still 0 */
    IC+=(L+1);
    return ERROR_FLAG;
}
