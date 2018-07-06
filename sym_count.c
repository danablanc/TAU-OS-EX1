/*
 * sym_count.c
 *
 *  Created on: Mar 31, 2018
 *      Author: gasha
 */
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 256

int instance_counter = 0;
char val= '\0';
int fd = -1;

void sigcont_handler(int signum) {
	printf("Process %d continues\n", getpid());
}

int sigcont_register() {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = sigcont_handler;
    return sigaction(SIGCONT, &new_action, NULL);
}

void sigterm_handler(int signum) {
	printf("Process %d finishes. Symbol %c. Instances %d.\n", getpid(), val, instance_counter);
	close(fd);
	exit(0);
}

int sigterm_register() {
    struct sigaction new_action_2;
    memset(&new_action_2, 0, sizeof(new_action_2));
    new_action_2.sa_handler = sigterm_handler;
    return sigaction(SIGTERM, &new_action_2, NULL);
}

int main(int argc, char** argv){
int i=0;
	if (argc < 3){ //assert there are 3 arguments
		printf("Error in number of given args!\n");
        return EXIT_FAILURE;
	}
	val = argv[2][0];
    if (sigterm_register() == -1) {
        printf("Signal handle registration failed\n");
        return EXIT_FAILURE;
    }
    if (sigcont_register() == -1) {
        printf("Signal handle registration failed\n");
        return EXIT_FAILURE;
    }
	fd = open(argv[1], O_RDWR);
	if( fd < 0 )
	{
		printf( "Error opening file: %s\n", strerror(errno) );
		return EXIT_FAILURE;
	}
	char buf[BUFSIZE+1];
	ssize_t len = read(fd, buf, BUFSIZE);
	buf[len+1] = '\0';
	while (len > 0){
		for (i=0;i<len;i++){
			if (buf[i] == val){
				instance_counter+=1;
				printf("Process %d, symbol %c, going to sleep\n", getpid(), val);
				int check_succ = raise(SIGSTOP);
				if (check_succ!=0){
					printf( "Error raising sigstop: %s\n", strerror(errno) );
					close(fd);
					return EXIT_FAILURE;
				}
			}
		}
		len = read(fd, buf, BUFSIZE);
		buf[len+1] = '\0';
	}
	int check_succ = raise(SIGTERM);
	if (check_succ!=0){
		printf( "Error raising sigterm: %s\n", strerror(errno));
		close(fd);
		return EXIT_FAILURE;
	}
}


