 /*
 *  Main function for C-- compiler: now launches parser and prints AST
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

void print_my_ast_node(ast_info *t);
void print_nltk_ast_node(FILE *out, ast_info *t); 

int main(int argc, char *argv[]) {

  FILE *fd = 0;

  if(argc != 2 && argc != 3) { 
    printf("usage: parser filename.c-- <nltk_outfile>\n");
    exit(1);
  }

  if(!(fd = fopen(argv[1], "rw")) ) {
    perror("no such file\n");
    exit(1);
  }

  // parser_init();  // if you need to init any global parser state
  
  parse(fd);
  printf("**********************************************\n");
  print_ast(ast_tree, print_my_ast_node);
  fclose(fd);

  // print AST in nltk format to output file
  if(argc == 3) {
    if(!(fd = fopen(argv[2], "w")) ) {
      perror("no such file:\n");
    } else {
      create_nltk(fd, ast_tree, print_nltk_ast_node);
    }
    fclose(fd);
  }

  destroy_ast(&ast_tree);
  exit(0);     /*  successful termination  */
  
}
/*********************************************************************/

// this is an example of how to define output strings corresponding to 
// different ast node state that is used by the print_ast_node function:
static char *t_strings[] = {"id", "num", "float", "read", "if", "else",
                            "int", "char", "break", "write", "writeln",
                            "while", "return", "[", "]", "==", "=", "!=", "!", "-",
                            "+", "*", "/", "<", "<=", ">", ">=", "&&", "||", "DONE"
                            };

static char *non_term_strings[] = {"PROGRAM", "PROG1", "PROG2", "FDL1", "FUNC_DECL",
                                  "VAR_DECL", "VDL1", "PDL", "PDL1", "PDL2",
                                  "BLOCK", "StmtList", "StmtList1", "Stmt",
                                  "Expr", "E0", "E0'", "E1", "E1'", "E2", "E2'",
                                  "E3", "E3'", "E4", "E4'", "E5", "E5'", 
                                  "E6", "E6'", "E7", "E8", "E8'", "ExprList", "EL'"};


// This is the function that is passed to print_ast, to print information
// that is stored in an ast node
//
// TODO: you will need to add more functionality than is currently here
//       and you may need to change what is here to match with the way you
//       defined tokens in your lexer implementation.
//
void print_my_ast_node(ast_info *t) {

  if(t != NULL) {
    if((t->token > STARTTOKEN) && (t->token <ENDTOKEN)) {

      if (t->token == ID) {
        printf("%s:%s", t_strings[(t->token - STARTTOKEN-1)], t->lexeme);
      } else if (t->token == NUM ) {
        printf("%s:%d", t_strings[(t->token - STARTTOKEN-1)], t->value);
      } else if (t->token == FLOAT ) {
        printf("%s:%f", t_strings[(t->token - STARTTOKEN-1)], t->float_val);
      } else {  
        printf("%s", t_strings[(t->token - STARTTOKEN-1)]);  
      }
    }
    else if ((t->token == NONTERMINAL)) {
       if((t->grammar_symbol >= START_AST_SYM) 
           && (t->grammar_symbol <= END_AST_SYM)) 
       {
           printf("%s", non_term_strings[(t->grammar_symbol - START_AST_SYM)]);
       }
       else {
           printf("unknown grammar symbol %d", t->grammar_symbol);
       }
    }
    else {
      printf("unknown token %d", t->token);
    }
  }
  else {
    printf("NULL token\n");
  }
}
/*********************************************************************/
//
// This is the function that is passed to create_nltk, that prints out
// the AST in nltk format to a file
// (it will likely be identical to print_my_ast_node except that it
//  calls fprint to a file vs. printf to stdout)
// TODO: you will need to add more functionality than is currently here
//       and you may need to change what is here to match with the way you
//       defined tokens in your lexer implementation.
//
void print_nltk_ast_node(FILE *out, ast_info *t) {


  if(t != NULL) {
    if((t->token >= STARTTOKEN) && (t->token <= ENDTOKEN)) {

      fprintf(out,"%s", t_strings[(t->token - STARTTOKEN)]);

    }
    else if ((t->token == NONTERMINAL)) {
       if((t->grammar_symbol >= START_AST_SYM) 
           && (t->grammar_symbol <= END_AST_SYM)) 
       {
           fprintf(out,"%s", 
                non_term_strings[(t->grammar_symbol - START_AST_SYM)]);
       }
       else {
           fprintf(out,"unknown grammar symbol %d", t->grammar_symbol);
       }
    }
    else {
      fprintf(out,"unknown token %d", t->token);
    }
  }
  else {
    fprintf(out,"NULL token\n");
  }
}
