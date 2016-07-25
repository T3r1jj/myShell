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

#include "myShellQueue.h"
#include "myShell.h"

Queue history;
void print_error_exit(char* error) {
    perror(error);
    exit(EXIT_FAILURE);
}
void print_error_clean_exit(char* error){
    deleteQueue(history);
    perror(error);
    exit(EXIT_FAILURE);
}
void print_errors_clean_exit(char* error1, char* error2){
    deleteQueue(history);
    fprintf(stderr, "%s%s", error1, error2);
    exit(EXIT_FAILURE);
}

void executeCommand(char **cmdArgv, char *file, int newDescriptor) {
        int commandDescriptor;
        if (newDescriptor == STDIN_FILENO) {
                commandDescriptor = open(file, O_RDONLY, 0600);
                if (commandDescriptor == -1) {
                    print_error_clean_exit("open");
                }
                if (dup2(commandDescriptor, STDIN_FILENO) == -1) {
                    print_error_clean_exit("dup2");
                }
                close(commandDescriptor);
        }
        if (newDescriptor == STDOUT_FILENO) {
                commandDescriptor = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0600);
                if (commandDescriptor == -1) {
                    print_error_clean_exit("open");
                }
                if (dup2(commandDescriptor, STDOUT_FILENO) == -1) {
                    print_error_clean_exit("dup2");
                }
                close(commandDescriptor);
        }
        if (execvp(cmdArgv[0], cmdArgv) == -1){
            if (errno == ENOENT) {
                print_errors_clean_exit(cmdArgv[0], ": command or program not found\n");
            } else {
                print_error_clean_exit(cmdArgv[0]);
            }
        }
}

int forkPipeExec(int inFd, int outFd, char **cmdArgv) {
  pid_t pid;
    if ((pid = fork ()) == 0) {
        if (inFd != STDIN_FILENO) {
            if (dup2(inFd, 0) == -1) {
                print_error_clean_exit("dup2");
            }
            close (inFd);
        }
        if (outFd != STDOUT_FILENO) {
            if (dup2(outFd, 1) == -1) {
                print_error_clean_exit("dup2");
            }
            close (outFd);
        }
        executeCommand(cmdArgv, "STANDARD", NO_REDIRECTION);
        exit(EXIT_SUCCESS);
    } else if (pid == -1) {
        print_error_clean_exit("fork");
    } else {
        return pid; // Rodzic zwraca pid utworzonego potomka
    }
}

int nextCmdOffset(int n, char** cmdArgv) {
    int i = 0;
    while(i<ARGC)
        if(strcmp("|",cmdArgv[i])==0) {
            cmdArgv[i] = NULL;
            return i+1;
        } else if(strcmp(">>",cmdArgv[i])==0) {
            cmdArgv[i] = NULL;
            return i+1;
        } else {
            i++;
        }
}

int handleForking(int pipesCount, int redirect, int plane, char** cmdArgv) {
    int i;
    pid_t pid;
    int status;
    pid = fork ();
    if (pid == -1) {
        print_error_exit("fork");
    } else if (pid == 0) {
        if (plane == BACKGROUND) {
            if (setpgid(0,0) == -1) {
                perror("setpgid()");
            }
        }

        int pipeInFd, pipeFd [2];
        int offset = 0;
        int tempOffset = 0;

        pipeInFd = STDIN_FILENO;


        for (i = 0; i < pipesCount; ++i) {
            if (pipe(pipeFd) == -1) {
                print_error_clean_exit("pipe");
            }
            tempOffset = nextCmdOffset(i, cmdArgv + offset);
            forkPipeExec(pipeInFd, pipeFd[1], cmdArgv + offset);

            offset += tempOffset;

            close(pipeFd[1]);
            close(pipeInFd);

            pipeInFd = pipeFd[0];
        }

        if (pipeInFd != STDIN_FILENO) {
            if (dup2(pipeInFd, STDIN_FILENO) == -1) {
                print_error_clean_exit("dup2");
            }
            close(pipeInFd);
        }

        if (redirect == 1) {
            int offsetRed = offset + nextCmdOffset(0, cmdArgv + offset);
            executeCommand(cmdArgv+offset, cmdArgv[offsetRed], STDOUT_FILENO);
            exit(EXIT_SUCCESS);
        } else {
            executeCommand(cmdArgv+offset, "STANDARD", NO_REDIRECTION);
            exit(EXIT_SUCCESS);
        }
    } else {
        if (plane == FOREGROUND) {
            int err;
            while(!(err = wait4(pid, &status, WNOHANG, NULL)));
            if (err == -1 && errno == ECHILD) {
                errno = 0;
            } else if (err == -1) {
                perror("wait4");
            }
        }
    }
}

void deleteZombies(int sig) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
    if (pid == -1 && errno == ECHILD) {
        errno = 0;
    } else if (pid == -1) {
        perror("waitpid");
    }
}

void tokenize(char* line, char **cmdArgv, Queue *q) {
    int j,i;
    char *cmdHistory = strdup(line);
    for (i=0; i<ARGC; i++) {
        cmdArgv[i]=NULL;
    }
    i = 0;
    char* token  = strtok(line, " ");
    while(token && i<ARGC) {
        cmdArgv[i++] = token;
        token = strtok(NULL, " ");
    }
    if(cmdArgv[0]!=NULL) {
        addLine(&history, cmdHistory);
    }
    free(cmdHistory);
}

void readline(char* line, int length) {
    line[0]='\0';
    fgets(line, length, stdin);
    line[strlen(line)-1]='\0';
}

void changeDir(char **cmdArgv) {
    if (cmdArgv[1] == NULL) {
        if (chdir(getenv("HOME")) == -1)
            perror("chdir");
    } else {
        if (chdir(cmdArgv[1]) == -1) {
            perror(cmdArgv[1]);
        }
    }
}

void handleCommand(char **cmdArgv) {
    if (cmdArgv[0]==NULL) {
        return;
    }
    if (strcmp("exit", cmdArgv[0]) == 0) {
        deleteQueue(history);
        exit(EXIT_SUCCESS);
    }
    if (strcmp("cd", cmdArgv[0]) == 0) {
        changeDir(cmdArgv);
        return;
    }
    int plane;
    int pipesCount = 0;
    int redirect = 0;
    int i = 0;
    while (cmdArgv[i] != NULL && i < ARGC) {
        if (strcmp("|", cmdArgv[i]) == 0) {
            pipesCount++;
        } else if (strcmp(">>", cmdArgv[i]) == 0) {
            redirect++;
        }
        i++;
    }
    if (strcmp("&", cmdArgv[i-1]) == 0) {
        plane = BACKGROUND;
        cmdArgv[i-1] = NULL;
    } else {
        plane = FOREGROUND;
    }

    handleForking(pipesCount,redirect,plane,cmdArgv);
    return;
}

void shellPrompt() {
    char currDir[512];
    if (getcwd(currDir, 512) == NULL) {
        perror("getcwd");
    }
    printf("SHELL: %s@-%s :> ",getenv("USER"), currDir);
}

void printHistory(int sig) {
    printf("\n");
    printQueue(history);
}

int readScriptLine(int fd, char* buffer) {
    int i = 1;
    if(!read(fd, buffer, 1)) return EOF;
    while (buffer[0] == '#') {
        while (buffer[0] != '\n') {
            if(!read(fd, buffer, 1)) return EOF;
        }
        if(!read(fd, buffer, 1)) return EOF;
    }
    while (buffer[i-1] != '\n') {
        if(!read(fd, buffer+i, 1)) return EOF;
        i++;
    }
    buffer[i-1] = '\0';
    return i-1;
}

void handleScript(char* source, char** cmdArgv, char* historyPath) {
    if (source == NULL) return;
    char *buffer = calloc(LINE_LEN, sizeof(char));
    int fd = open(source+2, O_RDONLY, 0600);
    int EOFcheck = readScriptLine(fd, buffer);
    while (EOFcheck != EOF) {
        tokenize(buffer, cmdArgv, &history);
        saveQueue(history, historyPath);
        handleCommand(cmdArgv);
        EOFcheck = readScriptLine(fd, buffer);
    }
    free(buffer);
    deleteQueue(history);
    close(fd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    char *cmdArgv[ARGC];
    initQueue(&history);
    int i;
    for (i = 0; i<ARGC; i++) {
        cmdArgv[i]=NULL;
    }
    char line[LINE_LEN];
    char historyPath[256];
    strcpy(historyPath, getenv("PWD"));
    strcat(historyPath, "/MyShellHistory.txt");

    struct sigaction childSig;
    memset (&childSig, '\0', sizeof(childSig));
    childSig.sa_handler = deleteZombies;
    childSig.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &childSig, NULL) == -1) {
        print_error_clean_exit("sigaction");
    }

    struct sigaction historySig;
    memset (&historySig, '\0', sizeof(historySig));
    sigaddset(&historySig.sa_mask, SIGCHLD);
    historySig.sa_handler = printHistory;
    historySig.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &historySig, NULL) < 0) {
        perror ("Sigaction: printing history - broken");
    }

    handleScript(argv[1], cmdArgv, historyPath);
    while (1) {
        shellPrompt();
        readline(line, LINE_LEN);
        tokenize(line, cmdArgv, &history);
        saveQueue(history, historyPath);
        handleCommand(cmdArgv);
    }
}
