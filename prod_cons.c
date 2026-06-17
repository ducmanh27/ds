#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;

pthread_mutex_t mutex;
pthread_cond_t not_full;
pthread_cond_t not_empty;

void* producer(void* arg)
{
    int item = 1;

    while (1)
    {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE)
        {
            printf("Producer waiting (buffer full)\n");
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[count++] = item;

        printf("Produced %d (count=%d)\n", item, count);

        item++;

        pthread_cond_signal(&not_empty);

        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    return NULL;
}

void* consumer(void* arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        while (count == 0)
        {
            printf("Consumer waiting (buffer empty)\n");
            pthread_cond_wait(&not_empty, &mutex);
        }

        int item = buffer[--count];

        printf("Consumed %d (count=%d)\n", item, count);

        pthread_cond_signal(&not_full);

        pthread_mutex_unlock(&mutex);

        sleep(2);
    }

    return NULL;
}

int main()
{
    pthread_t prod, cons;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}
