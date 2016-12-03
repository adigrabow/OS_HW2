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

#define BUFFER_SIZE 1024
#define FIFO_NAME "/tmp/osfifo"

int main(int argc, char* argv[]) {
	struct timeval t1, t2;
		double elapsed_microsec;

		int pipeOutFile; /*file descriptor*/
		int numOfBytesToSend = atoi(argv[1]);/*how many bytes to read from the pipe*/
		char buffer[numOfBytesToSend + 1];
		int numOfBytesWritten = 0;
		int returnVal = 0;
		int returnVal2 = 0;

		printf("num of bytes to send: %d\n",numOfBytesToSend);
		printf("creating message to be sent...\n");
		/*create the message to be sent*/

		for(int i = 0; i < numOfBytesToSend; i++) {
			buffer[i] = 'a';
		}

		buffer[numOfBytesToSend] = '\0';
		printf("message: \n");
		printf("%s\n",buffer);


		/*create a named pipe*/
		mkfifo(FIFO_NAME,0600);

		/*open the pipe file for writing*/
		pipeOutFile = open(FIFO_NAME, O_WRONLY);

		/*error handling*/
		if (pipeOutFile < 0) {
			printf("Error opening file: %s. %s\n",FIFO_NAME, strerror(errno));
			return errno;
		}

		/*get time before writing to pipe*/
		returnVal = gettimeofday(&t1, NULL);
		if (returnVal == -1) {
			printf("Could not get time of day. Exiting...\n");
			return errno;
		}


		numOfBytesWritten = write(pipeOutFile, buffer, numOfBytesToSend);
		if (numOfBytesWritten < 0) {
			printf("Could not write to pipe. Exiting...\n");
			return errno;
		}


		/*get time after writing to pipe*/
		returnVal2 = gettimeofday(&t2, NULL);
		if (returnVal2 == -1) {
			printf("Could not get time of day. Exiting...\n");
			return errno;
		}

		/*Counting time elapsed*/
		elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
		elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

		printf("%d were written in %f microseconds through FIFO\n", numOfBytesWritten ,elapsed_microsec);

		close(pipeOutFile);

		return 0;
}
