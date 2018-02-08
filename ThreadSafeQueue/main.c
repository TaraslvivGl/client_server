#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define QUEUE_SIZE (5)
#define MSG_SIZE (80)

struct safeQueue{
    char** queue;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    size_t counter;
} sQueue;

void init_queue(unsigned int size) {
    sQueue.queue = malloc(size * sizeof(char*));
    int i;
    for (i = 0; i < size; i++) {
        sQueue.queue[i] = NULL;
    }
    sQueue.counter = 0;
}

void* mq_get() {
    int i;
    char* ret;

    pthread_mutex_lock(&sQueue.mutex);

    while(sQueue.counter == 0) {
        pthread_cond_wait(&sQueue.cv, &sQueue.mutex);
    }

    ret = sQueue.queue[0];
    // TODO: use memmove to cut first
    for(i = 0; i < sQueue.counter-1; i++) {
        sQueue.queue[i] = sQueue.queue[i+1];
    }

    sQueue.counter--;
    pthread_mutex_unlock(&sQueue.mutex);

    return (void*)ret;
}

bool mq_put(const void* message)
{
    pthread_mutex_lock(&sQueue.mutex);

    bool ret = false;

    if(sQueue.counter < QUEUE_SIZE)
    {
        sQueue.queue[sQueue.counter] = (char *)malloc(MSG_SIZE);
        memset(sQueue.queue[sQueue.counter], 0, MSG_SIZE);
        snprintf(sQueue.queue[sQueue.counter], MSG_SIZE,"%s", (char*)message);
        sQueue.counter++;
        ret = true;
    }

    pthread_mutex_unlock(&sQueue.mutex);
    // wake up waiting thread
    pthread_cond_signal(&sQueue.cv);

    return ret;
}

void printQueue()
{
    int i;
    if(sQueue.counter != 0)
    {
        for (i = 0; i < sQueue.counter; i++)
            printf("%s ", sQueue.queue[i]);
    }
    else
    {
        printf("Queue is empty");
    }
}

void destroy() {
    int i;
    for(i = 0; i < QUEUE_SIZE; i++) {
        if(sQueue.queue[i] != NULL) {
            free(sQueue.queue[i]);
        }
    }

    free(sQueue.queue);
}

//Help functions:
void* getFromQueue(void* returnValue)
{
    printf("\nPop: %s", (char*)mq_get());
}

void* putToQueue(void* param)
{
    mq_put(param);
    printf("\nPush: %s", (char*)param);
}


/*The same amount of thread to read and write
 * Queue shell be empty*/
void testEmptyQueue();

/* 7 threads take from queue and 10 put into queue
 * Queue shell be not empty. 3 elements shell be left*/
void testNotEmptyQueue();

int main() {

    printf("Test 1 starts\n");
    testEmptyQueue();

    //for test finishing
    sleep(1);

    printf("Test 2 starts\n");
    testNotEmptyQueue();

    //for test finishing
    sleep(1);
    return 0;
}

void testEmptyQueue()
{
    init_queue(QUEUE_SIZE);

    size_t getThreds = 5;
    size_t putThreds = 5;
    size_t inputBuffSize = 12;

    int i;
    pthread_t id_get[getThreds];
    for(i = 0; i < getThreds; i++)
    {
        pthread_create(&id_get[i], NULL, getFromQueue, NULL);
    }

    char* buff = malloc(inputBuffSize);
    pthread_t id_put[putThreds];
    for(i = 0; i < putThreds; i++)
    {
        snprintf(buff, inputBuffSize, "%s(%d)", "thread", i);
        pthread_create(&id_put[i], NULL, putToQueue, (void*)buff);
        pthread_join(id_put[i], NULL);
    }

    //show Queue
    printf("\n\nQueue:");
    printQueue();
    printf("\n");

    //free memory
    free(buff);
    destroy();
}

void testNotEmptyQueue()
{
    init_queue(QUEUE_SIZE);

    size_t getThreds = 7;
    size_t putThreds = 10;
    size_t inputBuffSize = 12;

    int i;
    pthread_t id_get[getThreds];
    for(i = 0; i < getThreds; i++)
    {
        pthread_create(&id_get[i], NULL, getFromQueue, NULL);
    }

    char* buff = malloc(inputBuffSize);
    pthread_t id_put[putThreds];
    for(i = 0; i < putThreds; i++)
    {
        snprintf(buff, inputBuffSize, "%s(%d)", "thread", i);
        pthread_create(&id_put[i], NULL, putToQueue, (void*)buff);
        pthread_join(id_put[i], NULL);
    }

    //show Queue
    printf("\n\nQueue:");
    printQueue();
    printf("\n");

    //free memory
    free(buff);
    destroy();
}