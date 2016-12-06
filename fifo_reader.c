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


#define BUFFER_SIZE 2048
#define FIFO_NAME "/tmp/osfifo"

int main(int argc, char* argv[]) {

	sleep(2);

	struct timeval t1, t2;
	double elapsed_microsec;

	int pipeInFile;/*file descriptor*/
	int buffer[BUFFER_SIZE]; /*how many bytes to read from the pipe*/
	int numOfBytesRead = 0;
	int totalNumOfBytesRead = 0;
	int numOfIterations = 0;
//	int iterationNum = 0;
	struct stat fileStat;
	int fileSize = 0;
//	int index = 0;

	/*open the pipe file for reading*/
	pipeInFile = open(FIFO_NAME, O_RDONLY);

	/*error handling*/
	if (pipeInFile < 0) {
		printf("Error opening file: %s. %s\n",FIFO_NAME, strerror(errno));
		exit(errno);
	}

	/* Determine the file size (man 2 stat)*/
	if (stat(FIFO_NAME,&fileStat) < 0) {
		printf("Could not receive %s stats. Exiting...\n", FIFO_NAME);
		close(pipeInFile);
		exit(errno);
	}

	fileSize = fileStat.st_size; /* in bytes */

	numOfIterations = (fileSize /BUFFER_SIZE ) + 1;
	/*get time before reading from pipe*/
	int returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(pipeInFile);
		exit(errno);
	}

	printf("fileSize = %d\n", fileSize);
	printf("numOfIterations=%d\n",numOfIterations);

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

	/*
	while ( iterationNum < numOfIterations) {
		index = 0;
		numOfBytesRead = read(pipeInFile, buffer,BUFFER_SIZE);
		if (numOfBytesRead < 0) {

		}
		while (buffer[index] != '\0') {
			if (buffer[index] == 'a') {
				totalNumOfBytesRead++;
			}
			index++;
		}
		iterationNum++;
	}*/


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

	  printf("%d were read in %f microseconds through FIFO\n", totalNumOfBytesRead + 1 ,elapsed_microsec);

	  close(pipeInFile);

	 exit(0);

}
