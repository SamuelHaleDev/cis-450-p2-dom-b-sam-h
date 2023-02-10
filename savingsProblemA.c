#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

/*MONITOR VARIABLES*/
pthread_mutex_t mut;
pthread_cond_t cond;

float savingsAmount;
int numCustIn;
int numCustWaiting;

bool isSafe() {
    /*CHECKS TO MAKE SURE ITS SAFE TO STEP INTO A CS*/
    if (numCustIn > 0) {
        return false; // returns false as there is somebody writing currently
    } else return true;
}

// Write the withdrawal method
void *withdraw(void *args) {
    /*FUNCTIONS VIRTUALLY THE SAME AS WITHDRAW | WHILE THERE ARE NOT ENOUGH FUNDS WAIT FOR THERE TO BE ENOUGH FUNDS*/
    pthread_mutex_lock(&mut);
    float amount = *((float *) args);
    while(!isSafe() || (savingsAmount - amount) < 0) {
        if ((savingsAmount - amount) < 0) {
            printf("There are not enough funds to complete this withdraw\n");
        }
        pthread_cond_wait(&cond, &mut);
    }
    numCustIn += 1;
    savingsAmount -= amount;
    printf("Customer is withdrawing %f new Bank balance %f\n", amount, savingsAmount);
    numCustIn -= 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mut);
    sched_yield();
}
// Write the deposit method
void *deposit(void *args) { // args is amount
    /*GRAB MUTEX | CHECK IF ITS SAFE TO STEP IN | WRITE YOUR SAVINGSAMOUNT CHANGES | HANDLE THE NUMBER OF CUSTOMERS IN VARIABLE | BROADCAST AND UNLOCK MUTEX*/
    pthread_mutex_lock(&mut);
    while(!isSafe()) {
        pthread_cond_wait(&cond, &mut);
    }
    numCustIn++;
    float amount = *((float *) args);
    savingsAmount += amount; // savingsAmount is a global variable where amount is passed in from input collected in thread entry point
    printf("Customer is depositing %f new Bank balance %f\n", amount, savingsAmount);
    numCustIn--;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mut);
    sched_yield();
}

int main() {
    /*INTIALIZE MUTEX AND CONDITION VARIABLES TO ACHIEVE MONITOR FUNCTIONALITY*/
    pthread_mutex_t mut;
    pthread_mutex_init(&mut, NULL);
    pthread_cond_t cond;
    pthread_cond_init(&cond, NULL);
    /*INITIALIZE SAVINGS AMOUNT AND NUMCUSTOMERS IN FOR OUR SHARED BANK ACCOUNT AND ISSAFE FUNCTION*/
    savingsAmount = 0;
    numCustIn = 0;
    pthread_t id[50];
    float amount = 0; // VARIABLE FOR USERINPUT ON WITHDRAWS AND DEPOSITS
    char userInput = 'q'; // VARIABLE FOR MENU AND BRANCHING USEAGE
    int i = 0; // USED TO CREATE OUR THREADS 

    /*MENU*/
    printf("Enter d to deposit | Enter w to withdraw | Enter f to exit: ");
    scanf(" %c", &userInput); // gets userinput 
    printf("\n");
    while (userInput != 'f') {
        /*ARG WILL BE PASSED TO OUR PTHREAD_CREATE ENTRY POINT FUNCTION*/
        float *arg = (float *)malloc(sizeof(*arg)); 
        if (userInput == 'd') { // D FOR DEPOSIT | PROMPTS USER FOR AMOUNT INPUT | CREATES THE THREAD AND CARRIES OUT TRANSACTION | SLEEPS FOR A SECOND SO THE OUTPUT WILL BE FORMATTED NICELY | INCREMENTS I FOR NEXT THREAD CREATION
            printf("Please enter an amount to deposit into your savings account: ");
            scanf(" %f.2", &amount);
            printf("\n");
            *arg = amount;
            pthread_create(&id[i], NULL, deposit, arg);
            sleep(1); // sleep for 1 seconds while new thread does its job
            i++;
        } else if (userInput == 'w') { // FUNCTIONS THE SAME AS A DEPOSIT
            printf("please enter an amount to withdraw from your savings account: ");
            scanf(" %f.2", &amount);
            printf("\n");
            *arg = amount;
            pthread_create(&id[i], NULL, withdraw, arg);
            sleep(1);
            i++;
        } else if (userInput == 'f') { // EXITS LOOP
            printf("Goodbye!\n");
            break;
        }
        /*PUT THE PROMPT DOWN HERE IN AN ATTEMPT TO RESOLVE SOME OUTPUT BUGS I HAD*/
        printf("Enter d to deposit | Enter w to withdraw | Enter f to exit: ");
        scanf(" %c", &userInput); // gets userinput 
        printf("\n");
    }
    /*JOIN ALL THREADS*/
    for (int x = 0; x < i; x++) {
        pthread_join(id[x], NULL);
    }

    pthread_exit(0);
}