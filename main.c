#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h> // for wait()


int main(int argc, char *argv[])
{
  key_t kluczSem, kluczMem; // klucz do semaforow i pam. dzielonej
  int semID;            // identyfikator zestawu semaforow
  int SemCount = 6;
  int i;
  int shmID;
  char bufor[3];
  int *widelec;

  kluczSem = ftok(".", 'A');
  semID = semget(kluczSem, SemCount, IPC_CREAT | IPC_EXCL | 0666);
  if(semID == -1){
	perror("Blad semget");
  }
  for (i = 0; i < 5; i++)
    semctl(semID, i, SETVAL, 1);
  semctl(semID, 5, SETVAL, 4);
  kluczMem = ftok(".", 'B');
  shmID = shmget(kluczMem, 5 * sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
  if (shmID == -1)
  {
    perror("blad pamieci dzielonej");
    exit(1);
  }
  widelec = (int *)shmat(shmID, NULL, 0); // przydzielenie pam. dz.
  for (i = 0; i < 5; i++)
    widelec[i] = 0;
  for (i = 0; i < 5; i++)
    switch (fork())
    {
    case -1:
      perror("Blad fork");
      exit(2);
    case 0:
      sprintf(bufor, "%d", i);
      execl("./fil", "fil", bufor, NULL);
    }
  for (i = 0; i < 5; i++)
    wait(NULL);

  semctl(semID, SemCount, IPC_RMID, NULL);
  shmctl(shmID, IPC_RMID, NULL);
  printf("Wykonano zadania liczbe iteracji koniec progarmu\n");
}
