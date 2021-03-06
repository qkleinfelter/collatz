/*
	File: project1.c - Implements the collatz conjecture process in C using a pipe & fork
	
	This file implements the collatz conjecture using a linux pipe & fork,
	as well as unsigned long longs for numbers to ensure a large range.
	Takes a single, positive argument when running the executable, which
	is then run through the process with all numbers being passed through a pipe
	and printed.

	Author: Quinn Kleinfelter
	Class: EECS 3540-001 Operating Systems & Systems Programming Spring 2021
	Instructor: Dr. Thomas
	Last Edited: 2/23/21
	Copyright: Copyright 2021 by Quinn Kleinfelter. All rights reserved.
*/

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
#define BUFFER_SIZE 25
#define READ_END 0 
#define WRITE_END 1

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Please only enter a single, positive integer. [NON_SINGLE_ARG]\n");
		return 1;
	}

	// Compare the first letter of the argument to a "0" (only if the length of the argument is 1), or "-"
	if ((strncmp("0", argv[1], 1) == 0 && strlen(argv[1]) == 1) || strncmp("-", argv[1], 1) == 0) {
		// We only care about positive numbers
		fprintf(stderr, "Please only enter a single positive integer. [ZERO_OR_NEG] \n");
		return 1;
	}

	errno = 0; // Clear errno just in case
	char* endPtr; // pointer for strtoull's second argument
	unsigned long long num = strtoull(argv[1], &endPtr, 10);

	if (num == 0 || *endPtr) {
		// At this point, num will only be 0 if strtoull() failed to parse, or the user entered multiple 0's
		// *endPtr handles other cases where some of the input was a number, but not everything was, i.e. 500AD
		fprintf(stderr, "Please only enter a single, positive integer. [FAILED_PARSE] \n");
		return 1;
	}

	if (num == ULLONG_MAX && errno == ERANGE) {
		// If we have ULLONG_MAX in num & errno shows ERANGE then that means
		// we overflowed when parsing, i.e. 18446744073709551616 or higher was the input
		fprintf(stderr, "Number input was too large [ARG_OVERFLOW]\n");
		return 1;
	}

	char write_msg[BUFFER_SIZE];
	char read_msg[BUFFER_SIZE];
	int fd[2];
	
	if (pipe(fd) == -1) {
		fprintf(stderr, "Pipe Failed\n");
		return 1;
	}

	pid_t pid;
	pid = fork();

	if (pid < 0) { 
		// Error trying to fork?
		fprintf(stderr, "Fork Failed\n");
		return 1;
	} else if (pid == 0) {
		// Here we're the child process
		close(fd[READ_END]); // and thus we won't be reading

		sprintf(write_msg, "%llu", num); // put our starting digit into write_msg, formatted as a long
		write(fd[WRITE_END], write_msg, strlen(write_msg) + 1); // write the first number to the pipe

		unsigned long long s = 0; // keeps track of number of steps we make
		unsigned long long h = num; // largest number, we can start with the first number

		while (num != 1) {
			// If num isn't 1, continuously loop either dividing by 2 or multiplying by 3 and adding 1, thus running the collatz conjecture
			if (num % 2 == 0)
				num /= 2;
			else 
				num = (num * 3) + 1;

			s++; // increment the number of steps
			if (num > h) // and check if this is our new largest number
				h = num;

			// Write the next number to the pipe
			sprintf(write_msg, "%llu", num);
			write(fd[WRITE_END], write_msg, strlen(write_msg) + 1);
		}

		// Now that we are out of the loop, we need to write s and h to the pipe
		// so they can be printed as well
		sprintf(write_msg, "%llu", s);
		write(fd[WRITE_END], write_msg, strlen(write_msg) + 1);

		sprintf(write_msg, "%llu", h);
		write(fd[WRITE_END], write_msg, strlen(write_msg) + 1);

		close(fd[WRITE_END]); // Close the pipe, which will send EOF to the read end
	} else {
		// Parent process
		close(fd[WRITE_END]); // thus we won't be writing here

		// read returns > 0 if we aren't at EOF, so this will loop until we close the pipe on the other end 
		while (read(fd[READ_END], read_msg, BUFFER_SIZE) > 0) {
			printf("%s", read_msg); // read in and print out the digit given to us

			if (strcmp(read_msg, "1") != 0) // if the number isn't 1, print a comma and a space
				printf(", ");
			else // otherwise, break out of the loop so we can handle printing s & h
				break;
		}
		// Now that we're out of the loop, print out s & h formatted correctly
		read(fd[READ_END], read_msg, BUFFER_SIZE);
		printf(" (%s, ", read_msg);

		read(fd[READ_END], read_msg, BUFFER_SIZE);
		printf("%s)\n", read_msg);

		close(fd[READ_END]); // close the read end of the pipe
		wait(NULL); // wait for the child process, just in case we have something weird happening
	}
	
	return 0;
}
