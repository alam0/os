// mandelmovie.c 
// A program that runs mandel 50 times;
// Aron Lam
// 2/23/17

#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <math.h>


int main( int argc, char *argv[] ) {
	// Variables
	// keep -x and -y the same as in your chosen image from above
	float x = 0.286932;
	float y = 0.014287;
	// target scale
	float s = 0.000001;
	
	
	// Error checking
	if (argc != 2) {
		printf("Incorrect number of arguments\n");
		printf("Usage: ./mandelmovie <number of processes>\n");
	} else {
		int processes = atoi(argv[1]);
		// allow -s to vary from and initial value of 2 all the way down to target
		float iter = (2 - s) / 50;
		pid_t pids[processes];
		
		int i, j; // for iteration
		for (i = 0; i < 50; i += processes) {
			for (j = 0; j < processes; ++j) {
				if (i + j < 50) {
					
					if ((pids[j] = fork()) < 0) {
						printf("error: %s\n", strerror(errno));
						exit(EXIT_FAILURE);
					} else if (pids[j] == 0) {
						// run mandel
						char command[100];
						// end result 50 images named mandel1.bmp, mandel2.bmp ...
						sprintf(command, "./mandel -m 1000 -x %F -y %F -s %F -W 1366 -H 768 -o mandel%i.bmp", x, y, 2 - ((i + j)*iter), (i + j) + 1);
						system(command);
						exit(EXIT_SUCCESS);
					}
				}
			}
			pid_t pid;
			int status;
			
			int current = processes;
			while (current > 0) {
				pid = wait(&status);
				current--;
			}
			
		}
	}
	return 0;
}

// Manually allocate memory with `malloc`. If you are making an array of `types` with size `size`, then the amount of memory you need to reserve is `sizeof(types) * size`
