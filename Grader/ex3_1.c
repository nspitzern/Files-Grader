#include <stdlib.h>
#include "ex3_1.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error! Not enough arguments!\n");
        return -1;
    }
    // initialize the files path
    char *file1_path, *file2_path;
    int file1_fd, file2_fd, result;

    file1_path = argv[1];
    file2_path = argv[2];

    file1_fd = open(file1_path, O_RDONLY);
    file2_fd = open(file2_path, O_RDONLY);

    // check file paths
    if (file1_fd == -1 || file2_fd == -1) {
        fprintf(stderr, "Error! Bad files path!\n");
        return -1;
    }

    result = compare(file1_fd, file2_fd);

    // close FDs
    close(file1_fd);
    close(file2_fd);

    exit(result);
}

/*****
 * Compare both files.
 * If both files are exactly the same - return 3.
 * If both files are similar (same without white spaces, new lines, and case sensitivity) - return 2.
 * If the files are different - return 1.
 * For every error return -1.
 * @param fd1 first file descriptor.
 * @param fd2 second file descriptor.
 * @return int
 */
int compare(int fd1, int fd2) {
    int read1, read2;
    int sameFlag = 1, similarFlag = 1;
    char buff1[BUFFSIZE] = {0}, buff2[BUFFSIZE] = {0};

    read1 = read(fd1, buff1, BUFFSIZE);
    read2 = read(fd2, buff2, BUFFSIZE);

    if (read1 == 0 && read2 == 0) {
        return 3;
    } else if (read1 == 0 || read2 == 0) {
        return 1;
    }

    while(read1 != 0 && read2 != 0) {
        // check error in read
        if (read1 == -1 || read2 == -1) {
            fprintf(stderr, "Error! Error on read from files!\n");
            return -1;
        }

        // if until now the files are the same check the new two segments.
        if (sameFlag == 1) {
            // if both segments are not the same, change the sameFlag
            if (checkSame(buff1, buff2) != 0) {
                sameFlag = 0;
            }
        }
        if(similarFlag == 1) {
            // if both segments are not the similar, change the similarFlag
            if(checkSimilar(buff1, buff2) != 0) {
                similarFlag = 0;
            }
        } else {
            // both segments are different
            return 1;
        }
        read1 = read(fd1, buff1, BUFFSIZE);
        read2 = read(fd2, buff2, BUFFSIZE);
    }

    // check the flags
    if(sameFlag) {
        return 3;
    } else if(similarFlag) {
        return 2;
    }
    return 1;

}

/****
 * Check if both files are similar.
 * @param buff1 first file content.
 * @param buff2 second file content.
 * @return 0 if both files are similar, non zero otherwise.
 */
int checkSimilar(char *buff1, char *buff2) {
    // strip both segments from white spaces, new lines and case sensitivity.
    char *temp1, *temp2;
    int result;
    temp1 = strip(buff1);
    temp2 = strip(buff2);
    result = checkSame(temp1, temp2);

    free(temp1);
    free(temp2);
    return result;
}

/*****
 * Strips the segment from white spaces, new lines and case sensitivity.
 * @param buff the string.
 * @return a new string after strip.
 */
char *strip(char* buff) {
    char* tempBuff = (char*)malloc(strlen(buff));

    // initialize copy string
    char c[2];
    c[1] = '\0';
    int size = strlen(buff), i;

    for (i = 0; i < size; i++) {
        // check if the char is a letter
        if (buff[i] != ' ' && buff[i] != '\n') {
            if (buff[i] >= 'A' && buff[i] <= 'Z') {
                // convert to lower case
                c[0] = (char)(buff[i] + 32);
                strcat(tempBuff, c);
            } else {
                // concat the char
                c[0] = buff[i];
                strcat(tempBuff, c);
            }
        }
    }
    return tempBuff;
}

/*****
 * Check if both file segments are exactly the same.
 * @param buff1 first file segment.
 * @param buff2 second file segment.
 * @return 0 if both segments are the same.
 */
int checkSame(char* buff1, char* buff2) {
    return strcmp(buff1, buff2);
}