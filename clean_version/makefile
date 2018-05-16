#makefile
EXENAME = run

MAIN = main
CO = compute
GR = graph
FR = frequency
KB = keyboard

CC = gcc

CFLAGS = -Wall -lpthread -lrt -lm
OBJS = $(MAIN).o $(CO).o $(GR).o $(FR).o $(KB).o

LIBS = `allegro-config --libs`

$(MAIN): $(OBJS)
	$(CC) -o $(EXENAME) $(OBJS) $(LIBS) $(CFLAGS)

$(MAIN).o: $(MAIN).c
	$(CC) $(CFLAGS) -c $(MAIN).c
	
all: $(MAIN) 

clean:
	rm -f *.o $(EXENAME)

