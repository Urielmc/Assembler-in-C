
#include "declarations.h"

/*global variables */
int SYMBOL_FLAG,
IC, /* Instruction counter */
ICF, /* final Instruction counter */
DC, /* Data counter */
DCF, /* final Data counter */
line_num, /* The line number in the current file */
line_parsed_len /* The length of line_parsed array */;
char **line_parsed /* Holds current line read from the file, parsed by function parse_line*/;

/*functions*/
int parse_line(char *, int); /*defined in parsing.c */
int get_operation(char *); /*defined in code_image.c */
int empty_or_comment_line(char*); /*defined in parsing.c */
int is_data_or_string(); /* defined in data_image.c */
int get_data_or_string(char *); /*defined in data_image.c */
/*defined in labels.c */
void increase_address_to_data();
int entry_or_extern(int);
int is_label(int);


/**
 * preforms the first pass on the given file, searching for most of the syntax errors,
 * and translating most of the .as file to machine code
 * @param fptr - pointer to current file scanned
 * @return OK if no errors found, ERROR if errors found
 */
int first_pass(FILE *fptr)
{
    char line_from_file[MAX_LINE_SIZE];
    int ERROR_FLAG = OK;
    SYMBOL_FLAG=0;
    IC = 0, DC = 0;
    for(line_num = 1; fgets(line_from_file, MAX_LINE_SIZE, fptr); line_num++) /* Scanning through each line of the file */
    {
        if (empty_or_comment_line(line_from_file))
            continue;
        if (!parse_line(line_from_file, 1)) {/* parse_line splits the line to an array in which every word in a separate cell */
            ERROR_FLAG = ERROR;
            continue;
        }
        if (!is_label(1)) {/* check if first word is a label, if yes (raises flag) or no returns ok, if yes and illegal returns error */
            ERROR_FLAG = ERROR;
            continue;
        }
        if (is_data_or_string()) {
            if (!get_data_or_string(line_from_file))
                ERROR_FLAG = ERROR;
            continue;
        }
        if (entry_or_extern(1)) {
            if((line_parsed_len - index(0)) > 2 || line_parsed_len==1)
                ERROR_FLAG=print_error("invalid text after end of command");
            continue;
        }
        if (!get_operation(line_from_file)) {
            ERROR_FLAG = ERROR;
            continue;
        }
    }
    ICF=START_OF_CODE+IC;
    DCF=DC;
    rewind(fptr);
    if(ERROR_FLAG)
        increase_address_to_data();
    return ERROR_FLAG;

}

