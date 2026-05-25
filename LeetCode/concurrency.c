#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

typedef struct {
    int n;

    int curr;

    sem_t sem_fizz;
    sem_t sem_buzz;
    sem_t sem_fizzbuzz;
    sem_t sem_number;

} FizzBuzz;

FizzBuzz* fizzBuzzCreate(int n) {

    FizzBuzz* obj = (FizzBuzz*) malloc(sizeof(FizzBuzz));

    obj->n = n;
    obj->curr = 1;

    // number thread starts first
    sem_init(&obj->sem_number, 0, 1);

    sem_init(&obj->sem_fizz, 0, 0);
    sem_init(&obj->sem_buzz, 0, 0);
    sem_init(&obj->sem_fizzbuzz, 0, 0);

    return obj;
}


// ======================================
// DO NOT CHANGE
// ======================================

void printNumber(int a) {
    printf("%d ", a);
}

void printFizz() {
    printf("fizz ");
}

void printBuzz() {
    printf("buzz ");
}

void printFizzBuzz() {
    printf("fizzbuzz ");
}
// Threads
void fizz(FizzBuzz* obj) {
    while(1) {
        sem_wait(&obj->sem_fizz);
        printFizz(obj->curr);
        obj->curr++;
        sem_post(&obj->sem_number);
    }
}

void buzz(FizzBuzz* obj) {
    while(1) {
        sem_wait(&obj->sem_buzz);
        printFizz(obj->curr);
        obj->curr++;
        sem_post(&obj->sem_number);
    }
}

void fizzbuzz(FizzBuzz* obj) {
    while(1) {
        sem_wait(&obj->sem_fizzbuzz);
        printFizz(obj->curr);
        obj->curr++;
        sem_post(&obj->sem_number);
    }
}

void number(FizzBuzz* obj) {
    while (obj->curr <= obj->n){
        sem_wait(&obj->sem_number);
        if (obj->curr % 3 == 0 && obj->curr % 5 == 0){
            sem_post(&obj->sem_fizzbuzz);
        } else if (obj->curr % 3 == 0){
            sem_post(&obj->sem_fizz);
        } else if (obj->curr % 5 == 0) {
            sem_post(&obj->sem_buzz);
        } else {
            printNumber(obj->curr);
            obj->curr++;
            sem_post(&obj->sem_number);
        }
    }
    // wake everyone so threads can exit
    sem_post(&obj->sem_fizz);
    sem_post(&obj->sem_buzz);
    sem_post(&obj->sem_fizzbuzz);
}


// ======================================
// free
// ======================================

void fizzBuzzFree(FizzBuzz* obj) {

    sem_destroy(&obj->sem_fizz);
    sem_destroy(&obj->sem_buzz);
    sem_destroy(&obj->sem_fizzbuzz);
    sem_destroy(&obj->sem_number);

    free(obj);
}


// ======================================
// Thread Wrappers
// ======================================

void* fizzThread(void* arg) {
    fizz((FizzBuzz*) arg);
    return NULL;
}

void* buzzThread(void* arg) {
    buzz((FizzBuzz*) arg);
    return NULL;
}

void* fizzbuzzThread(void* arg) {
    fizzbuzz((FizzBuzz*) arg);
    return NULL;
}

void* numberThread(void* arg) {
    number((FizzBuzz*) arg);
    return NULL;
}


// ======================================
// MAIN
// ======================================

int main() {

    int n = 20;

    FizzBuzz* obj = fizzBuzzCreate(n);

    pthread_t t1, t2, t3, t4;

    pthread_create(&t1, NULL, fizzThread, obj);
    pthread_create(&t2, NULL, buzzThread, obj);
    pthread_create(&t3, NULL, fizzbuzzThread, obj);
    pthread_create(&t4, NULL, numberThread, obj);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    printf("\n");

    fizzBuzzFree(obj);

    return 0;
}
