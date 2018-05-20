// copyit.c: a program called which simply copies a file from one place to another
// Aron Lam (alam3)
// Operating System Principles
// Instructor Thain
// Date: 1/27/17

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
// forgot to put this in, resulted in segfaults and loss of 40 points.
#include <string.h>


// General Variables
extern int errno;
char keepLooping = 1;

// Helper Function
void display_message(int s) { // a function to emit a short message every second if the copy takes longer than one second
	printf("copyit: still copying...\n");
}

// Main Execution
int main( int argc, char *argv[] ) {
	// Ensure correct usage
	if(argc != 3) {
		printf("copyit: Incorrect number of arguments!\n");
		printf("usage: copyit <sourcefile> <targetfile>\n");
		exit(1);
	}
	
	// Set up periodic message to catch SIGALRM
	signal(SIGALRM, display_message);
	alarm(1);
	
	// Open the source file
	const char *sourceFile = argv[1];
	int fd = open(sourceFile, O_RDONLY, 0);
	if(fd < 0) { // error check
		printf("Unable to open %s: %s\n", sourceFile, strerror(errno));
		exit(1);
	}
	
	// Create the target file
	const char *targetFile = argv[2];
	int fd1 = open(targetFile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(fd1 < 0) { // error check
		printf("Unable to create %s: %s\n", targetFile, strerror(errno));
		exit(1);
	}
	
	// Loop Variables
	int result; // for reading bytes
	int wesult; // for writing bytes
	int totalBytes = 0; // keep track of all bytes copied
	unsigned char buffer[11]; // temporary byte storage
	int count = 10; // how many bytes to read

	// Loop
	while(keepLooping) {
		// read a bit of data form the source file
		result = read(fd, buffer, count); // read bytes, returns integer amount for bytes read, 0 for EOF, - for error
		// check for interrupts & errors
		if(result < 0) { 
			// if the read was interrupted, try it again
			if(errno == EINTR) {
				result = read(fd, buffer, count);
			// if there was an error reading, exit an error
			} else {
				printf("Unable to read %s: %s\n", sourceFile, strerror(errno));
				exit(1);
			}
		// if no data left, end the loop
		} else if(result == 0) {
			break;
		}
	
		// write a bit of data to the target file
		wesult = write(fd1, buffer, result); // write bytes, returns - for error
		// update totalBytes with amount copied
		totalBytes+=result;
		// check for interrupts and errors
		if (wesult < 0) { 
			// if the write was interrupted, try it again
			if(errno == EINTR) {
				wesult = write(fd1, buffer, count);
			// if not all the data was written, exit with an error
			} else {
				printf("Unable to write to %s: %s\n", targetFile, strerror(errno));
				exit(1);
			}
		}
		
		//to check for printing of interrupt message: "copyit: still copying...\n"
		//sleep(1) 
		// set off alarm every second to cause interrupt
		alarm(1);
	}
	
	// Close both files
	if(close(fd) < 0) {
        printf("Unable to close %s: %s\n", sourceFile, strerror(errno));
        exit(1);
    }
    if(close(fd1) < 0) {
        printf("Unable to close %s: %s\n", targetFile, strerror(errno));
        exit(1);
    }

    // Print success message
    printf("copyit: Copied %d bytes from file %s to %s\n", totalBytes, sourceFile, targetFile);

    return 0;
}

