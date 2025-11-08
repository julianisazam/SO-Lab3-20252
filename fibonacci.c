/* fibonacci.c
 * Create N Fibonacci numbers in a worker thread.
 * Usage: ./fibonacci N
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    unsigned long long *arr;
    int n;
} FibArg;

void *fib_worker(void *arg)
{
    FibArg *a = (FibArg *)arg;
    unsigned long long *arr = a->arr;
    int n = a->n;

    if (n <= 0) {
        pthread_exit(NULL);
    }
    if (n >= 1) arr[0] = 0ULL;
    if (n >= 2) arr[1] = 1ULL;
    for (int i = 2; i < n; ++i) {
        arr[i] = arr[i-1] + arr[i-2];
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s N\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    if (n < 0) {
        fprintf(stderr, "N must be >= 0\n");
        return 1;
    }

    unsigned long long *arr = malloc(sizeof(unsigned long long) * (n > 0 ? n : 0));
    if (n > 0 && arr == NULL) {
        perror("malloc");
        return 1;
    }

    FibArg *arg = malloc(sizeof(FibArg));
    if (!arg) {
        perror("malloc");
        free(arr);
        return 1;
    }
    arg->arr = arr;
    arg->n = n;

    pthread_t tid;
    if (pthread_create(&tid, NULL, fib_worker, arg) != 0) {
        perror("pthread_create");
        free(arg);
        free(arr);
        return 1;
    }

    /* wait for worker to finish */
    pthread_join(tid, NULL);

    /* print the sequence */
    for (int i = 0; i < n; ++i) {
        if (i) printf(" ");
        printf("%llu", (unsigned long long)arr[i]);
    }
    if (n > 0) printf("\n");

    free(arg);
    free(arr);
    return 0;
}
