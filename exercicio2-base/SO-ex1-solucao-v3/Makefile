# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2016-17

CFLAGS=-g -Wall -pedantic -pthread
CC=gcc

all: i-banco

i-banco: i-banco.o commandlinereader.o contas.o
	$(CC) $(CFLAGS) -o i-banco i-banco.o commandlinereader.o contas.o

i-banco.o: i-banco.c contas.h commandlinereader.h
	$(CC) $(CFLAGS) -c i-banco.c

contas.o: contas.c contas.h
	$(CC) $(CFLAGS) -c contas.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	$(CC) $(CFLAGS) -c commandlinereader.c

clean:
	rm -f *.o i-banco
