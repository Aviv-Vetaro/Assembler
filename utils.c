#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include "exception.h"
#include <ctype.h>
/*add two string together and returns the concatenated string*/
char* concat(char* string1, char* string2)
{
	char* new_string;
	if (string1 == NULL || string2 == NULL)
		return NULL;
	/*allocate sufficient memory for both strings*/
	new_string = malloc(sizeof(char) * (strlen(string1) + strlen(string2) + 1));
	if (new_string == NULL)
	{
		throw_exp("Memory allocation failed");
		return NULL;
	}
	/*copy the two strings (ont on top of the other) to the new string. note that "string1" comes first*/
	strcpy(new_string, string1);
	strcat(new_string, string2);
	/*note "string1" and "string2" are not freed*/
	return new_string;
}
/*the function tries to convert "number" to a numeric format and assign the result into "result" */
bool get_no(char* number, int* result)
{
	if(number == NULL || result == NULL)
	{
		return false;
	}
	if(isspace(*number))
	{
		throw_exp("no number provided");
		return false;
	}
	/*apply the conversion*/
	*result = (int)(strtol(number, &number, 10));
	/*check if the number consists of non-digit chars or if any other error interrupted the conversion*/
	if((!isspace(*number)) && (*number != '\0'))
	{
		throw_exp("invalid numeric format");
		return false;
	}
	return true;
}
/*the function tries to open a file on read mode and reports any error if failed*/
FILE* asm_fopen(char* path)
{
	/*try to open the file on read mode*/
	FILE* result = fopen(path, "r");
	/*report error if the file won't open*/
	if (result == NULL)
		throw_exp("could not open file");
	return result;
}
/*check if an entire string consists only of whitespace characters*/
bool is_whitespace(char *string)
{
	if(string == NULL)
		return false;
	while (*string)
	{
		if(!isspace(*string))
			return false;
		string++;
	}
	return true;
}