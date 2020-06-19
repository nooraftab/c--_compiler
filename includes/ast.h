// ast library: routines to create an ast tree
// (Thanks to Tia Newhall)
//  
//  ast_info struct: may need to be changed for your particular use
//              as well as the create_new_ast_node_info function
//              (see the TODO's in this file)
//
// To use the library:
// -------------------
//
// A. create and init an ast:
// -------------------------
//    (1) declare an ast struct:
//          // you likely want to use a global as this will persist
//          // between the parsing and the code generation phase
//          ast my_ast;
//
//    (2) create a new ast_node, n, for the root:
//     
//         ast_info *s;
//         ast_node *n;
//         // the reason why create_new_ast_node_info is a separate
//         // function (not just called inside create_ast_node) is
//         // because you may want to change it for your compiler
//         s = create_new_ast_node_info(NONTERMINAL, 0, ROOT, 0, 0);
//         n = create_ast_node(s);
//
//    (3) call init_ast to initialize the ast with the root ast_node n:
//
//        init_ast(&my_ast, n);
//
// B. use the ast:
// ---------------
//      (1) add new child nodes:
//           s = create_new_ast_node_info(ID, 0, 0, 0, "x", 0);
//           n = create_ast_node(s);
//           add_child_node(my_ast.root, n);
//
//           or add a child to a child... 
//           ----------------------------
//           add_child_node(my_ast.root->childlist[i], n);
//
//           usually, you will have a current node in the ast, and
//           you will add a child to it (you don't normally add from 
//           the root down following ->links)
//           -----------------------------------------------------------------
//           ast_node *curr_node;
//           ...
//           s = create_new_ast_node_info(EQ, 0, 0, 0, 0, 0);
//           n = create_ast_node(s);
//           add_child_node(curr_node, n);
//
//
//       (2) print it out:
//
//           print_ast(my_ast, my_token_printing_function);
//
// C. call destroy_ast when you are done using it:
// ------------------------------------------------
//        destroy_ast(&my_ast);
//
//
#ifndef __AST__H__
#define __AST__H__

#define AST_CHILDREN 2
#define MAXTOKEN_LEN 30
#define MAXTOKENVAL_LEN 30
#define MAXSYM_LEN 30

// Make sure this size matches the max lexeme size used by the lexer.
// Really we should dynamically alloc space, but since we use max
// size in lexer, we are just using it here too
#define MAX_LEXEME_SIZE   128   

// TODO: you may need to change this struct for your parser
//       (add more fields, change the type of fields, remove fields...)
//
struct ast_info {
  int token;     // which token or NONTERMINAL if AST node is not a terminal
  int value;    // token's value: symbol table index?  integer value? 
  float float_val;
  char lexeme[MAX_LEXEME_SIZE];  // for ID tokens likely need to keep this  
  int grammar_symbol;  // some ast nodes may correspond to nonterminals
  int line_no;    // the source code line number associated with this token
};
typedef struct ast_info ast_info;

struct ast_node {
  ast_info *symbol;  // usually the terminal associated with this node
  int num_children;    
  int max_children;
  struct ast_node **childlist;  // an array of pointers to child nodes
};
typedef struct ast_node ast_node;

// the ast (right now just root node)
struct ast {
  struct ast_node *root;
};
typedef struct ast ast;

/*
 * initialize a ast 
 *   tree: a reference to a ast struct to initialize 
 *   root_sym: an ast_node struct for the root AST node
 *   returns: 0 on success, non-zero on failure
 */
int  init_ast(ast *tree, ast_node *root_sym);

/*
 * an ast "destructor" frees all malloced space 
 * referred to by the ast's root field
 * (note:  does not free tree (the assumption is that this
 *  may be a statically declared struct that is passed
 *  by reference here).
 */
void  destroy_ast(ast *tree);

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
ast_info *create_new_ast_node_info(int token, int value, float float_val, 
                                  int grammar_sym, char * lexeme, int line_no);

/*
 * create a new ast_node
 * token: pointer to the ast_info struct to add as this node
 * returns: pointer to new ast_node with token as symbol field
 *          or NULL on error
 */
ast_node *create_ast_node(ast_info *token) ;

/*
 * add a new child node to the current ast_node
 *   child: pointer to the ast_node to add
 *   parent: pointer to parent ast_node into which to insert this node 
 *   returns: 0 on success, non-zero on error
 */
int add_child_node(ast_node *parent, ast_node *child) ;

/*
 * probably not necessary, but what the heck.
 * parent: a reference to a ast_node
 * returns: a reference to the node's child list
 */
ast_node **get_childlist(ast_node *parent) ;

/*
 * probably not necessary, but what the heck.
 * parent: a reference to a ast_node
 * returns: the number of children or -1 on error
 */
int get_num_children(ast_node *parent) ;

/*
 * prints out the ast tree, sideways, root last
 *
 *   tree: the ast tree to print
 *   
 *   print_func: a pointer to a function that takes a pointer to an
 *      ast_info struct and whose type is void.  The passed function 
 *      prints out an ast_info struct (the passed print function is 
 *      particular to the definition and use of the ast_info struct)
 */
void print_ast(ast tree, void (*print_func)(ast_info *t));                  

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
 *   outfile: file to which nltk output will written 
 *   tree: the ast tree to print
 *   print_func: a pointer to a function that takes a FILE pointer to
 *      and output file, and a pointer to an ast_info struct and whose 
 *      type is void.  The passed function prints out an ast_info struct 
 *      (the passed print function is particular to the definition and use 
 *      of the ast_info struct) to the file.
 *
 */
void create_nltk(FILE *outfile, ast tree,
    void (*print_func)(FILE *outfile, ast_info *t));


#endif
