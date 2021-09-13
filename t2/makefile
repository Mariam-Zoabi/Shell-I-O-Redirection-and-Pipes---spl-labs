# makefile

all: task2

task2:	LineParser.o	task2.o
	gcc -m32 -g -Wall LineParser.o task2.o -o mypipe

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c LineParser.c -o LineParser.o

task2.o: task2.c
	gcc -m32 -g -Wall -c task2.c -o task2.o

clean:
	rm -f *.o
	rm -f mypipe