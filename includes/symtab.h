#ifndef _SYMTAB_H
#define _SYMTAB_H

#include "traversaltotable.h"

//Structure for a single symbol table entry
typedef struct {
	char *name;
	int type; 

	int scope; //Globals - 0
	int dimension; //For arrays only
	int offset; //From $fp
	int isInit;

	//Only for functions
	int isFunction;
	int numParams;
} SymTabEntry;

//Structure for a single symbol table (for a given scope)
typedef struct {
	int numSymTabEntries;
	SymTabEntry **symTabEntries;
} SymTab;

//Structure for stack of SymTabs (for entire program)
typedef struct {
	int numSymTabs; 
	SymTab **symTabs;
} SymTabStack;

//Functions for setting up/tearing down heap memory
void init_symtab_stack();
void destroy_symtab_stack();

void push_symtab(); //Entering a scope
void pop_symtab(); //Leaving a scope

//Insert a variable's info. into symbol table @ top of stack
void insert_var_symtab_entry(char *name, int scope, int type, 
	int dimension, int offset, int isInit); 
//Insert a function's!
SymTabEntry *insert_func_symtab_entry(char *name, int returnType);

//Look for an entry, starting from top
SymTabEntry *symtab_lookup(char *name);

extern void symtab_error(char *err_message, FILE *in, FILE *out);

#endif
