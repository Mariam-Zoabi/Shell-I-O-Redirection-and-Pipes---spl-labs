# makefile

all: task3

task3:	LineParser.o	task3.o
	gcc -m32 -g -Wall LineParser.o task3.o -o mypipeline

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c LineParser.c -o LineParser.o

task3.o: task3.c
	gcc -m32 -g -Wall -c task3.c -o task3.o

clean:
	rm -f *.o
	rm -f mypipeline