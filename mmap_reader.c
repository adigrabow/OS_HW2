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
	//printf("Entered SIGUSR handler\n");
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


	/* Open the file */
	fileDescriptor = open(MAPPED_FILE_NAME, O_RDWR , PERMISSION_CODE); //TODO maek sure read write is okay.
	if (fileDescriptor < 0) {
		printf("Error opening/creating file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
		exit(errno);
	}

	/* Determine the file size (man 2 stat)*/
	if (stat(MAPPED_FILE_NAME,&fileStat) < 0) {
		printf("Could not receive %s stats. Exiting...\n", MAPPED_FILE_NAME);
		close(fileDescriptor);
		exit(errno);
	}

	fileSize = fileStat.st_size; /* in bytes */

	/* Create a memory map for the file */
	data = (char*) mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fileDescriptor ,0);
	if (MAP_FAILED == data) {
		printf("Error mapping the file: %s. %s\n",MAPPED_FILE_NAME, strerror(errno));
		close(fileDescriptor);
		exit(errno);
	}

	/*get time before reading the mapped file*/
	returnVal = gettimeofday(&t1, NULL);
	if (returnVal == -1) {
		printf("Could not get time of day. Exiting...\n");
		close(fileDescriptor);
		exit(errno);
	}

	int index = 0;
	while ( (char) data[index] != '\0') {
		if ((char) data[index] == 'a') {
			charCounter++;
		}
		index++;
		if (index == fileSize ) {
			printf("Missing End_Of_File Symbol...\n");

				/*get time after reading from mapped file*/
			returnVal2 = gettimeofday(&t2, NULL);
			if (returnVal2 == -1) {
				printf("Could not get time of day. Exiting...\n");
				munmap(data, fileSize);
				close(fileDescriptor);
				exit(errno);
			}

			/*Counting time elapsed*/
			elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
			elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

			printf("%d were read in %f miliseconds through MMAP\n", charCounter ,elapsed_microsec);
			close(fileDescriptor);
			exit(-1);
		}
	}

	/* munmap */
	munmapRes = munmap(data, fileSize);
	if (munmapRes < 0) {
		printf("Error while using munmap syscall. %s\n", strerror(errno));
		close(fileDescriptor);
		exit(errno);
	}

	//printf("read %d 'a' chars\n",charCounter);
	/*get time after reading from mapped file*/
	returnVal2 = gettimeofday(&t2, NULL);
	if (returnVal2 == -1) {
		printf("Could not get time of day. Exiting...\n");
		munmap(data, fileSize);
		close(fileDescriptor);
		exit(errno);
	}

	/*Counting time elapsed*/
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) / 1000.0;

	printf("%d were read in %f miliseconds through MMAP\n", charCounter+1 ,elapsed_microsec);
	close(fileDescriptor);

	/* Remove the file from the disk (man 2 unlink) */
	if (unlink(MAPPED_FILE_NAME) == -1) {
		printf("Could not unlink %s from file system. Exiting..,\n", MAPPED_FILE_NAME);
		close(fileDescriptor);
		exit(errno);
	}

	//printf("Reader: Exiting Handler\n");
	exit(0);
	//return;

}


int main(int argc, char* argv[]) {
	//printf("Entered reader main\n");
	struct sigaction new_action;
	struct sigaction sigterm_old_action; /* the old handler of SIGTERM will be stores here*/
	struct sigaction sigign_action; /* this is the handler that ignores SIGTERM*/

	new_action.sa_handler = sigusr_handler;
	new_action.sa_flags = 0;
	sigign_action.sa_handler = SIG_IGN; /*now we ignore the SIGTERM signal*/

	if (0 != sigaction (SIGUSR1, &new_action, NULL))
	{
		printf("Signal handle for SIGUSR1 registration failed. %s\n",strerror(errno));
		return errno;
	}

	/* Ignore SIGTERM */
	if (0 != sigaction (SIGTERM, &sigign_action, &sigterm_old_action))
	{
		printf("Signal handle for SIGTERM registration failed. %s\n",strerror(errno));
		return errno;
	}


	while (1) {

		sleep(2);

	}

	/* Stop Ignoring SIGTERM */
	if (0 != sigaction (SIGTERM, &sigterm_old_action, NULL))
	{
		printf("Could not register the default SIGTERM handler. %s\n",strerror(errno));
		return errno;
	}
	//printf("Exiting reader main\n");
	return 0;
}
