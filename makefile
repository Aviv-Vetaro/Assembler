assembler:  assembler.o buffer.o command.o exception.o instruction.o lable.o list.o output.o symbol_table.o utils.o
	gcc -g -Wall -ansi -pedantic assembler.o buffer.o command.o exception.o instruction.o lable.o list.o output.o symbol_table.o utils.o -o assembler

assembler.o: assembler.c buffer.h lable.h exception.h utils.h list.h command.h instruction.h symbol_table.h output.h
	gcc -c assembler.c -o assembler.o
    
buffer.o: buffer.c buffer.h list.h
	gcc -c buffer.c -o buffer.o

command.o: command.c command.h asm_line_types.h exception.h symbol_table.h utils.h
	gcc -c command.c -o command.o

exception.o: exception.c exception.h
	gcc -c exception.c -o exception.o
    
instruction.o: instruction.c instruction.h lable.h utils.h asm_line_types.h exception.h symbol_table.h
	gcc -c instruction.c -o instruction.o
    
lable.o: lable.c lable.h utils.h asm_line_types.h symbol_table.h exception.h
	gcc -c lable.c -o lable.o

list.o: list.c list.h exception.h
	gcc -c list.c -o list.o

output.o: output.c output.h list.h utils.h asm_line_types.h
	gcc -c output.c -o output.o            

symbol_table.o: symbol_table.c symbol_table.h exception.h asm_line_types.h command.h
	gcc -c symbol_table.c -o symbol_table.o

utils.o: utils.c utils.h exception.h
	gcc -c utils.c -o utils.o
