#include <ctype.h>
#include <math.h>
#include "declarations.h"

/* store each definition name and the number that matches it */
definition definitions[] = {{DATA, ".data"}, {STRING, ".string"},
                                      {EXTERN, ".extern"}, {ENTRY, ".entry"}};

int add_to_symbol_table(char *, int, int);


machine_word **data_image; /* Holds the data instructions in the current file, translated to machine code */

/**
 *  find if line is a .data or .string instruction
 *  @return OK if it is, ERROR if its not
 */
int is_data_or_string()
{
    if(!strcmp(line_parsed[index(0)],definitions[0].name)||!strcmp(line_parsed[index(0)],definitions[1].name)) {
        if(SYMBOL_FLAG)
            add_to_symbol_table(line_parsed[0],DATA,DC);
        return OK;
    }
    return ERROR;
}

/**
 * inserts a number to data image
 * @param num - the number to store in data image
 * @return OK if finished successfully
 */
int insert_num_to_data_image(int num)
{
    data_image=(machine_word **)realloc(data_image,(DC+1)*sizeof(machine_word*));
    memory_check(data_image)
    data_image[DC] = (machine_word*)calloc(1, sizeof(machine_word)); /* Allocating memory for each integer */
    memory_check(data_image[DC])
    data_image[DC++]->w = num;
    return OK;
}

/**
 * inserts a string to data image.
 * each character is stored in a separate cell as ASCII code
 * @param str - the string to store in data image
 * @return OK if finished successfully
 */
int insert_string_to_data_image(char *str) {
    int i = 1;
    for (; i < strlen(str); i++) {
        data_image=(machine_word **)realloc(data_image,(DC+1)*sizeof(machine_word*));
        memory_check(data_image)
        data_image[DC] = (machine_word *) calloc(1, sizeof(machine_word)); /* Allocating memory for each integer */
        memory_check(data_image[DC])
        data_image[DC++]->w = str[i];
    }
    data_image[DC - 1]->w = '\0';
    return OK;
}

/**
 * finds the string in the line, checks if valid and inserts to data image
 * @param line - the original line from the file, before parse
 * @return ERROR if errors found or line syntax is illegal, else OK
 */
int get_string(char *line)
{
    int i= strlen(line) - 2; /* set i as index of last character in line_before_parse, ignoring '\n' at the end*/
    char *opening_quote=strchr(line, QUOTE); /*find first appearance of quote mark */
    char closing_quote;
    while(line[i] == ' ' || line[i] == '\t')
        i--;
    closing_quote=line[i]; /* find last appearance of quote mark */
    line[i + 1]='\0'; /*end the string after last quote mark */
    if(!opening_quote||closing_quote!=QUOTE||line_parsed[index(1)][0]!=QUOTE) /* if quote marks are missplaced */
        return print_error("illegal operand, expected string surrounded by \" \"");
    return insert_string_to_data_image(opening_quote); /*insert to data image and return ERROR or OK from function*/
}

/**
 * finds the numbers in the line, checks if valid and inserts to data image
 * @return ERROR if errors found or line syntax is illegal, else OK
 */
int get_data()
{
    int j, i = index(1), ERROR_FLAG = OK;
    /* scan each cell in line parsed */
    for (; i < line_parsed_len; i++) {
        /* if cell's first char is not +,- or digit, return ERROR */
        if (!line_parsed[i] ||((line_parsed[i][0] != PLUS && line_parsed[i][0] != MINUS) && !isdigit(line_parsed[i][0])))
            return print_error("illegal operand, expected int");
        j = 1;
        /* if one of cell's next chars is not digit, return ERROR */
        while (line_parsed[i][j]!='\0') {
            if (!isdigit(line_parsed[i][j]))
               return print_error("illegal operand, expected int");
            j++;
        }
        /* if number is bigger than 2^23 its too big for machine code word */
        if ((atoi(line_parsed[i]) > pow(2.0, 21.0)/2)||(atoi(line_parsed[i]) < (pow(2.0, 21.0)/-2)))
            return print_error("illegal operand, number not in range");
        /* if reached here, number is legal. insert to data image. */
        ERROR_FLAG = insert_num_to_data_image(atoi(line_parsed[i]));
    }
    return ERROR_FLAG;
}

/*
 * finds if line is data or string instrution, and acts accordingly
 */
int get_data_or_string(char *line)
{
    if(!strcmp(line_parsed[index(0)],definitions[1].name)) /* if .string */
        return get_string(line);
    return get_data();
}
