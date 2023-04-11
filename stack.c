#include "stack.h"

struct Stack_t {
    size_t Size;
    int Top;
    int *Elements;
};

Stack StackInit(size_t StackSize)
{
    Stack
        Ret = malloc(sizeof(*Ret));

    if (!Ret)
        return NULL;

    Ret -> Size = StackSize;
    Ret -> Top = 0;
    Ret -> Elements = malloc(sizeof(*Ret -> Elements) * Ret -> Size);
    return Ret;
}

void StackPush(Stack Stack, int v)
{
    if (Stack -> Top >= Stack -> Size)
        asm("int3");

    Stack -> Elements[Stack -> Top++] = v;
}

int StackPop(Stack Stack)
{
    int v;
    if (!Stack -> Top)
        asm("int3");

    v = Stack -> Elements[Stack -> Top];
    Stack -> Top--;
    return v;
}

int StackTop(Stack Stack)
{
    if (!Stack -> Top)
        asm("int3");

    return Stack -> Elements[Stack -> Top];
}

int StackEmpty(Stack Stack)
{
    return !Stack -> Top;
}

void StackFree(Stack Stack)
{
    free(Stack -> Elements);
    free(Stack);
}
