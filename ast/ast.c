#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "lexer.h"

////////////////////////////////////////////////////////////////////
/*
 * initialize an ast 
 *   tree: a reference to an ast struct to initialize 
 *   root_sym: a reference to the ast_node to which to init the root field
 *   returns: 0 on success, non-zero on failure
 */
int  init_ast(ast *tree, ast_node *root_sym) {

  if (tree == NULL || root_sym == NULL) {
        printf("ERROR: passing unallocated ast struct to init\n"); 
        return -1;
  }
  tree->root = root_sym;
  return 0;
}
////////////////////////////////////////////////////////////////////
/*
 * TODO: you will likely want to change the ast_info struct, so you
 *       will need to change this routine too
 *
 * creates and initializes a new ast_info struct
 * the caller is responsible for freeing the returned space
 *
 *  token: the token (or NONTERMINAL for AST not representing terminals) 
 *  value: its value (usually a symbol table entry number)
 *  grammar_sym: the grammar symbol for non-terminal ast nodes 
 *  lexeme: the lexeme (may be needed for ID tokens)   
 *  line_no: the source code line number
 *
 * returns: a pointer to a new ast_info struct initialized to
 *          passed values, or NULL on failure
 */
ast_info *create_new_ast_node_info(int token, int value, float float_val, int grammar_sym,
                                  char * lexeme, int line_no)
{
  ast_info * new_token;

  new_token = malloc(sizeof(ast_info));
  if(new_token) { 
    new_token->token = token;
    new_token->grammar_symbol = grammar_sym;

    if (token==FLOAT) {
      new_token->float_val = float_val;
    } else if (token == NUM) {
      new_token->value = value;
    }
    
    if(lexeme != 0) {
      strncpy(new_token->lexeme, lexeme, MAX_LEXEME_SIZE);
    } else {
      new_token->lexeme[0] = '\0';
    }
    new_token->line_no = line_no;
  }
  return new_token;
}
////////////////////////////////////////////////////////////////////
/*
 * add a child node to the current ast_node
 * child: pointer to the ast_node to add
 * returns: 0 on success, non-zero on error
 */
int add_child_node(ast_node *parent, ast_node *child) {

  int n;
  if (parent == NULL || child == NULL) {
        printf("ERROR: passing unallocated parent of child to add_node\n"); 
        return -1;
  }
  if(parent->num_children >= parent->max_children) {
        parent->max_children += AST_CHILDREN;
        if(parent->max_children == AST_CHILDREN) { // childlist list was NULL
          parent->childlist = malloc(
              sizeof(ast_node *)*parent->max_children);
        }else {
          parent->childlist = realloc(parent->childlist,
              sizeof(ast_node *)*parent->max_children);
        }
  }
  if(parent->childlist == NULL) { 
    printf("ERROR: malloc failed\n"); 
    return -1;
  }
  n = parent->num_children;
  parent->childlist[n] = child;
  parent->num_children += 1;
  return 0;

}
////////////////////////////////////////////////////////////////////
/*
 * create a new ast_node
 * token: pointer to the ast_info struct to add as this node
 * returns: pointer to new ast_node with token as symbol field
 *          or NULL on error
 */
ast_node *create_ast_node(ast_info *token) {

  ast_node *new_node;
  if(token == NULL) { printf("Error token NULL\n"); return NULL; }
  new_node = malloc(sizeof(ast_node));
  if(new_node == NULL) { printf("Malloc failed\n"); return NULL; }
  new_node->symbol = token;
  new_node->max_children = 0;
  new_node->num_children = 0;
  new_node->childlist = NULL;
  return new_node;
}

////////////////////////////////////////////////////////////////////
/*
 * probably not necessary, but what the heck.
 * parent: a reference to an ast_node
 * returns: a reference to the node's child list
 */
ast_node **get_childlist(ast_node *parent) {
  if (parent == NULL) {
        printf("ERROR: passing unallocated parent to get_childlist\n"); 
        return NULL;
  }
  return parent->childlist;
}
////////////////////////////////////////////////////////////////////
/*
 * probably not necessary, but what  the heck.
 * parent: a reference to an ast_node
 * returns: the number of children or -1 on error
 */
int get_num_children(ast_node *parent) {
  if (parent == NULL) {
        printf("ERROR: passing unallocated parent to get_num_children\n"); 
        return -1;
  }
  return parent->num_children;
}
////////////////////////////////////////////////////////////////////
//compute the height of the ast
static int compute_height(ast_node *p, int h) {

  int max, ch, i;

  if (p->num_children == 0) {
    return h;
  }
  else {
    max = h;
    for (i=0; i < p->num_children; i++) {
        ch = compute_height((p->childlist[i]), h+1);
        if(ch > max) { max = ch; }
    }
    return max; 
  }
}

static char *indent_str = "       ";
// helper function to print_ast
// this does all the work of a recursive reverse order traversal to
// print out the tree
//  node: the current root
//  height: the height of the ast tree
//  depth: the depth of this node
//  print_func: function to call to print out the ast_info
static void print_ast_rec(ast_node *node, int height, 
    int depth, void (*print_func)(ast_info *t)) 
{

  int i;
  for(i = node->num_children-1; i >= 0; i--) { 
    print_ast_rec((node->childlist[i]), height, depth+1, print_func);
  }
  for (i=0; i < depth; i++) {
    printf(" %s", indent_str);
  }
  print_func(node->symbol); 
  printf("\n");

  // comment out this part if you don't want the /'s printed
  // between nodes
  if(depth > 0 ) {
    for (i=0; i < depth-1; i++) {
      printf(" %s", indent_str);
    }
    printf("%s/\n", indent_str);
  }

}
/*
 * prints out the ast tree, sideways 
 *  node: the current root
 *  height: the height of the ast tree
 *  depth: the depth of this node
 *  print_func: function to call to print out the ast_info
 */
void print_ast(ast tree, void (*print_func)(ast_info *t)) {      
        int n;
        n = compute_height(tree.root, 0);
        print_ast_rec(tree.root, n, 0, print_func);
}
////////////////////////////////////////////////////////////////////
// helper function to create_nltk
//
static void create_ntl_format(FILE *outfile, ast_node *node, 
    void (*print_func)(FILE *outfile, ast_info *t)) 
{

  int i;
  if(node->num_children) {
    fprintf(outfile, "(");
  }
  print_func(outfile, node->symbol); 
  if(node->num_children) {
    for(i = 0; i < node->num_children; i++ ){ 
      fprintf(outfile, "(");
      create_ntl_format(outfile, (node->childlist[i]), print_func);
      fprintf(outfile, ")");
    }
  }
  if(node->num_children) {
    fprintf(outfile, ")");
  }
  fprintf(outfile, "\\\n");

}
/*
 * outputs a version of the AST that can be viewed using the nltk 
 * package in python
 *
 * change your main program to call this with the ast returned by
 * the parser, and with a file pointer to an output file for the nltk
 * output (i.e. you have opened the file for writing in main already)
 *
 * python -i outputfile
 * >>> ast.draw()
 *
 *   outfile: file to which nltk output will be written 
 *   tree: the ast tree to print
 *   print_func: a pointer to a function that takes a pointer to an
 *      ast_info struct and whose type is void.  The passed function 
 *      prints out an ast_info struct (the passed print function is 
 *      particular to the definition and use of the ast_info struct)
 *
 */
void create_nltk(FILE *outfile, ast tree, 
    void (*print_func)(FILE *out, ast_info *t)) 
{

        fprintf(outfile, "from nltk import Tree\n\n");
        fprintf(outfile, "ast = Tree.parse(\"");
        create_ntl_format(outfile, tree.root, print_func);
        fprintf(outfile, "\")");
}

////////////////////////////////////////////////
//helper function for destroy_ast
static void destroy_ast_rec(ast_node *node) {

  int i;

  if(node == NULL) { return; } 
  for(i = node->num_children-1; i >= 0; i--) { 
    destroy_ast_rec(node->childlist[i]);
    free(node->childlist[i]);
  }
  if(node->symbol != NULL) { free(node->symbol); }
  if(node->childlist != NULL) { free(node->childlist); }
}
/*
 * "destructor" for an ast tree: deletes all malloc fields
 *              but does not free the space pointed to by tree
 *              (the assumption is that this is a statically
 *              declared struct passed by reference)
 *   tree: a reference to a ast 
 */
void  destroy_ast(ast *tree) {
  int i;

  for(i = tree->root->num_children-1; i >= 0; i--) { 
        destroy_ast_rec(tree->root->childlist[i]);
        free(tree->root->childlist[i]);
  }
  if(tree->root->symbol != NULL) { free(tree->root->symbol); }
  if(tree->root->childlist != NULL) { free(tree->root->childlist); }
  if(tree->root != NULL) { free(tree->root); }
}

