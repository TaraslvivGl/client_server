#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define QUEUE_SIZE (5)
#define MSG_SIZE (20)

/* Structure queue with strict size */
struct SafeQueue
{
    char** queue;
    pthread_mutex_t mutex;
    //if queue is empty
    pthread_cond_t empty;
    //amount of elements in queue
    size_t counter;
};

/* Initialization of SafeQueue */
void init_queue(struct SafeQueue * squeue, unsigned int size) {
    pthread_mutex_init(&squeue->mutex, NULL);
    pthread_cond_init(&squeue->empty, NULL);

    squeue->queue = malloc(size * sizeof(char*));
    int i;
    for (i = 0; i < size; i++) {
        squeue->queue[i] = NULL;
    }
    squeue->counter = 0;
}

/* Pop element from queue */
void* mq_get(struct SafeQueue * squeue)
{
    int i;
    char* ret;
    pthread_mutex_lock(&squeue->mutex);

    //wait while queue is empty
    while(squeue->counter == 0) {
        pthread_cond_wait(&squeue->empty, &squeue->mutex);
    }

    ret = squeue->queue[0];
    // TODO: use memmove to cut first
    for(i = 0; i < squeue->counter-1; i++) {
        squeue->queue[i] = squeue->queue[i+1];
    }

    squeue->counter--;
    pthread_mutex_unlock(&squeue->mutex);

    return (void*)ret;
}

/*Push element to queue*/
bool mq_put(struct SafeQueue * squeue, const void* message)
{
    pthread_mutex_lock(&(squeue->mutex));
    bool ret = false;

    if(squeue->counter < QUEUE_SIZE)
    {
        // add new element
        squeue->queue[squeue->counter] = (char *)malloc(MSG_SIZE);
        memset(squeue->queue[squeue->counter], 0, MSG_SIZE);
        snprintf(squeue->queue[squeue->counter], MSG_SIZE, "%s", (char*)message);
        squeue->counter++;
        ret = true;
    }

    pthread_mutex_unlock(&squeue->mutex);
    // notify waiting thread
    pthread_cond_signal(&squeue->empty);

    return ret;
}

/*Print elements of a queue*/
void printQueue(struct SafeQueue * squeue)
{
    int i;
    if(squeue->counter != 0)
    {
        for (i = 0; i < squeue->counter; i++)
            printf("%s ", squeue->queue[i]);
    }
    else
    {
        printf("Queue is empty");
    }
}

/* Free allocated memory of a queue*/
void destroy(struct SafeQueue * squeue) {
    int i;
    for(i = 0; i < QUEUE_SIZE; i++) {
        if(squeue->queue[i] != NULL) {
            free(squeue->queue[i]);
        }
    }

    free(squeue->queue);
}

//Help structure to set queue and element into thread
struct PutArgs
{
    struct SafeQueue * que;
    void* elem;
};

/*Calls  mq_get and print gotten element from queue*/
void* getFromQueue(void * param)
{
    struct SafeQueue * queue = (struct SafeQueue *) param;

    char* popElem = (char*)mq_get(queue);
    printf("\nPop: %s", popElem);
}

/*Calls  mq_put and print putted element to queue*/
void* putToQueue(void* param)
{
    struct PutArgs * args = (struct PutArgs *) param;
    mq_put(args->que, args->elem);
    printf("\nPush: %s", (char*) args->elem);
}

/* Function tests how much element are left in queue
 * after push and pop actions and print them.
 *
 * param getThreads - amount of threads which pop element from a queue.
 * param putThreads - amount of threads which push element to a queue.*/
void testQueue(struct SafeQueue * queue, size_t getThreads, size_t putThreads);

int main() {

    struct SafeQueue que;
    size_t getThreads = 5;
    size_t putThreads = 5;

    printf("Test 1: %lu times push and pop.\nQueue shell be empty.\n", getThreads);
    testQueue(&que, getThreads, putThreads);

    //delay for test finishing
    sleep(1);

    getThreads = 7;
    putThreads = 10;
    printf("Test 2: %lu times pop and %lu times push.\nQueue shell contain %lu element(s)\n",
           getThreads, putThreads, putThreads - getThreads);
    testQueue(&que, getThreads, putThreads);

    //delay for test finishing
    sleep(1);
    return 0;
}

void testQueue(struct SafeQueue * queue, size_t getThreads, size_t putThreads)
{
    init_queue(queue, QUEUE_SIZE);
    size_t inputBuffSize = 12;

    int i;
    pthread_t id_get[getThreads];
    for(i = 0; i < getThreads; i++)
    {
        pthread_create(&id_get[i], NULL, getFromQueue, (void*)queue);
    }

    struct PutArgs * putElem = malloc(sizeof(struct PutArgs));
    char buff[inputBuffSize];
    pthread_t id_put[putThreads];
    for(i = 0; i < putThreads; i++)
    {
        snprintf(buff, inputBuffSize, "%s(%d)", "thread", i);
        putElem->que = queue;
        putElem->elem = (void*)buff;
        pthread_create(&id_put[i], NULL, putToQueue, (void*)putElem);
        pthread_join(id_put[i], NULL);
    }

    //show Queue
    printf("\n\nQueue:");
    printQueue(queue);
    printf("\n");

    //free memory
    free(putElem);
    destroy(queue);
}