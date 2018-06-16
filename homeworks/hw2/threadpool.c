#include <stdio.h>
#include <unistd.h>

#include "wq.h"
#include "threadpool.h"

static void *thread_pool_routine(void *thpool);

/* Init thread pool for processing queue */
threadpool* thread_pool_init(int num_threads, wq_t* work_queue, void (*request_handler)(int)) {

  if(num_threads <= 0 || num_threads > MAX_THREADS) {
      return NULL;
  }

  /* Init pool */
  threadpool* pool;
  pool = (struct thread_pool*) malloc(sizeof(struct thread_pool));
  if (pool == NULL) {
    perror("Init pool error");
    return NULL;
  }

  /* Init params */
  pool->thread_count = 0;
  pool->shutdown = 0;
  pool->request_handler = request_handler;
  pool->queue = work_queue;

  /* Threads */
  pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
  if (pool->threads == NULL) {
    perror("Init threads error");
    return NULL;
  }

  /* Init lock and conditional variable */
  if((pthread_mutex_init(&(pool->lock), NULL) != 0) || (pthread_cond_init(&(pool->notify), NULL) != 0)){
    perror("Init lock or conditional var error");
    return NULL;
  }

  /* Start worker threads */
  for(int i = 0; i < num_threads; i++) {
      if(pthread_create(&(pool->threads[i]), NULL, thread_pool_routine, (void*)pool) != 0) {
          thread_pool_shutdown(pool);
          return NULL;
      }
      pool->thread_count++;
  }

  return pool;
}

/* Add client_socket_number to queue for processing */
int thread_pool_add(threadpool* pool, int client_socket_number) {

  if(pool == NULL) {
    perror("thread_pool_add(): Pool is null.\n");
    return -1;
  }

  if(pthread_mutex_lock(&(pool->lock)) != 0) {
    perror("thread_pool_add(): Can't lock.\n");
    return -1;
  }

  do {
    if(pool->queue->size == MAX_QUEUE) {
      perror("Queue full.\n");
      break;
    }

    if(pool->shutdown) {
      perror("Pool shutdown.\n");
      break;
    }

    /* Add task to queue */

    wq_push(pool->queue, client_socket_number);

    /* Unblock waiting thread */
    if(pthread_cond_signal(&(pool->notify)) != 0) {
      perror("pthread_cond_signal(): Can't notify.\n");
      break;
    }
  } while(0);

  if(pthread_mutex_unlock(&pool->lock) != 0) {
    perror("pthread_mutex_unlock(): Can't unlock");
    return -1;
  }
  return 0;
};


/* Shutdown the pool */
int thread_pool_shutdown(threadpool* pool) {

  if (pool == NULL) {
    perror("thread_pool_shutdown(): pool is null.\n");
    return -1;
  }

  if(pthread_mutex_lock(&(pool->lock)) != 0) {
    perror("thread_pool_shutdown(): Can't lock.\n");
    return -1;
  }

  do {
      if(pool->shutdown) {
        break;
      }

      pool->shutdown = 1;

      /* Wake up all worker threads */
      if((pthread_cond_broadcast(&(pool->notify)) != 0) || (pthread_mutex_unlock(&(pool->lock)) != 0)) {
        perror("Can't broadcast or can't lock.\n");
        break;
      }

      /* Join all worker thread */
      for(int i = 0; i < pool->thread_count; i++) {
        if (pthread_join(pool->threads[i], NULL) != 0) {
          perror("pthread_join(): Error.\n");
        }
      }
  } while(0);

  free(pool->threads);
  pthread_mutex_lock(&(pool->lock));
  pthread_mutex_destroy(&(pool->lock));
  pthread_cond_destroy(&(pool->notify));
  free(pool);
  return 0;
};


static void *thread_pool_routine(void *thpool) {

    threadpool *pool = (threadpool *) thpool;
    int client_socket_number;

    for(;;) {
        /* Lock first */
        pthread_mutex_lock(&(pool->lock));

        /* Then wait on condition variable */
        while ((pool->queue->size == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown || pool->queue->size == 0) {
            break;
        }

        /* Pull client_socket_number out of queue */
        client_socket_number = wq_pop(pool->queue);

        /* Unlock */
        pthread_mutex_unlock(&(pool->lock));

        /* Serve request */
        pool->request_handler(client_socket_number);
        close(client_socket_number);
    }

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return(NULL);
}
