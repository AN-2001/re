#include "set.h"
#include <stdlib.h>
#include <memory.h>
#include <stdlib.h>
#include "config.h"

struct Set_t {
    size_t NumElements;
    BOOL *Elements;
};

Set SetInit(size_t NumElements)
{
    Set
        Ret = malloc(sizeof(*Ret));

    Ret -> NumElements = NumElements;
    Ret -> Elements = malloc(sizeof(*Ret -> Elements) * NumElements);
    memset(Ret -> Elements, 0, sizeof(*Ret -> Elements) * NumElements);
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
    size_t i,
        Size = S1 -> NumElements;
    
    if (Size != S2 -> NumElements)
        asm("int3");

    for (i = 0; i < Size; i++)
        if (SetQuery(S1, i) | SetQuery(S2, i))
            SetSet(S1, i);

    return S1;
}

void SetSet(Set Set, size_t i)
{
    if (i >= Set -> NumElements)
        return;

    Set -> Elements[i] = 1;
}

int SetQuery(Set Set, size_t i)
{
    if (i >= Set -> NumElements)
        return 0;

    return Set -> Elements[i];
}

int SetEquals(Set S1, Set S2)
{
    if (S1 -> NumElements != S2 -> NumElements)
        asm("int3");

    int i;

    for (i = 0; i < S1 -> NumElements; i++)
        if (S1 -> Elements[i] != S2 -> Elements[i])
            return 0;

    return 1;
}

void SetClear(Set Set)
{
    int i;

    for (i = 0; i < Set -> NumElements; i++)
        Set -> Elements[i] = 0;

}

void SetFree(Set Set)
{
    free(Set -> Elements);
    free(Set);
}
