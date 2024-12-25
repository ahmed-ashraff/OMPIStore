CC = mpicxx
CFLAGS = -Wall -std=c++20

all: main

main: main.o coordinator.o shard.o client.o
	$(CC) $(CFLAGS) -o main main.o coordinator.o shard.o client.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

coordinator.o: coordinator.cpp
	$(CC) $(CFLAGS) -c coordinator.cpp

shard.o: shard.cpp
	$(CC) $(CFLAGS) -c shard.cpp

client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp

run: main
	mpirun -np 4 ./main

clean:
	rm -f *.o main
