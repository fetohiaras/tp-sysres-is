#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "segdef.h"

struct sembuf sop;
segment *seg;

long local_average(long t[], int n) {
    long s = 0;
    for(int i = 0; i < n; i++)
        s += t[i];
    return s / n;
}

void init_all(int *shmid, int *semid) {
    *shmid = shmget(cle, segsize, 0);
    if(*shmid == -1) {
        perror("shmget");
        exit(1);
    }

    *semid = semget(cle, 3, 0);
    if(*semid == -1) {
        perror("semget");
        exit(1);
    }

    seg = (segment*) shmat(*shmid, NULL, 0);
    if(seg == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    init_rand();
}

int main() {
    int shmid, semid;
    init_all(&shmid, &semid);

    for(int req = 0; req <= 5; req++) {

        acq_sem(semid, seg_dispo);

        seg->pid = getpid();
        seg->req = req;

        for(int i = 0; i < maxval; i++)
            seg->tab[i] = getrand();

        long avg_client = local_average(seg->tab, maxval);

        acq_sem(semid, seg_init);
        wait_sem(semid, res_ok);

        long avg_server = seg->result;

        lib_sem(semid, seg_init);
        wait_sem(semid, res_ok);
        lib_sem(semid, seg_dispo);

        printf("\n  REQUEST %d \n", req);
        printf("PID : %d\n", seg->pid);
        printf("Client average  = %ld\n", avg_client);
        printf("Server average  = %ld\n", avg_server);

    }

    shmdt(seg);
    return 0;
}
