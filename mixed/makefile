#makefile
EXENAME = run

MAIN = main
CA = calcolo
GR = grafico
FR = frequenza
LE = lettura
TA = tastiera

CC = gcc

CFLAGS = -Wall -lpthread -lrt -lm
OBJS = $(MAIN).o $(CA).o $(GR).o $(FR).o $(LE).o $(TA).o

LIBS = `allegro-config --libs`

$(MAIN): $(OBJS)
	$(CC) -o $(EXENAME) $(OBJS) $(LIBS) $(CFLAGS)

$(MAIN).o: $(MAIN).c
	$(CC) -c $(MAIN).c
	
all: $(MAIN) 

clean:
	rm -f *.o $(EXENAME)

