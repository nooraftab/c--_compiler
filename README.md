# c--_compiler

Hello! This is the README for Noor's C-- Compiler (also referred to as Code Generator Part 2 in this file bundle - a name carried over from my the format of my Compiler Design class).

## How do I run it?
To compile, cd into the codegen directory (`cd codegen`) and run `make`. <br/>
To then generate the MIPS file, run `./mycc insert_test_file_name.c-- mips_fileName.s` (if the .s file does not exist beforehand, it will be automatically generated!) <br/>
Finally, to run the MIPS file, make sure SPIM is installed and run "spim -file mipsFileName.s" <br/>

## How does this whole thing work?
#### Note: The bulk of the program is in the `codegen` directory! 
`main()` calls `parse()`, which creates the Abstract Syntaxt Tree (AST) of the test file. Main then calls `traverse_and_generate_code()` - lives in codetraversal.c - which kickstarts the entire process of traversing the tree and generating the corresponding MIPS code. 

As functions in `codetraversal.c` call each other during tree traversal, they also instruct `traversaltotable.c` on what code should be added to the Code Table on a high-level (in other words, it calls functions in `traversaltotable.c` and lets that handle the specifics). Once traversal is finished, main calls `output_code_table_to_file()` which writes the contents of the Code Table to the MIPS file.
##### Second note: a Code Table is a data structure that holds the list of instructions that go in the MIPS .s file.

## File Descriptions
#### `codegen `
• `codetraversal.c`: Contains functions that correspond to every AST node. Within these functions, they execute what should happen on seeing that node e.g. for a variable declaration, push it to the symbol table and make space for it on the stack according to its type.

• `traversalmechanics.c`: Contains functions that help `codetraversal.c` work its magic - essentially a file full of helpers, so codetraversal.c doesn't get cluttered.


• `traversaltotable.c`: Contains functions that `codetraversal.c` calls as it traverses, that handle exactly what gets put into the Code Table. codetraversal only needs to know what it does - the function names provide a high-level description of what they do (as one would expect from function names)

• `tablemechanics.c`: Serving a similar purpose to `traversalmechanics.c`, this file contains helpers that  traversaltotable.c   uses as it figures out the specifics of what gets entered into the Code Table. Arguably the nittiest-grittiest file of them all.
#### `symtab`

#### `ast`

#### `parser`

#### `lexer`
## Things to note!

• Compiler doesn't work if main() isn't the last declared function.

• There is 0 type checking.
	• Also, if you pass in wrong types to functions (especially arrays), the user might get a MIPS error (and weird outputs in general). I leave it to the programmer to re-check their work :)

• There isn't any support for nested conditionals - however, since we know how nested loops work, it is trivial to add this support. Consider it an improvement for v2!

• If you don't initialise array values, you'll get weird outputs. Currently, I figure the programmer knows what they're doing, so I assume arrays are initialised.

• We cannot `read` in with an array index - however, this is more due to the way the grammar is set up.

• The funcOffset variable in codetraversal arguably has the same purpose as stackCurrOffset, so it might be redundant. However, I was too afraid to touch it once I got things to work right - given more time, I would try to clean that up so there aren't so many globals floating around!

• There are no optimizations in this Compiler, mostly due to a lack of time (thanks to the pandemic messing with college schedules).

## Resources to orient yourself
[Grammar for C--](https://www.mtholyoke.edu/~vbarr/courses/COMSC-341CC/grammar.pdf)
