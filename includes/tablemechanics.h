/*
	Header file for codetable.c!

	A lot of "static" variables/functions are declared here that
	only codetable.c uses. We also define the MIPS Instruction
	struct and the Code Table (a list of instructions). Keeps
	the .c file from getting too cluttered! 

	@author Noor Aftab
	@date Friday, 24th April 2020
*/

#ifndef _CODETABLE_H
#define _CODETABLE_H

//Has access to everything in codegen
//#include "traversaltotable.h"

//Size of a register, in bytes
#define REGISTER_SIZE 4

//A MIPS instruction has at most 4 "things" 
typedef struct {
	char *command; //Can hold command or label/address
	char *op1; //R_D
	char *op2;	//R_S
	char *op3;	//R_T
} Instruction;

typedef struct {
	int numInstructions;
	Instruction **instrSet;
} CodeTable;

/** Static variables/functions that only codetable.c needs to know about **/

//The actual, glorious code table
extern CodeTable *codeTable;

//Keeps track of params and locals during a block
extern int paramOffset;
extern int localsOffset;

//Helper variables to store labels for if-else statements/while loops
extern Instruction *ifelseLabelHolder;
extern Instruction *whileLabelHolder;
extern Instruction *whileDoneLabelHolder;
extern int numUniqueLabels; //Useful for generating unique labels

extern void init_table_and_loop_stack();
extern Instruction *init_Instruction_struct();
extern void allocate_param_to_stack(int size);
extern void allocate_at_stack_top(int size);
extern void load_addr_instr(int dest_reg, char *addr);
extern char *generate_store_and_load_addr(int addr_offset, int src_reg2);

//Label-making helper functions
extern Instruction *generate_while_label(); 
extern Instruction *generate_while_done_label();
extern Instruction *generate_else_label();
extern Instruction *generate_ifelse_done_label();
extern Instruction *generate_unique_label();
extern Instruction *generate_given_label(char *label);
extern char* get_address_from_label(Instruction *label);

extern void jump(Instruction *label);
extern void jump_to_register(int reg);

extern void branch(Instruction *label); //If-else, while, logical OR/AND
extern void bnezInstr(int src_reg1, Instruction *label); //Logical OR
extern void beqzInstr(int src_reg1, Instruction *label); //Logical AND, if-else
extern void syscall_instr(); //write, writeln, read
//Cuts down on code repetition
extern Instruction *setup_3op_instr(char *command, char *dest_reg, char *src_reg1, char *src_reg2);
extern Instruction *setup_2op_instr(char *command, char *dest_reg, char *src_reg1);
//Converts register enums to strings for MIPS code
extern char *getRegStr(int reg);
extern void add_instr_to_code_table(Instruction *instr);

/** Support for nested while-loops **/
typedef struct {
	int numLoops;
	Instruction **startLabelStack;
	Instruction **doneLabelStack;
} WhileLabelStack;

extern WhileLabelStack *whileLabelStack;
extern void push_start_of_while_loop();
extern void push_end_of_while_loop();

#endif

