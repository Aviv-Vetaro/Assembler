#if !defined(__INSTRUCTION__HEADER__)
#define __INSTRUCTION__HEADER__
#include "list.h"
struct list instruction_to_numeric(char*);
struct list analyze_string(char*);
struct list analyze_data(char*);
struct list analyze_extern(char*);
struct list analyze_entry(char*);
#endif
