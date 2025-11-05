#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>

void init_rand(){

    srand(time(NULL) + getpid());

}

long getrand(){

    long r = rand();
    r = r % 1000;
    if (rand() % 2) r = -r;
    return r;

}


void test_rand() {
    init_rand();
    printf("Test of getrand() :\n");
    for (int i = 0; i < 10; i++) {
        printf("%ld ", getrand());
    }
    printf("\n");
}

int main() {
    test_rand();
    return 0;
}
