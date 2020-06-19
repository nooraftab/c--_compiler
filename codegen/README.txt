Hello! This is the README for Noor's Final C-- Compiler (also referred to as Code Generator Part 2 in this file bundle - a name carried over from my the format of my Compiler Design class).

To compile, cd into the codegen directory and run make. 
To generate the MIPS file, use "./mycc fileName.c-- mipsFileName.s" (if the .s file does not exist beforehand, it will be automatically generated!)
Finally, to run the MIPS file, make sure SPIM is installed and run "spim -file mipsFileName.s"

Do look at ProjectReport.txt for more details about the files and program.

I made three of my own test files in the test_suites directory - they should be sitting there, not in a folder. All the other test files (from starter code) are under "Normal Tests" and tests from CodeGen Part 1 (the version of this Compiler without features for functions/variables) are under "CG1 Tests".

As for what's inside the test files:

	• CG2_Test1: Since I did not feel very creative at the time, I was browsing through LeetCode and came across a program that looked like a good test case - it determines if two arrays of rectangle vertices overlapped (https://leetcode.com/problems/rectangle-overlap/). It has two cases hardcoded in and the expected output is:
1
0
This file is supposed to demonstrate that array parameters/assignments work, and that I fixed up my negation operator from CodeGen Part 1 (which was bitwise, and not logical)!

	• CG2_Test2: This file prints out the first 12 fibonacci numbers - anything larger and printing gets delayed (which is expected of recursion). It prints out:
0
1
1
2
3
5
8
13
21
34
55
89
It is meant to demonstrate that (double) recursion works, and global arrays can be used no problem! Also, loops work.

	•CG2_Test3: This file does 3 things - it takes in 2 numbers from user input, and prints out a really cool square (you'll see) from those 2 numbers (the first is length, the second is width). After that, it'll also print out the square of the first number that the user inputted. Say I put in 3 and 4 - I'd get:
3
4

100
010
001
000

9
This file is supposed to demonstrate that more complicated programs also work - we have nested loops, array parameters being assigned values, nested blocks in loops, a return within a conditional within a loop, and local arrays.
