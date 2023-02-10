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
int *FIFO;

int front;
int rear;

struct args {
    int customerNumber;
    float amount;
};

bool isSafe() {
    /*CHECKS TO MAKE SURE ITS SAFE TO STEP INTO A CS*/
    if (numCustIn > 0) {
        return false; // returns false as there is somebody writing currently
    } else return true;
}

// Write the withdrawal method
void *withdraw(void *arg) {
    /*FUNCTIONS VIRTUALLY THE SAME AS WITHDRAW | WHILE THERE ARE NOT ENOUGH FUNDS WAIT FOR THERE TO BE ENOUGH FUNDS*/
    pthread_mutex_lock(&mut);
    /*UNPACK ARGUMENTS*/
    float amount = (((struct args*)arg)->amount);
    int custNum = (((struct args*)arg)->customerNumber);
    /*LOGIC DIFFERS FROM PART A NOW IF THE CUSTOMER ISNT AT THE FRONT OF THE QUEUE THEY WILL STILL HAVE TO WAIT*/
    while(!isSafe() || (savingsAmount - amount) < 0 || custNum != FIFO[front]) {
        FIFO[rear] = custNum;
        rear  = (rear + 1 ) % 50; // Update the rear
        if ((savingsAmount - amount) < 0) {
            printf("There are not enough funds to complete this withdraw\n");
        }
        pthread_cond_wait(&cond, &mut);
    }
    numCustIn += 1;
    savingsAmount -= amount;
    printf("Customer is withdrawing %f new Bank balance %f\n", amount, savingsAmount);
    numCustIn -= 1;
    if (custNum == FIFO[front]) front = (front + 1) % 50; // After the front customer has went you can update the front and step out because CS is over after this line
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mut);
    sched_yield();
}
// Write the deposit method
void *deposit(void *arg) { // args is amount
    /*GRAB MUTEX | CHECK IF ITS SAFE TO STEP IN | WRITE YOUR SAVINGSAMOUNT CHANGES | HANDLE THE NUMBER OF CUSTOMERS IN VARIABLE | BROADCAST AND UNLOCK MUTEX*/
    pthread_mutex_lock(&mut);
    while(!isSafe()) {
        pthread_cond_wait(&cond, &mut);
    }
    numCustIn++;
    float amount = (((struct args*)arg)->amount);
    int custNum = (((struct args*)arg)->customerNumber);
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
    FIFO = (int *)malloc(sizeof(int)*50);      // allocate 50 ints
    struct args *tempArguments = (struct args *)malloc(sizeof(struct args));
    front = 0;
    rear = 0;

    /*MENU*/
    printf("Enter d to deposit | Enter w to withdraw | Enter f to exit: ");
    scanf(" %c", &userInput); // gets userinput 
    printf("\n");
    while (userInput != 'f') {
        /*ARG WILL BE PASSED TO OUR PTHREAD_CREATE ENTRY POINT FUNCTION*/
        if (userInput == 'd') { // D FOR DEPOSIT | PROMPTS USER FOR AMOUNT INPUT | CREATES THE THREAD AND CARRIES OUT TRANSACTION | SLEEPS FOR A SECOND SO THE OUTPUT WILL BE FORMATTED NICELY | INCREMENTS I FOR NEXT THREAD CREATION
            printf("Please enter an amount to deposit into your savings account: ");
            scanf(" %f.2", &amount);
            printf("\n");
            /*ARGUMENTS TO BE PASSED INTO ENTRY POINT*/
            tempArguments->amount = amount;
            tempArguments->customerNumber = i;
            pthread_create(&id[i], NULL, deposit, (void *)tempArguments);
            sleep(1); // sleep for 1 seconds while new thread does its job
            i++;
        } else if (userInput == 'w') { // FUNCTIONS THE SAME AS A DEPOSIT
            printf("please enter an amount to withdraw from your savings account: ");
            scanf(" %f.2", &amount);
            printf("\n");
            tempArguments->amount = amount;
            tempArguments->customerNumber = i;
            pthread_create(&id[i], NULL, withdraw, (void *)tempArguments);
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