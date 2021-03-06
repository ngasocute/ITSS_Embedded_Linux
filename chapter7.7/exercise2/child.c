#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>

#define LOOP_ON 1
#define LOOP_OFF 0

#define SHIMSIZE 10

#define MSGSIZE 100
#define KEYFILE_PATH "filepath"
#define ID 'M'
#define ID2 'S'
#define MSGQ_OK 0
#define MSGQ_NG -1

struct msgbuff
{
  long mtype;
  pid_t pid;
} message;

int msqid;
int semid;
int shmid;
int *ptr;

int loopFlag;

int st;

void sigHandleSigUsr1(int signo);

void doprocessing();

int main()
{
  //printf("main: start\n");
  time_t t;

  pid_t pid = 0, wpid;
  int i;
  int status;
  int readSize = 0;

  key_t keyx;
  struct msqid_ds msq;

  int semval;
  key_t keyval;

  keyx = ftok(KEYFILE_PATH, (int)ID);

  msqid = msgget(keyx, 0666 | IPC_CREAT);

  if (msqid == -1)
  {
    perror("msgget");
    exit(1);
  }

  keyval = ftok(KEYFILE_PATH, (int)ID2);

  // Semaphore ID acquisition
  semid = semget(keyval, 1, IPC_CREAT | 0660);

  // Control of semaphore (semaphore value acquisition)
  if (semctl(semid, 0, SETVAL, 4) == -1)
  {
    perror("semctl");
    exit(1);
  }

  //Get the shared memory ID
  if ((shmid = shmget(keyx, SHIMSIZE * sizeof(int), IPC_CREAT | 0660)) == -1)
  {
    perror("shmget");
    exit(1);
  }

  //Attach the shared memory
  ptr = (int *)shmat(shmid, 0, 0);
  if (ptr == (int *)-1)
  {
    perror("shmat");
    exit(1);
  }

  *ptr = *(ptr + 1) = 0;

  srand((unsigned)time(&t));

  for (i = 1; i <= 3; i++)
  {
    // only for checking if there are enough process forked
    // st = rand() % 5 + 1;
    // printf("%d\n", st);

    pid = fork();
    if (pid < 0)
    {
      perror("fork");
      exit(1);
    }
    else if (pid == 0)
    {
      // each spawned proceess will doprocessing()
      // which will examine 2 variables in the shared memory
      // and assign values to them according to some (dummy) conditions
      doprocessing();
      exit(0);
    }
  }
  return 0;
}

void doprocessing()
{
  struct sembuf buff;

  sleep(st);

  message.mtype = 1;
  message.pid = getpid();
  if ((msgsnd(msqid, (void *)&message, sizeof(pid_t), 0)) == MSGQ_NG)
  {
    perror("msgsnd");
    exit(1);
  }

  // by this signal handler, the processes will know if it is its turn to change the variable
  signal(SIGUSR1, sigHandleSigUsr1);
  loopFlag = LOOP_ON;
  while (loopFlag)
    ;

  // Sembuf structure setting for semaphore operation
  buff.sem_num = (ushort)0;
  buff.sem_op = (short)-4;
  buff.sem_flg = (short)0;

  //Semaphore operation
  semop(semid, &buff, 1);
  printf("locked semaphore\n");

  int *x = ptr;
  int *y = ptr + 1;
  printf("%d, %d\n", *x, *y);
  if (*x == 0 && *y == 0)
    *x = 10;
  else if (*x == 0 && *y == 10)
    *y = 0;
  else if (*x == 10 && *y == 0)
  {
    *x = 0;
    *y = 10;
  }
  //sleep(5);

  // Sembuf structure setting for semaphore operation
  buff.sem_num = (ushort)0;
  buff.sem_op = (short)4;
  buff.sem_flg = (short)0;

  //Semaphore operation
  semop(semid, &buff, 1);

  printf("unlocked semaphore\n");

  message.mtype = 1;
  message.pid = 1;
  if ((msgsnd(msqid, (void *)&message, sizeof(pid_t), 0)) == MSGQ_NG)
  {
    perror("msgsnd");
    exit(1);
  }
}

void sigHandleSigUsr1(int signo)
{
  if (signo == SIGUSR1)
  {
    printf("Signal No. %d\n", getpid());
    loopFlag = LOOP_OFF;
  }
}