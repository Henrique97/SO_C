main: i-banco.o contas.o commandlinereader.o
	gcc -o main i-banco.o contas.o commandlinereader.o

i-banco.o: i-banco.c
	gcc -Wall -g -pedantic -c i-banco.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc -Wall -g -pedantic -c commandlinereader.c

contas.o: contas.c contas.h
	gcc -Wall -g -pedantic -c contas.c



