/*
  Noor's Parser for C-- :)
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

static ast_node *match(int expected_token, FILE *fd);
//Error recovery functions
static void parser_error(char *err_string, FILE *fd);
static ast_node *insert_missing_token(int expected_token, FILE *fd);
static void skip_ahead(char *err_string, FILE *fd);

/* LL(1) Grammar functions. 

Note: [funcName]_helper functions are there to avoid repetitions in 
      funcName functions! The hope was for it to make things more modular
      and concise.
*/
static void program(FILE *fd, ast_node *parent);
static void prog1(FILE *fd, ast_node *parent, ast_node *type, ast_node *id);
static void prog2(FILE *fd, ast_node *parent, ast_node *type, ast_node *id);

static void fdl1(FILE *fd, ast_node *parent, ast_node *type, ast_node *id);
static void fdl(FILE *fd);
static void fdl_helper(FILE *fd, ast_node *type);

static void vdl(FILE *fd, ast_node *parent);
static void vdl1(FILE *fd, ast_node *parent, ast_node *type, ast_node *id);

static void pdl(FILE *fd, ast_node *parent);
static void pdl_helper(FILE *fd, ast_node *parent, ast_node *type);
static void pdl1(FILE *fd, ast_node *parent, ast_node *param);
static void pdl2(FILE *fd, ast_node *parent);

static void block(FILE *fd, ast_node *parent);
static void stmtlist(FILE *fd, ast_node *parent);
static void stmtlist1(FILE *fd, ast_node *parent);
static void stmt(FILE *fd, ast_node *parent);

static ast_node *expr(FILE *fd);
static ast_node *E0(FILE *fd);
static ast_node *E0_prime(FILE *fd, ast_node *left);

static ast_node *E1(FILE *fd);
static ast_node *E1_prime(FILE *fd, ast_node *left);
static ast_node *E2(FILE *fd);
static ast_node *E2_prime(FILE *fd, ast_node *left);

static ast_node *E3(FILE *fd);
static ast_node *E3_prime(FILE *fd, ast_node *left);
static ast_node *E3_prime_helper(FILE *fd, ast_node *op, ast_node *left);

static ast_node *E4(FILE *fd);
static ast_node *E4_prime(FILE *fd, ast_node *left);
static ast_node *E4_prime_helper(FILE *fd, ast_node *op, ast_node *left);

static ast_node *E5(FILE *fd);
static ast_node *E5_prime(FILE *fd, ast_node *left);
static ast_node *E5_prime_helper(FILE *fd, ast_node *op, ast_node *left);

static ast_node *E6(FILE *fd);
static ast_node *E6_prime(FILE *fd, ast_node *left);
static ast_node *E6_prime_helper(FILE *fd, ast_node *op, ast_node *left);

static ast_node *E7(FILE *fd);
static ast_node *E8(FILE *fd);
static ast_node *E8_prime(FILE *fd, ast_node *parent);

static void exprlist(FILE *fd, ast_node *parent);
static void el_prime(FILE *fd, ast_node *parent);

int lookahead;        
ast ast_tree; 
ast_node *match_node;  //Helper variable for terminals we want in AST

/**************************************************************************/
void parse(FILE *fd)  {
  ast_info *s;
  ast_node *n;

  // create the root AST node
  s = create_new_ast_node_info(NONTERMINAL, 0, 0, ROOT, 0, 0);
  n = create_ast_node(s);
  if(init_ast(&ast_tree, n)) {
    parser_error("ERROR: bad AST\n", fd);
  }

  lookahead = lexan(fd);
  program(fd, ast_tree.root);  // program corresponds to the start state
  
  // the last token should be DONE
  if (lookahead != DONE) {
    parser_error("EoF expected.", fd);   
  } else {
     match(DONE, fd);
  }
}

/*
  Matches current lookahead token with an expected token. If applicable,
  we return pointer to an AST node for a correctly matched token.

  @param expected_token: the token we expect to match with lookahead
  @param fd: file pointer
*/
static ast_node *match(int expected_token, FILE *fd) {
  if (lookahead == expected_token) {

    //If punctuation, we don't want to make an AST node so we move along
    if(lookahead >= ENDTOKEN) { 
      lookahead=lexan(fd);
      return NULL;
    }

    //Otherwise, make an AST node (if ID or num/float, add a value to it) 
    if (lookahead == ID) {
      match_node=create_ast_node(create_new_ast_node_info(lookahead, 0, 0, 0, tokenval, src_lineno));
    } else if (lookahead == NUM) {
      match_node=create_ast_node(create_new_ast_node_info(lookahead, atoi(tokenval), 0, 0, 0, src_lineno));
    } else if (lookahead == FLOAT) {
      match_node=create_ast_node(create_new_ast_node_info(lookahead, 0, atof(tokenval), 0, 0, src_lineno));
    } else { //Keywords! 
      match_node=create_ast_node(create_new_ast_node_info(lookahead, 0, 0, 0, 0, src_lineno));
    }
    
    lookahead = lexan(fd);
    return match_node;

  } else { 
    //On an unexpected token, see if it's something we can look over
    return insert_missing_token(expected_token, fd);
  }
}

/*
  Destroys our AST, closes our file, prints an error message 
  and exits the program! Truly washing our hands :)
  @param err_string: the message to print out
*/
static void parser_error(char *err_string, FILE *fd) {
  //Help prevent memory leaks in case of errors!
  destroy_ast(&ast_tree);
  fclose(fd);

  if(err_string) {
    if (lookahead == LEXERROR) {
      //Helps narrow down the error
      lexer_error(lexer_error_message, src_lineno);
    } else {
      printf("Line %d: %s\n", src_lineno, err_string);
    }
  }
  exit(1);
}  

/*
  Tries to see if an expected token can be inserted without trouble e.g if
  it's simple punctuation.

  @param expected_token: the token we want to 'insert'
  @return if its a bracket, return a node so we can add it to our AST
*/
static ast_node *insert_missing_token(int expected_token, FILE *fd) {
  printf("******* \nError at Line %d: Expected ", src_lineno);
  lexer_emit(expected_token, "filler_val");
  printf("Got following instead: ");
  lexer_emit(lookahead, "filler");
  printf("*******\n");

  /*
    By simply returning NULL (or a node), it appears that we've 
    'matched' the token we want so we can move on
  */
  if (expected_token == SEMICOLON || 
      expected_token == LBRACE ||
      expected_token == LPAREN || 
      expected_token == RPAREN ||
      expected_token == COMMA) {
    printf("Compiler inserted expected token for you. May lead to unexpected results - that's on you!\n");
    return NULL;
  } 

  if (expected_token == RBRACK) {
    match_node=create_ast_node(create_new_ast_node_info(RBRACK, 0, 0, 0, 0, src_lineno));
    return match_node;
  }

  //If it isn't a simple case we can handle/overlook...
  parser_error("\nUnexpected code. Recheck!\n", fd);
  return NULL;
}

/*
  This is a simple function for panic-mode error recovery - meaning that 
  if we see something we really don't like, we jump ahead to the end of
  a statement (or the end of a function)

  @param err_string warning string to print out
*/
static void skip_ahead(char *err_string, FILE *fd) {
  printf("*******\n");
  printf("Line %d: %s", src_lineno, err_string);
  printf("*******\n");

  //';' means we've reached end of line. '}' means we've reached end of function
  while (lookahead != SEMICOLON && lookahead != RBRACE) {
    if (lookahead == DONE) {
      parser_error("Unexpected EoF.\n", fd);
    }
    lookahead = lexan(fd);
  }
}

/**************************************************************************/
/*
 *  Corresponds to start symbol of the LL(1) Grammar. Marks off the 
 *  type and ID for either a variable or function declaration.
 *  @param fd: the input file
 *  @param parent: the parent AST node  (ROOT at the beginning)
 */
static void program(FILE *fd, ast_node *parent) {
  ast_node *typetok;
  ast_node *id_child;

  switch(lookahead) {
    case INTTOK:
      typetok=match(INTTOK, fd);
      id_child=match(ID, fd);
      prog1(fd, parent, typetok, id_child);
      break;
    
    case CHARTOK:
      typetok=match(CHARTOK, fd);
      id_child=match(ID, fd);
      prog1(fd, parent, typetok, id_child);
      break;

    /*
      Rather than going into panic-mode, on seeing an unexpected type we
      should just generate error since I feel like it'll be problematic
      down the road if we kept on parsing (especially if this is a function)
    */ 
    default:
      parser_error("Unexpected type to variable/function declaration.\n", fd);
  }
}

/* 
  In this non-terminal function, we check if we're handling variable
  or function declarations. The decision is made depending on the terminals
  we see.
  @param fd: input file
  @param parent: parent of PROG1
  @param type: of our variable/function declaration (to add tree nodes here)
  @param id: its ID, also to build the tree 
*/
static void prog1(FILE *fd, ast_node *parent, ast_node *type, ast_node *id) {
  ast_node *decl;

  switch(lookahead) {
    //';' means a variable declaration
    case SEMICOLON:
      decl = create_ast_node(create_new_ast_node_info(NONTERMINAL, 0, 0, VAR_DECL, 0, 0));

      //Node & Tree building function calls
      add_child_node(parent, decl);
      add_child_node(decl, type);
      add_child_node(decl, id);

      match(SEMICOLON, fd);
      program(fd, parent);
      break;

    //'[a_num];' is another type of variable (array) declaration
    case LBRACK:
      decl = create_ast_node(create_new_ast_node_info(NONTERMINAL, 0, 0, VAR_DECL, 0, 0));

      //Node & Tree building function calls
      add_child_node(parent, decl);
      add_child_node(decl, type);
      add_child_node(decl, id);

      add_child_node(decl, match(LBRACK, fd));
      add_child_node(decl, match(NUM, fd));
      add_child_node(decl, match(RBRACK, fd));

      match(SEMICOLON, fd);
      program(fd, parent);
      break;

    //A '(' means a function declaration, so we move on to checking that
    //case LPAREN:
    case LPAREN:
      prog2(fd, parent, type, id);
      break;

    //Don't know if we're looking at variable or function so just generate error
    default:
      parser_error("Unexpected end to variable declaration/start to function declaration.\n", fd);
  }
}

/* 
  Marks off beginning of function declaration, moves on to checking the rest
  @param fd: input file
  @param parent
  @param type: type of the func. definition, to pass on 
  @param id: its ID, also to pass on
*/
static void prog2(FILE *fd, ast_node *parent, ast_node *type, ast_node *id) {
  switch(lookahead) {
    case LPAREN:
      match(LPAREN, fd);
      fdl1(fd, parent, type, id);
      break;

    //Shouldn't be able to reach here, but just in case
    default:
      insert_missing_token(LPAREN, fd);
  }
}

/*
  This non-terminal function sets up basis for how a function should be
  structured (e.g. with curly braces, a block of code after etc.) after 
  the 'type id(' initialization.
  We also add in a function declaration node for it in the tree.

  @param fd: input file
  @param parent: parent node - always ends up being root, through fdl()
  @param type: func. declaration's type, to build AST node
  @param id: its ID, to build AST node
*/
static void fdl1(FILE *fd, ast_node *parent, ast_node *type, ast_node *id) {
  ast_node *decl;

  //decl beomes parent for param & block children
  decl=create_ast_node(create_new_ast_node_info(NONTERMINAL, 0, 0, FUNC_DECL, 0, 0));
  add_child_node(parent, decl); //Attach to root
  add_child_node(decl, type);
  add_child_node(decl, id);

  switch(lookahead) {
    //')' in case there are no params
    case CHARTOK: case INTTOK: case RPAREN: default:
      pdl(fd, decl);
      match(RPAREN, fd);
      match(LBRACE, fd);
      block(fd, decl);

      //Can't have functions within functions, so parent is always root
      fdl(fd); 
      break;
  }
}

/*
  Does essentially the same thing as prog() but with assurance that it's
  looking at a FUNC_DECL. No parent param since ROOT will always be the 
  parent in the AST
  @param fd: input file
*/
static void fdl(FILE *fd) {
  ast_node *type;
  
  switch(lookahead) {
    //Main code for 'int' and 'char' done below switch statement
    case INTTOK:
      type=match(INTTOK, fd);
      fdl_helper(fd, type);
      break;
    case CHARTOK:
      type=match(CHARTOK, fd);
      fdl_helper(fd, type);
      break;

    //Since it's possible to hit EoF after ending a func. definition
    case DONE: default:
      break;
  }
}

static void fdl_helper(FILE *fd, ast_node *type) {
  //Matched id and '(' for a correct function definition
  ast_node *id_child=match(ID, fd);
  match(LPAREN, fd);
  //Passes type and ID to fdl1() to make AST node
  fdl1(fd, ast_tree.root, type, id_child);
}

/*
  Checks for correct parameter declaration in a function, attaches
  them to AST.

  @param fd: input C-- file
  @param parent: the FUNC_DECL node these params stem from
*/
static void pdl(FILE *fd, ast_node *parent) {
  ast_node *type;  

  switch(lookahead) {
    //Matches param type
    case CHARTOK: 
      type=match(CHARTOK, fd);
      pdl_helper(fd, parent, type);
      break;
    case INTTOK:
      type=match(INTTOK, fd);
      pdl_helper(fd, parent, type);
      break;

    case ID:
      parser_error("Invalid type to parameter. Only use char or int.\n", fd);

    //')' if end of param list. Default since PDL can go to epsilon (synchronizing set)
    case RPAREN: default: 
      break;
  }
}

static void pdl_helper(FILE *fd, ast_node *parent, ast_node *type) {
  ast_node *param = create_ast_node(create_new_ast_node_info(NONTERMINAL, 0, 0, PDL, 0, 0));
  add_child_node(parent, param); 

  //Matches id, adds it to param AST node
  ast_node *id_child=match(ID, fd);
  add_child_node(param, type);
  add_child_node(param, id_child);
  //Checks for array params
  pdl1(fd, parent, param);
}

/*
  Handles if an array is passed in as a parameter

  @param fd: input C-- code
  @param parent: PDLs func. definition, to pass along for multiple params
  @parent param: if there IS an array, add on to  PDL we're currently at
*/
static void pdl1(FILE *fd, ast_node *parent, ast_node *param) {
  ast_node *brack;

  switch(lookahead) {
    //'[' means an array is an argument 
    case LBRACK:
      brack=match(LBRACK, fd);
      add_child_node(param, brack);

      brack=match(RBRACK, fd);
      add_child_node(param, brack);

      pdl2(fd, parent); //Check for other params
      break;

    //Check for other params
    case COMMA: case RPAREN: default:
      pdl2(fd, parent);
      break;
  }
}

/*
  Checks for end of param. list, or if there are other params
*/
static void pdl2(FILE *fd, ast_node *parent) {
  switch(lookahead) {
    //Multiple params - go back to code handling a param. declaration
    case COMMA:
      match(COMMA, fd);
      pdl(fd, parent);
      break;

    //Error recovery case - if we see char/int, assume comma is missing
    case CHARTOK: case INTTOK:
      insert_missing_token(COMMA, fd);
      pdl(fd, parent);

    case ID:
      parser_error("Invalid type to parameter. Only use char or int.\n", fd);

    //End of params
    case RPAREN: default:
      break;
  }
}

/* 
  Sets up structure of a block, where it can start with 
  variable declarations, if any, and must have statements following.

  @param fd: input file
  @param parent: FUNC_DECL node or outer block this block is in
*/
static void block(FILE *fd, ast_node *parent) {
  ast_node *block;

  block = create_ast_node(create_new_ast_node_info(NONTERMINAL, 0, 0, BLOCK, 0, 0));
  add_child_node(parent, block);

  switch(lookahead) {
    case INTTOK: case CHARTOK: 
    //FIRST(stmtlist), since VDL can go to epsilon
    case SEMICOLON: case RETURN: case READ:
    case WRITE: case WRITELN: case BREAK:
    case IF: case WHILE: case LBRACE: 
    //FIRST(expr), which is in FIRST(stmtlist)
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      vdl(fd, block);
      stmtlist(fd, block);
      break;

    //Could've skipped-ahead here, but felt like it didn't set a great ~tone~
    default:
      parser_error("Bad block start. Expected variable declaration or a statement.\n", fd);
  }
}

/*
  Structures a variable declaration, similar to prog(). 

  @param fd: C-- file pointer
  @param parent: block node to attach VAR_DECL to, in vdl1()
*/
static void vdl(FILE *fd, ast_node *parent) {
  ast_node *typetok;
  ast_node *id_child;

  switch(lookahead) {
    //Matches variable type, moves on to code after switch()
    case INTTOK:
      typetok=match(INTTOK, fd);
      id_child=match(ID, fd);
      vdl1(fd, parent, typetok, id_child);
      break;
    case CHARTOK:
      typetok=match(CHARTOK, fd);
      id_child=match(ID, fd);
      vdl1(fd, parent, typetok, id_child);
      break;

    //FIRST(stmt)
    case SEMICOLON: case RETURN: case READ:
    case WRITE: case WRITELN: case BREAK:
    case IF: case WHILE: case LBRACE: 
    //FIRST(expr)
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN: default:
      break;
  }  
}

/*
  Checks for array declarations, and checks for other declarations following.
  Create AST node for the variable!

  @param fd: C-- file pointer
  @param parent: encapsulating block node  
  @param type: variable's type
  @param id: variable's id
*/
static void vdl1(FILE *fd, ast_node *parent, ast_node *type, ast_node *id) {
  ast_node *var;

  //We create VAR_DECL node here since vdl() can still go to epsilon 
  var = create_ast_node(create_new_ast_node_info(NONTERMINAL, 0, 0, VAR_DECL, 0, 0));
  add_child_node(parent, var);
  add_child_node(var, type);
  add_child_node(var, id);

  switch(lookahead) {
    case SEMICOLON:
      match(SEMICOLON, fd);
      vdl(fd, parent); //Check for more declarations
      break;

    //Array variable!
    case LBRACK:
      add_child_node(var, match(LBRACK, fd));
      add_child_node(var, match(NUM, fd));
      add_child_node(var, match(RBRACK, fd));

      match(SEMICOLON, fd);
      vdl(fd, parent); //Check for more declarations
      break;

    //Assume user forgot a semicolon
    default:
      insert_missing_token(SEMICOLON, fd);
  }
}

/*
  Sets up structure of a statement list to be one statement after 
  the other, ending with a '}' for the end of a block/function.

  @param fd: input file pointer
  @param parent: encapsulating block node  
*/
static void stmtlist(FILE *fd, ast_node *parent) {
  switch(lookahead) {
    //FIRST(stmt)
    case SEMICOLON: case RETURN: case READ:
    case WRITE: case WRITELN: case BREAK:
    case IF: case WHILE: case LBRACE: 
    //FIRST(expr), since stmt can go to expr
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      stmt(fd, parent);
      stmtlist1(fd, parent);
      break;

    default:
      skip_ahead("Unexpected start to statement. Line ignored", fd);
      lookahead=lexan(fd);
      stmt(fd, parent);
      stmtlist1(fd, parent);
  }
}

/*
  Checks for either the end of statement list or for more statements.

  @param fd: file pointer
  @param parent: encapsulating block node
*/
static void stmtlist1(FILE *fd, ast_node *parent) {
  switch(lookahead) {
    //End of statements!
    case RBRACE:
      match(RBRACE, fd);
      break;

    //More statements to parse
    case SEMICOLON: case RETURN: case READ:
    case WRITE: case WRITELN: case BREAK:
    case IF: case WHILE: case LBRACE: 
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      stmtlist(fd, parent);
      break;

    default:
      skip_ahead("Unexpected start to statement. Line ignored.", fd);
      lookahead=lexan(fd);
      stmtlist(fd, parent);
  }
}

/*
  Parses a statement! Warning: this is a longer-than-normal function.

  @param fd: do I keep repeating this? maybe should've made this global...
  @param parent: encapsulating block node
*/
static void stmt(FILE *fd, ast_node *parent) {
  ast_node *exp;
  ast_node *keyword;
  ast_node *else_key;

  switch(lookahead) {
    case SEMICOLON:
      match(SEMICOLON, fd);
      break;

    //Parses 'return Expr;'
    case RETURN:
      keyword = match(RETURN, fd);
      exp=expr(fd);
      if (exp==NULL) parser_error("Invalid return expression.\n", fd);

      add_child_node(parent, keyword);
      add_child_node(keyword, exp);

      match(SEMICOLON, fd);
      break;

    //Parses 'read id;'
    case READ: 
      keyword = match(READ, fd);
      add_child_node(parent, keyword);
      add_child_node(keyword, match(ID, fd));
      match(SEMICOLON, fd);
      break;

    //Parses 'write Expr;'
    case WRITE: 
      keyword = match(WRITE, fd);
      exp=expr(fd);
      if(exp==NULL) parser_error("Invalid expression in write.\n", fd);

      add_child_node(parent, keyword);
      add_child_node(keyword, exp);
      match(SEMICOLON, fd);
      break;

    //Parses 'writeln;'
    case WRITELN: 
      add_child_node(parent, match(WRITELN, fd));
      match(SEMICOLON, fd);
      break;

    //Parses 'break;'
    case BREAK: 
      add_child_node(parent, match(BREAK, fd));
      match(SEMICOLON, fd);
      break;

    //Parses 'if(Expr) Stmt else Stmt'
    case IF: 
      keyword = match(IF, fd);
      match(LPAREN, fd);    
      exp = expr(fd);
      if (exp==NULL) parser_error("Invalid expression in if-statement.\n", fd);
      match(RPAREN, fd);

      add_child_node(parent, keyword);
      add_child_node(keyword, exp);

      stmt(fd, keyword);
      else_key = match(ELSE, fd);
      stmt(fd, else_key);

      add_child_node(keyword, else_key);
      break;

    //Parses 'while (Expr) Stmt'
    case WHILE: 
      keyword = match(WHILE, fd);
      match(LPAREN, fd);
      exp=expr(fd);
      if (exp==NULL) parser_error("Invalid expression in while loop.\n", fd);
      match(RPAREN, fd);

      add_child_node(parent, keyword);
      add_child_node(keyword, exp);

      stmt(fd, keyword);
      break;

    //Parses '{ Block'
    case LBRACE: 
      match(LBRACE, fd);
      block(fd, parent);
      break;

    //Parses 'Expr;'
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      exp = expr(fd);
      if (exp==NULL) break;

      add_child_node(parent, exp);
      match(SEMICOLON, fd);
      break;

    default:
      skip_ahead("Unexpected start to statement. Line ignored.", fd);
      lookahead=lexan(fd);
  }
}

/*
  Kick-starts the expression parse!
*/
static ast_node *expr(FILE *fd) {
  switch(lookahead) {
    //The base cases for an expression
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      return E0(fd);

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

static ast_node *E0(FILE *fd) {
  ast_node *e1;
  ast_node *E0_p;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      //This line lets it descend all the way to the base case
      e1= E1(fd);
      //On way up, checks operator after base case
      E0_p=E0_prime(fd, e1);
      return E0_p;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Handles assignment to an expression (=Expr)

  @param left: becomes left child of operator, if operator is matched
*/
static ast_node *E0_prime(FILE *fd, ast_node *left) {
  ast_node *op;
  ast_node *exp;

  switch(lookahead) {
    case ASSIGN: //Parses '=Expr'
      op=match(ASSIGN, fd);
      exp = expr(fd);

      if (exp==NULL || left==NULL) return NULL;

      add_child_node(op, left);
      add_child_node(op, exp);
      return op;

    //FOLLOW(Expr)
    case SEMICOLON: case RPAREN: case RBRACK: case COMMA: default:
      return left;
  }
}

//E1 -> E2E1'
static ast_node *E1(FILE *fd) {
  ast_node *e2;
  ast_node *E1_p;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      e2=E2(fd);
      E1_p=E1_prime(fd, e2);
      return E1_p;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Handles OR (||) operator

  @param left: becomes left child of operator, if matched
*/
static ast_node *E1_prime(FILE *fd, ast_node *left) {
  ast_node *op;
  ast_node *e2;
  ast_node *e1;

  switch(lookahead) {
    case OR:
      op=match(OR, fd);
      e2 = E2(fd);
      e1 = E1_prime(fd, e2);

      if (e2==NULL || e1 ==NULL || left==NULL) return NULL;

      add_child_node(op, left);
      add_child_node(op, e1);
      return op;

    case ASSIGN: case SEMICOLON: case RPAREN: 
    case RBRACK: case COMMA: default:
      return left;
  }
}

//E2 -> E3E2'
static ast_node *E2(FILE *fd) {
  ast_node *e3;
  ast_node *E2_p;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      e3=E3(fd);
      E2_p=E2_prime(fd, e3);
      return E2_p;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Handles AND (&&) operator

  @param left: becomes left child of operator, if matched
*/
static ast_node *E2_prime(FILE *fd, ast_node *left) {
  ast_node *op;
  ast_node *e3;
  ast_node *e2;

  switch(lookahead) {
    case AND:
      op=match(AND, fd);

      e3 = E3(fd);
      e2 = E2_prime(fd, e3);
      if (e3==NULL || e2 ==NULL || left==NULL) return NULL;
      
      add_child_node(op, left);
      add_child_node(op, e2);
      return op;

    case OR: case ASSIGN: case SEMICOLON: case RPAREN: 
    case RBRACK: case COMMA: default:
      return left;
  }
}

//E3 -> E4E3'
static ast_node *E3(FILE *fd) {
  ast_node *e4;
  ast_node *E3_p;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      e4=E4(fd);
      E3_p=E3_prime(fd, e4);
      return E3_p;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Parses EQ (==) and NEQ (!=) operators

  @param left: becomes left child of operator, if matched
*/
static ast_node *E3_prime(FILE *fd, ast_node *left) {
  ast_node *op;

  switch(lookahead) {
    case EQ:
      op=match(EQ, fd);
      return E3_prime_helper(fd, op, left);

    case NEQ:
      op=match(NEQ, fd);
      return E3_prime_helper(fd, op, left);

    case AND: case OR: case ASSIGN: case SEMICOLON: 
    case RPAREN: case RBRACK: case COMMA: default:
      return left;
  }
}

static ast_node *E3_prime_helper(FILE *fd, ast_node *op, ast_node *left) {
  ast_node *e4 = E4(fd);
  ast_node *e3 = E3_prime(fd, e4);
  if (e4==NULL || e3 ==NULL || left==NULL) return NULL;

  add_child_node(op, left);
  add_child_node(op, e3);
  return op;
}

//E4 -> E5E4'
static ast_node *E4(FILE *fd) {
  ast_node *e5;
  ast_node *E4_p;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      e5=E5(fd);
      E4_p=E4_prime(fd, e5);
      return E4_p;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Parses LESS (<), LEQ (<=), GREAT (>), and GEQ (>=)  operators

  @param left: becomes left child of operator, if matched
*/
static ast_node *E4_prime(FILE *fd, ast_node *left) {
  ast_node *op;

  switch(lookahead) {
    case LESS:
      op=match(LESS, fd);
      return E4_prime_helper(fd, op, left);
    case LEQ:
      op=match(LEQ, fd);
      return E4_prime_helper(fd, op, left);
    case GREAT:
      op=match(GREAT, fd);
      return E4_prime_helper(fd, op, left);
    case GEQ:
      op=match(GEQ, fd);
      return E4_prime_helper(fd, op, left);

    case EQ: case NEQ: case AND: case OR: case ASSIGN: case SEMICOLON:
    case RPAREN: case RBRACK: case COMMA: default:
      return left;
  } 
}

static ast_node *E4_prime_helper(FILE *fd, ast_node *op, ast_node *left) {
  ast_node *e5 = E5(fd);
  ast_node *e4 = E4_prime(fd, e5);
  if (e5==NULL || e4 ==NULL || left==NULL) return NULL;


  add_child_node(op, left);
  add_child_node(op, e4);
  return op;
}

//E5 -> E6E5'
static ast_node *E5(FILE *fd) {
  ast_node *e6;
  ast_node *E5_p;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      e6=E6(fd);
      E5_p=E5_prime(fd, e6);
      return E5_p;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Parses ADD (+) and SUB (-)  operators

  @param left: becomes left child of operator, if matched
*/
static ast_node *E5_prime(FILE *fd, ast_node *left) {
  ast_node *op;

  switch(lookahead) {
    case ADD:
      op = match(ADD, fd);
      return E5_prime_helper(fd, op, left);

    case SUB:
      op = match(SUB, fd);
      return E5_prime_helper(fd, op, left);
      

    case LESS: case LEQ: case GREAT: case GEQ:
    case EQ: case NEQ: case AND: case OR: case ASSIGN: 
    case SEMICOLON: case RPAREN: case RBRACK: case COMMA: default:
      return left;
  }
}

static ast_node *E5_prime_helper(FILE *fd, ast_node *op, ast_node *left) {
  ast_node *e6 = E6(fd);
  ast_node *e5 = E5_prime(fd, e6);
  if (e6==NULL || e5 ==NULL || left==NULL) return NULL;
  

  add_child_node(op, left);
  add_child_node(op, e5);
  return op;
}

//E6 -> E7E6'
static ast_node *E6(FILE *fd) {
  ast_node *e7;
  ast_node *E6_p;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case FLOAT: case ID: case LPAREN:
      e7 = E7(fd);
      E6_p = E6_prime(fd, e7);
      return E6_p;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Parses MULT (*) and DIV (/)  operators

  @param left: becomes left child of operator, if matched
*/
static ast_node *E6_prime(FILE *fd, ast_node *left) {
  ast_node *op;

  switch(lookahead) {
    case MULT:
      op = match(MULT, fd);
      return E6_prime_helper(fd, op, left);

    case DIV:
      op = match(DIV, fd);
      return E6_prime_helper(fd, op, left);
      
    case ADD: case SUB: case LESS: case LEQ: case GREAT: case GEQ:
    case EQ: case NEQ: case AND: case OR: case ASSIGN: 
    case SEMICOLON: case RPAREN: case RBRACK: case COMMA: default:
      return left;
  }
}

static ast_node *E6_prime_helper(FILE *fd, ast_node *op, ast_node *left) {
  ast_node *e7 = E7(fd);
  ast_node *e6 = E6_prime(fd, e7);
  if (e7==NULL || e6 ==NULL || left==NULL) return NULL;
  

  add_child_node(op, left);
  add_child_node(op, e6);
  return op;
}

/* 
  Parses unary NEG (!) and SUB (-)  operators. Also chugs along
  to base case!
*/
static ast_node *E7(FILE *fd) {
 ast_node *unary;
 ast_node *e7;

  switch(lookahead) {
    case NEG:
      unary=match(NEG, fd);
      e7 = E7(fd);
      if (e7==NULL) return NULL;
      add_child_node(unary, e7);
      return unary;

    case SUB: 
      unary=match(SUB, fd);
      e7=E7(fd);
      if (e7==NULL) return NULL;
      add_child_node(unary, e7);
      return unary;

    case NUM: case FLOAT: case ID: case LPAREN:
      return E8(fd);

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/*
  Expression base case! We can see either a number, an expression in
  parentheses, or an id (possibly followed by an expression list, for 
  function calls, or an expression for arrays)
*/
static ast_node *E8(FILE *fd) {
  ast_node *base_node;

  switch(lookahead) {
    case NUM: //For an integer
      base_node = match(NUM, fd);
      return base_node;

    case FLOAT: 
      base_node = match(FLOAT, fd);
      return base_node;

    case LPAREN: //For something like '(Expr)'
      match(LPAREN, fd);
      base_node = expr(fd);
      match(RPAREN, fd);
      return base_node;

    case ID: //For something like 'id(ExprList)' or 'id[Expr]'
      base_node = match(ID, fd);
      E8_prime(fd, base_node);
      return base_node;

    default:
      skip_ahead("Invalid expression. Line ignored.\n", fd);
      return NULL;
  }
}

/* 
  Handles the different things that can follow an ID in an expression

  @param parent: an ID node 
*/
static ast_node *E8_prime(FILE *fd, ast_node *parent) {
  ast_node *exp;
  ast_node *expList; //For a param. list in a function call

  switch(lookahead) {
    case LPAREN: //'(ExprList)' 
      match(LPAREN, fd);

      expList = create_ast_node(create_new_ast_node_info(NONTERMINAL, 0, 0, EXPR_LIST, 0, 0));
      exprlist(fd, expList); //Attaches expressions to EXPR_LIST node
      add_child_node(parent, expList);  //EXPR_LIST becomes child of the ID node

      match(RPAREN, fd);
      return expList;

    case LBRACK: //'[Expr]'
      add_child_node(parent,match(LBRACK, fd));

      exp = expr(fd);
      add_child_node(parent, exp); //Expr. becomes a child of the ID node

      add_child_node(parent, match(RBRACK, fd));
      return parent;

    case MULT: case DIV: case ADD: case SUB: case LESS: 
    case LEQ: case GREAT: case GEQ: case EQ: case NEQ:
    case AND: case OR: case ASSIGN: case SEMICOLON: 
    case RPAREN: case RBRACK: case COMMA: default:
      return parent;
  }
}

/*
  Parses a list of expressions, especially handy for function calls.

  @param parent: an ExprList Node
*/
static void exprlist(FILE *fd, ast_node *parent) {
  ast_node *exp;

  switch(lookahead) {
    case SUB: case NEG: case NUM: case LPAREN: case ID:
      exp=expr(fd);  
      add_child_node(parent, exp);   
      el_prime(fd, parent); //Check for more expressions
      break;

    case RPAREN: default:
      break;
  }
}

/*
  Checks if there are expressions left to parse, or if we've
  reached the end of the list.
  
  @param parent: an ExprList AST node
*/
static void el_prime(FILE *fd, ast_node *parent) {
  switch(lookahead) {
    case COMMA:
      match(COMMA, fd);
      exprlist(fd, parent);
      break;

    //Error recovery case - if we see these, assume user forgot a comma
    case SUB: case NEG: case NUM: case LPAREN: case ID:
      insert_missing_token(COMMA, fd);
      exprlist(fd, parent);
      break;

    case RPAREN: default:
      break;
  }
}
/**************************************************************************/