#if !defined(__EXCEPTION__HEADER__)
#define __EXCEPTION__HEADER__
#include <stdbool.h>

#define UNDEF_LINE -1

bool throw_exp(char *);
bool set_line(int);
bool set_file(char*);

bool is_valid_file(void);
#endif
