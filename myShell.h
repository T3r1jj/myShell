/* 
 * Copyright 2014 Damian Terlecki.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // EXIT_SUCCESS(FAILURE)
#include <fcntl.h>  // open(), O_CREAT, ...
#include <signal.h> // sigaction
#include <sys/wait.h>  // waitpid()
#include <unistd.h> // STDIN(OUT)_FILENO
#include <errno.h>  // errno

#define ARGC 32
#define ARG_LEN 32
#define LINE_LEN 128
#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define NO_REDIRECTION -1

void print_error_exit(char* error);
void print_error_clean_exit(char* error);
void print_errors_clean_exit(char* error1, char* error2);

void executeCommand(char **cmdArgv, char *file, int newDescriptor);

int forkPipeExec(int inFd, int outFd, char **cmdArgv);

int nextCmdOffset(int n, char** cmdArgv);

int handleForking(int pipesCount, int redirect, int plane, char** cmdArgv);

void deleteZombies(int sig);

void tokenize(char* line, char **cmdArgv, Queue *q);

void readline(char* line, int length);

void changeDir(char **cmdArgv);

void handleCommand(char **cmdArgv);

void shellPrompt(void);

void printHistory(int sig);

int readScriptLine(int fd, char* buffer);

void handleScript(char* source, char** cmdArgv, char* historyPath);

#endif
