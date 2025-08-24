#include <pthread.h>
#include <stdio.h>

#include "atomic.h"
#include "src/defines.h"

#define MAX_THREADS 50
#define MAX_LOOP 10000

int normal_counter = 0;
snuk_atomic_i32 atomic_counter = SNUK_ATOMIC_VAR_INIT(0);

#ifdef SNUK_OS_LINUX
void *thread_function(void *data) {
    for (int i = 0; i < MAX_LOOP; ++i) {
        ++normal_counter;
        snuk_atomic_fetch_add(&atomic_counter, 1);
    }

    return NULL;
}

#endif

int main(void) {
#ifdef SNUK_OS_LINUX
    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; ++i)
        pthread_create(&threads[i], NULL, thread_function, NULL);

    for (int i = 0; i < MAX_THREADS; ++i) pthread_join(threads[i], NULL);

    printf("normal counter = %d, atomic counter = %d\n", normal_counter,
           snuk_atomic_load(&atomic_counter));
#endif
}
