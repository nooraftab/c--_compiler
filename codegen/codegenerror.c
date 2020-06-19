#include <stdio.h>
#include "traversaltotable.h"
#include "traversalmechanics.h"
#include "parser.h"
#include "lexer.h"
#include "symtab.h"
#include "ast.h"
#include "parser.h"

//Generates error message, frees heap, closes files, and exits!
void codegen_error(char *err_message, FILE *in, FILE *out) {
	printf("~~~ Code Generation Error! ~~~\n");
	printf("%s\n", err_message);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

  destroy_code_table();
  destroy_ast(&ast_tree);
  destroy_symtab_stack();
  destroy_offset_stack();

  fclose(in);
  fclose(out);
  exit(1);
}