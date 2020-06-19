/*
	This file contains functions that help codetraversal.c
	as it traverse the AST and outputs MIPS code. A lot of
	these functions deal with the details of figuring out
	where to place variables, what to do with arrays, etc.

	@author Noor Aftab
	@data Tueday, 5th May 2020
*/

#include <stdio.h>
#include "parser.h"
#include "lexer.h"
#include "symtab.h"
#include "traversaltotable.h"
#include "traversalmechanics.h"

//Useful for tracking t_i registers being used and in storage
static int tempRegistersInUse[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static int tempRegistersInStorage[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//Not really needed to be honest, but why not?
static int localRegistersInUse[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static int localRegistersInStorage[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//Helpers for the helpers!
static int calc_global_offset(int type);
static int calc_local_or_param_offset(int type);
static int calc_array_size(int type, int dimension);
static int calc_index_address(ast_node *idNode, SymTabEntry *idInfo, 
 	int global_or_local);

/**************************************************************************/
/** Helpers for handling space-allocation for variables/function calling **/

//Handles a function call!
int handle_function_call(ast_node *elNode, int numParams, char* funcName) {
	tempRegister returnValue = findAvailableTempRegister();

	generate_function_precall();
	//Parameter handling
	handle_expression_list(elNode, numParams);
	jal_to_function(funcName);

	/** Function executes between these two lines**/

	generate_function_postcall();
	move_registers(returnValue, V0); 
	//Return the return value
	return returnValue;
}

/* 
	In global scope, update our globalOffset variable. This 
	effectively "allocated" space in $gp.
*/
int handle_global_allocation(int type, int dimension) {
	if (dimension == -1) {
		return calc_global_offset(type);
	} else { //Is an array

		/*
			Should start at 4-byte aligned address. I don't
			add any MIPS code to reflect this, because $gp 
			doesn't seem to need it to work. Or maybe I'm
			not testing well enough...
		*/
		if (globalOffset %ALIGN != 0) {
			globalOffset += ALIGN-(globalOffset%ALIGN);
		}
		return calc_array_size(type, dimension);
	}

}

//In local scope, create space in the stack accordingly
int handle_local_allocation(int type, int dimension) {
	if (dimension == -1) {
		return calc_local_or_param_offset(type);
	} else { //Is an array

		//Arrays should start at a 4-byte aligned address
		if (funcOffset%ALIGN != 0) {
			funcOffset -= (funcOffset%ALIGN);
			pad_locals(); //Reflect alignment in MIPS code
		}
		return calc_array_size(type, dimension);
	}
}

//For parameters, make space at the stack's
int handle_param_allocation(int type, int dimension) {
	if (dimension == -1) {
		return calc_local_or_param_offset(type);
	} else { //Is an array

		/*
			For array parameters, we set dimension to 0
			to signal that it's an array. Actual dimension
			doesn't matter since we're only worried about 
			an address.
		*/

		//Should start at a 4-byte aligned address
		if (funcOffset%ALIGN != 0) {
			//funcOffset -= (funcOffset%ALIGN);
			//pad_params();
		}
		pad_params();
		//Give 4 bytes for the address
		int offset = funcOffset;
		funcOffset -= INT_SIZE;
		return offset;
	}
}

/*
	Calculates a variable's offset from $gp, and updates 
	our program's ~tracker~ for $gp.

	Note: $gp grows differently from $sp, so we add instead 
	of subtract.
*/
static int calc_global_offset(int type) {
	int offset; 
	if (type == CHARTOK) {
		offset = globalOffset;
		globalOffset += CHAR_SIZE;
	} 
	//INTTOK, not 4-byte aligned
	else if (globalOffset%ALIGN != 0){ 
		globalOffset += ALIGN-(globalOffset%ALIGN); //Pad
		offset = globalOffset;
		globalOffset += INT_SIZE;
	} 
	//INTTOK, 4-byte aligned
	else { 
		offset = globalOffset;
		globalOffset += INT_SIZE;
	}
	return offset;
}

static int calc_local_or_param_offset(int type) {
	int offset;
	if (type == CHARTOK) {
		offset = funcOffset;
		funcOffset -= CHAR_SIZE;	
	} 
	//INTTOK, not 4-byte aligned
	else if (funcOffset%ALIGN != 0){ 
		funcOffset -= (funcOffset%ALIGN); //Padding
		funcOffset -= INT_SIZE; 
		offset = funcOffset;	
	} 
	//INTTOK, 4-byte aligned
	else {		
		funcOffset -= INT_SIZE;	
		offset = funcOffset;
	}
	return offset;
}

//Calculates total space array uses, given its type & dimension
static int calc_array_size(int type, int dimension) {
	int changeInOffset = 0;
	if (type == CHARTOK) {
		changeInOffset = CHAR_SIZE*dimension;
	} 
	else { //If INTTOK
		changeInOffset = INT_SIZE*dimension;
	} 
	return changeInOffset;
}

/************************************************************/
/** Helpers for "writing" (v. specific) MIPS code for arrays **/

/*
	If we come across a reference to something inside an array (e.g a1[3]):
	calculate the address of that item, load the value there into a 
	register, and return it.
*/
 int load_array_index(ast_node *idNode, SymTabEntry *idInfo, 
 	int global_or_local) {
 	//Get base + offset 
 	tempRegister indexAddress = calc_index_address(idNode, idInfo, global_or_local);
	
	//Put value into a register
	tempRegister container = findAvailableTempRegister();

	if (idInfo->type==CHARTOK) {
		load_byte_instr(container, 0, indexAddress);
	} else if (idInfo->type == INTTOK) {
		load_word_instr(container, 0, indexAddress);
	}

	//Free and return registers accordingly
	freeTempRegister(indexAddress);
	return container;
}

//Stores the value in src_reg at an array index
void store_array_index(ast_node *idNode, SymTabEntry *idInfo, 
 	int global_or_local, int src_reg) {
	//Get base + offset 
 	tempRegister indexAddress = calc_index_address(idNode, idInfo, global_or_local);

 	if (idInfo->type==CHARTOK) {
		store_byte_instr(src_reg, 0, indexAddress);
	} else if (idInfo->type == INTTOK) {
		store_word_instr(src_reg, 0, indexAddress);
	}

	freeTempRegister(indexAddress);
	freeTempRegister(src_reg);
}

/*
	If we come across just the variable name for an array (e.g a1),
	load its base address into a register and return that
*/
int load_array_base(SymTabEntry *idInfo, int global_or_local) {
	tempRegister container = findAvailableTempRegister();

	if (global_or_local == FP) {
		//Need to figure out if its a parameter or the usual local
		load_local_array_base(idInfo->scope, container, idInfo->offset);
	} else if (global_or_local == GP) {
		load_reg_address_instr(container, idInfo->offset, GP);
	}
	return container;
}

/* 
	Loads the base address, at a given offset) into a given register. 

	For a parameter, we use "lw" to get array address stored at 
	offset($fp). Otherwise for a local, we use "la" to get the
	actual address that the array starts from.
*/
 void load_local_array_base(int scope, int dest_reg, int offset) {
	if (scope == 1) { 
		load_word_instr(dest_reg, offset, FP);
	} else {
		load_reg_address_instr(dest_reg, offset, FP);
	}
}

static int calc_index_address(ast_node *idNode, SymTabEntry *idInfo, 
 	int global_or_local) {
	//Use two t_i registers for this entire function.
	tempRegister holder1 = handle_expr(idNode->childlist[1]);
	tempRegister holder2 = findAvailableTempRegister(); 

	if (idInfo->type == CHARTOK) {
		load_val_in_register(holder2, CHAR_SIZE);
	} else if (idInfo->type == INTTOK) {
		load_val_in_register(holder2, INT_SIZE);
	}

	/* 
		Calculates index*type_size = total offset within array (stored
		in holder1)
	*/
	add_instr_for_mult(holder1, holder1, holder2);

	//Load the base address of the array (methods differ betweens scopes)
	if (global_or_local == FP) {
		load_local_array_base(idInfo->scope, holder2, idInfo->offset);
	} else if (global_or_local == GP) {
		load_reg_address_instr(holder2, idInfo->offset, GP);
	}

	//holder2 now contains base_address+total offset = address @ index
	add_instr_for_addition(holder2, holder2, holder1);
	freeTempRegister(holder1);
	return holder2;
}


/**************************************/
/** Helpers related to register-use **/

/*
	Given an index, retrieves an a_i register. We don't
	really have to worry about keeping track of used a_i
	registers (like we do for temps/locals), so this is all
	the functionality needed.
*/
parameterRegister getParamRegister(int i) {
	switch (i) {
		case 0:
			return A0;
		case 1:
			return A1;
		case 2:
			return A2;
		case 3:
			return A3;
		default:
			codegen_error("Up to 4 parameters allowed", inFile, outFile);
			return -1;
	}
}

/*
	Finds an available t0-t9 register and returns 
	its enum value. If none are available, temporarily
	store a ti register on top of the stack and return
	the ti register.
*/
tempRegister findAvailableTempRegister() {
	//Looks for an available register
	for (int i=0; i < 8; i++) {
		if (tempRegistersInUse[i]==0) {
			tempRegistersInUse[i] = 1;
			return T0+i; 
		}
	}

	//If none available, find one with the least values in storage
	int minRegister = 0;
	for (int i=0; i < 8; i++){
		if (tempRegistersInStorage[minRegister] > tempRegistersInStorage[i]) {
			minRegister=i;
		}
	}
	//Increment counter of # of registers in storage for ti
	tempRegistersInStorage[minRegister]++;
	//Put it on top of the stack
	store_reg_on_stack(T0+minRegister);
	return T0+minRegister;
}

localRegister findAvailableLocalRegister() {
	//Looks for an available register
	for (int i=0; i < 8; i++) {
		if (localRegistersInUse[i]==0) {
			localRegistersInUse[i] = 1;
			return S0+i; 
		}
	}

	//If none available, find one with the least values in storage
	int minRegister = 0;
	for (int i=0; i < 8; i++){
		if (localRegistersInStorage[minRegister] > localRegistersInStorage[i]) {
			minRegister=i;
		}
	}
	//Increment counter of # of registers in storage for ti
	localRegistersInStorage[minRegister]++;
	//Put it on top of the stack
	store_reg_on_stack(S0+minRegister);
	return S0+minRegister;
}

//Frees up a given local register
void freeLocalRegister(int reg) {
	//If register has something waiting in storage, put that back
	if (localRegistersInStorage[reg-S0] > 0) {
		load_reg_from_stack(reg); 
		localRegistersInStorage[reg-S0]--;

	} else { //Otherwise, mark it as fully available
		localRegistersInUse[reg-S0] = 0;
	}
}

//Goes through all local registers and frees them
void freeAllLocalRegisters() {
	for (int i=0; i < 8; i++) {
		freeLocalRegister(S0+i);
	}
}

//Frees up a given temp. register
void freeTempRegister(int reg) {
	//If register has something waiting in storage, put that back
	if (tempRegistersInStorage[reg-T0] > 0) {
		load_reg_from_stack(reg); 
		tempRegistersInStorage[reg-T0]--;

	} else { //Otherwise, mark it as fully available
		tempRegistersInUse[reg-T0] = 0;
	}
}

//Goes through all temporary registers and frees them
void freeAllTempRegisters() {
	for (int i=0; i < 8; i++) {
		freeTempRegister(T0+i);
	}
}

/********************/
/** Helpers for scope changes**/

//Push a symbol table when we enter a new scope
 void push_scope() {
	push_symtab();
	currScope++;
	funcOffset = -stackCurrOffset;
}

//Pop top of symbol table stack on leaving a scope
 void pop_scope() {
	pop_symtab();
	currScope--;
}

/********************/
/* Helpers for nested blocks */

void init_offset_stack() {
	offsetStack = malloc(sizeof(OffsetStack));
	offsetStack->numOffsets=0;
	offsetStack->stack = malloc(0);
}

//Push a function offset to stack of offsets 
 void push_offset() {
	offsetStack->numOffsets++;
	offsetStack->stack = realloc(offsetStack->stack, 
		offsetStack->numOffsets*sizeof(int));

	offsetStack->stack[offsetStack->numOffsets-1] = funcOffset;
}

//Pop an offset and update the stack pointer to the popped offset
int pop_offset() {
 	//Note that popped offsets are negative (since stack grows "up")
	int poppedOffset = offsetStack->stack[offsetStack->numOffsets-1];
	offsetStack->numOffsets--;

	/* Update $sp in MIPS.
	stackCurrOffset+poppedOffset = total change in offset */
	add_immed_instr(SP, SP, stackCurrOffset+poppedOffset);
	//Update $sp in compiler 
	stackCurrOffset = -poppedOffset;

	return poppedOffset;
}

//Frees memory used by offset stack
void destroy_offset_stack() {
	offsetStack->numOffsets = 0;
	free(offsetStack->stack);
	free(offsetStack);
}

