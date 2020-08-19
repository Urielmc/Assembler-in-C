assembler: assembler.o file_handling.o first_pass.o labels.o parsing.o code_image.o data_image.o second_pass.o
	gcc -g -ansi -Wall -pedantic assembler.o file_handling.o first_pass.o labels.o parsing.o code_image.o data_image.o second_pass.o -o assembler -lm

assembler.o: assembler.c declarations.h
	gcc -c -g -ansi -Wall -pedantic assembler.c -o assembler.o

file_handling.o: file_handling.c declarations.h
	gcc -c -g -ansi -Wall -pedantic file_handling.c -o file_handling.o

first_pass.o: first_pass.c declarations.h
	gcc -c -g -ansi -Wall -pedantic first_pass.c -o first_pass.o

labels_functions.o: labels.c declarations.h
	gcc -c -g -ansi -Wall -pedantic labels.c -o labels.o

code_functions.o: code_image.c declarations.h
	gcc -c -g -ansi -Wall -pedantic code_image.c -o code_image.o

data_functions.o: data_image.c declarations.h
	gcc -c -g -ansi -Wall -pedantic data_image.c -o data_image.o

second_pass.o: second_pass.c declarations.h
	gcc -c -g -ansi -Wall -pedantic second_pass.c -o second_pass.o

parsing_functions.o: parsing.c declarations.h
	gcc -c -g -ansi -Wall -pedantic parsing.c -o parsing.o
