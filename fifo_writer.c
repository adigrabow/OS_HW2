/*
 * fifo_writer.c
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
#define PERMISSION_CODE 0600

void sigpipe_handler(int sig);

/* GLOBAL VARIABLES */

int totalNumberOFBytesWritten;
struct timeval t1, t2;
double elapsed_microsec;
int pipeOutFile; /*file descriptor*/

void sigpipe_handler(int sig) {
	//printf("inside sigpipe_handler\n");
	struct sigaction sigterm_old_action; /* the old handler of SIGINT*/
	/*get time after writing to pipe*/
	double elapsed_microsec;
	int returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(pipeOutFile);
		unlink(FIFO_NAME);
		exit(errno);
	}

	/*Counting time elapsed*/
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were written in %f miliseconds through FIFO\n", totalNumberOFBytesWritten ,elapsed_microsec);
	
	close(pipeOutFile);
	/* Stop Ignoring SIGINT*/
	if (0 != sigaction (SIGINT, &sigterm_old_action, NULL))
	{
		printf("Could not register the default SIGTERM handler. %s\n",strerror(errno));
		exit(errno);
	}
	if (unlink(FIFO_NAME) < 0 ) {
		printf("Error while trying to use unlink syscall. Exiting...");
		exit(errno);
	}
	exit(0);

}

int main(int argc, char* argv[]) {


	if (argc < 2) {
		printf("Expected 1 arguments: number_of_bytes_to_write.\n");
		return -1;
	}
	//printf("entered writer\n");
	/* making sure the fifo does not exists (if so - delete it!)*/
	//unlink(FIFO_NAME);


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

	/* structs to handle SIGPIPE */
	struct sigaction new_action;
	new_action.sa_handler = sigpipe_handler;
	new_action.sa_flags = 0;

	/* Set a handler for SIGPIPE*/
	if (0 != sigaction (SIGPIPE, &new_action, NULL))
	{
		printf("Signal handle for SIGPIPE registration failed. %s\n",strerror(errno));
		return errno;
	}

	double elapsed_microsec;/* for time measurements*/

	
	int numOfBytesToSend = atoi(argv[1]);/*how many bytes to read from the pipe*/
	int numberOfIteration = numOfBytesToSend / BUFFER_SIZE;
	int numberOfBytesLeft = numOfBytesToSend - (numberOfIteration * BUFFER_SIZE); 
	char buffer[BUFFER_SIZE]; 
	int numOfBytesWritten = 0;
	int returnVal = 0;
	int returnVal2 = 0;
	totalNumberOFBytesWritten = 0;

	pipeOutFile = open(FIFO_NAME, O_WRONLY | O_APPEND);
	
	if (errno == ENOENT) { /* no such file, so mkfifo! */
		/*create a named pipe*/
		//printf("errno = ENOENT, regular mkfifo!\n");
		if (mkfifo(FIFO_NAME,PERMISSION_CODE) < 0 ) {
			printf("Error while trying to use mkfifo for %s. %s",FIFO_NAME, strerror(errno));
			exit(errno);
		}

		/*open the pipe file for writing*/
		pipeOutFile = open(FIFO_NAME, O_WRONLY);
		/*error handling*/
		if (pipeOutFile < 0) {
			printf("Error opening file: %s. %s\n",FIFO_NAME, strerror(errno));
			unlink(FIFO_NAME);
			exit(errno);
		}

	}


	else if ( pipeOutFile > 0) { /* fifo file already exists*/
		//printf("fifo already exists, giving new permissions.\n");
		/*give it right permissions*/
		if (chmod(FIFO_NAME, PERMISSION_CODE) < 0) {
			printf("chmod Syscall faild. Exiting...\n");
			close(pipeOutFile);
			unlink(FIFO_NAME);
			exit(errno);

		}

	/* something went wrong while trying to open the file! */ 
	}else {
		printf("Error opening file: %s. %s\n",FIFO_NAME, strerror(errno));
		unlink(FIFO_NAME);
		exit(errno);

	}


	/*get time before writing to pipe*/
	returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(pipeOutFile);
		unlink(FIFO_NAME);
		exit(errno);
	}

	for (int i = 0; i < numberOfIteration; i++) {
		for (int j = 0; j < BUFFER_SIZE; j++) {
			buffer[j] = 'a';
		}

		numOfBytesWritten = write(pipeOutFile, buffer, BUFFER_SIZE);
		if (numOfBytesWritten < 0) {
			printf("Could not write to pipe. Exiting...\n");
			close(pipeOutFile);
			exit(errno);
		}
		totalNumberOFBytesWritten += numOfBytesWritten;
		//printf("totalNumberOFBytesWritten=%d\n", totalNumberOFBytesWritten);

	}

	for (int i = 0; i < numberOfBytesLeft - 1; i++) {
		buffer[i] = 'a';
	}
	buffer[numberOfBytesLeft - 1] = '\0';
	numOfBytesWritten = write(pipeOutFile, buffer, numberOfBytesLeft);
		if (numOfBytesWritten < 0) {
			printf("Could not write to pipe. Exiting...\n");
			close(pipeOutFile);
			exit(errno);
		}

	totalNumberOFBytesWritten += numOfBytesWritten;
	/*get time after writing to pipe*/
	returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(pipeOutFile);
		unlink(FIFO_NAME);
		exit(errno);
	}

	/*Counting time elapsed*/
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were written in %f miliseconds through FIFO\n", numOfBytesToSend ,elapsed_microsec);
	close(pipeOutFile);
	//printf("exiting writer2\n");

	/* Stop Ignoring SIGINT*/
	if (0 != sigaction (SIGINT, &sigterm_old_action, NULL))
	{
		printf("Could not register the default SIGTERM handler. %s\n",strerror(errno));
		return errno;
	}
	if (unlink(FIFO_NAME) < 0 ) {
		printf("Error while trying to use unlink syscall. Exiting...");
		exit(errno);
	}
	
	exit(0);
}
