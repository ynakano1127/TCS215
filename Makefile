# Which compiler
CC=gcc

# Compiler Options for development
CFLAGS=-Wall -g3

all:				tagServer tagClient cursesTest mazeTest

tagServer:	tagServer.c tagGame.o tagGameForServer.o maze.o mnet.a
						$(CC) $(CFLAGS) -o tagServer tagServer.c tagGame.o tagGameForServer.o maze.o mnet.a -lcurses

tagClient:	tagClient.c tagGame.o tagGameForClient.o maze.o snet.a
						$(CC) $(CFLAGS) -o tagClient tagClient.c tagGame.o tagGameForClient.o maze.o snet.a -lcurses

tagGameForServer.o:	tagGameForServer.c tagGameForServer.h 
						$(CC) $(CFLAGS) -c tagGameForServer.c 

tagGameForClient.o:	tagGameForClient.c tagGameForClient.h 
						$(CC) $(CFLAGS) -c tagGameForClient.c 

tagGame.o:	tagGame.c tagGame.h
						$(CC) $(CFLAGS) -c tagGame.c


snet.a:			setupServer.o setupClient.o snet.h
						ar crs snet.a setupServer.o setupClient.o
setupServer.o:	setupServer.c
						$(CC) $(CFLAGS) -c setupServer.c 
setupClient.o:	setupClient.c
						$(CC) $(CFLAGS) -c setupClient.c 


mnet.a:			mnet.o mnet.h
						ar crs mnet.a mnet.o
mnet.o:	mnet.c
						$(CC) $(CFLAGS) -c mnet.c 


maze.o:	maze.c maze.h
						$(CC) $(CFLAGS) -c maze.c


cursesTest:	tests/cursesTest.c
	$(CC) $(CFLAGS) -o cursesTest.out tests/cursesTest.c -lcurses

mazeTest:	tests/mazeTest.c maze.o
	$(CC) $(CFLAGS) -o mazeTest.out tests/mazeTest.c maze.o


clean:
						rm -f tagServer tagClient *.o *.out core.*
