#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int N;
int P;
int total = 0;
pthread_mutex_t lock;

void* thread(void *range) {
    int start = ((int*)range)[0];
    int end = ((int*)range)[1];

    for (int i = start; i <= end; i++) {
        if (N % i == 0) {
            pthread_mutex_lock(&lock);

            total += i;
            if (i != 1 && i != N / i)
                total += N / i;

            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Invalid number of arguments!\n");
        return 0;
    }

    N = atoi(argv[1]);
    P = atoi(argv[2]);
    if (N==0 || N==1) {
        printf("%d is not a Perfect Number\n", N);
        return 0;
    }
    pthread_t threads[P];
    pthread_mutex_init(&lock, NULL);

    int parts = sqrt(N) / P;

    for (int i = 0; i < P; i++) {
        int *range = malloc(2 * sizeof(int));
        range[0] = i * parts + 1;

        if (i == P - 1)
            range[1] = sqrt(N);
        else
            range[1] = (i + 1) * parts;

        pthread_create(&threads[i], NULL, thread, range);
    }

    for (int i = 0; i < P; i++)
        pthread_join(threads[i], NULL);

    if (total == N)
        printf("%d is a Perfect Number\n", N);
    else
        printf("%d is not a Perfect NUmber\n", N);

    pthread_mutex_destroy(&lock);
    return 0;
}