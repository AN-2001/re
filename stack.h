/******************************************************************************\
*  stack.h                                                                     *
*                                                                              *
*  A stack data-structure.                                                     *
*                                                                              *
*              Written by Abed Na'ran                          April 2023      *
*                                                                              *
\******************************************************************************/
#ifndef STACK_H
#define STACK_H
#include <stdlib.h>

typedef struct Stack_t *Stack;

Stack StackInit(size_t StackSize);
void StackPush(Stack Stack, int v);
int StackPop(Stack Stack);
int StackTop(Stack Stack);
int StackEmpty(Stack Stack);
void StackFree(Stack Stack);

#endif /* STACK_H */
