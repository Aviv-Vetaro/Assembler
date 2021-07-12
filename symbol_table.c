#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include "exception.h"
#include "asm_line_types.h"
#include "command.h"
static struct list symbols;
static struct list references;
/*allocate memory for the the symbol table*/
void init_symbols(void)
{
	symbols = init();
	references = init();
}
/*add to the references table info about a new reference, */
bool add_reference(char *reference_label, union word *referencer, unsigned int referencer_address, bool is_distance)
{
	struct reference new_reference, * new_reference_ptr;
	if (reference_label == NULL || referencer == NULL)
		return false;
	/*allocate memory for the new  reference*/
	new_reference_ptr = malloc(sizeof(struct reference));
	if(new_reference_ptr == NULL)
	{
		throw_exp("Memory allocation failed");
		return false;
	}
	/*get reference into a struct*/
	new_reference.is_dist = is_distance;
	new_reference.referenced = reference_label;
	new_reference.referencer_address = referencer_address;
	new_reference.referencer = referencer;
	*new_reference_ptr = new_reference;
	/*add the reference to the table*/
	insert(&references, new_reference_ptr);
	return true;
}
/*update the binaries to include data about label wherever they are referenced */
bool unreference(void)
{
	int i, j;
	struct symbol* current_symbol_ptr;
	struct reference* current_reference_ptr;
	bool been_found;
	unsigned int *temp;
	/*for each reference find the label he is referring to*/
	for(i = 0; i < references.length; i++)
	{
		been_found = false;
		current_reference_ptr = get(&references, i);
		for(j = 0; j < symbols.length; j++)
		{
			current_symbol_ptr = get(&symbols, j);
			if(!strcmp(current_reference_ptr->referenced, current_symbol_ptr->label))
			{
				/*find error such as undeclared label and report them*/
				if(!current_symbol_ptr->been_found)
				{
					throw_exp("label has been declared as \"entry\", but does not exists");
					return false;
				}
				/*if the addressing type was relative update the word to contain the distance*/
				if(current_reference_ptr->is_dist)
				{
					current_reference_ptr->referencer->number.numeric_value = (int)(current_symbol_ptr->address - current_reference_ptr->referencer_address);

					current_reference_ptr->referencer->number.A = 1;
					
				}
				/*if the addressing type is direct assign the word the address of the label*/
				else
				{
					current_reference_ptr->referencer->memory_address.address = current_symbol_ptr->address;
					/*update E,R,A according to the label type*/
					if (current_symbol_ptr->is_extern)
					{
						current_reference_ptr->referencer->memory_address.E = 1;
					}
					else
					{
						current_reference_ptr->referencer->memory_address.R = 1;
					}
				}
				been_found = true;
			}
		}
		/*if failed to find any label that matches the reference print an error message*/
		if (!been_found)
		{
			throw_exp("label is referenced but not declared");
			return false;
		}
	}
	return true;
}
/*add a new label to the symbol table. notice: the label could be extern or "regular" label.
 * in case of an extern label, the address provided will be ignored 
 */
bool add_symbol(char *symbol_name, unsigned int address, bool is_extern)
{
	int i;
	struct symbol new_symbol, * new_symbol_ptr, * current_symbol_ptr;
	if (symbol_name == NULL)
		return false;
	/*check if there was already a label in the symbol table with the same name*/
	for(i = 0; i < symbols.length; i++)
	{
		current_symbol_ptr = ((struct symbol *)(get(&symbols, i)));
		if(!strcmp(current_symbol_ptr->label, symbol_name))
		{
			/*if so, check if the label has already been declared. if the label has in fact been declared this is an error; print an error message.
			 * but if the label has not been declared, this means it was declared as entry, but have yet to be associated with any line. in such case, the values
			 of the not-yet-declared labels will be updated to match the newly - inserted label*/
			if (current_symbol_ptr->been_found)
			{
				if (current_symbol_ptr->is_extern)
				{
					throw_exp("extern labels can not be declared as part of a line");
					return false;
				}
				else
				{
					throw_exp("label is declared more than once");
					return false;
				}
			}
			else
			{
				current_symbol_ptr->been_found = true;
				if(is_extern)
					current_symbol_ptr->address = DEFAULT_EXTERN_VALUE;
				else
					current_symbol_ptr->address = address;
				current_symbol_ptr->is_extern = is_extern;
				return true;
			}
		}
	}
	/*such label have not been found - allocate memory for a new label*/
	new_symbol_ptr = malloc(sizeof(struct symbol));
	if(new_symbol_ptr == NULL)
	{
		throw_exp("Memory allocation failed");
		return false;
	}
	/*extern labels are being inserted a default value as an address*/
	if (is_extern)
	{
		new_symbol.address = DEFAULT_EXTERN_VALUE;
	}
	else
	{
		new_symbol.address = address;
	}
	/*add the rest of the info provided to the new symbol*/
	new_symbol.been_found = true;
	new_symbol.label = symbol_name;
	new_symbol.is_entry = false;
	new_symbol.is_extern = is_extern;
	*new_symbol_ptr = new_symbol;
	/*add the new symbol to the symbol table*/
	insert(&symbols, new_symbol_ptr);
	return true;
}
/*frees allocated memory of the symbol table*/
bool free_symbol_table(void)
{
	bool result;
	result = foreach(symbols, &free);
	result = result && foreach(references, &free);
	free_ls(&symbols);
	free_ls(&references);
	return result;
}
/*set a label to be from type entry*/
bool set_as_entry(char* label)
{
	int i;
	struct symbol* current_symbol_ptr, new_symbol, * new_symbol_ptr;
	if (label == NULL)
		return false;
	/*if there already have been a label declared by that name, update it's to info to include
	 the fact it is an entry*/
	for (i = 0; i < symbols.length; i++)
	{
		current_symbol_ptr = get(&symbols, i);
		if (!strcmp(label, current_symbol_ptr->label))
		{

			current_symbol_ptr->is_entry = true;
			return true;
		}
	}
	/*otherwise, create a new, temporary symbol to hold the entry flag.
	 * the label will be updated later to include meaningful details when the label itself 
	 will be declared*/
	new_symbol_ptr = malloc(sizeof(struct symbol));
	new_symbol.is_entry = true;
	new_symbol.been_found = false;
	new_symbol.is_extern = false;
	new_symbol.label = label;
	*new_symbol_ptr = new_symbol;
	/*insert the new symbol to the symbol table*/
	insert(&symbols, new_symbol_ptr);
	return true;
}
/*get all the extern symbols from the symbol table*/
struct list get_extern_symbols(void)
{
	struct list extern_symbols = init();
	int i;
	struct symbol* temp;
	for(i = 0; i < symbols.length; i++)
	{
		temp = get(&symbols, i);
		if (temp->is_extern)
		{
			insert(&extern_symbols, temp);
		}
	}
	return extern_symbols;
}
/*get all the references to labels*/
struct list get_refs(void)
{
	return references;
}
/*get all the entry symbols from the symbol table*/
struct list get_entry_symbols(void)
{
	struct list entry_symbols = init();
	int i;
	struct symbol* temp;
	for (i = 0; i < symbols.length; i++)
	{
		temp = get(&symbols, i);
		if (temp->is_entry)
		{
			insert(&entry_symbols, temp);
		}
	}
	return entry_symbols;
}
