#include "setOfSets.h"
#include "set.h"

#define START_SIZE (32)
#define GROWTH_SIZE (2)

struct SetSet_t {
    size_t Size, Head;
    Set *Elements;
};

SetOfSets SetOfSetsInit(size_t Size, size_t SetSize)
{
    int i;

    SetOfSets 
        Ret = malloc(sizeof(*Ret));

    if (!Ret)
        return NULL;

    Ret -> Size = START_SIZE;
    Ret -> Head = 0;
    Ret -> Elements = malloc(sizeof(Set*) * START_SIZE);
    for (i = 0; i < START_SIZE; i++)
        Ret -> Elements[i] = SetInit(SetSize);

    return Ret;
}

size_t SetOfSetsSize(SetOfSets SetSet)
{
    return SetSet -> Head;
}

size_t SetOfSetsSet(SetOfSets SetSet, Set S)
{
    int i;

    for (i = 0; i < SetSet -> Head; i++)
        if (SetEquals(S, SetSet -> Elements[i]))
            return i;

    SetSet -> Elements[SetSet -> Head++] = SetCopy(S);

    if (SetSet -> Head >= SetSet -> Size) {
        SetSet -> Size *= GROWTH_SIZE;
        SetSet -> Elements = malloc(sizeof(Set*) * SetSet -> Size);
    }

    return i;
}

int SetOfSetsQuery(SetOfSets SetSet, Set Set)
{
    size_t i;

    for (i = 0; i < SetSet -> Head; i++)
        if (SetEquals(Set, SetSet -> Elements[i]))
            return 1;

    return 0;
}

Set SetOfSetsGet(SetOfSets SetSet, size_t Index)
{
    return SetSet -> Elements[Index];
}

void SetOfSetsFree(SetOfSets SetSet)
{
    int i;

    for (i = 0; i < SetSet -> Head; i++)
        SetFree(SetSet -> Elements[i]);

    free(SetSet -> Elements);
    free(SetSet);
}
