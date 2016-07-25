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

void initQueue(Queue *q) {
    q->counter = 0;
    int i;
    for (i = 0; i < HISTORY_ARGC; i++) {
        q->buffer[i] = NULL;
    }
}


void addLine(Queue* q, char* line) {
    if(q->counter < HISTORY_ARGC) {
        q->buffer[q->counter++] = strdup(line);
    } else {
        int i;
        free(q->buffer[0]);
        for (i = 0; i < HISTORY_ARGC-1; i++) {
            q->buffer[i] = q->buffer[i+1];
        }
        q->buffer[q->counter-1] = strdup(line);
    }
}

void saveQueue(Queue q, char* path) {
    int i = 0;
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd == -1) {
        perror("While creating history file, open()");
    }
    write(fd, "Last 20 instructions (those at top are older) from last Shell run:\n", 67);
    for (; i < q.counter; i++) {
        write(fd, q.buffer[i], strlen(q.buffer[i]));
        write(fd, "\n",1);
    }
    close(fd);
}

void printQueue(Queue q) {
    int i = 0;
    printf("Last 20 commands from history of current Shell:\n");
    for (; i < q.counter; i++){
        printf("%s\n", q.buffer[i]);
    }
}

void deleteQueue(Queue q) {
    int i = 0;
    for (; i < q.counter; i++) {
        free(q.buffer[i]);
    }
    q.counter = 0;
}
