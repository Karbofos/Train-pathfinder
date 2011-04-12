lCC = gcc

CFLAGS = -Wall

all = trains

trains = trains.o

trains.o = trains.c

.PHONY: clean

.PHONY: compile

.PHONY: check-syntax

compile:
	gcc -o "trains" "trains.c" `sdl-config --cflags --libs`  -std=c99 -Wall -pedantic
clean:
	rm -f trains.o

check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
