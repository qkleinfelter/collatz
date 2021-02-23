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

	errno = 0;
	char* endPtr;
	unsigned long long num = strtoull(argv[1], &endPtr, 10);

	if (num == 0 || *endPtr) {
		// At this point, num will only be 0 if strtoull() failed to parse, or the user entered multiple 0's
		// *endPtr handles other cases where some of the input was a number, but not everything was, i.e. 500AD
		fprintf(stderr, "Please only enter a single, positive integer. [FAILED_PARSE] \n");
		return 1;
	}

	if (num == ULLONG_MAX && errno == ERANGE) {
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
		close(fd[READ_END]); // won't be reading here 	
		sprintf(write_msg, "%llu", num); // put our starting digit into write_msg, formatted as a long
		write(fd[WRITE_END], write_msg, strlen(write_msg) + 1); // write the first num to the pipe
		while (num != 1) {
			// If num isn't 1, continuously loop either dividing by 2 or multiplying by 3 and adding 1, thus running the collatz conjecture
			if (num % 2 == 0)
				num /= 2;
			else 
				num = (num * 3) + 1;

			// Writing the next number to the pipe
			sprintf(write_msg, "%llu", num);
			write(fd[WRITE_END], write_msg, strlen(write_msg) + 1);
		} 
		close(fd[WRITE_END]); // Close the pipe, will send EOF to the read end
	} else {
		// Parent process
		close(fd[WRITE_END]); // won't be writing here
		unsigned long long s = -1; // keeps track of number of steps we make, start at -1 since the initial number comes through the pipe as well
		unsigned long long h = 0; // largest number, starting with is fine since all numbers in a collatz conjecture are >= 0

		// read returns > 0 if we aren't at EOF, so this will loop until we close the pipe on the other end 
		while (read(fd[READ_END], read_msg, BUFFER_SIZE) > 0) {
			unsigned long long read_num = strtoull(read_msg, NULL, 10);
			printf("%llu", read_num); // read in and print out the digit given to us
			if (read_num != 1) // if the number isn't 1, print a comma and a space
				printf(", ");
			else // otherwise, just a space
				printf(" ");
			s++;
			if (read_num > h) // check if our new number is larger than the largest
				h = read_num;
		}
		close(fd[READ_END]); // close the read end of the pipe
		wait(NULL);
		printf("(%llu, %llu)\n", s, h); // and output our final statistics
	}
	return 0;
}
