import sys

# Automatically build an NFA template from a name and the number of states.
def entry(Name, Num):
    Header = f"""/* Defintion for _{Name}_: */
#define _{Name}_STATES ({Num})
static unsigned char
    _{Name}_TRANS[_{Name}_STATES][NFA_ALPHA_LEN][_{Name}_STATES] =
        {{"""
    Header1 = f"""
static unsigned char
    _{Name}_FINAL[_{Name}_STATES] ="""
    Footer =  f"""
/* End of _{Name}_. */"""
    OneBlock = " {" + "0, " * (Num - 1) + "0}" 
    OneRow = "{" + (OneBlock + ",") * 2 + OneBlock + " },"
    AllRows = [OneRow + f" /* Q{i} */" for i in range(Num)]
    FinalStates = "        { " + "0, " * (Num - 1) + "0 };"
    AllStates = [f"Q{i}," for i in range(Num)]

    # Dump them all on the screen.
    print(Header)
    for Row in AllRows:
        print("             " + Row)
    print("        };")
    print(Header1)
    print(FinalStates)
    print("      /* ", end ='')
    for State in AllStates:
        print(State, end = '')
    print(" */", end ='')
    print(Footer)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f"Usage {sys.argv[0]} <NAME> <NUM_STATES>")
        exit(1)

    entry(sys.argv[1], int(sys.argv[2]))
