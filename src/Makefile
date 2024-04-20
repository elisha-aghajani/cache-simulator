CC = g++
CFLAGS = -g -Wall

EXE = cache-sim

OBJS = main.o \
	simulator.o \
	cache.o

.PHONY: all clean

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS)

main.o: main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

simulator.o: simulator.cpp simulator.h
	$(CC) $(CFLAGS) -c $< -o $@

cache.o: cache.cpp cache.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXE)
