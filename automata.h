/******************************************************************************\
*  automata.h                                                                  *
*                                                                              *
*  Defines the automata header.                                                *
*                                                                              *
*              Written by Abed Na'ran                          April 2023      *
*                                                                              *
\******************************************************************************/
#ifndef NFA_H
#define NFA_H
#include "config.h"
#include <stdlib.h>
#include "set.h"

typedef struct NFA_t *NFA;
typedef struct DFA_t *DFA;

/* The alphabet is only {0, 1} + EPSILON for simplicity.                      */
/* State 0 is always the initial state.                                       */
/* The transition tables are in the following format:                         */
/* Each row represents the transitions for a state, each has two(3) columns,a */
/* column for each letter in the alphabet, the value is a set representing    */
/* the destination states, if a state is set to 1 then we travel to it, 0 we  */
/* don't.                                                                     */
/* A set of final states is also included.                                    */

/* CONVENTION: Each machine's definition should be prefixed with a '_' then   */
/* the regex pattern it corresponds to.                                       */

static inline NFA NFAInit(size_t States);
static inline NFA NFAAny();
static inline NFA NFALetter(char Letter);
static inline NFA NFAAtMostOne(NFA NFA);
static inline NFA NFAStar(NFA NFA);
static inline NFA NFAPlus(NFA NFA);
static inline NFA NFAConcat(NFA NFA1, NFA NFA2);
static inline NFA NFAOr(NFA NFA1, NFA NFA2);
static inline void NFAEpsClosure(NFA NFA, Set States, Set Return);
static inline void NFAFree(NFA NFA);

static inline DFA DFAInit(size_t States);
static inline void DFAAddTransition(DFA DFA,
                                    size_t Src,
                                    int Letter,
                                    size_t Dest);
static inline void DFAASetFinal(DFA DFA, size_t State);
static inline int DFARunInput(DFA DFA, char *InputWord, int *Matches);
static inline void DFAFree(DFA DFA);

static inline DFA NFA2DFA(NFA NFA);
#endif /* NFA_H */
