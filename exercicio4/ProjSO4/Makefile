# Makefile, versao 3
# Sistemas Operativos, DEI/IST/ULisboa 2016-17

#                         -std=gnu99  to allow c++ comments ("//")
CFLAGS=-g -Wall -pedantic -std=gnu99
CC=gcc

all: i-banco i-banco-terminal

i-banco: i-banco.o contas.o
	$(CC) $(CFLAGS) -pthread -o i-banco i-banco.o contas.o

i-banco-terminal: i-banco-terminal.o commandlinereader.o
	$(CC) $(CFLAGS) -o i-banco-terminal i-banco-terminal.o commandlinereader.o

i-banco.o: i-banco.c contas.h commandlinereader.h
	$(CC) $(CFLAGS) -c i-banco.c

contas.o: contas.c contas.h
	$(CC) $(CFLAGS) -c contas.c

i-banco-terminal.o: i-banco-terminal.c commandlinereader.h
	$(CC) $(CFLAGS) -c i-banco-terminal.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	$(CC) $(CFLAGS) -c commandlinereader.c

clean:
	rm -f *.o i-banco


zip: clean
	rm -f SO-ex2-solucao.zip
	zip   SO-ex2-solucao.zip *

run:
	./i-banco < test1.txt

