#include "queue.h"

struct Queue_t {
    size_t Size, Start, End;
    int *Elements;
};

Queue QueueInit(size_t Size)
{
    Queue 
        Ret = malloc(sizeof(*Ret));

    if (!Ret)
        return NULL;

    Ret -> Size = Size + 1;
    Ret -> Start =
    Ret -> End   = 0;
    Ret -> Elements = malloc(sizeof(*Ret -> Elements) * (Size + 1));

    return Ret;
}

void QueuePush(Queue Queue, int v)
{
    if (Queue -> End == (Queue -> Start - 1 + Queue -> Size) % Queue -> Size)
        return;

    Queue -> Elements[Queue -> End++] = v;
    Queue -> End = (Queue -> End % Queue -> Size);
}

int QueuePop(Queue Queue)
{
    int Val;
    if (Queue -> Start == Queue -> End)
        return -1;

    Val = Queue -> Elements[Queue -> Start];
    Queue -> Start++;
    Queue -> Start = (Queue -> Start % Queue -> Size);

    return Val;
}

int QueueFront(Queue Queue)
{
    if (Queue -> Start == Queue -> End)
        return -1;

    return Queue -> Elements[Queue -> Start];
}

int QueueEmpty(Queue Queue)
{
    return Queue -> Start == Queue -> End;
}

void QueueFree(Queue Queue)
{
    free(Queue -> Elements);
    free(Queue);
}
