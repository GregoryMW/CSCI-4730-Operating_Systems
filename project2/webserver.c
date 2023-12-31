#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "webserver.h"

#define MAX_REQUEST 100

int port, numThread;

int request[MAX_REQUEST]; // shared buffer
int count = 0;

// semaphores and a mutex lock
sem_t sem_full;
sem_t sem_empty;
pthread_mutex_t mutex;


void req_handler()
{
  int r;
  struct sockaddr_in sin;
  struct sockaddr_in peer;
  int peer_len = sizeof(peer);
  int sock;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  r = bind(sock, (struct sockaddr *) &sin, sizeof(sin));
  if(r < 0) {
    perror("Error binding socket:");
    exit(0);
  }

  r = listen(sock, 5);
  if(r < 0) {
    perror("Error listening socket:");
    exit(0);
  }

  printf("HTTP server listening on port %d\n", port);

  while (1)
    {
      int s;
      s = accept(sock, NULL, NULL);
      if (s <= 0)
	break;
      else
	producer(s);
    }
  close(sock);
}

void producer(int s)
{
  sem_wait(&sem_empty);
  pthread_mutex_lock(&mutex);
  request[count] = s;
  count++;
  pthread_mutex_unlock(&mutex);
  sem_post(&sem_full);
}

void consumer()
{
  sem_wait(&sem_full);
  pthread_mutex_lock(&mutex);
  count --;
  int i = request[count];
  pthread_mutex_unlock(&mutex);
  sem_post(&sem_empty);
  process(i);
}


int main(int argc, char *argv[])
{
  if(argc < 2 || atoi(argv[1]) < 2000 || atoi(argv[1]) > 50000)
    {
      fprintf(stderr, "./webserver PORT(2001 ~ 49999) (#_of_threads) (crash_rate(%))\n");
      return 0;
    }
		
  // port number
  port = atoi(argv[1]);
		
  // # of worker thread
  if(argc > 2) 
    numThread = atoi(argv[2]);
  else numThread = 1;

  // crash rate
  if(argc > 3) 
    CRASH = atoi(argv[3]);
  if(CRASH > 50) CRASH = 50;
		
  sem_init(&sem_empty, 0, MAX_REQUEST);
  sem_init(&sem_full, 0, 0);

  printf("[pid %d] CRASH RATE = %d\%\n", getpid(), CRASH);

  pthread_t producer;
  pthread_t threads[numThread];
  int num = 0;

  pthread_create(&producer, NULL, req_handler, NULL);
  for (int i = 0; i < MAX_REQUEST; i ++)
    {
      if (num == numThread)
	num = 0;
      pthread_create(&threads[num], NULL, consumer, NULL);
      pthread_detach(threads[num]);
      num++;
    }
  pthread_join(producer, NULL);

  return 0;
}

