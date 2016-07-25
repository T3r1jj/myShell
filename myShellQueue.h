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
 
#ifndef MYSHELLQUEUE_H
#define MYSHELLQUEUE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define HISTORY_ARGC 20

struct queue {
    char* buffer[HISTORY_ARGC];
    int counter;
};

typedef struct queue Queue;

void initQueue(Queue *q);
void addLine(Queue* q, char* line);
void saveQueue(Queue q, char* path);
void printQueue(Queue q);
void deleteQueue(Queue q);

#endif
