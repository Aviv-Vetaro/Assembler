#include "output.h"
#include <stdbool.h>
#include "list.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "asm_line_types.h"
#include <string.h>
/*outputs to the user (via .ob, .ent, .ext files) the binaries, and information about entry
 * and extern labels (for the linker). note tht no output file would get created if any fatal
 * error has occurred in the assembly code / system an that .ent or .ext files may not be created
if corresponding labels have not been found*/
bool output(char *path, struct list commands, unsigned int IC, unsigned int DC)
{
	unsigned int i;
	FILE* asm_file;
	/*check weather any errors have occurred*/
	if (is_valid_file())
	{
		/*create (if needed) and write to .ent and .ext files*/
		output_entry(path);
		output_extern(path);
		/*add the .ob file extension*/
		path = concat(path, ".ob");
		/*open the file*/
		asm_file = fopen(path, "w");
		free(path);
		if (asm_file == NULL)
		{
			throw_exp("Failed to create output files");
			return false;
		}
		/*print the "title" - info about the overall size of the data and instruction sections
		 * of the assembly file*/
		fprintf(asm_file, "% 7d %u\n", IC - 100, DC - IC);
		/*print the binaries*/
		for (i = 0; i < commands.length; i++)
		{
			fprintf(asm_file, "%07u %06x\n", i + 100, ((union word *)(get(&commands, i)))->machine_code);
		}
		foreach(commands, &free);
		free_ls(&commands);
		return true;
	}
	return false;
}
/*print to an .ext file all the references to extern labels*/
bool output_extern(char* path)
{
	struct list externs = get_extern_symbols();
	struct list refs = get_refs();
    FILE* ext_file;
	int i, j, counter = 0;
	struct symbol* current_symbol;
	struct reference* current_reference;
	/*if there are no extern labels don't create a file*/
	if (externs.length == 0)
	{
		free_ls(&externs);
		return false;
	}

	/*check if there are any references and create a file only if there are*/
	for (i = 0; i < externs.length; i++)
	{
		current_symbol = get(&externs, i);
		for (j = 0; j < refs.length; j++)
		{
			current_reference = get(&refs, j);
			if (!strcmp(current_symbol->label, current_reference->referenced))
				counter++;
		}
	}
	if(counter == 0)
		return false;
	
	/*add the .ext file extension*/
	path = concat(path, ".ext");
	ext_file = fopen(path, "w");
	if (ext_file == NULL)
	{
		free_ls(&externs);
		throw_exp("Failed to create output files");
		return false;
	}
	/*for each reference that references an extern label print the reference info*/
	for(i = 0; i < externs.length; i++)
	{
		current_symbol = get(&externs, i);
		for (j = 0; j < refs.length; j++)
		{
			current_reference = get(&refs, j);
			if(!strcmp(current_symbol->label, current_reference->referenced))
			{
				fprintf(ext_file, "%s %07u\n", current_reference->referenced, current_reference->referencer_address);
			}
		}
	}
	free_ls(&externs);
	return true;
}
bool output_entry(char* path)
{
	struct list entries = get_entry_symbols();
	FILE* ent_file;
	unsigned int i;
	/*don't create the file if there are no entry labels*/
	if (entries.length == 0)
	{
		free_ls(&entries);
		return false;
	}
	/*add the .ent file extension*/
	path = concat(path, ".ent");
	ent_file = fopen(path, "w");
	if (ent_file == NULL)
	{
		throw_exp("Failed to create output files");
		free_ls(&entries);
		return false;
	}
	/*print each entry label's info into the file*/
	for(i = 0; i < entries.length; i++)
		fprintf(ent_file, "%s %07u\n", ((struct symbol*)get(&entries, i))->label, ((struct symbol*)get(&entries, i))->address);
	free_ls(&entries);
	return true;
}
