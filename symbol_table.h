#if !defined(__SYMBOL__TABLE__HEADER__)
#define __SYMBOL__TABLE__HEADER__
#include <stdbool.h>

struct symbol
{
	bool is_extern;
	bool is_entry;
	bool been_found;
	char* label;
	unsigned int address;
};
struct reference
{
	bool is_dist;
	union word *referencer;
	char* referenced;
	unsigned int referencer_address;
};
bool add_reference(char*, union word *, unsigned int, bool);
bool unreference(void);
bool add_symbol(char *, unsigned int, bool);
bool free_symbol_table(void);
bool set_as_entry(char *);
struct list get_extern_symbols(void);
struct list get_entry_symbols(void);
void init_symbols(void);
struct list get_refs(void);

#define DEFAULT_EXTERN_VALUE 0
#endif


