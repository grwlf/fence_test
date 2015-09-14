#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <list>

using namespace std;


/* typedef unsigned int uint; */
typedef unsigned long int uint64_t;

static uint64_t flag = 0;

char x[2048];

static uint64_t data = 0;

volatile bool stop = false;

list<uint64_t> diffs;

void *reader(void *p) {
    uint64_t iter_counter = 0;
    printf("reader started\n");

    uint64_t cnt_le = 0, cnt_gt = 0;

    volatile uint64_t _flag = 0;
    volatile uint64_t _data = 0;

    while(!stop) {
        iter_counter++;

        asm volatile ("mfence" ::: "memory");
        asm volatile ("" ::: "memory");
        __sync_synchronize();
        /* __builtin_prefetch(&flag); */

        volatile uint64_t tmp_flag = flag;

        if( tmp_flag != _flag ) {

          asm volatile ("mfence" ::: "memory");
          asm volatile ("" ::: "memory");
          __sync_synchronize();
          /* __builtin_prefetch(&data); */

          volatile uint64_t tmp_data = data;

          if(tmp_data <= _data) {
            cnt_le++;
            diffs.push_back(tmp_flag - _flag);
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

    for(auto d:diffs) {
      printf("%llu ", d);
    }
    printf("\n");

    return NULL;
}

void *writer(void *p) {
    printf("writer started\n");
    volatile uint64_t counter = 1;
    volatile uint64_t overflows = 0;
    while(!stop) {

        data++;
        asm volatile ("mfence" ::: "memory");
        asm volatile ("" ::: "memory");
        __sync_synchronize();
        /* __builtin_prefetch; */

        flag++;
        asm volatile ("mfence" ::: "memory");
        asm volatile ("" ::: "memory");
        __sync_synchronize();
        /* __builtin_prefetch; */

        counter++;
        if(counter == (uint64_t)(-1))
          overflows++;
    }

    printf("counter=%llu overflows=%llu\n", counter, overflows);
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

