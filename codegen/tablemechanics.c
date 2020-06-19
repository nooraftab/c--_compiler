/*
	This file contains the nitty-gritty functions for
	traversaltotable.c, such as setting up instructions, 
	labels, and functionality for nested loops. These
	are functions codetraversal.c never needs to know
	about! 

	@author Noor Aftab
	@date Tuesday, 5th May 2020
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "traversaltotable.h"
#include "tablemechanics.h"
#include "lexer.h"

/** These are extern variables from tablemechanics.h **/

CodeTable *codeTable;
WhileLabelStack *whileLabelStack; //For nested loops

//Tracks space for parameters and locals in a function
int paramOffset = 0;
int localsOffset = 0;

//Helper for conditionals and loops
Instruction *ifelseLabelHolder = NULL;
Instruction *whileLabelHolder = NULL;
Instruction *whileDoneLabelHolder = NULL;
int numUniqueLabels = 0; //Helps generate unique labels

/** Actual functions below **/

void init_table_and_loop_stack() {
	//Initializes code table
	codeTable = malloc(sizeof(CodeTable));
	codeTable->numInstructions = 0;
	codeTable->instrSet = malloc(0);

	//Initialize stack of loop labels
	whileLabelStack = malloc(sizeof(WhileLabelStack));
	whileLabelStack->numLoops = 0;
	whileLabelStack->startLabelStack = malloc(0);
	whileLabelStack->doneLabelStack = malloc(0);
}

/*
	Makes space for an Instruction structure in the heap, initalizes
	all fields to NULL. We use this function everytime we want to make
	an instruction (to later add in the code table)!
*/
Instruction *init_Instruction_struct() {
	Instruction *instr = malloc(sizeof(Instruction));
	instr->command=NULL, instr->op1=NULL, instr->op2=NULL,
	instr->op3=NULL;
	return instr;
}

/*
	Given a size, update our trackers that space needs to be
	allocated for parameters during generate_function_prologue
*/
void allocate_param_to_stack(int size) {
	//add_immed_instr(SP, SP, -size);
	stackCurrOffset += size;
	paramOffset += size;
}

//Given a size, allocate it at top of $sp ('pushing')
void allocate_at_stack_top(int size) {
	//We can immediately add to $sp since we're at top of the stack
	add_immed_instr(SP, SP, -size);
	stackCurrOffset += size;
	localsOffset += size;
}

/* 
	Loads a memory address into a register. 
	Useful for writeln/syscalls
	la dest_reg, addr
*/
 void load_addr_instr(int dest_reg, char *addr) {
	Instruction *laInstr = setup_2op_instr(strdup("la"), 
		getRegStr(dest_reg), strdup(addr));
	add_instr_to_code_table(laInstr);
}

/*
	Returns string that represents address offset from a register e.g. 4($sp)
	Useful for load/store instructions!
*/
char *generate_store_and_load_addr(int addr_offset, int src_reg2) {
	//Gets length of the number
	int offsetLen = snprintf(NULL, 0, "%d", addr_offset);
	char *finalAddr = malloc(offsetLen + 5 + 1); //5: len of ($xi)

	char *Rt_holder = getRegStr(src_reg2);
	sprintf(finalAddr, "%d(%s)", addr_offset, Rt_holder);
	free(Rt_holder);
	return finalAddr;
}

/** Label making functions **/

//Generates label for a while loop
 Instruction *generate_while_label() {
	Instruction *baseLabel = generate_unique_label();
	char *baseAddress = get_address_from_label(baseLabel);

	baseLabel->command = realloc(baseLabel->command, 
		sizeof(baseLabel->command)+6);
	sprintf(baseLabel->command, "%s_while:", baseAddress);
	free(baseAddress);

	return baseLabel;
}

//Generates label to jump to on end of while loop
 Instruction *generate_while_done_label() {
	Instruction *baseLabel = generate_unique_label();
	char *baseAddress = get_address_from_label(baseLabel);

	baseLabel->command = realloc(baseLabel->command, 
		sizeof(baseLabel->command)+10);
	sprintf(baseLabel->command, "%s_whileDone:", baseAddress);
	free(baseAddress);

	return baseLabel;
}

//Generates label to jump to on else (for if-else conditionals)
 Instruction *generate_else_label() {
	Instruction *baseLabel = generate_unique_label();
	char *baseAddress = get_address_from_label(baseLabel);

	baseLabel->command = realloc(baseLabel->command, 
		sizeof(baseLabel->command)+5);
	sprintf(baseLabel->command, "%s_else:", baseAddress);
	free(baseAddress);

	return baseLabel;
}

//Generates label for code to run after a conditional is fully done
 Instruction *generate_ifelse_done_label() {
	Instruction *baseLabel = generate_unique_label();
	char *baseAddress = get_address_from_label(baseLabel);

	baseLabel->command = realloc(baseLabel->command, 
		sizeof(baseLabel->command)+11);
	sprintf(baseLabel->command, "%s_ifElseDone:", baseAddress);
	free(baseAddress);

	return baseLabel;
}

//As the name suggests, creates a unique label on each call!
 Instruction *generate_unique_label() {
	//Need to know how much space needed for the number in string 
	int numLength = snprintf(NULL, 0, "%d", numUniqueLabels);
	char uniqueLabel[1+numLength+1];

	sprintf(uniqueLabel, "L%d", numUniqueLabels);
	//Countof # of labels helps make sure each new one is unique
	numUniqueLabels++; 
	return generate_given_label(uniqueLabel);
}

//Creates a label, but gives user flexibility on labels name
 Instruction *generate_given_label(char *label) {
	Instruction *labelInstr = init_Instruction_struct();

	char tempHolder[1+strlen(label)+1+1];
	sprintf(tempHolder, ".%s:", label);
	labelInstr->command = strdup(tempHolder);

	return labelInstr;
}

//Get .X (actual label name/address) from a label of format .X:
 char* get_address_from_label(Instruction *label) {
	int addressLabelLength = strlen(label->command)-1; //Ignore ":"
	char *addr = malloc(addressLabelLength + 1); 

	//Copy the ".X" part into addr
	memcpy(addr, label->command, addressLabelLength);
	addr[addressLabelLength]='\0';
	return addr;
}

//j address
void jump(Instruction *label) {
	Instruction *jInstr = init_Instruction_struct();
	jInstr->command = strdup("j");
	jInstr->op1 = get_address_from_label(label);
	add_instr_to_code_table(jInstr);
}

//jr reg
 void jump_to_register(int reg) {
	Instruction *jrInstr = init_Instruction_struct();
	jrInstr->command = strdup("jr");
	jrInstr->op1 = getRegStr(reg);
	add_instr_to_code_table(jrInstr);
}

//b labelAddress
 void branch(Instruction *label) {
	Instruction *bInstr = init_Instruction_struct();
	bInstr->command = strdup("b");
	bInstr->op1 = get_address_from_label(label);
	add_instr_to_code_table(bInstr);
}

//bnez src_reg1, labelAddress (useful for OR)
 void bnezInstr(int src_reg1, Instruction *label) {
	Instruction *instr = setup_2op_instr(strdup("bnez"),
		getRegStr(src_reg1), get_address_from_label(label));
	add_instr_to_code_table(instr);
}

//beqz src_reg1, labelAddress (useful for AND)
 void beqzInstr(int src_reg1, Instruction *label) {
	Instruction *instr = setup_2op_instr(strdup("beqz"),
		getRegStr(src_reg1), get_address_from_label(label));
	add_instr_to_code_table(instr);
}

//syscall
 void syscall_instr() {
	Instruction *syscall = init_Instruction_struct();
	syscall->command=strdup("syscall");
	add_instr_to_code_table(syscall);
}

/*********************************/
/** Code repetition helpers. Note: strings are expected to be malloc-ed by caller **/

 Instruction *setup_3op_instr(char *command, char *dest_reg, 
 	char *src_reg1, char *src_reg2) {
 	Instruction *instr = init_Instruction_struct();
 	instr->command = command;

	instr->op1 = dest_reg;
	instr->op2 = src_reg1;
	instr->op3 = src_reg2;
	return instr;
}

 Instruction *setup_2op_instr(char *command, char *dest_reg, char *src_reg1) {
	Instruction *instr = init_Instruction_struct();
 	instr->command = command;

	instr->op1 = dest_reg;
	instr->op2 = src_reg1;
	return instr;
}

/**************************************************/

//Matches register enum to malloc-ed register string
 char *getRegStr(int reg) {
	switch (reg) {
		case S0:
			return strdup("$s0");
		case S1:
			return strdup("$s1");
		case S2:
			return strdup("$s2");
		case S3:
			return strdup("$s3");
		case S4:
			return strdup("$s4");
		case S5:
			return strdup("$s5");
		case S6:
			return strdup("$s6");
		case S7:
			return strdup("$s7");
		case A0:
			return strdup("$a0");
		case A1:
			return strdup("$a1");
		case A2:
			return strdup("$a2");
		case A3:
			return strdup("$a3");
		case T0:
			return strdup("$t0");
		case T1:
			return strdup("$t1");
		case T2:
			return strdup("$t2");
		case T3:
			return strdup("$t3");
		case T4:
			return strdup("$t4");
		case T5:
			return strdup("$t5");
		case T6:
			return strdup("$t6");
		case T7:
			return strdup("$t7");
		case SP:
			return strdup("$sp");
		case FP:
			return strdup("$fp");
		case RA:
			return strdup("$ra");
		case GP:
			return strdup("$gp");
		case V0:
			return strdup("$v0");
		default:
			codegen_error("Invalid register passed in!", inFile, outFile);
			return NULL;
	}
}

/*****************************************************/
/** Functions for manipulating malloc-ed structures **/

//Adds an instruction to codetable
 void add_instr_to_code_table(Instruction *instr) {
	codeTable->numInstructions++;
	
	//Re-allocates memory for instruction set of the code table
	codeTable->instrSet = realloc(codeTable->instrSet, 
	sizeof(Instruction)*codeTable->numInstructions);
	codeTable->instrSet[codeTable->numInstructions-1] = instr;
}

//Upon entering a while loop, push its start label to the stack
void push_start_of_while_loop(Instruction *startLabel) {
	int topOfStackNum = ++whileLabelStack->numLoops;
	//Re-allocate space
	whileLabelStack->startLabelStack = realloc(whileLabelStack->startLabelStack,
		topOfStackNum*sizeof(Instruction));

	//Initialize new top of stack
	whileLabelStack->startLabelStack[topOfStackNum-1] = startLabel;
}

//Also push its done label to the stack 
void push_end_of_while_loop(Instruction *doneLabel) {
	//No need to increment since we do it in push_start_of_while_loop
	int topOfStackNum = whileLabelStack->numLoops;

	//Re-allocate space
	whileLabelStack->doneLabelStack = realloc(whileLabelStack->doneLabelStack,
		topOfStackNum*sizeof(Instruction));

	//Initialize new top of stack
	whileLabelStack->doneLabelStack[topOfStackNum-1]=doneLabel;
}

