/*
// Projeto SO - exercicio 1, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/
#include "commandlinereader.h"
#include "contas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_AGORA "agora"
#define COMANDO_SAIR "sair"
#define COMANDO_INFO "info"

#define MAXARGS 3
#define BUFFER_SIZE 100
#define MAX_NUM_PIDS 20

void apanhaSignalMain (int sig);

int main (int argc, char** argv) {

    char *args[MAXARGS + 1];
    char buffer[BUFFER_SIZE];
	int iter = 0; /*iterador do vetor de filhos*/
    inicializarContas();

    printf("Bem-vinda/o ao i-banco\n\n");
    
    if (signal (SIGUSR1, apanhaSignalMain) == SIG_ERR) { /*arma o signal da main*/
			puts("Erro: A sair");
			exit(EXIT_FAILURE);
    }
    while (1) {
        int numargs;
    
        numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        /* EOF (end of file) do stdin ou comando "sair" */
        if (numargs < 0 ||
	        (numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0))) {
				int i;
				int status;
				puts("i-banco vai terminar.\n --");
				/*sair agora*/
				if(numargs==2 && (strcmp(args[1], COMANDO_AGORA) == 0)) {
					/*Erro no comando sair agora:Nao foi possivel mandar
					 * um sinal kill para os filhos. Sair agora e ignorado
					 * e o programa sai normalmente*/
					if (kill(0, SIGUSR1)==-1)
						puts("Erro: Nao e possivel sair agora");
				}	
				/*sair normal a esperar pelos filhos*/	
				for (i=0; i<iter; i++) {
					int pid = wait(&status);
					/*caso ocorra um erro inesperado o programa sai*/
					if(pid<=0) {
						printf("%s:Erro", COMANDO_SAIR);
						exit(EXIT_FAILURE);
					}
					/*verificacao do status para determinar se o processo
					 * filho foi morto ou saiu sem erros*/
					else{
						if(status==0) {
							printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n",pid);
						}
						else{
							printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n",pid);
						}
					}
				}
			puts("i-banco terminou.");
            exit(EXIT_SUCCESS);
        }
    
        else if (numargs == 0)
            /* Nenhum argumento; ignora e volta a pedir */
            continue;
        
        /*Comando Info*/
        else if (numargs == 1 && (strcmp(args[0], COMANDO_INFO) == 0)) {
			printf("Numero de filhos: %d\n", iter);			
		}
        /* Debitar */
        else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
            int idConta, valor;
            if (numargs < 3) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
	           continue;
            }

            idConta = atoi(args[1]);
            valor = atoi(args[2]);

            if (debitar (idConta, valor) < 0)
	           printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
            else
                printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
		}

		/* Creditar */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
			int idConta, valor;
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			idConta = atoi(args[1]);
			valor = atoi(args[2]);

			if (creditar (idConta, valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
		}
		
		/* Ler Saldo */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			int idConta, saldo;
			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}
			idConta = atoi(args[1]);
			saldo = lerSaldo (idConta);
			if (saldo < 0)
				printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
			else
				printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
		}

		/* Simular */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
			int pid = fork();
			/*caso de estarmos num processo filho. Executar uma simulacao*/
			if (pid == 0) {
				simular(atoi(args[1]));
				exit(EXIT_SUCCESS);
			}
			/*existiu um erro na criacao dum processo filho. Sair do programa*/
			else if (pid < 0) {
				printf("%s: Erro.\n\n", COMANDO_SIMULAR);
				exit(EXIT_FAILURE);
			}
			/*caso de estarmos no processo pai. Incrementar o numero de processos filhos*/
			else {
				iter++;
			} 
		}
			
		/*Comando Invalido*/
		else
		printf("Comando desconhecido. Tente de novo.\n");
	}
}
void apanhaSignalMain (int sig){
	apanhaSignal();
}
