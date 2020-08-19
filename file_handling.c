
#include "declarations.h"

char *file_name /* The name of the current file (used mainly by print_error) */;

/* functions */
int first_pass(FILE *);
int second_pass(FILE *);
void make_ext_file(char *name);
void make_ob_file(char *name);
void make_ent_file(char *name);

/**
 * Opens the assembly files, execute process, write output files
 * @param argc - the number of command line arguments
 * @param argv - the command line arguments
 */
void get_files(int argc, char *argv[])
{
    FILE *fptr;
    int i=1;
    for(; i < argc; i++) /* Scanning argv for the file name to be read */
    {
        /* Assigns memory for the name of the file and copies the name of the file, including the ".as" ending, to it */
        file_name = (char *)malloc(strlen(argv[i]) + strlen(".as")+1);
        if(!file_name)
            fprintf(stderr, "memory allocation failed");
        strcpy(file_name, argv[i]);
        strcat(file_name,".as");
        fptr = fopen(file_name, "r");/* Opens the file, if it exists, for reading */
        if(!fptr) /* If the wasn't found, or it isn't allowed for reading, the file pointer is NULL */
        {
            fprintf(stderr, "Couldn't open file %s\n", file_name);
            continue;
        }
        /* begin first_pass and second_pass, and if finished successfully - writes output files */
        if(first_pass(fptr) && second_pass(fptr)) {
            make_ob_file(argv[i]);
            make_ent_file(argv[i]);
            make_ext_file(argv[i]);
        }
        fclose(fptr); /* Closes the file after reading and frees the file_name string for the next file name */
        free(file_name);
    }
}

/**
 * creates a file and returns a pointer (with reading permission with the given name and ending
 * @param name - file's name
 * @param ending - file's ending
 * @return a file pointer to the new file
 */
FILE *create_file(char *name, char *ending)
{
    FILE *fptr;
    char *out_name = (char *)malloc(strlen(name) + strlen(ending)+1);
    if(!out_name)
        print_error("memory allocation failed");
    strcpy(out_name, name);
    strcat(out_name, ending);
    fptr = fopen(out_name, "w"); /* Opens the file, with writing permission */
    free(out_name);
    return fptr;
}

/**
 * writes the ".ob" file
 * @param name - the name of the given file
 */
void make_ob_file(char *name)
{
    FILE *fptr = create_file(name, ".ob"); /* Gets the file with ".ob" ending and writing permission */
    int i = 100;
    fprintf(fptr, "\t%d %d\n", ICF-START_OF_CODE, DCF); /* Prints the length of code_image and data_image */
    for(; i < ICF; i++) /* print all instruction words */
        fprintf(fptr, "%07d %06x\n", i, code_image[i-START_OF_CODE]->w);
    for(; i < DCF+ICF; i++) /* print all data words */
        fprintf(fptr, "%07d %06x\n", i , data_image[i-ICF]->w);
    fclose(fptr);
}

/**
 * writes the ".ent" file
 * @param name - the name of the given file
 */
void make_ent_file(char *name)
{
    int entry_found = ERROR; /*as long as entry is not found, it remains error */
    FILE *fptr;
    ptr p = symbol_table_head;
    while (p) /* Scanning the symbol_table */
    {
        if (p->is_entry == OK)
        {
            if(!entry_found) /* if the first entry found */
            {
                entry_found = OK; /* To not enter this condition again */
                fptr = create_file(name, ".ent"); /* Gets the file with ".ent" ending and writing permission */
            } /* print every entry symbol + its address to the file */
            fprintf(fptr, "%s %07d\n", p->name, p->address);
        }
        p = p->next;
    }
    if(entry_found)
        fclose(fptr); /* if file is created, free the file pointer */
}

/**
 * writes the ".ext" file
 * @param name - the name of the given file, without ".as" ending
 */
void make_ext_file(char *name)
{
    FILE *fptr;
    int i = 0;
    if(extern_count==0) /* return if there's no external variables appearances */
        return;
    fptr = create_file(name, ".ext"); /* Gets the file with ".ext" ending and writing permission */
    for(; i < extern_count; i++) /* from extern appearances list, print each extern appearance to the file */
        fprintf(fptr, "%s\t%07d\n", externs[i]->name, externs[i]->num);
    fclose(fptr); /* Frees the file pointer after use */
}