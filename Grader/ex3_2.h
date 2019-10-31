//
// Created by tom on 10/29/19.
//

#ifndef EX3_EX3_2_H
#define EX3_EX3_2_H

#define BUFFERSIZE 1024
#define STDIN 0
#define STDOUT 1

int parseConfig(char *configPath, char **configInfo);
void initialize2DChar(char **configInfo);
void free2DChar(char **configInfo);
void startHierarchy(char **configInfo);
void nextDir(char *filePath, int input_fd, char* outputFilePath, int* contains_c_file);
int handleFile(char filePath[160], int input_fd, char* compareOutputFilePath);
int checkFileType(char *path);
int compileCode(char *filePath);
int runProgram(int input_fd);
int compareFiles(char* file1_path, char* file2_path);
void writeResult(int result, char* dirPath);

#endif //EX3_EX3_2_H
