cc := gcc
flags :=
#flags += -DNDEBUG

libs :=
obj := automata.o queue.o set_2.o setOfSets.o
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
