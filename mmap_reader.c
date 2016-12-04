/*
 * mmap_reader.c
 *
 *  Created on: Dec 3, 2016
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
#include <sys/mman.h>
#include <signal.h>

#define MAPPED_FILE_NAME "/tmp/mmapped.bin"
#define PERMISSION_CODE 0600
#define BUFFER_SIZE 4096


void sigusr_handler(int sig); /* a SIGUSR1 handler function */


void sigusr_handler(int sig) {

	printf("Entered SIGUSR1 handler\n");

	struct timeval t1, t2;
	double elapsed_microsec;
	int returnVal = 0;
	int returnVal2 = 0;

	struct stat fileStat;
	int fileSize = 0;
	int fileDescriptor = 0;
	char* data; /* a pointer to the place in the mapped file we want to read from */
	int charCounter = 0; /* counts the  number of 'a' characters read from the file */
	int munmapRes = 0;
	int numberOFIterations = 0;
	int index = 0;


	/* Open the file */
	fileDescriptor = open(MAPPED_FILE_NAME, O_RDWR , PERMISSION_CODE); //TODO maek sure read write is okay.
	if (fileDescriptor < 0) {
		printf("Error opening/creating file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
		signal(SIGTERM, SIG_DFL);
		exit(errno);
	}

	/* Determine the file size (man 2 stat)*/
	if (stat(MAPPED_FILE_NAME,&fileStat) < 0) {
		printf("Could not receive %s stats. Exiting...\n", MAPPED_FILE_NAME);
		close(fileDescriptor);
		signal(SIGTERM, SIG_DFL);
		exit(errno);
	}

	fileSize = fileStat.st_size; /* in bytes */
	numberOFIterations = (fileSize / BUFFER_SIZE) + 1;
	printf("file size is = %d\nnumberOFIterations = %d",fileSize,numberOFIterations);

	/*get time before reading the mapped file*/
	returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(fileDescriptor);
		signal(SIGTERM, SIG_DFL);
		exit(errno);
	}

	int cnt = 0;

	for (int i = 0; i < numberOFIterations; i++) {

		/* Create a memory map for the file */
		data = (char*) mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fileDescriptor ,i * BUFFER_SIZE); /* loop over ofset 4096 */
		if (MAP_FAILED == data) {
			printf("Error mapping the file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
			close(fileDescriptor);
			signal(SIGTERM, SIG_DFL);
			exit(errno);
		}

		/* Count the number of 'a' bytes in the array until the first NULL ('\0') */
		index = 0;

		while (charCount < fileSize) {
			if ((char) data[index] == 'a') {
				charCounter++;
			}
		}

		/* munmap */
		munmapRes = munmap(data, BUFFER_SIZE);
		if (munmapRes < 0) {
			printf("Error while using munmap syscall. %s\n", strerror(errno));
			close(fileDescriptor);
			signal(SIGTERM, SIG_DFL);
			exit(errno);
		}
	}

	printf("read %d 'a' chars\n",charCounter);
	/*get time after reading from mapped file*/
	returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		munmap(data, fileSize);
		close(fileDescriptor);
		signal(SIGTERM, SIG_DFL);
		exit(errno);
	}

	/*Counting time elapsed*/
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were read in %f microseconds through MMAP\n", charCounter ,elapsed_microsec);


	/* Remove the file from the disk (man 2 unlink) */
	if (unlink(MAPPED_FILE_NAME) == -1) {
		printf("Could not unlink %s from file system. Exiting..,\n", MAPPED_FILE_NAME);
		close(fileDescriptor);
		signal(SIGTERM, SIG_DFL);
		exit(errno);
	}

	/* restore default signal handler*/
	signal(SIGTERM, SIG_DFL);
	close(fileDescriptor);
	exit(0);

}


int main(int argc, char* argv[]) {

	printf("My process ID : %d\n", getpid());
	struct sigaction new_action;
	new_action.sa_handler = sigusr_handler;
	new_action.sa_flags = 0;
	if (0 != sigaction (SIGUSR1, &new_action, NULL))
	{
		printf("Signal handle registration failed. %s\n",strerror(errno));
		return errno;
	}

	/* Ignore SIGTERM */
	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
		printf("Could not ignore SIGTERM signal as requested. Exiting...\n");
		return -1;
	}

	while (1) {

		sleep(2);
		printf("waiting for signal...\n");
	}

	return 0;
}
