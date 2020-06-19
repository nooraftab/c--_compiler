/*
	This header file declares enums and functions called/defined 
	in both codegen.c and codetable.c

	@author Noor Aftab
	@date Friday, 24th April 2020
*/

#ifndef _TRAVERSALTOTABLE_H
#define _TRAVERSALTOTABLE_H
#include "parser.h"

//Size of an integer, in bytes
#define INT_SIZE 4
//Size of a char, in bytes
#define CHAR_SIZE 1

//Since $sp and $gp are 4-byte aligned
#define ALIGN 4

typedef enum {
	S0=500, S1, S2, S3, S4, S5, S6, S7
} localRegister;

typedef enum {
	A0=508, A1, A2, A3
} parameterRegister;

typedef enum {
	T0=512, T1, T2, T3, T4, T5, T6, T7
} tempRegister;

typedef enum {
	SP=520, FP, GP, RA, V0, V1
} otherImptRegisters;

FILE *inFile;
FILE *outFile;

//$sp's total offset as program executes
extern int stackCurrOffset;

/** Functions codegen.c expects to have for "tracing out" MIPS code **/

extern void setup_mips_code();

//Functions for function entering/exiting
extern void generate_function_precall();
extern void jal_to_function(char *name);
extern void generate_function_prologue();
extern void generate_function_epilogue();
extern void generate_function_postcall();
extern void generate_function_label(char *name);

//Allocating space on stack for params
extern void allocate_space_for_params(int paramType, int paramReg, int offset, int dimension);
extern void pad_params(); 
//Allocating space on stack for locals
extern void allocate_space_for_locals(int type, int dimension, int arrOffset);
extern void pad_locals();

extern void assign_global(int src_reg, int type, int offset);
extern void assign_local(int src_reg, int type, int offset);
extern void load_global(int dest_reg, int type, int offset, int isInit);
extern void load_local(int dest_reg, int type, int offset, int isInit);

extern void move_registers(int dest_reg, int src_reg1);

//For saving t_i registers 
extern void store_reg_on_stack(int src_reg1);
extern void load_reg_from_stack(int src_reg1);

//Helpful for changing $sp when exiting a block (and in general)
extern void add_immed_instr(int dest_reg, int src_reg1, int immed);

extern void execute_return();
extern void add_read_instr(int src_reg1);
extern void add_newline_instr(); //writeln
extern void add_write_instr(int src_reg1); //write
extern void add_break_instr();

//If-else statements
extern void add_instr_for_if(int src_reg1);
extern void add_instr_for_else();
extern void add_instr_for_cond_finish();

//While loops
extern void add_instr_for_while();
extern void add_instr_for_while_middle(int src_reg1);
extern void add_instr_for_while_end();

//Expressions!
extern void add_instr_for_or(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_and(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_eq(int dest_reg, int src_reg1, int src_reg2); 
extern void add_instr_for_neq(int dest_reg, int src_reg1, int src_reg2); 
extern void add_instr_for_less(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_leq(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_great(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_geq(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_addition(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_sub(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_mult(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_div(int dest_reg, int src_reg1, int src_reg2);
extern void add_instr_for_neg(int dest_reg, int src_reg1);
extern void add_instr_for_unarysub(int dest_reg, int src_reg1);

extern void load_reg_address_instr(int dest_reg, int offset, int src_reg1);
extern void load_val_in_register(int dest_reg, int value);
extern void store_word_instr(int src_reg1, int addr_offset, int src_reg2);
extern void store_byte_instr(int src_reg1, int addr_offset, int src_reg2);
extern void load_word_instr(int dest_reg, int addr_offset, int src_reg1);
extern void load_byte_instr(int dest_reg, int addr_offset, int src_reg1);

/** End of "tracing out" functions **/

//Prints code table to given .s file
extern void output_code_table_to_file(FILE *out);
//Frees up heap memory used by table
extern void destroy_code_table();

//Error handling!
extern void codegen_error(char *err_message, FILE *in, FILE *out);

#endif
