SOURCES=$(wildcard *.c)
HEADERS=$(SOURCES:.c=.h)
FLAGS=-DDEBUG -g

all: main

main: $(SOURCES) $(HEADERS)
	mpicc $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

# run: main
# 	# mpirun -oversubscribe -np 8 ./main

run: main
	mpirun -oversubscribe -np $(shell nproc) ./main

run_cluster: main
	mpirun -np 28 -hostfile hosts.txt ./main
