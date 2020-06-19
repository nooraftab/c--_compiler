/** Error message for lexical analyzer **/
#include <stdlib.h>
#include "lexer.h"


void lexer_error(char *m, int lineno)  {
  fprintf(stderr, "Line %d: %s\n", lineno, m);
  exit(1);   /*   unsuccessful termination  */
}
