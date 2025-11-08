/*
 * Parallel version of pi.c using Pthreads
 * Each thread computes a partial sum for a subrange of iterations
 * and returns the partial sum via pthread_exit (malloc'd double).
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

double GetTime(void);

typedef struct {
    int start;
    int end;
    double h;
} ThreadData;

void *thread_work(void *arg)
{
    ThreadData *d = (ThreadData *)arg;
    double local_sum = 0.0;
    double x;
    for (int i = d->start; i < d->end; ++i)
    {
        x = d->h * ((double)i + 0.5);
        local_sum += 4.0 / (1.0 + x*x);
    }
    double *res = malloc(sizeof(double));
    if (!res) pthread_exit(NULL);
    *res = local_sum;
    pthread_exit(res);
}

int main(int argc, char **argv)
{
    int n = 2000000000;
    int T = 4; /* default number of threads */
    const double fPi25DT = 3.141592653589793238462643;
    double fPi;
    double tstart, tend;

    if (argc >= 2) T = atoi(argv[1]);
    if (argc >= 3) n = atoi(argv[2]);

    if (T <= 0) T = 1;

    if (n <= 0 || n > 2147483647 ) 
    {
        printf("\ngiven value has to be between 0 and 2147483647\n");
        return 1;
    }

    double h = 1.0 / (double)n;

    pthread_t *threads = malloc(sizeof(pthread_t) * T);
    ThreadData *tdata = malloc(sizeof(ThreadData) * T);
    if (!threads || !tdata) {
        fprintf(stderr, "allocation failed\n");
        return 1;
    }

    /* compute partitioning: distribute remainder among first threads */
    int base = n / T;
    int rem = n % T;

    /* start timing for the parallel CalcPi */
    tstart = GetTime();

    int cur = 0;
    for (int t = 0; t < T; ++t)
    {
        int add = (t < rem) ? 1 : 0;
        int start = cur;
        int end = start + base + add;
        cur = end;
        tdata[t].start = start;
        tdata[t].end = end;
        tdata[t].h = h;
        if (pthread_create(&threads[t], NULL, thread_work, &tdata[t]) != 0) {
            fprintf(stderr, "failed to create thread %d\n", t);
            /* join already created threads */
            for (int j = 0; j < t; ++j) pthread_join(threads[j], NULL);
            free(threads);
            free(tdata);
            return 1;
        }
    }

    double total = 0.0;
    for (int t = 0; t < T; ++t)
    {
        void *ret;
        pthread_join(threads[t], &ret);
        if (ret) {
            double *partial = (double *)ret;
            total += *partial;
            free(partial);
        }
    }

    tend = GetTime();

    fPi = h * total;

    printf("\npi is approximately = %.20f \nError               = %.20f\n",
           fPi, fabs(fPi - fPi25DT));
    printf("CalcPi (parallel) elapsed time = %.6f seconds\n", tend - tstart);

    free(threads);
    free(tdata);
    return 0;
}

double GetTime(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}
