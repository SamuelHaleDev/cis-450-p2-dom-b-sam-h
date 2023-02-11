/*
 * boundedBuffer.c
 *
 * A complete example of simple producer/consumer program. The Producer
 * and Consumer functions are executed as independent threads.  They
 * share access to a single buffer, data.  The producer deposits a sequence
 * of integers from 1 to numIters into the buffer.  The Consumer fetches
 * these values and adds them.  Two semaphores,empty and full are used to
 * ensure that the producer and consumer alternate access to the buffer.
 *
 * SOURCE: adapted from example code in "Multithreaded, Parallel, and
 *         Distributed Programming" by Gregory R. Andrews.
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define SHARED 1

void *Producer (void *); // the two threads
void *Consumer (void *);

sem_t empty, full, mutex;       //global semaphores
int *data;                // shared buffer, size = 1
int front = 0, rear = 0;
int numIters;

// main() -- read command line and create threads
int main(int argc, char *argv[]) {
    pthread_t pid1, pid2, pid3, cid1, cid2, cid3;
    data = (int *)malloc(sizeof(int)*atoi(argv[2])); // dynamically allocates buffer with specified size

    sem_init(&empty, SHARED, 1);    // sem empty = 1
    sem_init(&full, SHARED, 0); //sem full = 0
    sem_init(&mutex, SHARED, 1); // mutex = 1

    if (argc < 2) {
	    printf("Usage: boundedBuffer <Number of Iterations>\n");
	    exit(0);
    }
    numIters = atoi(argv[1]);

    /*CREATE 3 PRODS AND 3 CONS's PER INSTRUCTION*/
    pthread_create(&pid1, NULL, Producer, NULL);
    pthread_create(&pid2, NULL, Producer, NULL);
    pthread_create(&pid3, NULL, Producer, NULL);
    pthread_create(&cid1, NULL, Consumer, NULL);
    pthread_create(&cid2, NULL, Consumer, NULL);
    pthread_create(&cid3, NULL, Consumer, NULL);

    /*JOIN THREADS*/
    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    pthread_join(pid3, NULL);
    pthread_join(cid1, NULL);
    pthread_join(cid2, NULL);
    pthread_join(cid3, NULL);
    free(data); // FREE HEAP ALLOCATED MEMORY
    pthread_exit(0);
}

// deposit 1, ..., numIters into the data buffer
void *Producer(void *arg) {
    int produced;

    /*USE SEM WAITS TO ALLOW ATOMIC READING AND WRITING*/
    for (produced = 0; produced < numIters; produced++) {
        sem_wait(&empty);
        sem_wait(&mutex);
        data[rear] = produced;
        rear = (rear+1)%sizeof(data);
        sem_post(&mutex);
        sem_post(&full);
    }
}

//fetch numIters items from the buffer and sum them
void *Consumer(void *arg) {
    int total = 0;
    int consumed;
    /*VIRTUALLY SAME AS PRODUCER*/
    for (consumed = 0; consumed < numIters; consumed++) {
        sem_wait(&full);
        sem_wait(&mutex);
        total = total + data[front];
        front = (front+1)%sizeof(data);
        sem_post(&mutex);
        sem_post(&empty);
    }

    printf("the total is %d\n", total);
}
