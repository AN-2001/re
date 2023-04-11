/******************************************************************************\
*  queue.h                                                                     *
*                                                                              *
*  A queue data-structure.                                                     *
*                                                                              *
*              Written by Abed Na'ran                          April 2023      *
*                                                                              *
\******************************************************************************/
#ifndef QUEUE_H
#define QUEUE_H
#include <stdlib.h>

typedef struct Queue_t *Queue;

Queue QueueInit(size_t Size);
void QueuePush(Queue Queue, int v);
int QueuePop(Queue Queue);
int QueueFront(Queue Queue);
int QueueEmpty(Queue Queue);
void QueueFree(Queue Queue);

#endif /* QUEUE_H */
