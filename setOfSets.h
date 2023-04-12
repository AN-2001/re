/******************************************************************************\
*  SetSet.h                                                                    *
*                                                                              *
*  Defines a set of sets data structure.                                       *
*                                                                              *
*              Written by Abed Na'ran                          April 2023      *
*                                                                              *
\******************************************************************************/
#ifndef SET_SET_H
#define SET_SET_H
#include "set.h"

typedef struct SetSet_t *SetOfSets;

SetOfSets SetOfSetsInit();
size_t SetOfSetsSet(SetOfSets SetSet, Set Set);
size_t SetOfSetsSize(SetOfSets SetSet);
int SetOfSetsQuery(SetOfSets SetSet, Set Set);
Set SetOfSetsGet(SetOfSets SetSet, size_t Index);
void SetOfSetsFree(SetOfSets SetSet);

#endif /* SET_SET_H */

