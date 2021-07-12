#include "buffer.h"

#include <stdlib.h>
#include <string.h>


#include "list.h"
#include <stdbool.h>

#include "exception.h"

static struct list lines;
int line_no;

/*reads the content of a provided file row by row and insert the lines into an array(list)*/
bool init_buf(FILE* input_file)
{
	char* current_line;
	char* result;
	if (input_file == NULL)
	{
		return false;
	}
	/*allocate memory for the array*/
	lines = init();
	line_no = 0;
	/*as long as there are more lines to read*/
	while (true)
	{
		/*allocate memory for the next line*/
		current_line = (char*)malloc(sizeof(char*) * (MAX_LINE_LENGTH + 2));
		/*read the next line from file*/
		result = fgets(current_line, MAX_LINE_LENGTH + 2, input_file);
		/*stop reading if eof (or error) is reached*/
		if (result == NULL)
		{
			free(current_line);
			break;
		}
		/*set line for error reporting*/
		set_line(++line_no);
		/*if input is too long alert the user*/
		if (strlen(current_line) == MAX_LINE_LENGTH + 1)
		{
			throw_exp("Line is too big");
			insert(&lines, NULL);
		}
		/*otherwise add the line to the array*/
		else insert(&lines, current_line);
	}
	line_no = 0;
	return true;
}
/*return the next unread line*/
char* get_line(void)
{
	return get(&lines, line_no++);
}
/*check if all lines as been read. if so, set the counter to point to the first line again*/
bool is_eof(void)
{
	if (line_no == lines.length)
	{
		line_no = 0;
		return true;
	}
	return false;
}
/*free allocated memory of the buffer*/
void free_buf(void)
{
	foreach(lines, free);
    free_ls(&lines);
}
