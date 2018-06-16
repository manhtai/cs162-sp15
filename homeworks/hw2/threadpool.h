#ifndef __THPOOL__
#define __THPOOL__

#include <pthread.h>
#include <stdlib.h>
#include "wq.h"

#define MAX_THREADS 64
#define MAX_QUEUE 65536

typedef struct thread_pool {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  wq_t *queue;
  void (*request_handler)(int);
  int thread_count;
  int shutdown;
} threadpool;

threadpool* thread_pool_init(int num_threads, wq_t* work_queue, void (*request_handler)(int));

int thread_pool_add(threadpool* thpool, int client_socket_number);

int thread_pool_shutdown(threadpool* thpool);

#endif
