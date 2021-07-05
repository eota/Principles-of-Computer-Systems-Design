/*********************************************************************
 *
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

#include "fileman.h"
#include <stdio.h>  
#include <unistd.h>  
#include <fcntl.h> 
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// References: https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
// 			   https://linux.die.net/man/2/lseek
//             https://linux.die.net/man/3/scandir
//             https://stackoverflow.com/questions/5901181/c-string-append
//             Lecture Secret Sauce

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_read(char *fname, size_t foffset, char *buf, size_t boffset, size_t size) {
	int fd = open(fname, O_RDONLY);
	lseek(fd, foffset, SEEK_SET);
	if (fd == -1) {
		return -1;
	}

	int bytes_read = read(fd, buf+boffset, size);
	// printf("\nbuf = %s\n", buf);

	close(fd);
	return bytes_read;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_write(char *fname, size_t foffset, char *buf, size_t boffset, size_t size) {
	int fd = open(fname, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != -1) {
		return -1;
	} else {
		fd = open(fname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	}
	// printf("\nfd = %d\n", fd);
	// int fd = open(fname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	lseek(fd, foffset, SEEK_SET);

	int bytes_written = write(fd, buf+boffset, size);
	// printf("\nbytes_written = %d\n", bytes_written);
	close(fd);
	return bytes_written;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_append(char *fname, char *buf, size_t size) {
	int fd = open(fname, O_RDWR | O_APPEND);
	// printf("\nfd = %d\n", fd);
	if (fd == -1) {
		return -1;
	}
	int bytes_written = write(fd, buf, size);
	// printf("\nbytes_written = %d\n", bytes_written);
	close(fd);
	return bytes_written;
}

/*
 * You need to implement this function, see fileman.h for details 
 */
int fileman_copy(char *fsrc, char *fdest) {
	int fd_src = open(fsrc, O_RDWR);
	int fd_dest = open(fdest, O_RDWR);
	// printf("\nfd_src = %d\n", fd_src);
	// printf("\nfd_dest = %d\n", fd_dest);
	if (fd_src == -1 || fd_dest != -1) {
		return -1;
	}
	fd_dest = open(fdest, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	char *buf[4096];
	size_t size = 4096;
	int bytes_read = read(fd_src, buf, size);
	int bytes_written = write(fd_dest, buf, size);
	// printf("\nbytes_read = %d\n", bytes_read);
	// printf("\nbytes_written = %d\n", bytes_written);
	while (bytes_read == size) {
		lseek(fd_src, bytes_written, SEEK_SET);
		lseek(fd_dest, bytes_written, SEEK_SET);
		bytes_read = read(fd_src, buf, size);
		bytes_written += write(fd_dest, buf, bytes_read);
	}
	// printf("\nbytes_read = %d\n", bytes_read);
	// printf("\nbytes_written = %d\n", bytes_written);
	close(fd_src);
	close(fd_dest);
	return bytes_written;
}

void get_dir(int fd, char *dname, char *dnameFull, int indent) {
	// char *buf = dnameFull + "\n";
	struct dirent **namelist;
	int bytes_written = 0;
	if (indent <= 4) {
		bytes_written = write(fd, dname, strlen(dname));
		bytes_written += write(fd, "\n", strlen("\n"));
	}

	int scan = scandir(dnameFull, &namelist, NULL, alphasort);
	// printf("scan = %d\n", scan);
	if (scan == -1) {
		return;
	}

	for (int i = 2; i < scan; i++) {
		for (int j = 0; j < indent; j++) {
			bytes_written += write(fd, " ", 1); // write indent spaces to fd
		}
		bytes_written += write(fd, namelist[i]->d_name, strlen(namelist[i]->d_name));
		bytes_written += write(fd, "\n", strlen("\n"));
		if (namelist[i]->d_type == DT_DIR) {  // if entry is a directory
			// printf("FOUND DIRECTORY: %s\n", namelist[i]->d_name);
			char *newDName = "";
			if((newDName = malloc(strlen(dnameFull)+strlen(namelist[i]->d_name)+2)) != NULL){
				newDName[0] = '\0';   // ensures the memory is an empty string
				strcat(newDName,dnameFull);
				strcat(newDName,"/");
				strcat(newDName,namelist[i]->d_name);
			} else {
				printf("ERROR: malloc failed");
				// exit?
			}
			// printf("newDName = %s\n", newDName);
			get_dir(fd, namelist[i]->d_name, newDName, indent + 4);
			free(newDName);
		}
		free(namelist[i]);
	}
	free(namelist[0]);
	free(namelist[1]);
	free(namelist);
}

void get_dir_tree(int fd, char *dname, char *dnameFull, int indent) {
	// char *buf = dnameFull + "\n";
	struct dirent **namelist;
	int bytes_written = 0;
	if (indent <= 4) {
		bytes_written = write(fd, dname, strlen(dname));
		bytes_written += write(fd, "\n", strlen("\n"));
	}

	int scan = scandir(dnameFull, &namelist, NULL, alphasort);
	// printf("scan = %d\n", scan);
	// printf("dname = %s\n", dname);
	
	if (scan == -1) {
		return;
	}

	for (int i = 2; i < scan; i++) {
		for (int j = 0; j < indent; j++) {
			if (j < (indent - 4) && (j%4 == 0)) {
				bytes_written += write(fd, VER, strlen(VER));
			} else if (j < (indent-4)) {
				bytes_written += write(fd, " ", 1);
			} else if (j < (indent-3)) {
				if (i == (scan-1)) {
					bytes_written += write(fd, ELB, strlen(ELB));
				} else {
					bytes_written += write(fd, TEE, strlen(TEE));
				}
			} else if (j < (indent-1)) {
				bytes_written += write(fd, HOR, strlen(HOR));
			} else {
				bytes_written += write(fd, " ", 1);
			}
		}
		bytes_written += write(fd, namelist[i]->d_name, strlen(namelist[i]->d_name));
		bytes_written += write(fd, "\n", strlen("\n"));
		if (namelist[i]->d_type == DT_DIR) {  // if entry is a directory
			// printf("FOUND DIRECTORY: %s\n", namelist[i]->d_name);
			char *newDName = "";
			if((newDName = malloc(strlen(dnameFull)+strlen(namelist[i]->d_name)+2)) != NULL){
				newDName[0] = '\0';   // ensures the memory is an empty string
				strcat(newDName,dnameFull);
				strcat(newDName,"/");
				strcat(newDName,namelist[i]->d_name);
			} else {
				printf("ERROR: malloc failed");
				// exit?
			}
			// printf("newDName = %s\n", newDName);
			get_dir_tree(fd, namelist[i]->d_name, newDName, indent + 4);
			free(newDName);
		}
		free(namelist[i]);
	}
	free(namelist[0]);
	free(namelist[1]);
	free(namelist);
}

/*
 * You need to implement this function, see fileman.h for details 
 */
void fileman_dir(int fd, char *dname) {
	get_dir(fd, dname, dname, 4);
}

/*
 * You need to implement this function, see fileman.h for details 
 */
void fileman_tree(int fd, char *dname) {
	get_dir_tree(fd, dname, dname, 4);
}

