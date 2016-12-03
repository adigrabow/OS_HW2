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

#define BUFFER_SIZE 1024
#define FIFO_NAME "/tmp/osfifo"

int main(int argc, char* argv[]) {

	struct timeval t1, t2;
	double elapsed_microsec;

	int pipeInFile;/*file descriptor*/
	int buffer[BUFFER_SIZE]; /*how many bytes to read from the pipe*/
	int numOfBytesRead = 0;
	int totalNumOfBytesRead = 0;

	/*open the pipe file for reading*/
	pipeInFile = open(FIFO_NAME, O_RDONLY);

	/*error handling*/
	if (pipeInFile < 0) {
		printf("Error opening file: %s. %s\n",FIFO_NAME, strerror(errno));
		return errno;
	}

	/*get time before reading from pipe*/
	int returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		return errno;
	}

//TODO what if read didn't work?
	while ((numOfBytesRead = read(pipeInFile, buffer,BUFFER_SIZE)) > 0) {
		printf("%s ",buffer);
		totalNumOfBytesRead += numOfBytesRead;
	}

	/*get time after reading from pipe*/
	int returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		return errno;
	}

	  /*Counting time elapsed*/
	  elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	  elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	  printf("%d were read in %f microseconds through FIFO\n", totalNumOfBytesRead ,elapsed_microsec);

	  close(pipeInFile);

	  return 0;

}
