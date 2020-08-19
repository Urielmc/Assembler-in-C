
#include "declarations.h"

/**
 * Check if there are commas placed wrong. If there are - print an error message
 * @param line_len - number of words in line_parsed array
 * @return ERROR or OK, according to result of check
 */
int error_in_commas(int line_len)
{
    int i;
    for(i=0; line_parsed[0][i]!='\0'; i++) /*first word cant contain ',' */
       if(line_parsed[0][i]==COMMA)
           return ERROR;
    if(line_len>1&&line_parsed[1][0]==COMMA) /*second word cant contain ',' as first character*/
        return ERROR;
    if(line_parsed[line_len-1][strlen(line_parsed[line_len-1])-2]==COMMA) /*last character cant be ','*/
        return ERROR;
    return OK;
}

/**
 * Check if there are consecutive commas anywhere in the line. If there are - print an error message
 * @param line - the string to check
 * @return ERROR or OK, according to result of check
 */
int consecutive_commas(char *line)
{
    int i=0,comma=OK;
    for(; i < strlen(line); i++)
    {
        if(!comma && line[i] !=SPACE && line[i] != '\t'&&line[i] !=COMMA)
            comma=OK;
        if(!comma && line[i] == COMMA) { /* If comma flag is on and in line[i] we found comma, it is an error */
            return ERROR;
        }
        else if(line[i] == COMMA)
            comma=ERROR;
    }
    return OK;
}

/**
 * Parsing a line from the file to an array of strings.
 * if it is first pass - first parse words by spaces and tabs, second parse by commas as well.
 * between first and second parse checks for error in commas placement.
 * return ERROR if memory allocation failed or if found error in commas placement.
 * @param line_to_parse - the original line from the file, passed as string
 * @param pass_num - flag represents if calling function is first pass or second pass
 */
int parse_line(char *line_to_parse, int pass_num)
{
    int i, error_flag=OK;
    char *token;
    /*strtok function modifies its string parameter, so create a copy of original line, for first and second parse*/
    char *line_copy1=(char*)malloc(strlen(line_to_parse) + 1);
    char *line_copy2=(char*)malloc(strlen(line_to_parse) + 1);
    memory_check(line_copy1)
    memory_check(line_copy2)
    strcpy(line_copy1, line_to_parse); /*store original line*/
    strcpy(line_copy2, line_to_parse);/*store original line*/
    if(pass_num==1) /* if first pass, perform first pass then check for errors in commas */
    {
        line_parsed = (char **)malloc(sizeof(char *)); /*line parsed (global char*) is the main array holding the line parsed by words */
        memory_check(line_parsed)
        /* Getting the first argument */
        token = strtok(line_copy2, " \t\n");
        i=0;
        while(token!=NULL) /* Get arguments into array */
        {
            line_parsed[i] = (char *)malloc(strlen(token)+1);
            memory_check(line_parsed[i])
            strcpy(line_parsed[i],token);
            i++;
            token = strtok(NULL, " \t");
            if(token!=NULL) {
                line_parsed = (char **) realloc(line_parsed, (i + 1) * sizeof(char *));
                memory_check(line_parsed)
            }
        }
        if(!error_in_commas(i)||!consecutive_commas(line_copy1))
            error_flag= print_error("wrong placement of commas");
        line_parsed_len=i;
    }
    /* second parse, parse by and remove commas as well as spaces and tabs */
    token = strtok(line_copy1, ", \t\n");
    i=0;
    line_parsed = (char **)malloc(sizeof(char *));
    memory_check(line_parsed)
    while(token!=NULL) /* Get arguments into array */
    {
        line_parsed[i] = (char *) malloc(strlen(token)+1);
        memory_check(line_parsed[i])
        strcpy(line_parsed[i],token);
        i++;
        token = strtok(NULL, ", \t\n");
        if(token) {
            line_parsed=(char **) realloc(line_parsed, ((i+1) * sizeof(char *)));
            memory_check(line_parsed)
        }
    }
    line_parsed_len=i;
    free(line_copy1); /* line copies are no longer needed */
    free(line_copy2);
    return error_flag;
}

/**
 * check if current line from file is empty or a comment line. if so - return ok to skip it, else return error
 * @param line - the line from the file passed as string
 */
int empty_or_comment_line(char* line)
{
    char *token;
    char *copy=(char *)malloc(strlen(line)+1);
    memory_check(copy)
    strcpy(copy,line);
    token = strtok(copy, " \t\n");
    if(!token||token[0]=='\0'||token[0]==';') /* if line is empty or comment, return OK*/ {
        free(copy);
        return OK;
    }
    free(copy);
    return ERROR;
}

