//
// Created by tom on 10/29/19.
//

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "ex3_2.h"


int main(int argc, char *argv[]) {
    // validate input
    if(argc != 2) {
        fprintf(stderr, "Error! Not enough arguments to main!\n");
        return 0;
    }

    char* configPath = argv[1];

    char **configInfo = (char**)malloc(3);
    configInfo[0] = (char*)malloc(160);
    configInfo[1] = (char*)malloc(160);
    configInfo[2] = (char*)malloc(160);
    initialize2DChar(configInfo);

    // parse the config info from the file
    int success = parseConfig(configPath, configInfo);

    if(success) {
        startHierarchy(configInfo);
        unlink("a.out");
        unlink("outputFile");
    }

    free2DChar(configInfo);
}

/*****
 * Handle the root of the directories hierarchy.
 * Writes the results into a .CSV file.
 * @param configInfo the information in the config file.
 */
void startHierarchy(char **configInfo) {
    char* startDirPath = configInfo[0];
    char* inputFilePath = configInfo[1];
    char* outputFilePath = configInfo[2];
    int result, *contains_c_file = (int*)malloc(sizeof(int));
    *contains_c_file = 0;

    // initialize first directory
    DIR* startingDirectory = opendir(startDirPath);

    if(startingDirectory == NULL) {
        fprintf(stderr, "Error! Could not open directory: %s!\n", startDirPath);
        free(contains_c_file);
        return;
    }

    // initialize input file
    int inputFile_fd = open(inputFilePath, O_RDONLY);

    if(inputFile_fd == -1) {
        fprintf(stderr, "Error! Could not open input/output file!\n");
        free(contains_c_file);
        return;
    }

    // initialize directory info and stat
    struct dirent *pDirent;
    struct stat p_stat;

    while((pDirent = readdir(startingDirectory)) != NULL) {
        // skip current and parent directory
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
            continue;
        } else {
            char nextFilePath[160] = {0};
            strcpy(nextFilePath, startDirPath);
            strcat(nextFilePath, "/");
            strcat(nextFilePath ,pDirent->d_name);
            if(stat(nextFilePath, &p_stat) == -1) {
                fprintf(stderr, "Error! Could not determine file stat!\n");
                free(contains_c_file);
                return;
            }

            // check if is a file or a directory
            if (S_ISREG(p_stat.st_mode)) {
                result = handleFile(nextFilePath, inputFile_fd, outputFilePath);
                if (result != -1) {
                    // write the result into .CSV
                    writeResult(result, startDirPath);
                }
            } else if(S_ISDIR(p_stat.st_mode)) {
                nextDir(nextFilePath, inputFile_fd, outputFilePath, contains_c_file);
                if (*contains_c_file == 0) {
                    writeResult(-1, nextFilePath);
                }
            }
        }
        *contains_c_file = 0;
    }


    // close the directory
    if(closedir(startingDirectory) == -1) {
        fprintf(stderr, "Error! Could not close directory: %s!\n", startDirPath);
        free(contains_c_file);
        return;
    }
    free(contains_c_file);
}

/****
 * Check s if the given file is a .c file.
 * If true - tries to compile it.
 * If true - tries to execute it with limit of 5 seconds.
 * Returns the result (-1 for no .c file, -2 for compilation error, -3 for timeout, 1/2/3 for comparision result).
 * @param filePath path to the file.
 * @param input_fd the file descriptor of the input file.
 * @param compareOutputFilePath the path to the correct output file.
 * @return the result.
 */
int handleFile(char filePath[160], int input_fd, char* compareOutputFilePath) {

    int isCFile = checkFileType(filePath);
    if (isCFile) {
        // if it is a .c file - compile it and execute
        int result = compileCode(filePath);
        if (result == 0) {
            // compilation was successful
            result = runProgram(input_fd);

            if (result == 0) {
                result = compareFiles("./outputFile", compareOutputFilePath);
                if (result > 0) {
                    return result;
                }
            } else { // TIMEOUT
                return -3;
            }

        } else { // compilation error
            return -2;
        }
    }
    return -1; // not a .c file

}

/*****
 * Use "comp.out" to compare the output file and the correctOutput.txt file.
 * If the files are the same  - return 3.
 * If the files are similar - return 2.
 * If the files are different - return 1.
 * In case of an error - return -1.
 * @param file1_path path to the first file.
 * @param file2_path path to the second file.
 * @return the result number.
 */
int compareFiles(char* file1_path, char* file2_path) {
    int stat = -1;
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Error! Could not open new process!\n");
        return -1;
    }

    // son process
    if (pid == 0) {
        char* args[] = {"./comp.out", file1_path, file2_path, NULL};
        execv(args[0], args);
    } else { // father process
        waitpid(pid, &stat, 0);
        stat = WEXITSTATUS(stat);
    }

    return stat;
}


/****
 * Gets an input FD and runs the compiled program.
 * The output is written into an output file for further comparision
 * @param input_fd the FD of the input file.
 * @return -1 is case of error, 0 in case of timeout, 1 in case of success.
 */
int runProgram(int input_fd) {
    pid_t pid;
    int stat = -1, status = -1;
    int output_fd;

    output_fd = open("./outputFile", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);

    if (output_fd < 0) {
        fprintf(stderr, "Error! Could not open new output file\n");
        return -1;
    }

    if(lseek(input_fd, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Could not offset input FD from start of file!\n");
        return -1;
    }

    pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Error! Could not open new process!\n");
        return -1;
    }

    // at son process
    if(pid == 0) {
        dup2(input_fd, STDIN);
        dup2(output_fd, STDOUT);
        char* args[] = {"./a.out", NULL};

        execv(args[0], args);
    } else { // at parent process
        sleep(5);
        waitpid(pid, &stat, WNOHANG);
        status = WEXITSTATUS(stat);
    }
    close(output_fd);
    unlink("./a.out");

    return status;
}

/*****
 * Gets a path to a .c file and compiles it.
 * @param filePath a path to a .c file.
 * @return -1 in case of error, 1 in case of success.
 */
int compileCode(char *filePath) {
    pid_t pid;
    int stat = -1, status = -1;
    char* args[] = {"gcc", filePath, NULL};

    pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Error! Could not open new process!\n");
        return -1;
    }

    if(pid == 0) {
        // compile the code
        execvp(args[0], args);
    } else {
        // wait for compilation to end
        waitpid(pid, &stat, 0);
        status = WEXITSTATUS(stat);
    }
    return status;
}

/****
 * Check the file type by its extension.
 * @param path the path to the file.
 * @return 1 if it is a .c file, o.w. return 0.
 */
int checkFileType(char *path) {
    char* p = (path + strlen(path) - 2);
    // check if is a .c file
    if (strcmp(p, ".c") == 0) {
        return 1;
    } else {
        return 0;
    }
}

/****
 * Go to the next directory and check for .c files.
 * @param dirPath the current directory path.
 * @param input_fd the FD of the input file.
 * @param outputFilePath the path to the correct output.
 */
void nextDir(char *dirPath, int input_fd, char* outputFilePath, int* contains_c_file) {
    // initialize directory info and stat
    struct dirent *pDirent;
    struct stat p_stat;
    DIR *currentDir = opendir(dirPath);
    int result = -1;

    while((pDirent = readdir(currentDir)) != NULL) {
        // skip current and parent directory
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
            continue;
        } else {
            char nextFilePath[160] = {0};
            strcpy(nextFilePath, dirPath);
            strcat(nextFilePath, "/");
            strcat(nextFilePath ,pDirent->d_name);
            if(stat(nextFilePath, &p_stat) == -1) {
                fprintf(stderr, "Error! Could not determine file stat!\n");
                closedir(currentDir);
                return;
            }

            // check if is a file or a directory
            if (S_ISREG(p_stat.st_mode)) {
                // write the result into .CSV
                result = handleFile(nextFilePath, input_fd, outputFilePath);
                if (result != -1) { // if the directory contains a .c file
                    *contains_c_file = 1;
                    writeResult(result, nextFilePath);
                }
            } else if(S_ISDIR(p_stat.st_mode)) {
                nextDir(nextFilePath, input_fd, outputFilePath, contains_c_file);
            }
        }
    }
    closedir(currentDir);
}

/*****
 * Write the result into a .CSV file.
 * Each file path has a grade and th reason for that grade.
 * @param result the result of comparision between both files.
 * @param dirPath the path to the directory.
 */
void writeResult(int result, char* dirPath) {
    int resultsFile_fd, status;
    char* reason = "", *score;
    resultsFile_fd = open("./results.csv", O_CREAT | O_APPEND | O_RDWR, S_IRWXU);

    if(resultsFile_fd == -1) {
        fprintf(stderr, "Error! Could not open results file!\n");
        return;
    }

    switch (result) {
        case -1:
            score = "0";
            reason = "NO_C_FILE";
            break;
        case -2:
            score = "0";
            reason = "COMPILATION ERROR";
            break;
        case -3:
            score = "0";
            reason = "TIMEOUT";
            break;
        case 1:
            score = "60";
            reason = "BAD_OUTPUT";
            break;
        case 2:
            score = "80";
            reason = "SIMILAR_OUTPUT";
            break;
        case 3:
            score = "100";
            reason = "GREAT_JOB";
            break;
        default:
            score = "-1";
            break;
    }

    status = write(resultsFile_fd, dirPath, strlen(dirPath));
    status = write(resultsFile_fd, ",", strlen(","));
    status = write(resultsFile_fd, score, strlen(score));
    status = write(resultsFile_fd, ",", strlen(","));
    status = write(resultsFile_fd, reason, strlen(reason));
    status = write(resultsFile_fd, "\n", strlen("\n"));

    if (status == -1) {
        fprintf(stderr, "Error! Could not write to results file!\n");
        return;
    }

    status = close(resultsFile_fd);

    if (status == -1) {
        fprintf(stderr, "Error! Could not close results file!\n");
        return;
    }
}

/*****
 * Parse the config file.
 * First row is the root of hierarchy.
 * Second row is the input file.
 * Third row is the output file we should compare to.
 * @param configPath the path to the config file.
 * @param configInfo an empty 2D char array.
 * @return -1 in case of an error, 1 in case of success.
 */
int parseConfig(char* configPath, char **configInfo) {
    int fd = open(configPath, O_RDONLY);

    // check if open file failed
    if(fd == -1) {
        fprintf(stderr, "Error! Could not open config file!\n");
        return 0;
    }

    char buff[BUFFERSIZE] = {0};

    int r = read(fd, buff, BUFFERSIZE);

    char *tok = "\n";
    int i = 0;
    char* ptr = strtok(buff, tok);

    // parse the config file by new line
    while(ptr != NULL) {
        strcpy(configInfo[i], ptr);
        i++;

        // go to the next line
        ptr = strtok(NULL, tok);
    }


    if(close(fd) == -1) {
        fprintf(stderr, "Error! Could not close config file!\n");
        return  0;
    }
    return 1;
}

/****
 * Initialize 2D char array with '\0'.
 * @param configInfo the array.
 */
void initialize2DChar(char **configInfo) {
    int rows = 3, cols = strlen(configInfo[0]);
    int i, j;

    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            configInfo[i][j] = 0;
        }
    }
}

/*****
 * Free 2d array.
 * @param configInfo the array.
 */
void free2DChar(char **configInfo) {
    int rows = 3;
    int i;

    for(i = 0; i < rows; i++) {
        free(configInfo[i]);
    }

    free(configInfo);
}
