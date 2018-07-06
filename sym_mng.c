/*
 * sym_mng.c
 *
 *  Created on: Mar 31, 2018
 *      Author: gasha
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

int checkPids(int* pids, int arr_size){
int i=0;
	for (i=0;i<arr_size;i++){
		if (pids[i]!=-1){
			return 1;
		}
	}
	return 0;
}

void noKids(int* pids, int arr_size){
int i=0;
	for (i=0;i<arr_size;i++){
		if (pids[i]!=-1){
			kill(pids[i],SIGKILL);
		}
	}
}

void freeArr(int * arr){
	if (arr!=NULL){
		free(arr);
	}
}

int main(int argc, char** argv){
	if (argc < 4){ //assert there are 3 arguments
		printf("Error in number of given args!\n");
        return EXIT_FAILURE;
	}
	int status;
	int pattern_len = strlen(argv[2]);
int i=0;
int k=0;
	for (i=0;i<strlen(argv[3]);i++){
		if (!(isdigit(argv[3][i]))){
			printf("Termination bound is not valid, should be a number!\n");
	        return EXIT_FAILURE;
		}
	}
	int term_bound = atoi(argv[3]);
	int* counter = malloc(sizeof(int)* pattern_len);
	if (counter == NULL){
		printf("Memory allocation error\n");
        return EXIT_FAILURE;
	}
	int* pids = malloc(sizeof(int)* pattern_len);
	if (pids == NULL){
		freeArr(pids);
		printf("Memory allocation error\n");
        return EXIT_FAILURE;
	}
	for (k=0;k<pattern_len;k++){ //init pids
		pids[k] = -1;
	}
	for (i=0;i<pattern_len; i++){
		pid_t pid = fork();
		if (pid == -1) {
			noKids(pids,pattern_len);
			freeArr(pids);
			freeArr(counter);
			printf("Error in forking! %s \n", strerror(errno));
			return EXIT_FAILURE;
		 }
		if (pid == 0){
			char index [] = {argv[2][i], '\0'};
		    char *exec_args[] = { "./sym_count", argv[1], index ,NULL };
			int check_error = execvp("./sym_count",exec_args);
			if (check_error == -1){
				noKids(pids, pattern_len);
				freeArr(pids);
				freeArr(counter);
				printf("Error in execvp! %s \n", strerror(errno));
				return EXIT_FAILURE;
			}
		}
		else{
			pids[i] = pid;
		}
	}
	sleep(1);
	int wait_success = 0;
	int sig_success = 0;
	while (checkPids(pids, pattern_len)){
		for (i=0;i<pattern_len;i++){
			wait_success = waitpid(pids[i], &status, WUNTRACED | WNOHANG);
			if (wait_success == -1) {
				noKids(pids, pattern_len);
				freeArr(pids);
				freeArr(counter);
				printf("waitpid failed! %s\n", strerror(errno));
		        return EXIT_FAILURE;
			}
			 if (WIFSTOPPED(status)) {
				 counter[i] = counter[i]+1;
				 if (counter[i]==term_bound){
					 sig_success = kill(pids[i],SIGCONT);
					 if (sig_success == -1){
						 printf("sigcont failed!%s\n", strerror(errno));
						 noKids(pids, pattern_len);
						 freeArr(pids);
						 freeArr(counter);
						 return EXIT_FAILURE;
					 }
					 sig_success = kill(pids[i],SIGTERM);
					 if (sig_success == -1){
							printf("sigterm failed!%s\n", strerror(errno));
							noKids(pids, pattern_len);
							freeArr(pids);
							freeArr(counter);
					        return EXIT_FAILURE;
					 }
					 pids[i]=-1;
				 }
				 else{
					 sig_success = kill(pids[i],SIGCONT);
					 if (sig_success == -1){
						 printf("sigcont failed!%s\n", strerror(errno));
						 noKids(pids, pattern_len);
						 freeArr(pids);
						 freeArr(counter);
						 return EXIT_FAILURE;
					 }
				 }
			 }
			 if (WIFEXITED(status)){
				 pids[i]=-1;

			 }

	}
		sleep(1);
	}
	freeArr(pids);
	freeArr(counter);
	return EXIT_SUCCESS;

}

