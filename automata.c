#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include "set.h"
#include "setOfSets.h"
#include "automata.h"
#include "config.h"
#include <setjmp.h>
#include <signal.h>

#define DFA_START_SIZE (32)
#define DFA_GROWTH_FACTOR (2)

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

    int (*Transitions)[DFA_ALPHA_LEN]; 
    Set FinalStates;
    NFA EquivNFA;
};

static NFA Head = NULL;
static jmp_buf ExitJmp;
static inline char *VerifyRegex(const char *Regex);
static inline NFA NFAFromRegex(char *Regex);
static inline DFA DFAInitFromNFA(NFA NFA);
static inline void DFARunChar(DFA DFA, char c);
static inline int DFAIsDone(DFA DFA);
static inline void DFAReset(DFA DFA);
static void ExitHandler(int Sig);

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        printf("FORMAT: %s <pattern>\n", argv[0]);    
        return 1;
    }

    DFA DFA;
    NFA NFA, Next;
    char Buff[BUFF_SIZE];
    char *Regex;
    int *Matches;
    size_t i,
           Line = 0;

    signal(SIGINT, ExitHandler);
    signal(SIGTERM, ExitHandler);

    Regex = VerifyRegex(argv[1]);
    if (!Regex) {
        printf("!INVALID REGEX!\n");
        return 1;
    }

    NFA = NFAFromRegex(Regex);
    free(Regex);
    printf("!BUILT NFA!\n");

    DFA = DFAInitFromNFA(NFA);
    Matches = malloc(sizeof(*Matches) * BUFF_SIZE);

    while (Head) {
        Next = Head -> Next; 
        if (Head != NFA)
            NFAFree(Head);
        Head = Next;
    }
    printf("!READY!\n");

    if(setjmp(ExitJmp)) {
        DFAFree(DFA);
        NFAFree(NFA);
        free(Matches);
        return 0;
    }

    while (fgets(Buff, BUFF_SIZE, stdin)) {
        Line++;

        if (!DFARunInput(DFA, Buff, Matches))
            continue;

        printf(ASCII_TEXT_COLOR(0, 255, 0) "LINE:%ld  ", Line);
        for (i = 0; Buff[i]; i++) {
            if (Matches[i])
                printf(ASCII_TEXT_COLOR(255, 0, 0) "%c", Buff[i]);
            else
                printf(ASCII_TEXT_COLOR(255, 255, 255) "%c", Buff[i]);
        }

    }

    DFAFree(DFA);
    NFAFree(NFA);
    free(Matches);
    return 0;
}

void ExitHandler(int Sig)
{
    longjmp(ExitJmp, 1);
}
 
static inline NFA NFAFromRegex(char *Str)
{
    int Counter;
    size_t e;
    NFA Res = NULL;

    for (;*Str;) {
        if (Str[0] == '\\') {
            Str += 1;
            Res = NFAConcat(Res, NFALetter(*Str));
            Str += 1;
        } else if (Str[0] == '(') {
            Counter = 1;
            for (e = 1; Counter; e++)
                Counter += (Str[e] == '(') - (Str[e] == ')');

            e--;
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
        } else if (Str[1] == '*') {
            Res = NFAConcat(Res, NFAStar(NFALetter(*Str)));
            Str += 2;
        } else if (Str[1] == '+') {
            Res = NFAConcat(Res, NFAPlus(NFALetter(*Str)));
            Str += 2;
        } else if (Str[1] == '?') {
            Res = NFAConcat(Res, NFAAtMostOne(NFALetter(*Str)));
            Str += 2;
        } else {
            Res = NFAConcat(Res, NFALetter(*Str));
            Str += 1;
        }
    }

    return Res;
}

char *VerifyRegex(const char *Regex)
{
    size_t i,
           Size = strlen(Regex);
    int 
        Brackets = 0;
    char *Cpy;

    if (Regex[0] == '*' ||
        Regex[0] == '?' ||
        Regex[0] == '+' ||
        Regex[0] == '|')
        return NULL;
 
    for (i = 0; i < Size; i++) {
        Brackets += (Regex[i] == '(') - (Regex[i] == ')');
        if (Brackets < 0)
            return NULL;
    }

    if (Brackets)
        return NULL;

    Cpy = malloc(Size + 1);
    memcpy(Cpy, Regex, Size + 1);
    return Cpy;
}

DFA DFAInitFromNFA(NFA NFA) 
{
    size_t i, k;
    DFA 
        Ret = malloc(sizeof(*Ret));
    Set
        Closure = SetInit();

    Ret -> Transitions = malloc(sizeof(*Ret -> Transitions) * DFA_START_SIZE);

    for (i = 0; i < DFA_START_SIZE; i++)
        for (k = 0; k < DFA_ALPHA_LEN; k++)
            Ret -> Transitions[i][k] = -1;

    Ret -> FinalStates = SetInit();
    Ret -> AllocatedStates = DFA_START_SIZE;
    Ret -> AllStates = SetOfSetsInit();
    Ret -> CurrentState = 0;
    Ret -> EquivNFA = NFA;

    NFAEpsClosure(NFA, NFA -> Transitions[0][NFA_EPS], Closure);
    SetOfSetsSet(Ret -> AllStates, Closure);

    for (i = 0; i < NFA -> States; i++) {
        if (SetQuery(Closure, i) && SetQuery(NFA -> FinalStates, i)) {
            SetSet(Ret -> FinalStates, 0);
            break;
        }
    }


    SetFree(Closure);
    return Ret;
}

void DFARunChar(DFA DFA, char c)
{
    Set CurrSet, CurrLetter;
    size_t j, i;
    int Exists;

    if (c < 0)
        return;

    if (DFA -> Transitions[DFA -> CurrentState][c] == -1) {
        CurrSet = SetOfSetsGet(DFA -> AllStates, DFA -> CurrentState);
        CurrLetter = SetInit();
        for (j = 0; j < DFA -> EquivNFA -> States; j++) {
            if (!SetQuery(CurrSet, j))
                continue;

            NFAEpsClosure(DFA -> EquivNFA,
                    DFA -> EquivNFA -> Transitions[j][c],
                    CurrLetter);
        }
        
        Exists = SetOfSetsQuery(DFA -> AllStates, CurrLetter);
        i = SetOfSetsSet(DFA -> AllStates, CurrLetter);
        DFAAddTransition(DFA, DFA -> CurrentState, c, i);

        if (!Exists) {
            for (j = 0; j < DFA -> EquivNFA -> States; j++) {
                if (SetQuery(CurrLetter, j) && SetQuery(DFA -> EquivNFA -> FinalStates, j)) {
                    SetSet(DFA -> FinalStates, i);
                    break;
                }
            }
        }

        SetFree(CurrLetter);
    }
    DFA -> CurrentState = DFA -> Transitions[DFA -> CurrentState][c];
}

int DFAIsDone(DFA DFA)
{
    return SetQuery(DFA -> FinalStates, DFA -> CurrentState);
}

void DFAReset(DFA DFA)
{
    return;
}

int DFARunInputAux(DFA DFA, char *InputWord, int s, int *Matches)
{
    int e,
        FoundMatch = 0;

    DFA -> CurrentState = 0;
    if (DFAIsDone(DFA))
        FoundMatch = 1;

    for (e = s; InputWord[e]; e++) {
        DFARunChar(DFA, InputWord[e]);
        if (DFAIsDone(DFA)) {
            memset(Matches + s, 1, sizeof(*Matches) * (e - s + 1));
            FoundMatch = 1;
        }
    }

    return FoundMatch;
}

int DFARunInput(DFA DFA, char *InputWord, int *Matches)
{
    int s, 
        FoundMatch = 0;

    DFAReset(DFA);
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
            Ret -> Transitions[i][j] = SetInit();

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
        Visited = SetInit();

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

void DFAAddTransition(DFA DFA, size_t Src, int Letter, size_t Dest)
{
    size_t OldSize, i, k;

    if (Src >= DFA -> AllocatedStates || Dest >= DFA -> AllocatedStates) {
        OldSize = DFA -> AllocatedStates;
        DFA -> AllocatedStates *= DFA_GROWTH_FACTOR;
        DFA -> Transitions = realloc(DFA -> Transitions,
                                sizeof(*DFA -> Transitions) * DFA -> AllocatedStates); 

        for (i = OldSize; i < DFA -> AllocatedStates; i++)
            for (k = 0; k < DFA_ALPHA_LEN; k++)
                DFA -> Transitions[i][k] = -1;

    }

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
    SetOfSetsFree(DFA -> AllStates);
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
