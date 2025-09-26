#include <stdio.h>

/* 数组队列 ： 这是一个静态大小队列*/
/* 如需动态调整大小，需要使用链表或者动态数组来实现 */

/* 队列最大容量 */
#define QUEUE_MAX  100

struct queue {
    int arr[QUEUE_MAX];
    int front;
    int rear;
};

/* 判断队列是否为空 */
int is_empty(struct queue *q)
{
    return q->front == -1;
}

/* 判断队列是否已满 */
int is_full(struct queue *q)
{
    /* 因为索引是从0开始 */
    return q->rear == QUEUE_MAX - 1;
}

void init(struct queue *q)
{
    q->front = -1;
    q->rear  = -1;
}

/* 入队操作 */
void enqueue(struct queue *q, int value)
{
    if (is_full(q)) {
        fprintf(stderr, "queue is full\n");
        return;
    }

    if (is_empty(q)) {
        q->front = 0;
    }

    q->rear++;
    q->arr[q->rear] = value;
    fprintf(stderr, "Inserted: %d\n", value);

    return;
}

/* 出队 */
int dequeue(struct queue *q)
{
    if (is_empty(q)) {
        fprintf(stderr, "queue is empty\n");
        return -1;
    } else {
        int val = q->arr[q->front++];
        /* 如果出队后，队列为空，则重置队列 */
        if (q->front > q->rear) {
            init(q);
        }
        return val;
    }
}

int main(void)
{
    struct queue q;
    init(&q);

    enqueue(&q, 10);
    enqueue(&q, 20);

    printf("Dequeued: %d\n", dequeue(&q));
    printf("Dequeued: %d\n", dequeue(&q));

    return 0;
}
