/******************************************************************************\
*  config.h                                                                    *
*                                                                              *
*  Config settings.                                                            *
*                                                                              *
*              Written by Abed Na'ran                          Aptil 2023      *
*                                                                              *
\******************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

typedef unsigned char BOOL;

#define DFA_ALPHA_LEN (128)
#define NFA_ALPHA_LEN (DFA_ALPHA_LEN + 1)
#define NFA_EPS (DFA_ALPHA_LEN)

#define BUFF_SIZE (1024)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#endif /* CONFIG_H */
