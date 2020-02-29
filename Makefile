CCC = gcc
CXX_FLAGS := -Wall
LINK_FLAGS := -pthread -lncurses
TARGETS = elevator_19.o ncurse_elevator_simulation.o

all: $(TARGETS)
	rm log.txt && touch log.txt
	$(CCC) $(CXX_FLAGS) $(TARGETS) -o elevator $(LINK_FLAGS)
	./elevator log.txt
elevator_19.o: elevator_19.c elevator_19.h
	$(CCC) $(CXX_FLAGS) -c elevator_19.c $(LINK_FLAGS)
clean:
	rm -rf *.o elevator
