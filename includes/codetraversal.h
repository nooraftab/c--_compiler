#ifndef _CODETRAVERSAL_H
#define _CODETRAVERSAL_H

typedef struct {
	int numOffsets;
	int *stack;
} OffsetStack;

extern OffsetStack *offsetStack;

//1 is break is encountered
extern int breakOccured; 
//1 if return is enocuntered
extern int returnOccured;
extern int funcOffset;   //Within a function
extern int globalOffset; //How many globals
extern int currScope;  

//Kickstars traversal
extern void traverse_and_generate_code();

/*
	Functions only codegen needs for traversing. Not in the codegen.h
	header file because we only want it visible here. Not in another
	seperate header file because doesn't feel like it warrants that.
*/

//AST traversal functions
void handle_program(ast_node *program);
void handle_variable_declaration(ast_node *varDecl);
void handle_parameter(ast_node *paramDecl, int numParam);
void handle_function(ast_node *funcDecl);
void handle_block(ast_node *block);
void handle_stmt(ast_node *stmt);

//Keyword functionality
void handle_return(ast_node *returnNode);
void handle_read(ast_node *readNode);
void handle_writeln();
void handle_write(ast_node *writeNode);
void handle_break();
void handle_if(ast_node *ifNode);
void handle_else(ast_node *elseNode);
void handle_while(ast_node *whileNode);

//Functions for handling expressions, and operators in expressions
extern int handle_expr(ast_node *exprNode);
int handle_assign(ast_node *assignNode);
int handle_or(ast_node *orNode);
int handle_and(ast_node *andNode);
int handle_equal(ast_node *eqNode);
int handle_not_equal(ast_node *neqNode);
int handle_less(ast_node *lessNode);
int handle_less_or_equal(ast_node *leqNode);
int handle_greater(ast_node *greaterNode);
int handle_greater_or_equal(ast_node *geqNode);
int handle_add(ast_node *addNode);
int handle_subtraction(ast_node *subNode);
int handle_multiplication(ast_node *multNode);
int handle_division(ast_node *divNode);
int handle_negation(ast_node *negNode);
int handle_num(ast_node *numNode);

int handle_id(ast_node *idNode);
extern void handle_expression_list(ast_node *elNode, int numParams); 


#endif