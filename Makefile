# Which compiler
CC=gcc

# Compiler Options for development
CFLAGS=-Wall

all:				tagServer tagClient cursesTest mazeTest

tagServer:	tagServer.c tagGame.o
						$(CC) $(CFLAGS) -o tagServer tagServer.c tagGame.o snet.a -lcurses

tagClient:	tagClient.c tagGame.o
						$(CC) $(CFLAGS) -o tagClient tagClient.c tagGame.o snet.a -lcurses

tagGame.o:	tagGame.c tagGame.h
						$(CC) $(CFLAGS) -c tagGame.c

maze.o:	maze.c maze.h
						$(CC) $(CFLAGS) -c maze.c

cursesTest:	tests/cursesTest.c
	$(CC) $(CFLAGS) -o cursesTest.out tests/cursesTest.c -lcurses

mazeTest:	tests/mazeTest.c maze.o
	$(CC) $(CFLAGS) -o mazeTest.out tests/mazeTest.c maze.o

clean:
						rm -f tagServer tagClient *.o *.out
