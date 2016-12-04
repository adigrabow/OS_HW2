/*
 * mmap_writer.c
 *
 *  Created on: Dec 3, 2016
 *      Author: adigrabow
 */
#include <sys/time.h> //for time measurements
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>



#define MAPPED_FILE_NAME "/tmp/mmapped.bin"
#define BUFFER_SIZE 4096


int main (int argc, char* argv[]) {

	if (argc < 3) {
		printf("Expected 2 arguments: number_of_bytes_to_write and reader_process_ID.\n");
		return -1;
	}

	/* Ignore SIGTERM */
	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
		printf("Could not ignore SIGTERM signal as requested. Exiting...\n");
		return -1;
	}

	struct timeval t1, t2;
	double elapsed_microsec;
	int returnVal = 0;
	int returnVal2 = 0;

	int numOfBytesToWrite = atoi(argv[1]);
	int readerProcID = atoi(argv[2]);
	int fileDesc = 0;
	char* data; /* a pointer to the place in the mapped file we want to write to */
	int lseekRes = 0;
	int munmapRes = 0;
	int writeRes = 0;
	int numberOFIterations = (numOfBytesToWrite / BUFFER_SIZE) + 1;
	int charCount = 0; /*the number of characters already written*/
	int index = 0;

	printf("numberOFIterations = %d\n", numberOFIterations);

	/* open/create the file to be mapped to memory */
	fileDesc = open(MAPPED_FILE_NAME, O_RDWR | O_CREAT, 0600);
	if (fileDesc < 0) {
		printf("Error opening/creating file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
		signal(SIGTERM, SIG_DFL);
		return errno;
	}

	/* Force the file to be of the same size as the (mmapped) array */
	lseekRes = lseek(fileDesc, numOfBytesToWrite - 1, SEEK_SET); //TODO make sure the minus 1 is okay
	if (lseekRes < 0) {
		printf("Error using lseek() to 'stretch' the file: %s\n", strerror(errno));
		close(fileDesc);
		signal(SIGTERM, SIG_DFL);
		return errno;
	}

	/* write at the end of the file*/
	writeRes = write(fileDesc, "\0", 1);
	if (writeRes < 0 ){
		printf("Error writing last byte of the file: %s\n", strerror(errno));
		close(fileDesc);
		signal(SIGTERM, SIG_DFL);
		return errno;
	}

	/*get time before writing to mapped file*/
	returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(fileDesc);
		signal(SIGTERM, SIG_DFL);
		return errno;
	}

	/* each iteration we will write 4096 bytes to the file*/
	for (int i = 0; i < numberOFIterations; i++) {

		/* map the file: both read & write (same as 'open'), and make sure we can share it */
		data = (char*) mmap(NULL, numOfBytesToWrite, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesc ,i * BUFFER_SIZE);
		if (MAP_FAILED == data) {
			printf("Error mapping the file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
			close(fileDesc);
			signal(SIGTERM, SIG_DFL);
			return errno;
		}
		index = 0;
		/* write to the file (it's in the memory!) */
		while (charCount < numOfBytesToWrite - 1) {
			data[index] = 'a';
			index++;
			charCount++;
		}

		/* release the memory - unmap the file */
		munmapRes = munmap(data, BUFFER_SIZE);
		if (munmapRes < 0) {
			printf("Error while using munmap syscall. %s\n", strerror(errno));
			close(fileDesc);
			signal(SIGTERM, SIG_DFL);
			return errno;
		}

	}


	/*Send a signal (SIGUSR1) to the reader process (man 2 kill)*/
	printf("right before sending the SIGUSR1 signal\n");
	if (kill(readerProcID, 10) == -1 ) {
		printf("kill didn't work...\n");
		munmap(data, numOfBytesToWrite);
		close(fileDesc);
		signal(SIGTERM, SIG_DFL);
		return errno;
	}


	/*get time after writing to mapped file*/
	returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(fileDesc);
		munmap(data, numOfBytesToWrite);
		signal(SIGTERM, SIG_DFL);
		return errno;
	}

	/*Counting time elapsed*/
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were written in %f microseconds through MMAP\n", numOfBytesToWrite ,elapsed_microsec);

	/* restore default signal handler*/
	signal(SIGTERM, SIG_DFL);
	close(fileDesc);
	return 0;
}
