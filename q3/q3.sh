mpicc -g -Wall main.c
mpirun --oversubscribe -np "$1" a.out
