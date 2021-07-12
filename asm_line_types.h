#if !defined(__ASM__LINE__TYPE__HEADER__)
#define __ASM__LINE__TYPE__HEADER__
#define E_FIELD_SIZE 1
#define R_FIELD_SIZE 1
#define A_FIELD_SIZE 1
#define FUNCT_FIELD_SIZE 5
#define SOURCE_REGISTER_FIELD_SIZE 3
#define SOURCE_TYPE_FIELD_SIZE 2
#define DESTINATION_REGISTER_FIELD_SIZE 3
#define DESTINATION_TYPE_FIELD_SIZE 2
#define OPCODE_FIELD_SIZE 6 

#define NUMERIC_VALUE_FIELD_SIZE 21

#define ADDRESS_FIELD_SIZE 21

#define INSTRUCTION_NUMERIC_VALUE_FIELD_SIZE 24

#define WORD_SIZE 24

struct command_word
{
	unsigned int E : E_FIELD_SIZE;
	unsigned int R : R_FIELD_SIZE;
	unsigned int A : A_FIELD_SIZE;
	unsigned int funct : FUNCT_FIELD_SIZE;
	unsigned int destination_register : DESTINATION_REGISTER_FIELD_SIZE;
	unsigned int destination_type : DESTINATION_TYPE_FIELD_SIZE;
	unsigned int source_register : SOURCE_REGISTER_FIELD_SIZE;
	unsigned int source_type : SOURCE_TYPE_FIELD_SIZE;
	unsigned int opcode : OPCODE_FIELD_SIZE;
};

struct numeric_value_word
{
	unsigned int E : E_FIELD_SIZE;
	unsigned int R : R_FIELD_SIZE;
	unsigned int A : A_FIELD_SIZE;
	int numeric_value : NUMERIC_VALUE_FIELD_SIZE;
};

struct address_word
{
	unsigned int E : E_FIELD_SIZE;
	unsigned int R : R_FIELD_SIZE;
	unsigned int A : A_FIELD_SIZE;
	unsigned int address : ADDRESS_FIELD_SIZE;
};

struct instruction_numeric_value
{
	int instruction_numeric_value : INSTRUCTION_NUMERIC_VALUE_FIELD_SIZE;
};

struct instruction_char_value
{
	char char_value;
};

typedef unsigned long word_numeric;

union word
{
	word_numeric machine_code : WORD_SIZE;
	struct command_word command;
	struct numeric_value_word number;
	struct address_word memory_address;
	struct instruction_numeric_value instruction_number;
	struct instruction_char_value ascii_value;
};

#define ASM_LINE_SPACES " \n\t\v\f\r,"
#endif
