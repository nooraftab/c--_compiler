/*
	This file traverses a C-- AST and tells the Code Table
	what MIPS code to spit out in the .s file

	@author Noor Aftab :)
	@date Tuesday, 5th May 2020
*/

#include <stdio.h>
#include "parser.h"
#include "lexer.h"
#include "symtab.h"
#include "codetraversal.h"
#include "traversaltotable.h"
#include "traversalmechanics.h"

//Stack of offsets within a function (for nested blocks)
OffsetStack *offsetStack;

//If encountered a break while traversing
int breakOccured = 0;
//If encounted a return while traversing
int returnOccured = 0;

/*
	Offset within a function. I realized quite late that I 
	basically treat funcOffset the same as stackCurrOffset, so
	it's actually a little pointless. Given more time, 
*/
int funcOffset = 0;
//A measure of how mnay globals we have
int globalOffset = 0;
int currScope = 0;

//Tracks $sp throughout program
int stackCurrOffset = 0;

//Kickstarts code generation
void traverse_and_generate_code() {
	//Print .s file beginning, 
	setup_mips_code();
	//Start traversing AST!
	handle_program(ast_tree.root);
}

/*
	Starts traversing from the root - direct children will
	only be VAR_DECLs (if any) follow by FUNC_DECLs

	@param program: AST root
*/
void handle_program(ast_node *program) {
 	init_offset_stack();
	int i = 0;

	//Go through global variables
	while (i < program->num_children && 
		program->childlist[i]->symbol->grammar_symbol == VAR_DECL) {
		handle_variable_declaration(program->childlist[i]);
		i++;
	}

	//Go through function declarations
	while (i < program->num_children) {
		handle_function(program->childlist[i]);
		i++;
	}

	destroy_offset_stack();
}


void handle_variable_declaration(ast_node *varDecl) {
	//Initialize values in its symbol table entry 
	int type = varDecl->childlist[0]->symbol->token; 
	char *name = varDecl->childlist[1]->symbol->lexeme;
	int dimension = -1; //Assume non-array
	int isInit = 0;
	int offset; //From $gp or $fp (for arrays - it's actually size!)

	//Meaning, its an array
	if (varDecl->num_children > 2) { 
		dimension = varDecl->childlist[3]->symbol->value;
		//Treat it as already initialized - let user be in charge
		isInit = 1; 
	}
	int arrayOffset = 0;
	
	//Global
	if (currScope == 0) { 
		offset = handle_global_allocation(type, dimension);

		if (dimension != -1) { //Array
			globalOffset += offset;
			//$gp grows towards bottom, so this is how we get offset from $gp
			offset = globalOffset - offset; 
		}

	} else {
		//Allocates in terms of calculated offsets
		offset = handle_local_allocation(type, dimension);
		if (dimension != -1) {
			arrayOffset = offset;
			funcOffset -= arrayOffset;
			//Top of array is at top of stack. since $sp grows negatively
			offset = funcOffset; 
		}
		//Actually make space in the stack. changeInOffset only needed for arrays
		allocate_space_for_locals(type, dimension, arrayOffset);
	}
	insert_var_symtab_entry(name, currScope, type, dimension, offset, isInit);
}


void handle_parameter(ast_node *paramDecl, int numParam) {
	//Intialize values in symbol table entry
	int type = paramDecl->childlist[0]->symbol->token;
	char *name = paramDecl->childlist[1]->symbol->lexeme;
	int dimension = -1; //Assume non-array
	int isInit = 1; //Parameter, so assume already initialized

	//Case of an array!
	if (paramDecl->num_children > 2) { 
		/* 
			Don't need an actual size, but set to 0 to 
			signal its an array and make space for a pointer.
		*/
		dimension = 0; 
	}

	int offset = handle_param_allocation(type, dimension);
	allocate_space_for_params(type, getParamRegister(numParam), offset, dimension); 
	insert_var_symtab_entry(name, currScope, type, dimension, offset, isInit);
}

/*
	Traverses function declaration. Direct children are
	the return type, id/name, possibly a PDL, and a BLOCK

	@param funcDecl: FUNC_DECL tree node
*/
void handle_function(ast_node *funcDecl) {
	freeAllTempRegisters();
	freeAllLocalRegisters();

	//Get function name
	char *funcName = funcDecl->childlist[1]->symbol->lexeme;
	//Create entry for function in ST - 1st param is name, 2nd is type
	SymTabEntry *funcEntry = insert_func_symtab_entry(funcName, 
		funcDecl->childlist[0]->symbol->token);
	generate_function_label(funcName); //Make a label

	push_scope(); //Also updates funcOffset

	/** Parameters handling **/
	int i;
	for (i=2; i < funcDecl->num_children && 
		funcDecl->childlist[i]->symbol->grammar_symbol == PDL; i++) {
		handle_parameter(funcDecl->childlist[i], i-2);
	}
	pad_params();

	//Some error-checking here
	if ((i-2) <= 4) {
		funcEntry->numParams = i-2; //i-2 is the number of parameters
	} else {
		codegen_error("Compiler only supports 4 or fewer arguments/parameters", inFile, outFile);
	}

	/** Body handling **/
	generate_function_prologue();
	handle_block(funcDecl->childlist[i]); //Traverse block of code
	generate_function_epilogue();

	pop_scope();
	freeAllTempRegisters();
	freeAllLocalRegisters();
}

/*
	Traverses block of code. Direct children could be 
	VAR_DECL (if any), followed by a bunch of statements

	@param block: BLOCK tree node
*/
void handle_block(ast_node *block) {
	int i = 0;
	push_scope();
	push_offset();

	while (i < block->num_children && 
	 	block->childlist[i]->symbol->grammar_symbol == VAR_DECL) {
		handle_variable_declaration(block->childlist[i]);
		i++;
	}
	pad_locals(); //Ensure 4-byte alignment

	/*
		Statements! If a break occurs, we want to exit the body
		traversal since there's no point. 
	*/
	while (i < block->num_children && breakOccured == 0) {
		handle_stmt(block->childlist[i]);
		i++;
	} 

	funcOffset = pop_offset();
	pop_scope();
	
	if (breakOccured==1) {
		add_break_instr();
		breakOccured = 0;
	}

}

//Checks through different code statements 
void handle_stmt(ast_node *stmt) {
	switch (stmt->symbol->token) {
		case RETURN:
			handle_return(stmt);
			break;
		case READ:
			handle_read(stmt);
			break;
		case WRITE:
			handle_write(stmt);
			break;
		case WRITELN:
			handle_writeln();
			break;	
		case BREAK:
			handle_break();
			break;
		case IF:
			handle_if(stmt);
			break;
		case WHILE:
			handle_while(stmt);
			break;
		case NONTERMINAL: //Only NONTERMINAL we can have is a BLOCK
			handle_block(stmt);
			break;
		//Otherwise, be optimistic and assume we're looking at expression
		default: 
			handle_expr(stmt);
	}
}

//return Expr;
void handle_return(ast_node *returnNode) {
	int finalReg = handle_expr(returnNode->childlist[0]);

	//Put return value in V0
	move_registers(V0, finalReg);
	freeTempRegister(finalReg);

	/*
		Since pop_offset changes stackCurroffset, restore it and 
		push offset back in.
		This is good to do since we won't necessarily always 
		return when we see a return e.g. in conditionals
	*/
	int startOfBlockOffset = offsetStack->stack[0];
	add_immed_instr(SP, SP, stackCurrOffset+startOfBlockOffset);

	execute_return();
	returnOccured = 1; //Signal that we came across a return

}

//read id;
void handle_read(ast_node *readNode) {
	//Finds the id we're using 
	SymTabEntry *idInfo = symtab_lookup(readNode->childlist[0]->symbol->lexeme);
	tempRegister placeHolder = findAvailableTempRegister();
	add_read_instr(placeHolder);

	//Assign the value in placeholder to the id
	if (idInfo->scope == 0) {
		assign_global(placeHolder, idInfo->type, idInfo->offset);
	} else {
		assign_local(placeHolder, idInfo->type, idInfo->offset);
	}
	idInfo->isInit = 1; //Flag that the id has been initialized
}

//writeln;
void handle_writeln() {
	add_newline_instr();
}

//write Expr; 
void handle_write(ast_node *writeNode) {
	tempRegister expr = handle_expr(writeNode->childlist[0]);
	add_write_instr(expr);
}

//break;
void handle_break() {
	//Flag it! MIPS code is added later, so we can also pop off block
	breakOccured = 1; 
}

//if (Expr) Stmt...
void handle_if(ast_node *ifNode) {
	//Evaluate condition expression
	tempRegister condResult = handle_expr(ifNode->childlist[0]);

	//Inserts branching code depending on value of condResult
	add_instr_for_if(condResult); 
	freeTempRegister(condResult);

	//Conditional is here in case of empty statement
	if (ifNode->num_children == 3) handle_stmt(ifNode->childlist[1]); 

	//Handles the else code
	handle_else(ifNode->childlist[ifNode->num_children-1]);
}

//... else Stmt
void handle_else(ast_node *elseNode) {
	//Inserts else label
	add_instr_for_else();

	//Conditional is here case of empty statement
	if (elseNode->num_children ==1) handle_stmt(elseNode->childlist[0]);

	//Adds label for subsqeuent code to go under
	add_instr_for_cond_finish();
}

//while (Expr) Stmt
void handle_while(ast_node *whileNode) {
	//Generates label
	add_instr_for_while();
	tempRegister condResult = handle_expr(whileNode->childlist[0]);
	//Checks condResult 
	add_instr_for_while_middle(condResult);
	freeTempRegister(condResult);

	//Conditional is here case of empty statement
	if (whileNode->num_children == 2) handle_stmt(whileNode->childlist[1]);

	//If our while Stmt is not a block, just the break statement
	if (breakOccured==1) {
		add_break_instr();
		breakOccured=0;
	}

	//Branches up to beginning of while-loop (where condition is rechecked)
	add_instr_for_while_end();
}

/*
	Goes through different forms of an expression, returning 
	register containing the final output 
*/
int handle_expr(ast_node *exprNode) {
	switch(exprNode->symbol->token) {
		case ASSIGN:
			return handle_assign(exprNode);
		case OR:
			return handle_or(exprNode);
		case AND:
			return handle_and(exprNode);
		case EQ:
			return handle_equal(exprNode);
		case NEQ:
			return handle_not_equal(exprNode);
		case LESS:
			return handle_less(exprNode);
		case LEQ:
			return handle_less_or_equal(exprNode);
		case GREAT:
			return handle_greater(exprNode);
		case GEQ:
			return handle_greater_or_equal(exprNode);
		case ADD:
		 	return handle_add(exprNode);
		case SUB:
		 	return handle_subtraction(exprNode);
		case MULT:
			return handle_multiplication(exprNode);
		case DIV:
			return handle_division(exprNode);
		case NEG:
			return handle_negation(exprNode);
		case NUM:
			return handle_num(exprNode);
		case ID:
			return handle_id(exprNode);
		default:
			codegen_error("Bad expression in AST", inFile, outFile);
			return -1;
	}
}

// id = Expr
int handle_assign(ast_node *assignNode) {
	//Get left node and find its symbol table entry
	ast_node *lhsNode = assignNode->childlist[0];
	SymTabEntry *idInfo = symtab_lookup(lhsNode->symbol->lexeme);

	//Just a little bit of error checking
	if (idInfo->isFunction == 1) {
		codegen_error("Cannot assign functions a value.", inFile, outFile);
	}
	
	//Evaluate expression on right
	tempRegister right = handle_expr(assignNode->childlist[1]);

	if (idInfo->scope == 0) { //Global scope
		if (idInfo->dimension == -1) { //Normal variable
			assign_global(right, idInfo->type, idInfo->offset);
			idInfo->isInit = 1;
			freeTempRegister(right);
		} else { //Assigning an array index
			store_array_index(lhsNode, idInfo, GP, right);
		}
	} else { //Function scope
		if (idInfo->dimension == -1) { //Normal variable
			/* 
				Use local variables - not actually needed! Just
				wanted to use the s_i registers.
			*/
			localRegister local = findAvailableLocalRegister();
			move_registers(local, right);

			assign_local(local, idInfo->type, idInfo->offset);
			idInfo->isInit=1;
			freeTempRegister(right);
			freeLocalRegister(local);
		} else { //Assigning an array index
			store_array_index(lhsNode, idInfo, FP, right);
		}	
	}

	//For assignment statements, no need to return anything
	return -1;
}

// ||
int handle_or(ast_node *orNode) {
	tempRegister left = handle_expr(orNode->childlist[0]);
	tempRegister right = handle_expr(orNode->childlist[1]);
	add_instr_for_or(left, left, right);
	freeTempRegister(right); //Can just free the right - only need left
	return left;
}

// &&
int handle_and(ast_node *andNode) {
	tempRegister left = handle_expr(andNode->childlist[0]);
	tempRegister right = handle_expr(andNode->childlist[1]);
	add_instr_for_and(left, left, right);
	freeTempRegister(right);
	return left;
}

// ==
int handle_equal(ast_node *eqNode) {
	tempRegister left = handle_expr(eqNode->childlist[0]);
	tempRegister right = handle_expr(eqNode->childlist[1]);
	add_instr_for_eq(left, left, right);
	freeTempRegister(right);
	return left;
}

// !=
int handle_not_equal(ast_node *neqNode) {
	tempRegister left = handle_expr(neqNode->childlist[0]);
	tempRegister right = handle_expr(neqNode->childlist[1]);
	add_instr_for_neq(left, left, right);
	freeTempRegister(right);
	return left;
}

// <
int handle_less(ast_node *lessNode) {
	tempRegister left = handle_expr(lessNode->childlist[0]);
	tempRegister right = handle_expr(lessNode->childlist[1]);
	add_instr_for_less(left, left, right);
	freeTempRegister(right);
	return left;
}

// <=
int handle_less_or_equal(ast_node *leqNode) {
	tempRegister left = handle_expr(leqNode->childlist[0]);
	tempRegister right = handle_expr(leqNode->childlist[1]);
	add_instr_for_leq(left, left, right);
	freeTempRegister(right);
	return left;
}

// >
int handle_greater(ast_node *greaterNode) {
	tempRegister left = handle_expr(greaterNode->childlist[0]);
	tempRegister right = handle_expr(greaterNode->childlist[1]);
	add_instr_for_great(left, left, right);
	freeTempRegister(right);
	return left;
}

// >=
int handle_greater_or_equal(ast_node *geqNode) {
	tempRegister left = handle_expr(geqNode->childlist[0]);
	tempRegister right = handle_expr(geqNode->childlist[1]);
	add_instr_for_geq(left, left, right);
	freeTempRegister(right);
	return left;
}

// +
int handle_add(ast_node *addNode) {
	tempRegister left = handle_expr(addNode->childlist[0]);
	tempRegister right = handle_expr(addNode->childlist[1]);
	add_instr_for_addition(left, left, right);
	freeTempRegister(right);
	return left;
}

// - (either binary or unary!)
int handle_subtraction(ast_node *subNode) {
	tempRegister left = handle_expr(subNode->childlist[0]);

	//If only one child, is unary operator
	if (subNode->num_children == 1) {
		add_instr_for_unarysub(left, left);
		return left;
	}

	tempRegister right = handle_expr(subNode->childlist[1]);
	add_instr_for_sub(left, left, right);
	freeTempRegister(right);
	return left;
}

// *
int handle_multiplication(ast_node *multNode) {
	tempRegister left = handle_expr(multNode->childlist[0]);
	tempRegister right = handle_expr(multNode->childlist[1]);
	add_instr_for_mult(left, left, right);
	freeTempRegister(right);
	return left;
} 

// /
int handle_division(ast_node *divNode) {
	tempRegister left = handle_expr(divNode->childlist[0]);
	tempRegister right = handle_expr(divNode->childlist[1]);
	add_instr_for_div(left, left, right);
	freeTempRegister(right);
	return left;
}

// ! (unary operator)
int handle_negation(ast_node *negNode) {
	tempRegister left = handle_expr(negNode->childlist[0]);
	add_instr_for_neg(left, left);
	return left;
}

//Base Case of Num: Loads a number into a register (which is returned)
int handle_num(ast_node *numNode) {
	tempRegister numReg = findAvailableTempRegister();
	load_val_in_register(numReg, numNode->symbol->value);
	return numReg;
}

//Base case of ID: loads value/address into a register (which is returned)
int handle_id(ast_node *idNode) {
	SymTabEntry *idInfo = symtab_lookup(idNode->symbol->lexeme);
	tempRegister varValue = findAvailableTempRegister();

	if (idInfo->isFunction == 1) {
		//For function call, return a register containing the return value
		return handle_function_call(idNode->childlist[0], idInfo->numParams, 
			idInfo->name);
	}

	//Global scope
	if (idInfo->scope == 0) {
		if (idInfo->dimension == -1) { //Normal variable
			load_global(varValue, idInfo->type, idInfo->offset, idInfo->isInit);
		} else if (idNode->num_children > 0) { //Array index e.g. a1[6]
			return load_array_index(idNode, idInfo, GP);
		} else { //Just the name of an array e.g. a1
			return load_array_base(idInfo, GP);
		}	
	} 
	//Local scope
	else  {
		if (idInfo->dimension == -1) { //Normal variable
			load_local(varValue, idInfo->type, idInfo->offset, idInfo->isInit);
		} else if (idNode->num_children > 0) { //Array index e.g. a1[6]
			return load_array_index(idNode, idInfo, FP);
		} else { //Just the name of an array e.g. a1
			return load_array_base(idInfo, FP);
		}
	}
	return varValue;
}

//For arguments in a function call!
void handle_expression_list(ast_node *elNode, int numParams) {
	//Some error-checking
	if (numParams != elNode->num_children) {
		codegen_error("Wrong number of arguments to function!", inFile, outFile);
	}
	if (numParams > 4) {
		codegen_error("Compiler only supports 4 or fewer arguments/parameters", inFile, outFile);
	}

	//for each parameter, move into an A_i register
	for (int i=0; i < numParams; i++) {
		int reg = handle_expr(elNode->childlist[i]);
		int param = getParamRegister(i);
		move_registers(param, reg);
		freeTempRegister(reg);
	}
}

