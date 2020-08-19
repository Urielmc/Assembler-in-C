
#include "declarations.h"

extern int line_num /* The line number in the current file */;

/* functions */
int parse_line(char *, int); /*defined in parsing.c */
int empty_or_comment_line(char*); /*defined in parsing.c */
int is_data_or_string(); /*defined in data_image.c */
/*defined in labels.c */
int entry_or_extern(int);
int is_label(int);
int add_address_to_symbols();
int add_entry(char *);

/**
 * preforms the second pass on .as file. find symbols in operation lines and updates code image accordingly
 * @param fptr - pointer to current file scanned
 * @return OK if no errors found, ERROR if errors found
 */
int second_pass(FILE *fptr)
{
    char line_from_file[MAX_LINE_SIZE];
    int ERROR_FLAG = OK;
    SYMBOL_FLAG = 0;
    IC = 0, DC = 0;
    for (line_num = 1; fgets(line_from_file, MAX_LINE_SIZE, fptr); line_num++) /* Scanning through each line of the file */
    {
        if (empty_or_comment_line(line_from_file)) /*skip the line if it is empty or comment line */
            continue;
        if (!parse_line(line_from_file, 2)) /* parse line again */
            ERROR_FLAG = ERROR;
        if (!is_label(2)) /* turn on SYMBOL_FLAG if there is a label at start of the line */
            ERROR_FLAG = ERROR;
        if (is_data_or_string())  /*skip the line if it is .string or .data instruction */
            continue;
        if (entry_or_extern(2)) { /*if it is entry or extern line, add entries to symbol table or extern to externs list */
            ERROR_FLAG = add_entry(line_parsed[index(1)]);
            continue;
        }
        if(!add_address_to_symbols()) /* if it is operation line, scan line to find symbols and update address or distance on code image */
            ERROR_FLAG = ERROR;
    }
    return ERROR_FLAG;
}