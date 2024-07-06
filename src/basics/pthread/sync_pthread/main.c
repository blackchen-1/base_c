#include <stdio.h>
#include <pthread.h>
#include <string.h>
/* introduce:
 *  这是个线程同步例子
 *  多线程程序中，正确的数据同步和线程间的协调是非常重要的，以避免竞态条件和数据不一致等问题
 */


/* 定义一个变量,作为多线程的共享资源 */
int shared_variable = 0;
/* 初始化线程锁 */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* 多线程的线程体 */
void *do_work(void *arg)
{
    /*
     * 这里实现了互斥锁:
     * 多线程间，通过互斥锁,确保在同一时刻只有一个线程可以访问共享资源
     */
    pthread_mutex_lock(&lock);
    pthread_mutex_unlock(&lock);

    return NULL;
    (void)arg;
}

int main(void)
{
    // 定义线程id
    pthread_t tid1, tid2;

    // 创建线程
    pthread_create(&tid1, NULL, do_work, NULL);
    pthread_create(&tid2, NULL, do_work, NULL);

    // 等待线程结束
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_mutex_destroy(&lock);
}
