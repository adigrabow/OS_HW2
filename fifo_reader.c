/*
 * fifo_reader.c
 *
 *  Created on: Dec 2, 2016
 *      Author: adigrabow
 */
#include <sys/time.h> //for time measurements
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

#define BUFFER_SIZE 4096
#define FIFO_NAME "/tmp/osfifo"

int main(int argc, char* argv[]) {

	/* structs to ignore SIGINT */
	struct sigaction sigterm_old_action; /* the old handler of SIGINT*/
	struct sigaction sigign_action; /* this is the handler that ignores SIGTERM*/
	sigign_action.sa_handler = SIG_IGN; /*now we ignore the SIGINT signal*/

	/* Set the SIGINT handler to  ignore*/
	if (0 != sigaction (SIGINT, &sigign_action, &sigterm_old_action))
	{
		printf("Signal handle for SIGINT registration failed. %s\n",strerror(errno));
		return errno;
	}

	sleep(2);

	struct timeval t1, t2;
	double elapsed_microsec;

	int pipeInFile;/*file descriptor*/
	char buffer[BUFFER_SIZE]; /*how many bytes to read from the pipe*/
	int numOfBytesRead = 0;
	int totalNumOfBytesRead = 0;

	/*open the pipe file for reading*/
	pipeInFile = open(FIFO_NAME, O_RDONLY);
	/*error handling*/
	if (pipeInFile < 0) {
		printf("Error opening file: %s. %s\n",FIFO_NAME, strerror(errno));
		exit(errno);
	}

	/*get time before reading from pipe*/
	int returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(pipeInFile);
		exit(errno);
	}

	while((numOfBytesRead = read(pipeInFile, buffer,BUFFER_SIZE)) > 0) {
		for (int i = 0; i < numOfBytesRead; i++) {
			if (buffer[i] == 'a') {
				totalNumOfBytesRead++;
			}

		}
		
	}

	if (numOfBytesRead < 0) {
		printf("Error reading from file %s.", FIFO_NAME);
		close(pipeInFile);
		exit(errno);
	}

	/*get time after reading from pipe*/
	int returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(pipeInFile);
		exit(errno);
	}

	  /*Counting time elapsed*/
	  elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	  elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	  printf("%d were read in %f miliseconds through FIFO\n", totalNumOfBytesRead ,elapsed_microsec);

	  close(pipeInFile);

	
	exit(0);

}
