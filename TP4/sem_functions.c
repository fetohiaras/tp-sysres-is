#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>

//#include "segdef.h"

#ifndef _SEM_SEMUN_UNDEFINED
#else
union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};
#endif

static void do_semop(int semid, unsigned short semnum, short delta, int flags)
{
    // normally already in sys/sem.h
    struct sembuf sop;
    sop.sem_num = semnum;   
    sop.sem_op  = delta;    
    sop.sem_flg = flags;    

    for (;;) {
        if (semop(semid, &sop, 1) == 0)
            return;
        if (errno == EINTR)   
            continue;
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

void acq_sem(int semid, int sem_index)
{
    do_semop(semid, (unsigned short)sem_index, -1, 0);
}

void lib_sem(int semid, int sem_index)
{
    do_semop(semid, (unsigned short)sem_index, +1, 0);
}

void wait_sem(int semid, int sem_index)
{
    do_semop(semid, (unsigned short)sem_index, -1, 0);
}

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
