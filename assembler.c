#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "lable.h"
#include "exception.h"
#include "utils.h"
#include "list.h"
#include "command.h"
#include "instruction.h"
#include "symbol_table.h"
#include "output.h"
/*TODO butify code according to C89*/
/*TODO try on other files*/
/*TODO check for ubuntu support*/
/*TODO try at multiple files*/
bool convert_to_machine_lang(char* asm_file_path);
int main(int argc, char* argv[])
{
	int i;
	/*if no files were provided, print an error message and exit the program*/
	if(argc == 1)
	{
		throw_exp("No files provided");
		return 1;
	}
	/*for each file provided, start the assembler's operation on that file*/
	for(i = 1; i < argc; i++)
		convert_to_machine_lang(argv[i]);
}
/*the method receives an assembly file path (without the file extension) and output to the corresponding
 * output files the content of the assembly file converted to binary machine code, as well as
 entries and external labels in the program. returns true on success.*/
bool convert_to_machine_lang(char *asm_file_path)
{
	/*add file extension*/
	char* full_path = concat(asm_file_path, ".as");
	int i = 0;
	char *current_line;
	struct list current_command, result = init();
	struct line_statement *current_line_statement;
	/*open input file*/
	FILE* asm_file = asm_fopen(full_path);
	unsigned int IC = 100, DC;
	/*provide info to the user about the file in which errors (if any) occurs*/
	set_file(full_path);
	init_symbols();
	/*check if the file was successfully opened, and if not, print an error message*/
	if (asm_file == NULL)
	{
		return false;
	}
	init_buf(asm_file);
	/*loop through all the lines in the file, converting each line to
	 * binary code. notice that some of the binaries would be incomplete as
	 the linkage between label's addresses and the commands has yet
	 to accrue*/

	/*The first loop will only handle commands, ignoring comments, empty lines, as
	 * well as instructions. notice that is_eof() resets the current line being return to
	 * the start when it reaches the end.*/
	while (!is_eof())
	{
		i++;
		/*provide info to the user about the line in which errors (if any) occurs*/
		set_line(i);
		/*get next line*/
		current_line = get_line();
		/*ignore all lines but commands*/
		if(current_line == NULL)
			continue;
		if (is_whitespace(current_line))
			continue;
		if (*current_line == ';')
			continue;
		if (strchr(current_line, '.'))
		{
			continue;
		}
		/*get the part in the command after the label, inserting the label to
		 * the symbol table in the process*/
		current_line = get_label(current_line, IC);
		if (current_line == NULL)
		{
			throw_exp("Statement can not contain label only");
			continue;
		}
		/*convert the command (without the label) into machine code.
		 please note the return value: it would be list of 1-3 numeric values,
		 (in form of a union), or an empty list with invalid values in case of an error
		 (the assembler is structured in a way that the function itself reports errors,
		 and not the caller, but the caller will notice that an error occurred by the return values)*/
		current_command = cmd_to_numeric(current_line, IC);
		/*add to the instructions counter the amount of words the current line has taken*/
		IC += current_command.length;
		/*add the words to the overall list of words generated from the file*/
		lstcat(&result, &current_command);
		free_ls(&current_command);
	}
	/*the data counter starts at the end of the instructions counter*/
	DC = IC;
	i = 0;
	/*as long as the end of the file has not been reached*/
	while(!is_eof())
	{
		i++;
		/*provide info to the user about the line in which errors (if any) occurs*/
		set_line(i);
		/*get next line*/
		current_line = get_line();
		if (current_line == NULL || *current_line == ';')
			continue;
		/*convert to binaries only instruction ignoring comments, empty lines, as
	    well as commands*/
		if (strchr(current_line, '.'))
		{
			/*get the part in the instruction after the label, inserting the label to
			the symbol table in the process*/
			current_line = get_label(current_line, DC);
			/*convert the instruction (without the label) into machine code.*/
			current_command = instruction_to_numeric(current_line);
			/*add to the data counter the amount of words the current line has taken*/
			DC += current_command.length;
			/*add the words to the overall list of words generated from the file*/
			lstcat(&result, &current_command);
			free_ls(&current_command);
		}
	}
	/*complete the binaries by linking labels to label references found in the assembly code*/
	unreference();
	/*print into the output file the binaries, as well as info about the extern and entry
	 * labels in the code*/
	output(asm_file_path, result, IC, DC);
	free_symbol_table();
	free(full_path);
	free_buf();
	return true;
}
