#include "command.h"
#include <stdlib.h>
#include <string.h>

#include "asm_line_types.h"
#include "exception.h"
#include "symbol_table.h"
#include "utils.h"

#define AMOUNT_OF_COMMANDS 16
#define AMOUNT_OF_REGISTERS 8
static struct command_info commands_data[AMOUNT_OF_COMMANDS] = {
	{"mov", 0, 0},
	{"cmp", 1,0},
	{"add", 2,1},
	{"sub", 2,2},
	{"lea", 4,0},
	{"clr", 5,1},
	{"not", 5,2},
	{"inc", 5,3},
	{"dec", 5,4},
	{"jmp", 9,1},
	{"bne",9, 2},
	{"jsr",9,3},
	{"red",12,0},
	{"prn", 13,0},
	{"rts", 14,0},
	{"stop", 15,0}};
char* registers[AMOUNT_OF_REGISTERS] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7" };
/*receives a command and returns its binaries*/
struct list cmd_to_numeric(char* command, unsigned int address)
{
	if (command == NULL)
		return NOT_FOUND_LIST;
	/*check each keyword the command can be and send the command to its respective function
	 * please note that because some commands have the same arguments they are handled together*/
	if ((!strncmp(command, "mov", strlen("mov"))) || (!strncmp(command, "sub", strlen("sub"))) || (!strncmp(command, "add", strlen("add"))))
		return analyze_mov_add_sub(command, address);
	if (!strncmp(command, "lea", strlen("lea")))
		return analyze_lea(command, address);
	if ((!strncmp(command, "cmp", strlen("cmp"))))
		return analyze_cmp(command, address);
	if ((!strncmp(command, "clr", strlen("clr"))) || (!strncmp(command, "not", strlen("not"))) || (!strncmp(command, "inc", strlen("inc"))) || (!strncmp(command, "dec", strlen("dec"))) || (!strncmp(command, "red", strlen("red"))))
		return analyze_clr_not_inc_dec_red(command, address);
	if ((!strncmp(command, "jmp", strlen("jmp"))) || (!strncmp(command, "bne", strlen("bne"))) || (!strncmp(command, "jsr", strlen("jsr"))))
		return analyze_jmp_bne_jsr(command, address);
	if ((!strncmp("prn", command, strlen("prn"))))
		return analyze_prn(command, address);
	if ((!strncmp(command, "rts", strlen("rts"))) || (!strncmp(command, "stop", strlen("stop"))))
		return analyze_rts_stop(command, address);
	/*if the command is non of thos print an error message*/
	throw_exp("Command has not been found");
	return  NOT_FOUND_LIST;
}
/*returns the binaries of mov, add, sub*/
struct list analyze_mov_add_sub(char* command, unsigned int address)
{
	/*the result list will contain all the binaries, ranging from 1-3 words*/
	struct list result = init();
	/*current word is the word that is used to store the command. words 2 and 3 are the optional words. they may or may not be used*/
	union word* current_word = calloc(1, sizeof(union word)), * word_2 = calloc(1, sizeof(union word)), * word_3 = calloc(1, sizeof(union word));
	/*this booleans are for checking weather the words will be used later or not. if not, they won't be inserted*/
	bool is_word_2_used = false, is_word_3_used = false;
	/*this two temps will hold the result of getting register, opcode, etc..*/
	int temp_numeric_result = 0;
	char* temp_string_result = NULL;
	/*if allocation failed print an error message*/
	if (current_word == NULL || word_2 == NULL || word_3 == NULL)
	{
		if (current_word != NULL)
			free(current_word);
		if (word_2 != NULL)
			free(word_2);
		if (word_3 != NULL)
			free(word_3);
		free_ls(&result);
		throw_exp("Memory allocation failed");
		return NOT_FOUND_LIST;
	}
	/*if argument was not provided return an empty list*/
	if (command == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		return NOT_FOUND_LIST;
	}
	/*get the first space separated part of the command. This should be the command itself*/
	temp_string_result = strtok(command, ASM_LINE_SPACES);
	/*if the command have not been found print an error message. notice: I check twice for the command in order to make sure there are spaces after the command*/
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	/*get the opcode, funct number using the command*/
	current_word->command.opcode = get_opcode(temp_string_result);
	current_word->command.funct = func_no(temp_string_result);
	/*AMOUNT_OF_COMMANDS means the command have not been found. Print an error message*/
	if (current_word->command.opcode == AMOUNT_OF_COMMANDS)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Command has not been found");
		return NOT_FOUND_LIST;
	}
	/*the first word always has the A flag on. the other two flags are guaranteed to be 0 as calloc() is used*/
	current_word->command.A = 1;
	/*get the next part of the command. this should be a source operand*/
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	/*if this part was not found print an error message*/
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}


	
	/*checking what type the command is. note that labels are the only non identifiable addressing type, thus will be place at the end of all the conditionals*/
	/*check weather the command is a register. note that is such case, word 2 will not be used at this point, although  it can be used later in the second argument.
	 * word 3 is guaranteed not to be used*/
	/*if the operand is a register*/
	if (is_register(temp_string_result))
	{
		/*get the register number*/
		current_word->command.source_register = register_no(temp_string_result);
		/*set the type of the operand to be register (in the command)*/
		current_word->command.source_type = 3;
	}
	/*if the command start with a '#' it means it's from instant addressing type*/
	else if (*temp_string_result == '#')
	{
		/*try to covert the number provided to numeric value. print an error message on fail*/
		if (!get_no(temp_string_result + 1, &temp_numeric_result))
		{
			free_ls(&result);
			free(current_word);
			free(word_2);
			free(word_3);
			return NOT_FOUND_LIST;
		}
		/*the number is assigned to a new word*/
		word_2->number.numeric_value = temp_numeric_result;
		/*instant addressing type always has the A field on*/
		word_2->number.A = 1;
		/*mark word 2 as used. this means that future processing of the function word 2 will not be used*/
		is_word_2_used = true;
		/*set the type of the operand to be a number (in the command)*/
		current_word->command.source_type = 0;
	}
	/*if it is not number nor register, it needs to be a label*/
	else
	{
		/*add to the symbol table the reference to this label. note word 2 is being passed and will be changed later*/
		add_reference(temp_string_result, word_2, address, false);
		/*set the type of the operand to be a label (in the command)*/
		current_word->command.source_type = 1;
		/*mark word 2 as used*/
		is_word_2_used = true;
	}


	
	/*get the next part of the command. this should be a destination operand*/
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	/*if this part was not found print an error message*/
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}


	
	/*check weather the command is a register. note that is such case, word 3 will not be used at this point*/
	 /*if the operand is a register*/
	if (is_register(temp_string_result))
	{
		/*get the register number*/
		current_word->command.destination_register = register_no(temp_string_result);
		/*set the type of the operand to be register (in the command)*/
		current_word->command.destination_type = 3;
	}
	/*if it is a label*/
	else
	{
		/*if word 2 has been used use word 3*/
		if (is_word_2_used)
		{
			/*add to the symbol table the reference to this label*/
			add_reference(temp_string_result, word_3, address, false);
			/*set word 3 as used*/
			is_word_3_used = true;
		}
		/*otherwise, use word 2*/
		else
		{
			/*add to the symbol table the reference to this label*/
			add_reference(temp_string_result, word_2, address, false);
			/*set word 2 as used*/
			is_word_2_used = true;
		}
		/*set the type of the operand to be a label (in the command)*/
		current_word->command.destination_type = 1;
	}


	
	/*make sure there is no redundant text after the end of command*/
	if (strtok(NULL, ASM_LINE_SPACES) != NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("redundant statement after the end of command");
		return NOT_FOUND_LIST;
	}
	/*the first word will be used at any case. as for the other two, check weather they should be used and if so, add
	 *them to the result (otherwise free the memory)*/
	insert(&result, current_word);
	if (is_word_2_used)
		insert(&result, word_2);
	else
		free(word_2);
	if (is_word_3_used)
		insert(&result, word_3);
	else
		free(word_3);
	return result;
	
}
/*returns the binaries of cmp*/
struct list analyze_cmp(char* command, unsigned int address)
{
	/*the result list will contain all the binaries, ranging from 1-3 words*/
	struct list result = init();
	/*current word is the word that is used to store the command. words 2 and 3 are the optional words*/
	union word* current_word = calloc(1, sizeof(union word)), * word_2 = calloc(1, sizeof(union word)), * word_3 = calloc(1, sizeof(union word));
	/*this booleans are for checking weather the words will be used later or not*/
	bool is_word_2_used = false, is_word_3_used = false;
	/*this two temps will hold the result of getting register, opcode, etc..*/
	int temp_numeric_result = 0;
	char* temp_string_result = NULL;
	/*if allocation failed print an error message*/
	if (current_word == NULL || word_2 == NULL || word_3 == NULL)
	{
		free_ls(&result);
		if (current_word != NULL)
			free(current_word);
		if (word_2 != NULL)
			free(word_2);
		if (word_3 != NULL)
			free(word_3);
		throw_exp("Memory allocation failed");
		return NOT_FOUND_LIST;
	}
	/*if argument was not provided return an empty list*/
	if (command == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		return NOT_FOUND_LIST;
	}
	/*get the first space separated part of the command. This should be the command itself*/
	temp_string_result = strtok(command, ASM_LINE_SPACES);
	/*if the command have not been found print an error message*/
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	/*get the opcode, funct number using the command*/
	current_word->command.opcode = get_opcode(temp_string_result);
	current_word->command.funct = func_no(temp_string_result);
	/*if the command have not been found print an error message*/
	if (current_word->command.opcode == AMOUNT_OF_COMMANDS)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Command has not been found");
		return NOT_FOUND_LIST;
	}
	/*the first word always has the A flag on*/
	current_word->command.A = 1;
	/*get the next part of the command. this should be a source operand*/
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	/*if this part was not found print an error message*/
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}


	
	/*if the operand is a register*/
	if (is_register(temp_string_result))
	{
		/*get the register number*/
		current_word->command.source_register = register_no(temp_string_result);
		/*set the type of the operand to be register*/
		current_word->command.source_type = 3;
	}
	/*if it's a instant addressing type*/
	else if (*temp_string_result == '#')
	{
		/*try to covert the number provided to numeric value. print an error message on fail*/
		if (!get_no(temp_string_result + 1, &temp_numeric_result))
		{
			free_ls(&result);
			free(current_word);
			free(word_2);
			free(word_3);
			return NOT_FOUND_LIST;
		}
		/*the number is assigned to a new word*/
		word_2->number.numeric_value = temp_numeric_result;
		/*instant addressing type always has the A field on*/
		word_2->number.A = 1;
		/*mark word 2 as used. this means that future processing of the function word 2 will not be used*/
		is_word_2_used = true;
		/*set the type of the operand to be a number (in the command)*/
		current_word->command.source_type = 0;
	}
	/*if it as a label add it to the symbol table*/
	else
	{
		add_reference(temp_string_result, word_2, address, false);
		is_word_2_used = true;
		current_word->command.source_type = 1;
	}


	/*same for destenation operand*/
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	if (is_register(temp_string_result))
	{
		current_word->command.destination_register = register_no(temp_string_result);
		current_word->command.destination_type = 3;
	}
	else if (*temp_string_result == '#')
	{
		if (!get_no(temp_string_result + 1, &temp_numeric_result))
		{
			free_ls(&result);
			free(current_word);
			free(word_2);
			free(word_3);
			return NOT_FOUND_LIST;
		}
		if (is_word_2_used)
		{
			word_3->number.numeric_value = temp_numeric_result;
			word_3->number.A = 1;
			is_word_3_used = true;
		}
		else
		{
			word_2->number.numeric_value = temp_numeric_result;
			word_2->number.A = 1;
			is_word_2_used = true;
		}
		current_word->command.destination_type = 0;
	}
	else
	{
		if (is_word_2_used)
		{
			add_reference(temp_string_result, word_3, address, false);
			is_word_3_used = true;
		}
		else
		{
			add_reference(temp_string_result, word_2, address, false);
			is_word_2_used = true;
		}
		current_word->command.destination_type= 1;
	}
	if (strtok(NULL, ASM_LINE_SPACES) != NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("redundant statement after the end of command");
		return NOT_FOUND_LIST;
	}
	insert(&result, current_word);
	if (is_word_2_used)
		insert(&result, word_2);
	else
		free(word_2);
	if (is_word_3_used)
		insert(&result, word_3);
	else
		free(word_3);
	return result;
}
/*in order to prevent repetitive commenting, I will not comment the rest of the analyze methods as they all basically the same.*/
/*returns the binaries of lea*/
struct list analyze_lea(char* command, unsigned int address)
{
	struct list result = init();
	union word* current_word = calloc(1, sizeof(union word)), * word_2 = calloc(1, sizeof(union word)), * word_3 = calloc(1, sizeof(union word));
	bool is_word_3_used = false;/*BUG word3*/
	int temp_numeric_result = 0;
	char* temp_string_result = NULL;
	if (current_word == NULL || word_2 == NULL || word_3 == NULL)
	{
		if (current_word != NULL)
			free(current_word);
		if (word_2 != NULL)
			free(word_2);
		if (word_3 != NULL)
			free(word_3);
		free_ls(&result);
		throw_exp("Memory allocation failed");
		return NOT_FOUND_LIST;
	}
	if (command == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		return NOT_FOUND_LIST;
	}
	temp_string_result = strtok(command, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	current_word->command.opcode = get_opcode(temp_string_result);
	current_word->command.funct = 0; 
	if (current_word->command.opcode == AMOUNT_OF_COMMANDS)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Command has not been found");
		return NOT_FOUND_LIST;
	}
	current_word->command.A = 1;
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	add_reference(temp_string_result, word_2, address, false);
	current_word->command.source_type = 1;
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}

	if (is_register(temp_string_result))
	{
		current_word->command.destination_register = register_no(temp_string_result);
		current_word->command.destination_type = 3;
	}
	else
	{
		add_reference(temp_string_result, word_3, address, false);
		is_word_3_used = true;
		current_word->command.destination_type = 1; 
	}
	if (strtok(NULL, ASM_LINE_SPACES) != NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		free(word_3);
		throw_exp("redundant statement after the end of command");
		return NOT_FOUND_LIST;
	}
	insert(&result, current_word);
	insert(&result, word_2);
	if (is_word_3_used)
		insert(&result, word_3);
	else
		free(word_3);
	return result;
}
/*returns the binaries of clr, not, inc, dec, red*/
struct list analyze_clr_not_inc_dec_red(char* command, unsigned int address)
{
	struct list result = init();
	union word* current_word = calloc(1, sizeof(union word)), * word_2 = calloc(1, sizeof(union word));
	bool is_word_2_used = false;
	int temp_numeric_result = 0;
	char* temp_string_result = NULL;
	if (current_word == NULL || word_2 == NULL)
	{
		if (current_word != NULL)
			free(current_word);
		if (word_2 != NULL)
			free(word_2);
		free_ls(&result);
		throw_exp("Memory allocation failed");
		return NOT_FOUND_LIST;
	}
	if (command == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		return NOT_FOUND_LIST;
	}
	temp_string_result = strtok(command, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	current_word->command.opcode = get_opcode(temp_string_result);
	current_word->command.funct = func_no(temp_string_result);
	if (current_word->command.opcode == AMOUNT_OF_COMMANDS)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Command has not been found");
		return NOT_FOUND_LIST;
	}
	current_word->command.A = 1;
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	if (is_register(temp_string_result))
	{
		current_word->command.destination_register = register_no(temp_string_result);
		current_word->command.destination_type= 3;
	}
	else
	{
		add_reference(temp_string_result, word_2, address, false);
		is_word_2_used = true;
		current_word->command.destination_type = 1;
	}
	if(strtok(NULL, ASM_LINE_SPACES) != NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("redundant statement after the end of command");
		return NOT_FOUND_LIST;
	}
	insert(&result, current_word);
	if (is_word_2_used)
		insert(&result, word_2);
	else
		free(word_2);
	return result;
}
struct list analyze_jmp_bne_jsr(char* command, unsigned int address)
{
	struct list result = init();
	union word* current_word = calloc(1, sizeof(union word)), * word_2 = calloc(1, sizeof(union word));
	int temp_numeric_result = 0;
	char* temp_string_result = NULL;
	if (current_word == NULL || word_2 == NULL)
	{
		if (current_word != NULL)
			free(current_word);
		if (word_2 != NULL)
			free(word_2);
		free_ls(&result);
		throw_exp("Memory allocation failed");
		return NOT_FOUND_LIST;
	}
	if (command == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		return NOT_FOUND_LIST;
	}
	temp_string_result = strtok(command, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	temp_numeric_result = get_opcode(temp_string_result);
	current_word->command.funct = func_no(temp_string_result);
	if (temp_numeric_result == AMOUNT_OF_COMMANDS)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Command has not been found");
		return NOT_FOUND_LIST;
	}
	current_word->command.opcode = temp_numeric_result;
	current_word->command.A = 1;
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	if (*temp_string_result == '&')
	{
		add_reference(temp_string_result + 1, word_2, address, true);
		current_word->command.destination_type = 2;
	}
	else
	{
		add_reference(temp_string_result, word_2, address, false);
		current_word->command.destination_type = 1;
	}
	if (strtok(NULL, ASM_LINE_SPACES) != NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("redundant statement after the end of command");
		return NOT_FOUND_LIST;
	}
	insert(&result, current_word);
	insert(&result, word_2);
	return result;
}
/*returns the binaries of prn*/
struct list analyze_prn(char *command, unsigned int address)
{
	struct list result = init();
	union word* current_word = calloc(1, sizeof(union word)), * word_2 = calloc(1, sizeof(union word));
	bool is_word_2_used = false;
	int temp_numeric_result = 0;
	char* temp_string_result = NULL;
	if (current_word == NULL || word_2 == NULL)
	{
		if (current_word != NULL)
			free(current_word);
		if (word_2 != NULL)
			free(word_2);
		free_ls(&result);
		throw_exp("Memory allocation failed");
		return NOT_FOUND_LIST;
	}
	if (command == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		return NOT_FOUND_LIST;
	}
	
	temp_string_result = strtok(command, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	temp_numeric_result = get_opcode(temp_string_result);
	current_word->command.funct = func_no(temp_string_result);
	if (temp_numeric_result == AMOUNT_OF_COMMANDS)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Command has not been found");
		return NOT_FOUND_LIST;
	}
	current_word->command.opcode = temp_numeric_result;
	current_word->command.A = 1;
	temp_string_result = strtok(NULL, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	if (is_register(temp_string_result))
	{
		current_word->command.destination_register = register_no(temp_string_result);
		current_word->command.destination_type = 3;
	}
	else if (*temp_string_result == '#')
	{
		if (!get_no(temp_string_result + 1, &temp_numeric_result))
			return NOT_FOUND_LIST;
		word_2->number.numeric_value = temp_numeric_result;
		word_2->number.A = 1;
		is_word_2_used = true;
		current_word->command.destination_type = 0;
	}
	else
	{
		add_reference(temp_string_result, word_2, address, false);
		is_word_2_used = true;
		current_word->command.destination_type = 1;
	}
	if (strtok(NULL, ASM_LINE_SPACES) != NULL)
	{
		free_ls(&result);
		free(current_word);
		free(word_2);
		throw_exp("redundant statement after the end of command");
		return NOT_FOUND_LIST;
	}
	insert(&result, current_word);
	if (is_word_2_used)
		insert(&result, word_2);
	else
		free(word_2);
	return result;
}
/*return the binaries of rts, stop*/
struct list analyze_rts_stop(char* command, unsigned int address)
{
	struct list result = init();
	union word* current_word = calloc(1, sizeof(union word));
	int temp_numeric_result = 0;
	char* temp_string_result = NULL;
	if (current_word == NULL)
	{
		free_ls(&result);
		throw_exp("Memory allocation failed");
		return NOT_FOUND_LIST;
	}
	if (command == NULL)
	{
		free_ls(&result);
		free(current_word);
		return NOT_FOUND_LIST;
	}
	
	temp_string_result = strtok(command, ASM_LINE_SPACES);
	if (temp_string_result == NULL)
	{
		free_ls(&result);
		free(current_word);
		throw_exp("Statement is missing mandatory components");
		return NOT_FOUND_LIST;
	}
	temp_numeric_result = get_opcode(temp_string_result);
	current_word->command.funct = func_no(temp_string_result);
	if (temp_numeric_result == AMOUNT_OF_COMMANDS)
	{
		free_ls(&result);
		free(current_word);
		throw_exp("Command has not been found");
		return NOT_FOUND_LIST;
	}
	current_word->command.opcode = temp_numeric_result;
	current_word->command.A = 1;
	if (strtok(NULL, ASM_LINE_SPACES) != NULL)
	{
		free_ls(&result);
		free(current_word);
		throw_exp("redundant statement after the end of command");
		return NOT_FOUND_LIST;
	}
	insert(&result, current_word);
	return result;
}
/*get an opcode of a giver command*/
unsigned int get_opcode(char *command)
{
	unsigned int i;
	if (command == NULL)
		return AMOUNT_OF_COMMANDS;
	/*for each command in the dictionary, if the key is correct look for its value and return it*/
	for (i = 0; i < AMOUNT_OF_COMMANDS; i++)
		if (!strcmp(commands_data[i].command_name, command))
			return commands_data[i].op_code;
	return AMOUNT_OF_COMMANDS;
}
/*returns weather a given string is an assembly - compliant register*/
bool is_register(char *reg)
{
	int i;
	if (reg == NULL)
		return false;
	/*for each register is the array if it matches the register in the string return true*/
	for (i = 0; i < AMOUNT_OF_REGISTERS; i++)
		if (!strcmp(reg, registers[i]))
			return true;
	/*if no match was found return false*/
	return false;
}
/*get the register number of a given register*/
unsigned int register_no(char* reg)
{
	unsigned int i;
	if (reg == NULL)
		return -1;
	if (!is_register(reg))
		return -1;
	/*for each register is the array if it matches the register in the string return its index*/
	for (i = 0; i < AMOUNT_OF_REGISTERS; i++)
		if (!strcmp(reg, registers[i]))
			return i;
	return AMOUNT_OF_REGISTERS;
}
/*get the "funct" number of a given command\\*/
unsigned int func_no(char *command)
{
	unsigned int i;
	if (command == NULL)
		return -1;
	/*for each command in the dictionary, if the key is correct look for its value and return it*/
	for(i = 0; i < AMOUNT_OF_COMMANDS; i++)
	{
		if(!strcmp(command, commands_data[i].command_name))
		{
			return commands_data[i].funct;
		}
	}
	return AMOUNT_OF_COMMANDS;
}
