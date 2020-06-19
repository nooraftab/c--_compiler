#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "traversaltotable.h"
#include "codetraversal.h"
#include "symtab.h"

/** Function prototypes **/
extern int handle_function_call(ast_node *elNode, int numParams, char* funcName);
extern int handle_global_allocation(int type, int dimension);
extern int handle_local_or_param_allocation(int type, int dimension);

extern int load_array_index(ast_node *idNode, SymTabEntry *idInfo, int global_or_local);
extern void store_array_index(ast_node *idNode, SymTabEntry *idInfo, 
 	int global_or_local, int src_reg);
extern int load_array_base(SymTabEntry *idInfo, int global_or_local);
extern void load_local_array_base(int scope, int dest_reg, int offset);

/*
	The return types are basically ints! Just wanted to 
	emphasize the different types of ints these functions
	are returning.
*/
extern parameterRegister getParamRegister(int i);
extern localRegister findAvailableLocalRegister();
extern tempRegister findAvailableTempRegister();

extern void freeLocalRegister(int reg);
extern void freeAllLocalRegisters();
extern void freeTempRegister(int reg);
extern void freeAllTempRegisters();

extern void push_scope();
extern void pop_scope();

extern void init_offset_stack();
extern void push_offset();
extern int pop_offset();

#endif

