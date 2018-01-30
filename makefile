#makefile
EXENAME = run

MAIN=qrs

CC=gcc

CFLAGS= -Wall -lpthread -lrt -lm
OBJS = $(MAIN).o

LIBS = `allegro-config --libs`

$(MAIN): $(OBJS)
	$(CC) -o $(EXENAME) $(OBJS) $(LIBS) $(CFLAGS)

$(MAIN).o: $(MAIN).c
	$(CC) -c $(MAIN).c

all: $(MAIN) 

clean:
	rm -f *.o $(EXENAME)

