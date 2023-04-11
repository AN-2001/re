# re
A regular expression matcher using a modified version of Thompson's algorithm and a dynamic NFA to DFA converter.

The static branch contains a static NFA to DFA converter which works in exponential complexity.
The next goal of the project is making a dynamic converter which is in O(n).
To achieve this, we build the DFA (and cache it) as we match different strings, this way we don't 
compute redundant states and only use what we need.
