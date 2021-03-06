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
	struct sigaction sigterm_old_action; /* the old handler of SIGTERM will be stores here*/
	struct sigaction sigign_action; /* this is the handler that ignores SIGTERM*/
	sigign_action.sa_handler = SIG_IGN; /*now we ignore the SIGTERM signal*/

	if (0 != sigaction (SIGTERM, &sigign_action, &sigterm_old_action))
	{
		printf("Signal handle for SIGTERM registration failed. %s\n",strerror(errno));
		return errno;
	}

	char* ptr1;
	char* ptr2;
	int numOfBytesToWrite =(int) strtol(argv[1], &ptr1, 10);
	int readerProcID = (int) strtol(argv[2], &ptr2, 10);


	struct timeval t1, t2;
	double elapsed_microsec;
	int returnVal = 0;
	int returnVal2 = 0;
	int fileDesc = 0;
	char* data; /* a pointer to the place in the mapped file we want to write to */
	int lseekRes = 0;
	int munmapRes = 0;
	int writeRes = 0;
	int charCount = 0; /*the number of characters already written*/

	/* open/create the file to be mapped to memory */
	fileDesc = open(MAPPED_FILE_NAME, O_RDWR | O_CREAT, 0600);
	if (fileDesc < 0) {
		printf("Error opening/creating file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
		return errno;
	}

	/* Force the file to be of the same size as the (mmapped) array */
	lseekRes = lseek(fileDesc, numOfBytesToWrite - 1, SEEK_SET); //TODO make sure the minus 1 is okay
	if (lseekRes < 0) {
		printf("Error using lseek() to 'stretch' the file: %s\n", strerror(errno));
		close(fileDesc);
		return errno;
	}

	/* write at the end of the file*/
	writeRes = write(fileDesc, "", 1);
	if (writeRes < 0 ){
		printf("Error writing last byte of the file: %s\n", strerror(errno));
		close(fileDesc);
		return errno;
	}

	/*get time before writing to mapped file*/
	returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(fileDesc);
		return errno;
	}

	/* map the file: both read & write (same as 'open'), and make sure we can share it */
	data = (char*) mmap(NULL, numOfBytesToWrite, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesc ,0);
	if (MAP_FAILED == data) {
		printf("Error mapping the file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
		close(fileDesc);
		return errno;
	}

	/* write to the file (it's in the memory!) */
	while (charCount < numOfBytesToWrite - 1) {
		data[charCount] = 'a';
		charCount++;
	}

	/* release the memory - unmap the file */
	munmapRes = munmap(data, numOfBytesToWrite);
	if (munmapRes < 0) {
		printf("Error while using munmap syscall. %s\n", strerror(errno));
		close(fileDesc);
		return errno;
	}


	/*Send a signal (SIGUSR1) to the reader process (man 2 kill)*/
	if (kill(readerProcID, 10) == -1 ) {
		printf("kill didn't work...\n");
		munmap(data, numOfBytesToWrite);
		close(fileDesc);
		return errno;
	}


	/*get time after writing to mapped file*/
	returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(fileDesc);
		munmap(data, numOfBytesToWrite);
		return errno;
	}

	/*Counting time elapsed*/
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were written in %f miliseconds through MMAP\n", numOfBytesToWrite ,elapsed_microsec);

	/* restore default SIGTERM signal handler*/
	if (0 != sigaction (SIGTERM, &sigterm_old_action, NULL))
	{
		printf("Could not register the default SIGTERM handler. %s\n",strerror(errno));
		return errno;
	}


	close(fileDesc);
	return 0;
}
