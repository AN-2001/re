#include <stdio.h>
#include <stdlib.h>
#include "set.h"

#define START_SIZE (32)
#define GROWTH_FACTOR (1.5f)

static inline int IntCompare(const void *Int1, const void *Int2);

struct Set_t {
    size_t NumElements, Head;
    int *Elements;
};

Set SetInit(size_t NumElements)
{
    Set
        Ret = malloc(sizeof(*Ret));

    if (!Ret)
        return NULL;

    Ret -> NumElements = START_SIZE;
    Ret -> Head = 0;
    Ret -> Elements = malloc(sizeof(*Ret -> Elements) * NumElements);
    return Ret;
}

Set SetCopy(Set S)
{
    Set
        Ret = SetInit(S -> NumElements);

    Ret = SetUnion(Ret, S);
    return Ret;
}

Set SetUnion(Set S1, Set S2)
{
    size_t i;

    for (i = 0; i < S2 -> Head; i++)
        SetSet(S1, S2 -> Elements[i]);

    return S1;
}

void SetSet(Set Set, size_t i)
{
    size_t Curr;

    for (Curr = 0; Curr < Set -> Head; Curr++)
        if (Set -> Elements[Curr] == i)
            return;

    Set -> Elements[Set -> Head++] = i;

    if (Set -> Head >= Set -> NumElements) {
        Set -> NumElements *= GROWTH_FACTOR;
        Set -> Elements = realloc(Set -> Elements,
                                  sizeof(Set -> Elements) * Set -> NumElements);
    }
}

void SetClear(Set Set)
{
    Set -> Head = 0;
}

int SetEquals(Set S1, Set S2)
{
    size_t i;

    if (S1 -> Head != S2 -> Head)
        return 0;

    qsort(S1 -> Elements, S1 -> Head, sizeof(*S1 -> Elements), IntCompare);
    qsort(S2 -> Elements, S2 -> Head, sizeof(*S2 -> Elements), IntCompare);

    for (i = 0; i < S1 -> Head; i++) 
        if (S1 -> Elements[i] != S2 -> Elements[i])
            return 0;

    return 1;
}

int SetQuery(Set Set, size_t i)
{
    size_t Curr;

    for (Curr = 0; Curr < Set -> Head; Curr++)
        if (Set -> Elements[Curr] == i)
            return 1;

    return 0;
}

void SetFree(Set Set)
{
    free(Set -> Elements);
    free(Set);
}

static inline int IntCompare(const void *Int1, const void *Int2)
{
    int N1 = *(int*)Int1,
        N2 = *(int*)Int2;

    return N1 - N2;
}
