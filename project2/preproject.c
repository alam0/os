// Aron Lam
// Preproject.c for Project 2
// Instructor Thain
// 2/3/17

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


int main (int argc, char * argv[]) {
	// input check
	if(argc != 2) {
		printf("copyit: Incorrect number of arguments!\n");
		printf("usage: ./a.out <sourcefile> \n");
		exit(1);
	}
	
   // Fork
   const char *cFile = "CloneFile";
	int rc = fork();
	if(rc < 0) {  // fork failed, exit
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) { // child process
		//Copy the file via exec and cp (using argument 1)
		execl("/bin/cp","cp", argv[1], cFile, NULL);
	} else { // parent process
		//Wait for the child
		int wc = wait(NULL);
		
      //Exec the md5 sum command
      char *myargs[4];
      myargs[0] = strdup("md5sum"); 
      myargs[1] = strdup(argv[1]);
      myargs[2] = strdup(cFile);
      myargs[3] = NULL;
      execvp(myargs[0], myargs);
	}
	
   /* This never gets executed */
   return 0;
}
