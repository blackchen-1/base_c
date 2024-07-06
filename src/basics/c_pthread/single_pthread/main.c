#include <stdio.h>
#include <pthread.h>

/* introduce:
 *              这是一个单线程例子
 */

/* 线程的执行回调函数 */
void *do_work(void *arg)
{
    printf("hello,world\n");
    return NULL;
}

int main(void)
{
    /* 定义线程号 */
    pthread_t tid;

    /* 创建线程 */
    pthread_create(&tid, NULL, do_work, NULL);

    /* 等待线程结束 */
    pthread_join(tid, NULL);
    return 0;
}
