#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>

void SignalSemaphore(int semaphoreID, int semaphoreNumber);

int WaitSemaphore(int semaphoreID, int semaphoreNumber, int flags);

int main(int argc, char* argv[]) {
    key_t semaphoreKey, sharedMemoryKey;
    int semaphoreID;
    int numberOfSemaphores = 6;
    int i;
    int philosopherNumber;
    int sharedMemoryID;
    int* forks;

    semaphoreKey = ftok(".", 'A');
    if (semaphoreKey == -1) {
        printf("Blad ftok fil\n");
        exit(1);
    }
    semaphoreID = semget(semaphoreKey, numberOfSemaphores, IPC_CREAT | 0666);

    sharedMemoryKey = ftok(".", 'B');
    sharedMemoryID = shmget(sharedMemoryKey, 5 * sizeof(int), IPC_CREAT | 0666);
    if (sharedMemoryID == -1) {
        printf("blad pamieci dzielone fil\n");
        exit(1);
    }
    forks = (int*)shmat(sharedMemoryID, 0, 0);

    philosopherNumber = atoi(argv[1]);

    for (i = 1; i <= 10; i++) {
        printf("Filozof %d mysli\n", philosopherNumber);
        WaitSemaphore(semaphoreID, 5, SEM_UNDO); // jadalna

        printf("filozof %d wchodzi do jadalni %d raz\n", philosopherNumber, i);
        WaitSemaphore(semaphoreID, philosopherNumber, SEM_UNDO);           // lewy widelec
        WaitSemaphore(semaphoreID, (philosopherNumber + 1) % 5, SEM_UNDO); // prawy widelec modulo zeby wyodrebnic ten po prawej

        forks[(philosopherNumber + 1) % 5] = philosopherNumber;
        forks[philosopherNumber] = philosopherNumber;
        printf("\nFilozof %d je\n", philosopherNumber);

        printf("\n\n---Stan Widelcow---\n");
        for (int j = 0; j < 5; j++)
            printf("%d ", forks[j]);
        printf("\n");

        sleep(philosopherNumber);

        forks[(philosopherNumber + 1) % 5] = 0;
        forks[philosopherNumber] = 0;

        SignalSemaphore(semaphoreID, philosopherNumber);
        SignalSemaphore(semaphoreID, (philosopherNumber + 1) % 5);
        SignalSemaphore(semaphoreID, 5);

        printf("filozof %d wychodzi z jadalni\n", philosopherNumber);
    }
}

void SignalSemaphore(int semaphoreID, int semaphoreNumber) {
    struct sembuf operations;

    operations.sem_num = semaphoreNumber;
    operations.sem_op = 1;
    operations.sem_flg = SEM_UNDO;

    if (semop(semaphoreID, &operations, 1) == -1) {
        perror("Blad SignalSemaphore: ");
        exit(1);
    }
}

int WaitSemaphore(int semaphoreID, int semaphoreNumber, int flags) {
    struct sembuf operations;
    operations.sem_num = semaphoreNumber;
    operations.sem_op = -1;
    operations.sem_flg = 0 | flags; // SEM_UNDO;

    if (semop(semaphoreID, &operations, 1) == -1) {
        perror("Blad WaitSemaphore: ");
        return -1;
    }

    return 1;
}
