# Collatz

Linux program that uses fork & a simple pipe, applying the algorithm of the collatz conjecture.
The child process will run the algorithm, writing to the pipe along the way, and the parent process will print the values nicely.

## Usage

Compile the program by running `gcc -g project1.c -o project1`
And run it as so: `./project1 5`, changing 5 for whatever integer you'd like to run the algorithm on.
