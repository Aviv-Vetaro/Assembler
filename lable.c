#include "lable.h"
#include "utils.h"
#include "asm_line_types.h"
#include "string.h"
#include "symbol_table.h"
#include "exception.h"
#include <ctype.h>
#define KEYWORD_AMOUNT 24
char* keywords[KEYWORD_AMOUNT] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "mov",	"cmp","add","sub","lea", "clr",	"not",	"inc",	"dec",	"jmp",	"bne",	"jsr",	"red",	"prn",	"rts",	"stop" };
/*the function will insert the label in the start of the line (if any) into the symbol table,
 * and then will return a pointer the first non - whitespace char in the line after the label*/
char* get_label(char *label, unsigned int address)
{
	char* temp = label;
	bool have_str = false;
	if (label == NULL)
		return label;
	while (*temp != '\"' && *temp != '\0') temp++;
	if (*temp == '\"')
	{
		*temp = '\0';
		have_str = true;
	}
	/*if there is no label skip the spaces and return a  pointer to the first
	 * non - whitespace
	 */
	if (!strchr(label, ':'))
	{
		if(label != NULL)
		{
			while (isspace(*label) && (*label) != '\0') label++;
		}
		return label;
	}
	if (have_str)
		*temp = '\"';
	/*get the label*/
	label = strtok(label, ":");
	/*get the rest of the line*/
	temp = strtok(NULL, "");
	if (temp != NULL)
	{
		if (!isspace(*temp))
			throw_exp("at least one space expected after a label");
		/*skip whitespaces*/
		while (isspace(*temp) && (*temp) != '\0') temp++;
	}
	else
		return NULL;
	if (strncmp(temp, ".extern", strlen(".extern")) && strncmp(temp, ".entry", strlen(".entry")))
	{
		/*skip the spaces at the start of the line (if any)*/
		while (isspace(*label) && (*label) != '\0') label++;
		/*check for label validity*/
		if (is_valid_lable(label))
			/*add the label to the symbol table*/
			add_symbol(label, address, false);
	}
	return temp;
}
/*checks weather a given label is valid*/
bool is_valid_lable(char* label)
{
	int label_length = 1, i;
	if(label == NULL)
		return false;
	/*check if it any of the keywords*/
	for(i = 0; i < KEYWORD_AMOUNT; i++)
		if(!strcmp(label, keywords[i]))
			return false;
	/*check if the first letter is alphabetic*/
	if (!isalpha(*label))
	{
		throw_exp("Label is invalid");
		return false;
	}
	/*check if all the other letter is alphanumeric*/
	label++;
	while((*label))
	{
		if(!isalnum(*label))
		{
			throw_exp("Label is invalid");
			return false;
		}
		/*alert if label is too big*/
		if(label_length > MAX_LABEL_LENGTH)
		{
			throw_exp("Label is too long");
			return false;
		}
		label_length++;
		label++;
	}
	return true;
}