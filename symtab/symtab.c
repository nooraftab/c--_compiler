#include <stdio.h>
#include <stdlib.h>
#include "symtab.h"

//Some helper functions
static void add_symtab_entry_to_symtab(SymTabEntry *entry);
static void add_symtab_to_stack(SymTab *table);
static void check_duplicate_entry(char *name);

//The almight symbol table stack
static SymTabStack *stack; 

void init_symtab_stack() {
	//Initialize the stack
	stack = malloc(sizeof(SymTabStack));
	stack->numSymTabs = 0;
	stack->symTabs = malloc(0);

	//Push global scope!
	push_symtab();
}

void push_symtab() {
	//Initialize new symbol table
	SymTab *newTable = malloc(sizeof(SymTab));
	newTable->numSymTabEntries = 0;
	newTable->symTabEntries = malloc(0);

	//Push to top of the stack
	add_symtab_to_stack(newTable);
}

//Look for a symbol table entry, starting from top of SymtabStack
SymTabEntry *symtab_lookup(char *name) {
	//Start at current enclosing scope
	for (int i=stack->numSymTabs-1; i >=0 ; i--) {
		SymTab *table = stack->symTabs[i];
		
		//For each symbol table, look through all entries
		for (int j=0; j < table->numSymTabEntries; j++) {
			if (strcmp(table->symTabEntries[j]->name, name)==0) {
				return table->symTabEntries[j];
			}
		}

	}
	symtab_error("Variable not declared!", inFile, outFile);
	return NULL;
}

//Insert a variable to the SymTab at the top of the stack
void insert_var_symtab_entry(char *name, int scope, int type, 
	int dimension, int offset, int isInit) {
	check_duplicate_entry(name);
	SymTabEntry *entry = malloc(sizeof(SymTabEntry));

	entry->name = name;
	entry->scope = scope;
	entry->type = type;
	entry->dimension = dimension;
	entry->offset = offset;
	entry->isInit = isInit;
	entry->isFunction = 0; 

	add_symtab_entry_to_symtab(entry);
}

//Insert a function to the SymTab at top of the stack
SymTabEntry *insert_func_symtab_entry(char *name, int returnType) {
	check_duplicate_entry(name);
	SymTabEntry *entry = malloc(sizeof(SymTabEntry));

	//Only need this much info. for functions
	entry->name = name;
	entry->type = returnType;
	entry->isFunction = 1;

	add_symtab_entry_to_symtab(entry);
	return entry;
}

//Check if there is a duplicate entry in current scope
static void check_duplicate_entry(char *name) {
	SymTab *topOfStack = stack->symTabs[stack->numSymTabs-1];

	for (int i=0; i < topOfStack->numSymTabEntries; i++) {
		if (strcmp(topOfStack->symTabEntries[i]->name, name) == 0) {
			symtab_error("Multiply defined variable/function! Check names.", inFile, outFile);
		}
	}
}

//Add an entry to symbol table at the top of the stack
static void add_symtab_entry_to_symtab(SymTabEntry *entry) {
	//Get SymTab at top
	SymTab *topOfStack = stack->symTabs[stack->numSymTabs-1];
	topOfStack->numSymTabEntries++;

	//Re-allocate space in that SymTab
	topOfStack->symTabEntries = realloc(topOfStack->symTabEntries,
		sizeof(SymTabEntry)*(topOfStack->numSymTabEntries));

	//Set the new empty space at end to be the entry we want to insert
	topOfStack->symTabEntries[topOfStack->numSymTabEntries-1] = entry;
}

//Add a symbol table to the SymTabStack
static void add_symtab_to_stack(SymTab *table) {
	stack->numSymTabs++;

	stack->symTabs = realloc(stack->symTabs, 
		sizeof(SymTab)*(stack->numSymTabs));

	stack->symTabs[stack->numSymTabs-1] = table;
}

//Pops a symbol table from the stack
void pop_symtab() {
	SymTab *topOfStack = stack->symTabs[stack->numSymTabs-1];

	//Go through all entries and free their memory
	for (int i=0; i < topOfStack->numSymTabEntries; i++) {
		free(topOfStack->symTabEntries[i]);
	}

	//Free symbol table's memory
	free(topOfStack->symTabEntries);
	free(topOfStack);

	//Decrement count
	stack->numSymTabs--;
}

//Frees memory of anything remaining on the stack
void destroy_symtab_stack() {
	for (int i=0; i < stack->numSymTabs; i++) {
		SymTab *table = stack->symTabs[i];
		for (int j=0; j < table->numSymTabEntries; j++) {
			free(table->symTabEntries[j]);
		}
		free(stack->symTabs[i]->symTabEntries);
		free(stack->symTabs[i]);
	}

	free(stack->symTabs);
	free(stack);
}
