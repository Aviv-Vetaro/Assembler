#include "instruction.h"
#include "lable.h"
#include <stdlib.h>
#include "utils.h"
#include "asm_line_types.h"
#include "string.h"
#include "exception.h"
#include "symbol_table.h"
#include <ctype.h>
/*the function receives an data instruction, and return its corresponding binaries
 *(in list form) or an empty list if an error have occurred
 */
struct list instruction_to_numeric(char *instruction)
{
	/*look at first few chars in the instruction to find what instruction it is.
	 * call the corresponding function to analyze teh instruction and convert it
	 * into binaries*/
	if (instruction == NULL)
		return (struct list) { NULL, 0, 0 };
	if (!strncmp(instruction, ".string", strlen(".string")))
		return analyze_string(instruction);
	if (!strncmp(instruction, ".data", strlen(".data")))
		return analyze_data(instruction);
	if (!strncmp(instruction, ".extern", strlen(".extern")))
		return analyze_extern(instruction);
	if (!strncmp(instruction, ".entry", strlen(".entry")))
		return analyze_entry(instruction);
	throw_exp("instruction has not been found");
	return NOT_FOUND_LIST;
}
/*the function analyzes the ".string" instruction*/
struct list analyze_string(char* instruction)
{
    struct list results = init();
    char* temp_string_result;
    union word* current_word;
    if (instruction == NULL)
    {
        free_ls(&results);
        return NOT_FOUND_LIST;
    }
    /*get the first space-separated word (the instruction)*/
    temp_string_result = strtok(instruction, ASM_LINE_SPACES);
    if(temp_string_result == NULL)
    {
        free_ls(&results);
        throw_exp("Statement is missing mandatory components");
        return NOT_FOUND_LIST;
    }
    /*get the string itself and alert if not provided*/
    temp_string_result = strtok(NULL, "");
    if (temp_string_result == NULL)
    {
        free_ls(&results);
        throw_exp("Statement is missing mandatory components");
        return NOT_FOUND_LIST;
    }
    while (isspace(*temp_string_result) && *temp_string_result != '\0') temp_string_result++;
    /*check whether the string starts with a quotation mark*/
    if(*temp_string_result != '\"')
    {
        free_ls(&results);
        throw_exp("String must start with a \' \" \'");
        return NOT_FOUND_LIST;
    }
    temp_string_result++;
    /*iterate each char in the string*/
    while (*temp_string_result != '\"' && *temp_string_result != '\0')
    {
        /*allocate memory for the current char*/
        current_word = calloc(1, sizeof(union word));
        if(current_word == NULL)
        {
            foreach(results, &free);
            free_ls(&results);
            throw_exp("Memory allocation failed");
            return NOT_FOUND_LIST;
        }
        current_word->ascii_value.char_value = *temp_string_result;
        /*add the char to the list*/
        insert(&results, current_word);
        temp_string_result++;
    }
    /*check whether the string is terminated with a quotation mark*/
    if(*temp_string_result != '\"')
    {
        foreach(results, &free);
        free_ls(&results);
        throw_exp("String must be terminated with a \' \" \'");
        return NOT_FOUND_LIST;
    }
    /*check for redundant text at the same instruction*/
    if (!is_whitespace(temp_string_result + 1))
    {
        foreach(results, &free);
        free_ls(&results);
        throw_exp("redundant statement after the end of instruction");
        return NOT_FOUND_LIST;
    }
    /*insert a terminating null character*/
    current_word = calloc(1, sizeof(union word));
    if (current_word == NULL)
    {
        foreach(results, &free);
        free_ls(&results);
        throw_exp("Memory allocation failed");
        return NOT_FOUND_LIST;
    }
    current_word->ascii_value.char_value = '\0';
    insert(&results, current_word);
    return results;
}
/*the function analyzes the ".data" instruction*/
struct list analyze_data(char* instruction)
{
	struct list results = init();
	char* temp_string_result;
	int temp_numeric_result;
	union word* current_word;
	/*get the first space-separated word (the instruction)*/
	temp_string_result = strtok(instruction, ASM_LINE_SPACES);
	if (instruction == NULL)
	{
		free_ls(&results);
		return NOT_FOUND_LIST;
	}
	if (temp_string_result == NULL)
	{
		free_ls(&results);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	/*loop over each number provided*/
	while((temp_string_result = strtok(NULL, ASM_LINE_SPACES)) != NULL)
	{
		/*try to numeric value - print error message if the conversion have failed*/
		if(!get_no(temp_string_result, &temp_numeric_result))
		{
			foreach(results, &free);
			free_ls(&results);
			return NOT_FOUND_LIST;
		}
		/*allocate memory and insert the next number*/
		current_word = calloc(1, sizeof(union word));
		if (current_word == NULL)
		{
			foreach(results, &free);
			free_ls(&results);
			throw_exp("Memory allocation failed");
			return NOT_FOUND_LIST;
		}
		current_word->instruction_number.instruction_numeric_value = temp_numeric_result;
		insert(&results, current_word);
	}
	return results;
}
/*the function analyzes the ".extern" instruction*/
struct list analyze_extern(char* instruction)
{
	char* temp_string_result;
	if (instruction == NULL)
	{
		return NOT_FOUND_LIST;
	}
	/*get the first space-separated word (the instruction)*/
	temp_string_result = strtok(instruction, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	/*get the label the .extern instruction is referring to*/
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	/*check for label validity*/
	if(!is_valid_lable(temp_string_result))
	{
		throw_exp("Label is invalid");
		return NOT_FOUND_LIST;
	}
	/*add the label to the symbol table*/
	add_symbol(temp_string_result, 0, true);
	return init();
}
/*the function analyzes the ".entry" instruction*/
struct list analyze_entry(char* instruction)
{
	char* temp_string_result;
	if (instruction == NULL)
	{
		return NOT_FOUND_LIST;
	}
	/*get the first space-separated word (the instruction)*/
	temp_string_result = strtok(instruction, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	/*get the label the .entry instruction is referring to*/
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	/*add the label to the symbol table*/
	if(is_valid_lable(temp_string_result))
		set_as_entry(temp_string_result);
	return init();
}
