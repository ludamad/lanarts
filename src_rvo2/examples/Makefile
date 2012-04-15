CC = g++
DEBUG = -g
OMP = -fopenmp
CFLAGS = -Wall -c $(DEBUG) $(OMP) -I../include/
LFLAGS = -Wall $(DEBUG) -L../lib -lRVO -lgomp

all : ExampleBlocks ExampleCircle ExampleRoadmap

ExampleBlocks : ExampleBlocks.o
	$(CC) $(LFLAGS) ExampleBlocks.o -o ExampleBlocks
	rsync ExampleBlocks ../bin/
	
ExampleBlocks.o : ExampleBlocks.cpp
	$(CC) $(CFLAGS) ExampleBlocks.cpp

ExampleCircle : ExampleCircle.o
	$(CC) $(LFLAGS) ExampleCircle.o -o ExampleCircle
	rsync ExampleCircle ../bin/
	
ExampleCircle.o : ExampleCircle.cpp
	$(CC) $(CFLAGS) ExampleCircle.cpp
	
ExampleRoadmap : ExampleRoadmap.o
	$(CC) $(LFLAGS) ExampleRoadmap.o -o ExampleRoadmap
	rsync ExampleRoadmap ../bin/
	
ExampleRoadmap.o : ExampleRoadmap.cpp
	$(CC) $(CFLAGS) ExampleRoadmap.cpp

clean:
	rm -f *.o ExampleBlocks ExampleCircle ExampleRoadmap ../bin/ExampleBlocks ../bin/ExampleCircle ../bin/ExampleRoadmap
