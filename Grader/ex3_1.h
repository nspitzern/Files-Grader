#ifndef EX3_EX3_1_H
#define EX3_EX3_1_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFSIZE 1024

int compare(int fd1, int fd2);
int checkSame(char* buff1, char* buff2);
int checkSimilar(char *buff1, char *buff2);
char *strip(char *buff);

#endif