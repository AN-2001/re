#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include "set.h"
#include "setOfSets.h"
#include "queue.h"
#include "automata.h"
#include "config.h"

#define ASCII_TEXT_COLOR(r,g,b) "\033[38;2;"#r";"#g";"#b"m"
#define CONS(N, H)     \
    do {               \
        N -> Next = H; \
        H = N;         \
    }while(0); 

struct NFA_t {
    struct NFA_t *Next;
    size_t States;
    Set FinalStates;
    Set (*Transitions)[NFA_ALPHA_LEN]; 
};

struct DFA_t {
    size_t CurrentState;
    size_t AllocatedStates;
    SetOfSets AllStates;

    Set FinalStates;
    size_t (*Transitions)[DFA_ALPHA_LEN]; 
};

static NFA Head = NULL;
static inline NFA NFAFromRegex(char *Str);

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        printf("FORMAT: %s <pattern>\n", argv[0]);    
        return 1;
    }

    DFA DFA;
    NFA NFA, Next;
    char Buff[BUFF_SIZE];
    int 
        *Matches = malloc(sizeof(*Matches) * BUFF_SIZE);
    size_t i, j;
    int M;

    NFA = NFAFromRegex((char*)argv[1]);
    printf("!BUILT NFA!\n");

    DFA = NFA2DFA(NFA);
    while (Head) {
        Next = Head -> Next; 
        NFAFree(Head);
        Head = Next;
    }
    printf("!READY!\n");

    while (fgets(Buff, BUFF_SIZE, stdin)) {
        if (!DFARunInput(DFA, Buff, Matches))
            continue;

        for (i = 0; Buff[i]; i++) {
            if (Matches[i])
                printf(ASCII_TEXT_COLOR(255, 0, 0) "%c", Buff[i]);
            else
                printf(ASCII_TEXT_COLOR(255, 255, 255) "%c", Buff[i]);
            
        }

    }

    DFAFree(DFA);
    free(Matches);
    return 0;
}
 
static inline NFA NFAFromRegex(char *Str)
{
    size_t e;
    NFA Res = NULL;

    for (;*Str;) {
        if (Str[1] == '*') {
            Res = NFAConcat(Res, NFAStar(NFALetter(*Str)));
            Str += 2;
        } else if (Str[1] == '+') {
            Res = NFAConcat(Res, NFAPlus(NFALetter(*Str)));
            Str += 2;
        } else if (Str[1] == '?') {
            Res = NFAConcat(Res, NFAAtMostOne(NFALetter(*Str)));
            Str += 2;
        } else if (Str[0] == '(') {
            for (e = 1; Str[e] != ')'; e++); 
            Str[e] = '\0';
            if (Str[e + 1] == '*') {
                Res = NFAConcat(Res, NFAStar(NFAFromRegex(Str + 1)));
                Str += e + 2;
            } else if (Str[e + 1] == '+') {
                Res = NFAConcat(Res, NFAPlus(NFAFromRegex(Str + 1)));
                Str += e + 2;
            }  else if (Str[e + 1] == '?') {
                Res = NFAConcat(Res, NFAAtMostOne(NFAFromRegex(Str + 1)));
                Str += e + 2;
            } else {
                Res = NFAConcat(Res, NFAFromRegex(Str + 1));
                Str += e + 1;
            }
        } else if(Str[0] == '|') {
            return NFAOr(Res, NFAFromRegex(Str + 1));
        } else {
            Res = NFAConcat(Res, NFALetter(*Str));
            Str += 1;
        }
    }

    return Res;
}

DFA NFA2DFA(NFA NFA)
{
    size_t i, j, Curr, k,
        NFASize = NFA -> States,
        DFASize = (size_t)1 << NFASize;
    Set CurrSet,
        Letters[DFA_ALPHA_LEN],
        VisitedSet = SetInit(DFASize);
    SetOfSets 
        States = SetOfSetsInit(DFASize, NFASize);
    Queue
        Queue = QueueInit(DFASize);
    DFA 
        DFA = DFAInit(DFASize);

    for (k = 0; k < DFA_ALPHA_LEN; k++)
        Letters[k] = SetInit(NFASize);

    NFAEpsClosure(NFA, NFA -> Transitions[0][NFA_EPS], Letters[0]);
    i = SetOfSetsSet(States, Letters[0]);
    QueuePush(Queue, i);
    SetSet(VisitedSet, i);
    
    while (!QueueEmpty(Queue)) {
        for (k = 0; k < DFA_ALPHA_LEN; k++)
            SetClear(Letters[k]);

        Curr = QueuePop(Queue);
        CurrSet = SetOfSetsGet(States, Curr);

        for (j = 0; j < NFASize; j++) {
            if (!SetQuery(CurrSet, j))
                continue;
        
            for (k = 0; k < DFA_ALPHA_LEN; k++)
                NFAEpsClosure(NFA, NFA -> Transitions[j][k], Letters[k]);

        }

        for (k = 0; k < DFA_ALPHA_LEN; k++) {
            i = SetOfSetsSet(States, Letters[k]);
            DFAAddTransition(DFA, Curr, k, i);

            if (!SetQuery(VisitedSet, i)) {
                SetSet(VisitedSet, i);
                QueuePush(Queue, i);
            }
        }

    }

    for (i = 0; i < SetOfSetsSize(States); i++) {
       CurrSet = SetOfSetsGet(States, i); 
       for (j = 0; j < NFASize; j++) {
            if (SetQuery(CurrSet, j) && SetQuery(NFA -> FinalStates, j))
                DFAASetFinal(DFA, i);
       }
    }

    for (k = 0; k < DFA_ALPHA_LEN; k++)
        SetFree(Letters[k]);

    SetFree(VisitedSet);
    SetOfSetsFree(States);
    QueueFree(Queue);
    return DFA;
}

int DFARunInputAux(DFA DFA, char *InputWord, int s, int *Matches)
{

    char CurrentChar;
    int Letter, e,
        FoundMatch = 0;
    size_t
        CurrentState = 0;

    if (SetQuery(DFA -> FinalStates, CurrentState))
        FoundMatch = 1;

    for (e = s; InputWord[e]; e++) {
        CurrentChar = InputWord[e];
        CurrentState = DFA -> Transitions[CurrentState][CurrentChar];
        if (SetQuery(DFA -> FinalStates, CurrentState)) {
            memset(Matches + s, 1, sizeof(*Matches) * (e - s + 1));
            FoundMatch = 1;
        }
    }

    return FoundMatch;
}

int DFARunInput(DFA DFA, char *InputWord, int *Matches)
{
    char CurrentChar;
    int s, Letter,
        FoundMatch = 0;

    memset(Matches, 0, sizeof(*Matches) * BUFF_SIZE);
    for (s = 0; InputWord[s]; s++)
        if (DFARunInputAux(DFA, InputWord, s, Matches))
            FoundMatch = 1;

    return FoundMatch;
}

NFA NFAInit(size_t States)
{
    size_t i, j;
    NFA
        Ret = malloc(sizeof(*Ret));

    Ret -> States = States;
    Ret -> Transitions = malloc(sizeof(*Ret -> Transitions) * States);
    for (i = 0; i < States; i++)
        for (j  = 0; j < NFA_ALPHA_LEN; j++)
            Ret -> Transitions[i][j] = SetInit(States);

    for (i = 0; i < States; i++)
        SetSet(Ret -> Transitions[i][NFA_EPS], i);

    Ret -> FinalStates = SetInit(States);
    CONS(Ret, Head);
    return Ret;
}

void NFAEpsClosureAux(NFA NFA, Set States, Set Visited, Set Return);
void NFAEpsClosure(NFA NFA, Set States, Set Return)
{
    Set 
        Visited = SetInit((size_t)1 << NFA -> States);

    NFAEpsClosureAux(NFA, States, Visited, Return);
    SetFree(Visited);
}

void NFAEpsClosureAux(NFA NFA, Set States, Set Visited, Set Return)
{
    int i;

    for (i = 0; i < NFA -> States; i++) {
        if (!SetQuery(States, i) || SetQuery(Visited, i))
            continue;

        SetSet(Visited, i);
        Return = SetUnion(Return, NFA -> Transitions[i][NFA_EPS]);
        NFAEpsClosureAux(NFA, NFA -> Transitions[i][NFA_EPS], Visited, Return);
    }

}

DFA DFAInit(size_t States)
{
    size_t i, j;
    DFA 
        Ret = malloc(sizeof(*Ret));

    Ret -> Transitions = malloc(sizeof(*Ret -> Transitions) * States);
    for (i = 0; i < States; i++)
        for (j = 0; j < DFA_ALPHA_LEN; j++)
            Ret -> Transitions[i][j] = 0;

    Ret -> FinalStates = SetInit(States);
    return Ret;
}

void DFAAddTransition(DFA DFA, size_t Src, int Letter, size_t Dest)
{
    DFA -> Transitions[Src][Letter] = Dest;
}

void DFAASetFinal(DFA DFA, size_t State)
{
    SetSet(DFA -> FinalStates, State);
}

void NFAFree(NFA NFA)
{
    size_t i, j;

    SetFree(NFA -> FinalStates);
    for (i = 0; i < NFA -> States; i++)
        for (j = 0; j < NFA_ALPHA_LEN; j++) 
            SetFree(NFA -> Transitions[i][j]);
   
    free(NFA -> Transitions);
    free(NFA);
}

void DFAFree(DFA DFA)
{
    SetFree(DFA -> FinalStates);
    free(DFA -> Transitions);
    free(DFA);
}

NFA NFAAny()
{
    NFA
        Ret = NFAInit(2);
    size_t Letter;

    for (Letter = 0; Letter < DFA_ALPHA_LEN; Letter++)
        SetSet(Ret -> Transitions[0][Letter], 1);

    SetSet(Ret -> FinalStates, 1);
    return Ret;
}

NFA NFALetter(char Letter)
{
    if (Letter == '.')
        return NFAAny();

    NFA 
        Ret = NFAInit(2);

    SetSet(Ret -> Transitions[0][Letter], 1);

    SetSet(Ret -> FinalStates, 1);
    return Ret;
}

NFA NFAStar(NFA NFA)
{
    size_t i;

    for (i = 0; i < NFA -> States; i++)
        if (SetQuery(NFA -> FinalStates, i))
            SetSet(NFA -> Transitions[i][NFA_EPS], 0);

    SetSet(NFA -> FinalStates, 0);
    return NFA;
}

NFA NFAPlus(NFA NFA)
{
    size_t i;

    for (i = 0; i < NFA -> States; i++)
        if (SetQuery(NFA -> FinalStates, i))
            SetSet(NFA -> Transitions[i][NFA_EPS], 0);

    return NFA;
}

NFA NFAAtMostOne(NFA NFA)
{
    SetSet(NFA -> FinalStates, 0);
    return NFA;
}

NFA NFAConcat(NFA NFA1, NFA NFA2)
{
    if (!NFA1)
        return NFA2;

    if (!NFA2)
        return NFA1;

    size_t i, j, k,
        S1 = NFA1 -> States,
        S2 = NFA2 -> States,
        NFASize = S1 + S2;
    NFA
        NFA = NFAInit(NFASize);

    for (i = 0; i < S1; i++)
        for (j = 0; j < S1; j++)
            for (k = 0; k < NFA_ALPHA_LEN; k++) 
                if (SetQuery(NFA1 -> Transitions[i][k], j))
                    SetSet(NFA -> Transitions[i][k], j);

    for (i = 0; i < S1; i++)
        if (SetQuery(NFA1 -> FinalStates, i)) 
            SetSet(NFA -> Transitions[i][NFA_EPS], S1);

    for (i = 0; i < S2; i++)
        for (j = 0; j < S2; j++)
            for (k = 0; k < NFA_ALPHA_LEN; k++)
                if (SetQuery(NFA2 -> Transitions[i][k], j))
                    SetSet(NFA -> Transitions[i + S1][k], j + S1);


    for (j = 0; j < S2; j++)
        if (SetQuery(NFA2 -> FinalStates, j))
            SetSet(NFA -> FinalStates, j + S1);

    return NFA;
}

NFA NFAOr(NFA NFA1, NFA NFA2) 
{
    if (!NFA1)
        return NFA2;

    if (!NFA2)
        return NFA1;

    size_t i, j, k,
        S1 = NFA1 -> States,
        S2 = NFA2 -> States,
        NFASize = S1 + S2;
    NFA
        NFA = NFAInit(NFASize);

    for (i = 0; i < S1; i++)
        for (j = 0; j < S1; j++)
            for (k = 0; k < NFA_ALPHA_LEN; k++)
                if (SetQuery(NFA1 -> Transitions[i][k], j))
                    SetSet(NFA -> Transitions[i][k], j);

    for (i = 0; i < S2; i++)
        for (j = 0; j < S2; j++)
            for (k = 0; k < NFA_ALPHA_LEN; k++)
                if (SetQuery(NFA2 -> Transitions[i][k], j))
                    SetSet(NFA -> Transitions[i + S1][k], j + S1);

    for (j = 0; j < S1; j++)
        if (SetQuery(NFA1 -> FinalStates, j))
            SetSet(NFA -> FinalStates, j);

    for (j = 0; j < S2; j++)
        if (SetQuery(NFA2 -> FinalStates, j))
            SetSet(NFA -> FinalStates, j + S1);

    SetSet(NFA -> Transitions[0][NFA_EPS], S1);
    return NFA;
}
