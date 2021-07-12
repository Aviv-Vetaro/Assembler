#if !defined(__OUTPUT__HEADER__)
#define __OUTPUT__HEADER__

#include "stdbool.h"
#include "exception.h"
#include "symbol_table.h"
#include "list.h"

bool output(char*, struct list, unsigned int, unsigned int);
bool output_extern(char*);
bool output_entry(char*);
#endif
