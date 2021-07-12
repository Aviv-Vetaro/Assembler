#include "exception.h"

#include <stdio.h>
static int current_line_number = UNDEF_LINE;
static char* current_file = NULL;
static bool generate_output = true;
/*print to the user an error message, providing information such as
 * line in which the error occurred, file name, error info, etc..  */
bool throw_exp(char *exp)
{
	if(exp == NULL)
		return false;
	/*print error type - fatal error*/
	fprintf(stderr, "FATAL ERROR:\n");
	/*print more error details. If the function caller don't want some details specified,
	 * it can set them to their undefined values*/
	if (current_file != NULL)
		fprintf(stderr, "\t-at file: \"%s\"\n", current_file);
	if (current_line_number != UNDEF_LINE)
		fprintf(stderr, "\t-at line %d\n", current_line_number);
	/*provide actual error info*/
	fprintf(stderr,"%s\n", exp);
	/*output files should not be generated if a fatal error have occurred*/
	generate_output = false;
	return true;
}
/*Sets the current line the error message should report*/
bool set_line(int current_line)
{
	if(current_line < 0)
	{
		current_line_number = UNDEF_LINE;
		return false;
	}
	current_line_number = current_line;
	return true;
}
/*Sets the current file the error message should report*/
bool set_file(char *file)
{
	if(file == NULL)
	{
		current_file = NULL;
		return false;;
	}
	current_file = file;
	/*new file - meaning output can be generated*/
	generate_output = true;
	return true;
}
/*whether the file had any errors*/
bool is_valid_file(void)
{
	return generate_output;
}