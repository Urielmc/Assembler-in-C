
#include "declarations.h"

/*functions */
void get_files(int, char *[]);
void free_allocated_memory();

/* main function. gets files names in command line and calls get_files to start the process of the assembler */
int main(int argc, char *argv[])
{
    get_files(argc,argv);
    free_allocated_memory();
    return 0;
}

extern char *file_name /* The name of the current file */;
extern int line_num /* The line number in the current file */;
/**
 * prints an error, given by the parameter of the character pointer, to stderr
 * @param - error - description of the error to print
 * @return ERROR
 */
int print_error(char *error)
{
    fprintf(stdout, "Error in file %s in line %d: %s\n", file_name, line_num, error);
    return ERROR;
}

/**
 * frees all memory allocated for global structures and variables
 */
void free_allocated_memory()
{
    int i = 0;
    ptr next, p = symbol_table_head;
    for(; i < ICF-START_OF_CODE; i++) /* free code image */
        free(code_image[i]);
    for(; i < DCF; i++) /* free data image */
        free(data_image[i]);
    for(i = 0; i < extern_count; i++) /* frees the extern_appearances list */
    {
        free(externs[i]->name);
        free(externs[i]);
    }
    free(externs);
    for(i = 0; i < line_parsed_len; i++) /* free the line parsed array */
        free(line_parsed[i]);
    free(line_parsed);
    while(p) /* free symbol table */
    {
        next = p->next;
        free(p);
        p = next;
    }
}