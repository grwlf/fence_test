#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* typedef unsigned int uint; */
typedef unsigned long int uint64_t;

volatile uint64_t flag = 0;
volatile uint64_t data = 0;

volatile bool stop = false;

void *reader(void *p) {
    uint64_t iter_counter = 0;
    printf("reader started\n");

    uint64_t cnt_le = 0, cnt_gt = 0;

    uint64_t _flag = 0;
    uint64_t _data = 0;

    while(!stop) {
        iter_counter++;

        uint64_t tmp_flag = __sync_val_compare_and_swap(&flag, _flag, flag);

        if( tmp_flag != _flag ) {
          uint64_t tmp_data = data;

          if(tmp_data <= _data) {
            cnt_le++;
          }
          else {
            /* Right */
            cnt_gt++;
          }

          _data = tmp_data;
          _flag = tmp_flag;
        }
    }

    printf("iters=%llu, less=%llu, more=%llu\n", iter_counter, cnt_le, cnt_gt);

    return NULL;
}

void *writer(void *p) {
    printf("writer started\n");
    volatile uint64_t counter = 1;
    volatile uint64_t overflows = 0;
    while(!stop) {

        /* data = counter; */
        __sync_fetch_and_add(&data,1);
        asm volatile ("mfence" ::: "memory");

        __sync_fetch_and_add(&flag,1);
        /* flag = counter; */
        asm volatile ("mfence" ::: "memory");

        counter++;
        if(counter == (uint64_t)(-1))
          overflows++;
    }

    printf("overflows=%llu\n", overflows);
}

int main() {
    pthread_t reader_thr, writer_thr;
    int t1 = pthread_create(&reader_thr, NULL, reader, NULL);
    int t2 = pthread_create(&writer_thr, NULL, writer, NULL);
    if (t1 !=0 || t2!=0) { printf("fail start\n");
        return 1;
    }

    void *ret;

    sleep(3);
    stop = true;
    pthread_join(reader_thr, &ret);
    pthread_join(writer_thr, &ret);
}

