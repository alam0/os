#include <stdio.h>
#include <signal.h>

char bKeepLooping = 1;

void sigHandler(int sig) {
	bKeepLooping = 0;

}

int main( int argc, char *argv[] ) {
	
	signal(SIGINT, sigHandler);
		
    while(bKeepLooping) {
    	printf("Running\n");
    }
    
    printf("Exited successfully\n");
    return 0;
}

