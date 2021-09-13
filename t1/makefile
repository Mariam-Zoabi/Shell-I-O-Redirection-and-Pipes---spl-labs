# makefile

all: task1

task1:	LineParser.o	task1.o
	gcc -m32 -g -Wall LineParser.o task1.o -o mypipe

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c LineParser.c -o LineParser.o

task1.o: task1.c
	gcc -m32 -g -Wall -c task1.c -o task1.o

clean:
	rm -f *.o
	rm -f mypipe