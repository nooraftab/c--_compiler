/*
	This file sets up the code table and contains functions
	that codetraversal.c uses to outline what MIPS code
	should be spit out as it's traversing. 

	@author Noor Aftab :)
	@date Tuesday 5th May 2020
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "traversaltotable.h"
#include "tablemechanics.h"
#include "lexer.h"

static Instruction *functionEpilogueLabelHolder = NULL;

/*
	Called at the beginning of every traversal - sets up .s file.
	Hardcoded!
*/
void setup_mips_code() {
	init_table_and_loop_stack();

	//.data
	Instruction *dataDir = init_Instruction_struct();
	dataDir->command = strdup(".data");
	add_instr_to_code_table(dataDir);

	//_newline_:
	Instruction *newlineLabel = init_Instruction_struct();
	newlineLabel->command = strdup("_newline_:");
	add_instr_to_code_table(newlineLabel);

	//.asciiz \n
	Instruction *newline = init_Instruction_struct();
	newline->command = strdup(".asciiz \"\\n\"");
	add_instr_to_code_table(newline);

	//.text
	Instruction *textDir = init_Instruction_struct();
	textDir->command = strdup(".text");
	add_instr_to_code_table(textDir);

	//.globl main
	Instruction *mainGlobal = init_Instruction_struct();
	mainGlobal->command = strdup(".globl main");
	add_instr_to_code_table(mainGlobal);
}

/*
	Called before a function call. Stores temporary registers and 
	parameter registers, so the called function can use them.
*/

void generate_function_precall() {
	//Saving 12 registers, so make space for 48 bytes
	int spaceToMake = (8+4)*REGISTER_SIZE;

	//Update $sp in MIPS file and in our counter
	add_immed_instr(SP, SP, -spaceToMake);
	stackCurrOffset += spaceToMake;

	//I use numbers here because I felt it would be more readable
	store_word_instr(T0, 44, SP);
	store_word_instr(T1, 40, SP);
	store_word_instr(T2, 36, SP);
	store_word_instr(T3, 32, SP);
	store_word_instr(T4, 28, SP);
	store_word_instr(T5, 24, SP);
	store_word_instr(T6, 20, SP);
	store_word_instr(T7, 16, SP);

	store_word_instr(A0, 12, SP);
	store_word_instr(A1, 8, SP);
	store_word_instr(A2, 4, SP);
	store_word_instr(A3, 0, SP);
}

//Jumps to the function we're calling
void jal_to_function(char *calledFuncName) {
	Instruction *jalInstr = init_Instruction_struct();
	jalInstr->command = strdup("jal");
	jalInstr->op1 = strdup(calledFuncName);
	add_instr_to_code_table(jalInstr);
}

/*
	Called at beginning of a function. Sets up the new
	functions' stack/frame pointers, as well as the return
	address. Stores local registers too!
*/
void generate_function_prologue() {
	functionEpilogueLabelHolder = generate_unique_label();

	//First make space for parameters!
	add_immed_instr(SP, SP, -paramOffset);
	stackCurrOffset = paramOffset; 

	//Update $sp
	int spaceToMake = (2+8)*REGISTER_SIZE;
	add_immed_instr(SP, SP, -spaceToMake);
	stackCurrOffset += spaceToMake; 

	store_word_instr(RA, 36, SP);
	store_word_instr(FP, 32, SP);
	store_word_instr(S0, 28, SP);
	store_word_instr(S1, 24, SP);
	store_word_instr(S2, 20, SP);
	store_word_instr(S3, 16, SP);
	store_word_instr(S4, 12, SP);
	store_word_instr(S5, 8, SP);
	store_word_instr(S6, 4, SP);
	store_word_instr(S7, 0, SP);
	add_immed_instr(FP, SP, stackCurrOffset); //Update $fp
}

/*
	Called at the end of a function. Basically undoes
	what happens in generate_function_prologue()
*/
void generate_function_epilogue() {
	add_instr_to_code_table(functionEpilogueLabelHolder);

	int spaceToReload = (8+2)*REGISTER_SIZE;

	load_word_instr(S7, 0, SP);
	load_word_instr(S6, 4, SP);
	load_word_instr(S5, 8, SP);
	load_word_instr(S4, 12, SP);
	load_word_instr(S3, 16, SP);
	load_word_instr(S2, 20, SP);
	load_word_instr(S1, 24, SP);
	load_word_instr(S0, 28, SP);
	load_word_instr(FP, 32, SP);
	load_word_instr(RA, 36, SP);
	add_immed_instr(SP, SP, paramOffset);
	add_immed_instr(SP, SP, spaceToReload);  //Pops stack frame

	//localsOffset+paramOffset = space used by locals/params we must reload
	stackCurrOffset -= localsOffset + paramOffset + spaceToReload;
	jump_to_register(RA);

	//Reset for the next function
	paramOffset = 0;
	localsOffset = 0;
}

/*
	Called after a function call. Basically undoes what happens in
	generate_function_precal()
*/
void generate_function_postcall() {
	int spaceToReload = (8+4)*REGISTER_SIZE;

	//Restore temporaries
	load_word_instr(T0, 44, SP);
	load_word_instr(T1, 40, SP);
	load_word_instr(T2, 36, SP);
	load_word_instr(T3, 32, SP);
	load_word_instr(T4, 28, SP);
	load_word_instr(T5, 24, SP);
	load_word_instr(T6, 20, SP);
	load_word_instr(T7, 16, SP);

	//Restore arguments
	load_word_instr(A0, 12, SP);
	load_word_instr(A1, 8, SP);
	load_word_instr(A2, 4, SP);
	load_word_instr(A3, 0, SP);

	//Pop stack
	add_immed_instr(SP, SP, spaceToReload);
	stackCurrOffset -= spaceToReload;
}

//Create a label with a function's name - need for jal-ing
void generate_function_label(char *name) {
	Instruction *label = init_Instruction_struct();

	char tempHolder[1+strlen(name)+1+1];
	sprintf(tempHolder, "%s:", name);
	label->command = strdup(tempHolder);

	add_instr_to_code_table(label);
}

/*
	Signals that space must be made for parameters. Stores value
	of a_i register into where the parameter would be stores.
*/
void allocate_space_for_params(int paramType, int paramReg, int offset, int dimension) {
	//Storing an array address: just need 4-bytes!
	if (dimension != -1) { 
		//SP at this point is the same as FP after prologue
		store_word_instr(paramReg, offset, SP);
		allocate_param_to_stack(ALIGN);
		return;
	}

	//Otherwise, if it's a normal variable
	switch(paramType) {
		case INTTOK: 
			store_word_instr(paramReg, offset, SP);
			allocate_param_to_stack(INT_SIZE);
			break;
		case CHARTOK:  
			store_byte_instr(paramReg, offset, SP);
			allocate_param_to_stack(CHAR_SIZE);
			break;
	}
}

//After allocating space for parameters, pad to ensure 4-byte alignement
void pad_params() {
	if (stackCurrOffset%ALIGN != 0) {
		//Basically like allocating a parameter! Except you're not using it
		allocate_param_to_stack(ALIGN-stackCurrOffset%ALIGN);
	}
}

//Update $sp to make space on the stack for local variables
void allocate_space_for_locals(int type, int dimension, int arrOffset) {
	if (dimension != -1) { //An array
		allocate_at_stack_top(arrOffset);
		return;
	}

	//A normal variable
	switch(type) {
		case INTTOK: 
			allocate_at_stack_top(INT_SIZE);	
			break;
		case CHARTOK:  
			allocate_at_stack_top(CHAR_SIZE);
			break;
	}
}

//Similar to pad_params - allocate space on stack for alignment
void pad_locals() {
	if (stackCurrOffset%ALIGN != 0) {
		//Basically the same process as allocating locals to stack
		allocate_at_stack_top(ALIGN-stackCurrOffset%ALIGN);
	}
}

//Stores value in src_reg1 into the given offset from $gp
void assign_global(int src_reg, int type, int offset) {
	switch(type) {
		case INTTOK: 
			store_word_instr(src_reg, offset, GP);
			break;
		case CHARTOK:  
			store_byte_instr(src_reg, offset, GP);
			break;
	}
}

//Stores value in src_reg1 into the given offset from $fp
void assign_local(int src_reg, int type, int offset) {
	switch(type) {
		case INTTOK: 
			store_word_instr(src_reg, offset, FP);
			break;
		case CHARTOK:  
			store_byte_instr(src_reg, offset, FP);
			break;
	}
}

//Loads value at given offset from $gp, into dest_reg
void load_global(int dest_reg, int type, int offset, int isInit) {
	//A dash of error-checking
	if (isInit == 0) {
		codegen_error("Trying to use undeclared global!", inFile, outFile);
	}

	switch(type) {
		case INTTOK: 
			load_word_instr(dest_reg, offset, GP);
			break;
		case CHARTOK:  
			load_byte_instr(dest_reg, offset, GP);
			break;
	}
}

//Loads value at the passed-in offset from $fp into dest_reg
void load_local(int dest_reg, int type, int offset, int isInit) {
	//Another dash of error-checking
	if (isInit == 0) {
		codegen_error("Trying to use undeclared local!", inFile, outFile);
	}

	switch(type) {
		case INTTOK: 
			load_word_instr(dest_reg, offset, FP);
			break;
		case CHARTOK:  
			load_byte_instr(dest_reg, offset, FP);
			break;
	}
}

//move R_d, R_s
void move_registers(int dest_reg, int src_reg1) {
	Instruction *moveInstr = setup_2op_instr(strdup("move"), 
		getRegStr(dest_reg), getRegStr(src_reg1));
	add_instr_to_code_table(moveInstr);
}

//@param src_reg1 register stored at top of stack
void store_reg_on_stack(int src_reg1) {
	allocate_at_stack_top(REGISTER_SIZE);
	store_word_instr(src_reg1, 0, SP);
}

//@param src_reg1 register loaded from top of stack
void load_reg_from_stack(int src_reg1) {
	//No helper function for this sadly - not repeated much
	load_word_instr(src_reg1, 0, SP);
	add_immed_instr(SP, SP, REGISTER_SIZE);
	stackCurrOffset -= REGISTER_SIZE;
	localsOffset -= REGISTER_SIZE;
}

/*
	Helps increment/decrement stack/frame pointers
	addiu dest_reg, src_reg1, immed
*/
 void add_immed_instr(int dest_reg, int src_reg1, int immed) {
 	//Get length of immediate value
	int immedLength = snprintf(NULL, 0, "%d", immed);
	char *immedHolder = malloc(sizeof(immedLength)+1);
	sprintf(immedHolder, "%d", immed);

	Instruction *addiuInstr = setup_3op_instr(strdup("addiu"), 
		getRegStr(dest_reg), getRegStr(src_reg1), immedHolder);

	add_instr_to_code_table(addiuInstr);
}

void execute_return() {
	branch(functionEpilogueLabelHolder);
}

//Uses syscall to read in an integer from user
void add_read_instr(int dest_reg) {
	//Does the read_int syscall
	load_val_in_register(V0, 5);
	syscall_instr();

	//Integer read in is saved in V0 - move it to dest_reg!
	move_registers(dest_reg, V0);
}

//Uses syscall to write new line to console
void add_newline_instr() {
	load_val_in_register(V0, 4);
	load_addr_instr(A0, "_newline_");
	syscall_instr();
}

/*
	Uses syscall to write to console
	@param src_reg1 register value to write
*/
void add_write_instr(int src_reg1) {
	load_val_in_register(V0, 1);
	load_reg_address_instr(A0, 0, src_reg1);
	syscall_instr();
}

//Adds instruction to break out of a while loop
void add_break_instr() {
	Instruction *topOfStack = whileLabelStack->doneLabelStack[whileLabelStack->numLoops-1];

	if (topOfStack != NULL) {
		//Branches to enclosing loops done label
		branch(topOfStack);
	} else {
		//Moreee error-checking
		codegen_error("'break;' can only be used within while loops!", inFile, outFile);
	}
}

/** Functions for if-else statements **/

//Inserts branching code to else 
void add_instr_for_if(int src_reg1) {
	Instruction *elseLabel = generate_else_label();
	beqzInstr(src_reg1, elseLabel);

	ifelseLabelHolder = elseLabel;
}

/* 
	Inserts branching code for when conditional is done, and 
	inserts label for else-bodys (which should follow after 
	this function call)
*/
void add_instr_for_else() {
	Instruction *condDoneLabel = generate_ifelse_done_label();
	branch(condDoneLabel);

	Instruction *elseLabel = ifelseLabelHolder;

	add_instr_to_code_table(elseLabel);
	ifelseLabelHolder = condDoneLabel;
}

//Inserts label for code after the conditional
void add_instr_for_cond_finish() {
	Instruction *condFinish = ifelseLabelHolder;
	add_instr_to_code_table(condFinish);
}

/** Functions for while() loops **/

//Adds while-label to code table
void add_instr_for_while() {
	Instruction *whileStartLabel = generate_while_label();
	add_instr_to_code_table(whileStartLabel);
	push_start_of_while_loop(whileStartLabel);

	Instruction *whileDoneLabel = generate_while_done_label();
	push_end_of_while_loop(whileDoneLabel);
}

/* 
	Checks if register in-charge of branching is 0 (so 
	conditional is false)
*/
void add_instr_for_while_middle(int src_reg1) {
	beqzInstr(src_reg1, whileLabelStack->doneLabelStack[whileLabelStack->numLoops-1]);
}

//Jumps back up to while conditional
void add_instr_for_while_end() {
	int topOfStackIndex =  whileLabelStack->numLoops-1;

	branch(whileLabelStack->startLabelStack[topOfStackIndex]);
	add_instr_to_code_table(whileLabelStack->doneLabelStack[topOfStackIndex]);

	//"Popping" the loop stack
	whileLabelStack->numLoops--; 
}

/*
	Puts down instructions that execute a logical OR. 

	@param dest_reg: 0 if both R_s and R_t are 0, otherwise 1
	@param src_reg1: if src_reg1 is 1, result is set to 1
	@param src_reg2: only check src_reg2 is src_reg1 is 0
*/
void add_instr_for_or(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *trueLabel = generate_unique_label();
	Instruction *restOfCodeLabel = generate_unique_label();

	bnezInstr(src_reg1, trueLabel);
	bnezInstr(src_reg2, trueLabel);
	load_val_in_register(dest_reg, 0);
	branch(restOfCodeLabel);

	add_instr_to_code_table(trueLabel);
	load_val_in_register(dest_reg, 1);
	add_instr_to_code_table(restOfCodeLabel);
}

/*
	Puts down instructions that execute a logical AND. 

	@param dest_reg: 1 if both R_s and R_t are 1, otherwise 0
	@param src_reg1: if src_reg1 is 0, result is set to 0
	@param src_reg2: only check src_reg2 is src_reg1 is 1
*/
void add_instr_for_and(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *falseLabel = generate_unique_label();
	Instruction *restOfCodeLabel = generate_unique_label();

	beqzInstr(src_reg1, falseLabel);
	beqzInstr(src_reg2, falseLabel);
	load_val_in_register(dest_reg, 1);
	branch(restOfCodeLabel);

	add_instr_to_code_table(falseLabel);
	load_val_in_register(dest_reg, 0);
	add_instr_to_code_table(restOfCodeLabel);
}

// ==
void add_instr_for_eq(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *eqInstr = setup_3op_instr(strdup("seq"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(eqInstr);
}

// !=
void add_instr_for_neq(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *neqInstr = setup_3op_instr(strdup("sne"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(neqInstr);
}

// dest_reg=1 if src_reg1 < src_reg2, otherwise dest_reg=0
void add_instr_for_less(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *lessInstr = setup_3op_instr(strdup("slt"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(lessInstr);
}

// dest_reg=1 if src_reg1 <= src_reg2, otherwise dest_reg=0
void add_instr_for_leq(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *leqInstr=setup_3op_instr(strdup("sle"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(leqInstr);
}

// dest_reg=1 if src_reg1 > src_reg2, otherwise dest_reg=0
void add_instr_for_great(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *greatInstr = setup_3op_instr(strdup("sgt"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(greatInstr);
}

// dest_reg=1 if src_reg1 >= src_reg2, otherwise dest_reg=0
void add_instr_for_geq(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *geqInstr= setup_3op_instr(strdup("sge"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(geqInstr);
}

// dest_reg = src_reg1 + src_reg2
void add_instr_for_addition(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *addInstr = setup_3op_instr(strdup("add"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(addInstr);
}

// dest_reg = src_reg1 - src_reg2
void add_instr_for_sub(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *subInstr = setup_3op_instr(strdup("sub"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(subInstr);
}

// dest_reg = src_reg1*src_reg2
void add_instr_for_mult(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *multInstr = setup_3op_instr(strdup("mulo"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(multInstr);
}

// dest_reg = src_reg1/src_reg2
void add_instr_for_div(int dest_reg, int src_reg1, int src_reg2) {
	Instruction *divInstr = setup_3op_instr(strdup("div"), 
		getRegStr(dest_reg), getRegStr(src_reg1), getRegStr(src_reg2));
	add_instr_to_code_table(divInstr);
}

// dest_reg = ~src_reg1 (I'm assuming ~ is the same operator as !)
void add_instr_for_neg(int dest_reg, int src_reg1) {
	Instruction *falseLabel = generate_unique_label();
	Instruction *restOfCodeLabel = generate_unique_label();

	beqzInstr(src_reg1, falseLabel);
	//If 1, value we return is 0
	load_val_in_register(dest_reg, 0);
	branch(restOfCodeLabel);

	add_instr_to_code_table(falseLabel);
	//If 0, we return 1  
	load_val_in_register(dest_reg, 1);
	add_instr_to_code_table(restOfCodeLabel);

}

// dest_reg = -src_reg1
void add_instr_for_unarysub(int dest_reg, int src_reg1) {
	Instruction *unSubInstr = setup_2op_instr(strdup("neg"), 
		getRegStr(dest_reg), getRegStr(src_reg1));
	add_instr_to_code_table(unSubInstr);
}

// Loads an immediate value (param immed) into a register (param dest_reg)
void load_val_in_register(int dest_reg, int immed) {
	int numDigits = snprintf(NULL, 0, "%d", immed);
	char *immedString = malloc(numDigits+1);
	sprintf(immedString, "%d", immed);

	Instruction *liInstr = setup_2op_instr(strdup("li"), 
		getRegStr(dest_reg), immedString);

	add_instr_to_code_table(liInstr);
}

//Prints the code table to a given file (param out)
void output_code_table_to_file(FILE *out) {
	for (int i=0; i < codeTable->numInstructions; i++) {

		if (codeTable->instrSet[i]->command != NULL) {
			fprintf(out, "%s ", codeTable->instrSet[i]->command);
		}

		if (codeTable->instrSet[i]->op1 != NULL) {
			fprintf(out, "%s", codeTable->instrSet[i]->op1);
		}

		if (codeTable->instrSet[i]->op2 != NULL) {
			fprintf(out, ", %s", codeTable->instrSet[i]->op2);
		}

		if (codeTable->instrSet[i]->op3 != NULL) {
			fprintf(out, ", %s", codeTable->instrSet[i]->op3);
		}

		fprintf(out, "\n");
	}
}

//Frees up heap memory used for the code table
void destroy_code_table() {
	//Free space used by nested loop supports
	free(whileLabelStack->startLabelStack);
	free(whileLabelStack->doneLabelStack);
	free(whileLabelStack);	

	//Frees the actual code table
	for (int i=0; i < codeTable->numInstructions; i++) {
		if (codeTable->instrSet[i]->command != NULL) {
			free(codeTable->instrSet[i]->command);
		}

		if (codeTable->instrSet[i]->op1 != NULL) {
			free(codeTable->instrSet[i]->op1);
		}

		if (codeTable->instrSet[i]->op2 != NULL) {
			free(codeTable->instrSet[i]->op2);
		}

		if (codeTable->instrSet[i]->op3 != NULL) {
			free(codeTable->instrSet[i]->op3);
		}				
		
		free(codeTable->instrSet[i]);

	}
	free(codeTable->instrSet);
	free(codeTable);
}

/* 
	Loads information from memory into a register
	lw dest_reg, addr_offset(src_reg1)
*/
void load_word_instr(int dest_reg, int addr_offset, int src_reg1) {
	//Set-up the register address
	char *tempAddrStorage = generate_store_and_load_addr(addr_offset, src_reg1);

	Instruction *loadInstr = setup_2op_instr(strdup("lw"), 
		getRegStr(dest_reg), tempAddrStorage);

	add_instr_to_code_table(loadInstr);
}

/*
	Similar to load_word_instr, except it loads a single
	byte instead of 4.
	lb dest_reg, addr_offset(src_reg1)
*/
void load_byte_instr(int dest_reg, int addr_offset, int src_reg1) {
	char *tempAddrStorage = generate_store_and_load_addr(addr_offset, src_reg1);

	Instruction *loadInstr = setup_2op_instr(strdup("lb"), 
		getRegStr(dest_reg), tempAddrStorage);

	add_instr_to_code_table(loadInstr);
}

/* 
	Loads a register address into a register. 
	Useful for writing/syscalls
	la dest_reg, addr_offset(src_reg1)
*/
 void load_reg_address_instr(int dest_reg, int addr_offset, int src_reg1) {
	char *tempAddrStorage = generate_store_and_load_addr(addr_offset, src_reg1);

	Instruction *loadInstr = setup_2op_instr(strdup("la"), 
		getRegStr(dest_reg), tempAddrStorage);

	add_instr_to_code_table(loadInstr);
}

/*
	Stores register information into a place in memory
	sw src_reg1, addr_offset(src_reg2)
*/
 void store_word_instr(int src_reg1, int addr_offset, int src_reg2) {
	char *tempAddrStorage = generate_store_and_load_addr(addr_offset, src_reg2);

	Instruction *storeInstr = setup_2op_instr(strdup("sw"),
		getRegStr(src_reg1), tempAddrStorage);

	add_instr_to_code_table(storeInstr);
}

/* 
	Same function as store_word_instr, but only stores a single byte
	instead of 4.
	sb src_reg1, addr_offset(src_reg2)
*/
void store_byte_instr(int src_reg1, int addr_offset, int src_reg2) {
	char *tempAddrStorage = generate_store_and_load_addr(addr_offset, src_reg2);

	Instruction *storeInstr = setup_2op_instr(strdup("sb"), 
		getRegStr(src_reg1), tempAddrStorage);

	add_instr_to_code_table(storeInstr);
}

