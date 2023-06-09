cc := gcc
flags := -g
#flags += -DNDEBUG

libs :=
obj := automata.o set.o setOfSets.o
proj := re

all: $(proj)

$(proj): $(obj) automata.h
	$(cc) $(flags) $(libs) $(obj) -o $@

%.o : %.c %.h automata.h
	$(cc) -c $(flags) $< -o $@

%.o : %.c automata.h
	$(cc) -c $(flags) $< -o $@

clean:
	rm $(proj) $(obj)
