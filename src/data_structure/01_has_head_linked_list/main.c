#include <stdio.h>
#include <stdlib.h>

/*
 * 此文件是写的是有头链表
    无头链表：
    没有特殊的头节点，链表的头指针直接指向第一个实际的数据节点。
    如果链表为空，头指针为 NULL。
    有头链表：

    有头链表:
    包含一个特殊的头节点，该节点不存储实际的数据，仅用于简化链表的操作。
    头指针始终指向这个头节点，即使链表为空，头指针也不为 NULL。
 * */

/* 节点结构体 */
struct Node {
    int data;
    struct Node *next;
};

/* 创建新节点 */
static struct Node *createNode(int data)
{
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    if (new_node == NULL) {
        fprintf(stderr, "malloc error\n");
        goto fail;
    }

    new_node->data = data;
    new_node->next = NULL;

    return new_node;

fail:
    return NULL;
}

/* 创建头结点 */
static struct Node *headNodecreate(void)
{
    /* 不存实际数据值 */
    struct Node *headNode = createNode(-1);
    if (!headNode) {
        fprintf(stderr, "createHeadNode failed\n");
        goto fail;
    }

    return headNode;

fail:
    return NULL;
}

/* 在头部插入数据(这个头不指头结点， 而是头结点后面的第一个节点) */
static int insertAtHead(struct Node *headNode, int data)
{
    if (!headNode) {
        fprintf(stderr, "headNode is NUll \n");
        goto fail;
    }

    struct Node *new_node = createNode(data);
    if (!new_node) {
        fprintf(stderr, "insertAtHead failed\n");
        goto fail;
    }

    new_node->next = headNode->next;
    new_node->data = data;
    headNode->next = new_node;

    return 0;

fail:
    return -1;
}

/* 在尾部插入节点 */
static int insertAtTail(struct Node *headNode, int data)
{
    if (!headNode) {
        fprintf(stderr, "headNode is null\n");
        goto fail;
    }

    struct Node *new_node = createNode(data);
    if (!new_node) {
        fprintf(stderr, "insertAtHead failed\n");
        goto fail;
    }

    struct Node *tmp = headNode;

    while (tmp->next != NULL) {
        tmp = tmp->next;
    }

    tmp->next = new_node;

    return 0;

fail:
    return -1;
}

/* 在指定位置(这个position，这里逻辑就是写往第几个位置插入， 就往第几个位置插入)，插入节点 */
/* 假设postion = 1; 那就是将节点插入第1个位置 */
static int insertByPosition(struct Node *head_node, int position, int data)
{
    if (!head_node) {
        fprintf(stderr, "head node is null \n");
        goto fail;
    }

    if (position - 1 < 0) {
        fprintf(stderr, "head_node is null \n");
        goto fail;
    }


    struct Node *new_node = createNode(data);
    if (!new_node) {
        fprintf(stderr, "new_node create failed \n");
        goto fail;
    }

    struct Node *head_node_tmp = head_node;
    int position_tmp = position -1;

    while (position_tmp > 0  && (head_node_tmp->next != NULL)) {
        position_tmp--;
        head_node_tmp = head_node_tmp->next;
    }

    if (position_tmp != 0 && !head_node_tmp) {
        fprintf(stderr, "插入位置不合理\n");
        goto fail;
    }

    new_node->next = head_node_tmp->next;
    head_node_tmp->next = new_node;
    new_node->data = data;

    return 0;
fail:
    return -1;
}

/* 头删(这个头删不是删除头结点， 是删除头结点的下一个节点) */
static int deleteFirstNode(struct Node *head_node)
{
    if (!head_node) {
        fprintf(stderr, "head_node is null\n");
        goto fail;
    }

    struct Node *tmp = head_node->next;
    if (tmp == NULL) {
        fprintf(stderr, "头删失败， 链表已为空\n");
        goto fail;
    }

    head_node->next = tmp->next;
    free(tmp);

    fprintf(stderr, "头删成功\n");
    return 0;
fail:
    return -1;
}

/* 尾删 */
static int deleteFinalNode(struct Node *head_node)
{
    if (!head_node) {
        fprintf(stderr, "head_node is null\n");
        goto fail;
    }

    struct Node *tmp = head_node;
    struct Node *Prev = NULL;

    if (tmp->next == NULL) {
        fprintf(stderr, "尾删失败, 链表已为空\n");
        goto fail;
    }

    while (tmp->next) {
        Prev = tmp;
        tmp = tmp->next;
    }

    Prev->next = NULL;
    free(tmp);
    fprintf(stderr, "尾删成功\n");
    return 0;

fail:
    return -1;
}

/* 指定位置删除 */
static int deleteNodeByPosition(struct Node *head_node, int position)
{
    if (!head_node) {
        fprintf(stderr, "head_node is null \n");
        goto fail;
    }

    if (!head_node->next) {
        fprintf(stderr, "指定位置删除失败，链表为空 \n");
        goto fail;
    }

    if (position < 0) {
        fprintf(stderr, "删除的位置不合理\n");
        goto fail;
    }

    struct Node *tmp = head_node;
    struct Node *prev = NULL;
    position = position - 1;

    while (position >= 0 && tmp->next) {
        position--;
        prev = tmp;
        tmp  = tmp->next;
    }

    if (position != 0 && !tmp) {
        fprintf(stderr, "不存在该节点\n");
        goto fail;
    }

    prev->next = tmp->next;
    free(tmp);
    tmp == NULL;
    return 0;

fail:
    return -1;
}

/* 打印链表中的所有节点 */
int printAllNodeData(struct Node *head_node)
{
    if (head_node == NULL || head_node->next == NULL) {
        fprintf(stderr, "headnode or first node is null\n");
        goto fail;
    }
    int i = 0;
    struct Node *tmp = head_node->next;

    while (tmp->next) {
        ++i;
        tmp = tmp->next;
        fprintf(stderr, "第%d个节点数据: %d\n", i, tmp->data);
    }

    fprintf(stderr, "遍历链表结束\n");
    return 0;

fail:
    return -1;
}

int main(void)
{
    struct Node *head = headNodecreate();
    if (!head) {
        fprintf(stderr, "headnode is null\n");
        goto fail;
    }

    /* 头插数据 */
    /* insertAtHead(head, 5); */
    /* insertAtHead(head, 4); */
    /* insertAtHead(head, 3); */
    /* insertAtHead(head, 2); */
    /* insertAtHead(head, 1); */
    /* /1* 尾插数据 *1/ */
    /* insertAtTail(head, 200); */
    /* insertAtTail(head, 300); */
    /* /1* 指定位置插入数据 *1/ */
    /* insertByPosition(head, 2, 10); */

    /* 头删数据 */
    deleteFirstNode(head);
    /* 尾删数据 */
    deleteFinalNode(head);

    /* 打印链表中的所有节点 */
    printAllNodeData(head);

    /* 指定位置删除数据 */
    deleteNodeByPosition(head, 3);


    /* 打印链表中的所有节点 */
    printAllNodeData(head);

fail:
    return -1;
}
