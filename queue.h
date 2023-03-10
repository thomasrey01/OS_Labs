#ifndef QUEUE_H
#define QUEUE_H

struct Queue {
    int *arr;
    int start;
    int end;
    int size;
};

struct Queue *createQueue(int size);
void queue(struct Queue *q, int val);
int dequeue(struct Queue *q);

#endif
