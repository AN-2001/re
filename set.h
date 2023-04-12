/******************************************************************************\
*  set.h                                                                       *
*                                                                              *
*  A set data structure.                                                       *
*                                                                              *
*              Written by Abed Na'ran                          April 2023      *
*                                                                              *
\******************************************************************************/
#ifndef SET_H
#define SET_H
#include <stdlib.h>

/* All set-set operations are written with the assumption that they're the s- */
/* ame size. The result of the operations is done in-place and saved into th- */
/* e first set.                                                               */

typedef struct Set_t *Set;

Set SetInit();
Set SetCopy(Set Set);
Set SetUnion(Set S1, Set S2);
void SetSet(Set Set, size_t i);
void SetClear(Set Set);
int SetEquals(Set S1, Set S2);
int SetQuery(Set Set, size_t i);
void SetFree(Set Set);

#endif /* SET_H */
