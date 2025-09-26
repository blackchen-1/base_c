#include <stdio.h>
#include <stdlib.h>

/* 无头节点链表 */

struct Node {
    int data;
    struct Node *next;
};

/* 创建节点 */
struct Node *createNode(int data)
{
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    if (!new_node) {
        fprintf(stderr, "malloc failed\n");
        goto fail;
    }

    new_node->data = data;
    new_node->next = NULL;

    return new_node;
fail:
    return NULL;
}

/* 插入节点(头插) */
int insertNodeAtHead(struct Node **firstNode, int data)
{
    struct Node *newNode = createNode(data);
    if (!newNode) {
        fprintf(stderr, "create new node failed\n");
        goto fail;
    }

    if (*firstNode == NULL) {
        *firstNode = newNode;
        return 0;
    }

    newNode->next = *firstNode;
    *firstNode = newNode;

    return 0;
fail:
    return -1;
}

/* 插入节点(尾插) */
int insertNodeAtTail(struct Node **firstNode, int data)
{
    if (!firstNode) {
        fprintf(stderr, "链表为空，无需打印\n");
        goto fail;
    }

    struct Node *newNode= createNode(data);

    if (!(*firstNode)) {
        *firstNode = newNode;
        return 0;
    }

    struct Node *tmp = *firstNode;

    while (tmp->next) {
        tmp = tmp->next;
    }

    tmp->next = newNode;
    return 0;

fail:
    return -1;
}

/* 删除节点(头删) */
int deleteNodeAtHead(struct Node **firstNode)
{
    if (!firstNode) {
        fprintf(stderr, "头删失败，链表已空\n");
        goto fail;
    }

    /* 暂存头结点 */
    struct Node *delNode = *firstNode;
    /* 将头指针指向下一个节点 */
    *firstNode = (*firstNode)->next;
    /* 释放原头节点的内存 */
    free(delNode);
    return 0;

fail:
    return -1;
}

/* 删除节点(尾删) */
/* 这里使用二级指针是因为在，释放一级指针节点时可以与外部的指针同步 */
int deleteNodeAtTail(struct Node **firstNode)
{
    if (!(*firstNode)) {
        fprintf(stderr, "尾删失败，链表已空\n");
        goto fail;
    }

    struct Node *delNode = *firstNode;
    struct Node *prev = NULL;

    // 如果链表只有一个节点
    if (delNode->next == NULL) {
        *firstNode = NULL;
        free(delNode);
        return 0;
    }

    while (delNode->next) {
        prev    = delNode;
        delNode = delNode->next;
    }

    prev->next = NULL;
    free(delNode);
    delNode = NULL;

    return 0;
fail:
    return -1;
}

int printAllNodeInfo(struct Node *firstNode)
{
    if (!firstNode) {
        fprintf(stderr, "链表为空，无需打印\n");
        goto fail;
    }

    int i = 0;
    struct Node *tmp = firstNode;

    while (tmp) {
        ++i;
        fprintf(stderr, "第%d个参数数据: %d\n", i, tmp->data);
        tmp = tmp->next;
    }
    return 0;

fail:
    return -1;
}

int main(void)
{
    struct Node *firstNode = NULL;
    /* 头插 */
    /* insertNodeAtHead(&firstNode, 5); */
    /* insertNodeAtHead(&firstNode, 4); */
    /* insertNodeAtHead(&firstNode, 3); */
    /* insertNodeAtHead(&firstNode, 2); */
    /* insertNodeAtHead(&firstNode, 1); */
    /* printAllNodeInfo(firstNode); */

    /* 尾插 */
    insertNodeAtTail(&firstNode, 5);
    insertNodeAtTail(&firstNode, 4);
    insertNodeAtTail(&firstNode, 3);
    insertNodeAtTail(&firstNode, 2);
    insertNodeAtTail(&firstNode, 1);
    printAllNodeInfo(firstNode);

    /* 头删 */
    deleteNodeAtHead(&firstNode);
    printAllNodeInfo(firstNode);

    /* 尾删 */
    deleteNodeAtTail(&firstNode);
    deleteNodeAtTail(&firstNode);
    deleteNodeAtTail(&firstNode);
    deleteNodeAtTail(&firstNode);
    deleteNodeAtTail(&firstNode);
    printAllNodeInfo(firstNode);
}
