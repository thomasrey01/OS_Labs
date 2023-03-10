#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

struct Queue *createQueue(int size)
{
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->arr = malloc(size * sizeof(int));
    q->start = 0;
    q->end = 0;
    q->size = size;
    return q;
}

int dequeue(struct Queue *q)
{
    if (q->start == q->end) {
        printf("ERROR: EMPTY QUEUE");
        exit(1);
    }
    int res = q->arr[q->start];
    q->start = (q->start + 1) % q->size;
    return res;
}

void doubleQueueSize(struct Queue *q)
{
    int *newArr = malloc(q->size * 2 * sizeof(int));
    int idx = 0;
    for (int i = q->start; i != q->end; i = (i + 1) % q->size) {
        newArr[idx] = q->arr[i];
        idx++;
    }
    int *tmp = q->arr;
    q->start = 0;
    q->end = q->size - 1;
    q->size *= 2;
    q->arr = newArr;
    free(tmp);
}

void queue(struct Queue *q, int val)
{
    if ((q->end + 1) % q->size == q->start) {
        doubleQueueSize(q);
    }
    q->arr[q->end] = val;
    q->end = (q->end + 1) % q->size;
}
