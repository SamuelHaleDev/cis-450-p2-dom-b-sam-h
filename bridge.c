#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define NUM_ITERS 50

pthread_mutex_t mut; // mutex variable
pthread_cond_t cond; // condition variable

int currentDirec; // the current direction on the one lane bridge
int currentNumber; // the currentNumber of cars on the bridge
int CarNum = 0;

struct arg_struct { // so I can pass car number through in pthread_create
    int carNum;
};

/*Function prototype declarations*/
void *OneVehicle(void *arguments);
void CrossBridge(int direc, int carNum);
void ArriveBridge(int direc, int carNum);
void ExitBridge(int direc, int carNum);
bool isSafe(int direc);

// Purpose calling function from pthread_create, allows each thread to emulate car, enter, cross and exit bridge
void *OneVehicle(void *arguments) { 
    /*ENTRY POINT FOR THREADS | UNPACKS CAR NUMBER | SELECTS A RANDOM DIRECTION | CALLS 3 OTHER ENTRY POINTS*/
    int CarNum = *((int *) arguments);
    int direc = rand() % 2;
    ArriveBridge(direc, CarNum);
    CrossBridge(direc, CarNum);
    ExitBridge(direc, CarNum);
    pthread_exit(NULL);
    sched_yield();
}

// Purpose is to handle bridge arrival per pseudocode provided
void ArriveBridge(int direc, int carNum) {
    /*LOCK MUTEX | PRINT FOR DEBUGGING | LOOP WHILE IT IS NOT SAFE TO CROSS BRIDGE USING A CONDITION WAIT | AFTER THE WAIT INCREMENT NUMBER OF CARS ON THE BRIDGE*/
    pthread_mutex_lock(&mut);
    printf("Car %d arriving at bridge\n", carNum);
    while(!isSafe(direc)) {
        pthread_cond_wait(&cond, &mut);
    } 
    currentNumber++;
    pthread_mutex_unlock(&mut);
    sched_yield();
}

// Purpose is to handle bridge exit per pseudo code provided
void ExitBridge(int direc, int carNum) {
    /*LOCK MUTEX IN ORDER TO DO A CONDITION WAIT | DECREMENT NUMBER OF CARS ON BRIDGE | PRINT FOR DEBUGGING*/
    pthread_mutex_lock(&mut);
    currentNumber = currentNumber - 1;
    printf("Car %d exiting bridge. Current number of cars on the bridge %d\n", carNum, currentNumber);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mut);
    sched_yield();
}

// Purpose is to sense for bridge entrance safety using monitors direction and number of cars variable
bool isSafe(int direc) {
    if (currentNumber == 0) return true;
    else if((currentNumber < 3) && (currentDirec == direc)) return true;
    else return false;
}

// Purpose is mainly for debugging to see when a thread enters this portion of the code
void CrossBridge(int direc, int carNum) {
    /*PRINTS FOR CROSSING DEBUGGING PURPOSES*/
    pthread_mutex_lock(&mut);
    if(direc == 0) {
        printf("Car %d crossing bridge heading North. Current number of cars on bridge: %d\n", carNum, currentNumber);
    }
    else {
        printf("Car %d crossing bridge heading South. Current number of cars on bridge: %d\n", carNum, currentNumber);
    }
    pthread_mutex_unlock(&mut);
    sched_yield();
}

// Purpose is to drive the function. Initializes our args, monitor and 50 children threads
int main() {

    int numChildren = 50; // number of threads to create
    pthread_t id[numChildren]; // Create an array to store pthread_create pids/cids
    /*INTIIALIZE MUTEX AND CONDITION FOR MONITOR*/
    pthread_mutex_init(&mut, NULL);
    pthread_cond_init(&cond, NULL);
    /*INITIALIZE currentNumber global variable*/
    currentNumber = 0;
    
    for(int i = 0; i < NUM_ITERS; i++) {
        /*SYNTAX TO PASS IN AN ARGUMENT TO PTHREAD_CREATE ENTRY POINT*/
        int *arg = (int *)malloc(sizeof(*arg));
        *arg = i+1;
        /*LAST ARGUMENT PASSES IN i+1 which is car number*/
        pthread_create(&id[i], NULL, &OneVehicle, arg);
    }
    for(int i = 0; i < numChildren; i++) {
        pthread_join(id[i], NULL);
    }
    free(arg); // deletes heap allocated memory
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mut);
    pthread_exit(0);
    return 0;
}