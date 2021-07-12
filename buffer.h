
#if !defined(__BUFFER__HEADER__)
#define __BUFFER__HEADER__
#define MAX_LINE_LENGTH 81
#include <stdbool.h>
#include "stdio.h"
bool init_buf(FILE *);
char* get_line(void);
bool is_eof(void);
void free_buf(void);
#endif

