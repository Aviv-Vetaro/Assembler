#if !defined(__COMMAND__HEADER__)
#define __COMMAND__HEADER__

#include "list.h"

struct list cmd_to_numeric(char* , unsigned int);

struct list analyze_mov_add_sub(char*, unsigned int);
struct list analyze_cmp(char*, unsigned int);
struct list analyze_lea(char*, unsigned int);
struct list analyze_clr_not_inc_dec_red(char*, unsigned int);
struct list analyze_jmp_bne_jsr(char*, unsigned int);
struct list analyze_prn(char*, unsigned int);
struct list analyze_rts_stop(char*, unsigned int);

unsigned int get_opcode(char *);
bool is_register(char*);
unsigned int register_no(char*);
unsigned int func_no(char *);

struct command_info
{
	char* command_name;
	unsigned int op_code;
	unsigned int funct;
};
#endif
