//o write tem que verificar a operacao antes de escrever!!! semaforo ou trinco ou???
/*
// Projeto SO - exercicio 1, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/
#include "commandlinereader.h"
#include "contas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

#define COMANDO_DEBITAR "debitar"
#define COMANDO_STATS "stats"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_ARG_SAIR_AGORA "agora"
#define OPERACAO_DEBITAR 1
#define OPERACAO_CREDITAR 2
#define OPERACAO_LER_SALDO 3
#define OPERACAO_TERMINAR 4
#define OPERACAO_STATS 5

#define MAXARGS 3
#define BUFFER_SIZE 100
#define MAXFILHOS 20
#define NUM_TRABALHADORAS 3
#define CMD_BUFFER_DIM  (NUM_TRABALHADORAS * 2)

void *consome();
void creditarM(int idConta, int valor);
void lerSaldoM(int idConta);
void debitarM(int idConta, int valor);
void statsM();

comando_t cmd_buffer[CMD_BUFFER_DIM];

int buff_write_idx = 0, buff_read_idx = 0;
						
pthread_t vecthreads[NUM_TRABALHADORAS]; 

pthread_mutex_t trinco;
pthread_mutex_t trincocontas[NUM_CONTAS];

sem_t sharedSem;
sem_t sharedSemBuffer;

int main (int argc, char** argv) {
  char *args[MAXARGS + 1];
  char buffer[BUFFER_SIZE];

  int numFilhos = 0;
  pid_t pidFilhos[MAXFILHOS];

  inicializarContas();
  
  pthread_mutex_init(&trinco, NULL);
  for (int k = 0; k<NUM_CONTAS; k++)
	pthread_mutex_init(&trincocontas[k], NULL);
  sem_init(&sharedSem, 0, 0);
  sem_init(&sharedSemBuffer, 0, 6);


  if (signal(SIGUSR1, trataSignal) == SIG_ERR) {

    perror("Erro ao definir signal.");

    exit(EXIT_FAILURE);
  }

  printf("Bem-vinda/o ao i-banco\n\n");
  
  for(int j= 0; j < NUM_TRABALHADORAS; j++) { 
		pthread_create(&(vecthreads[j]), NULL, consome,NULL);
  }
	
  while (1) {

    int numargs;

    numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

    /* EOF (end of file) do stdin ou comando "sair" */
    if (numargs < 0 || (numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0))) {

      printf("i-banco vai terminar.\n--\n");

      if (numargs > 1 && (strcmp(args[1], COMANDO_ARG_SAIR_AGORA) == 0)) {
	        int i;

	        for (i=0; i<numFilhos; i++)
	           kill(pidFilhos[i], SIGUSR1);
      }
      /* Uma alternativa igualmente correta: kill(0, SIGUSR1); */
      /* Espera pela terminacao de cada filho */
      while (numFilhos > 0) {

        int status;
        pid_t childpid;

        childpid = wait(&status);

        if (childpid < 0) {

	         if (errno == EINTR) {

	            /* Este codigo de erro significa que chegou um signal que interrompeu a espera
	             pela terminacao do filho, logo, voltamos a esperar */
	           continue;
	         }

           else {
	            perror("Erro inesperado ao esperar por processo filho.");
	            exit (EXIT_FAILURE);
	         }
	      }

	      numFilhos --;

	      if (WIFEXITED(status))
	         printf("FILHO TERMINADO: (PID=%d; terminou normalmente)\n", childpid);
	      else
	         printf("FILHO TERMINADO: (PID=%d; terminou abruptamente)\n", childpid);
        }
              
	  for(int k = 0; k < NUM_TRABALHADORAS; k++) {
	  sem_wait(&sharedSemBuffer);
	  pthread_mutex_lock(&trinco);
	  cmd_buffer[buff_write_idx].idConta = 11;
	  cmd_buffer[buff_write_idx].valor = 0;
      cmd_buffer[buff_write_idx].operacao = OPERACAO_TERMINAR;
	  buff_write_idx++;
	  if (buff_write_idx>5)
		buff_write_idx = 0;
	  if(pthread_mutex_unlock(&trinco)!=0) abort();
	  sem_post(&sharedSem);
      }
	  for(int w=0;w<NUM_TRABALHADORAS;w++) {
		pthread_join(vecthreads[w],NULL);
	  }	
	  
	  pthread_mutex_destroy(&trinco);
	  sem_destroy(&sharedSemBuffer);
      sem_destroy(&sharedSem);
      for (int k = 0; k<NUM_CONTAS; k++)
		pthread_mutex_destroy(&trincocontas[k]);

      printf("--\ni-banco terminou.\n");

      exit(EXIT_SUCCESS);
    }

    else if (numargs == 0)
      /* Nenhum argumento; ignora e volta a pedir */
      continue;

    /* Debitar */
    else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {

      int idConta, valor;
      
      if (numargs < 3) {

	       printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
         continue;
      }
      
      idConta = atoi(args[1]);
      valor = atoi(args[2]);
      
      sem_wait(&sharedSemBuffer);
      
      pthread_mutex_lock(&trinco);
      
      cmd_buffer[buff_write_idx].idConta = idConta;
      cmd_buffer[buff_write_idx].valor = valor;
      cmd_buffer[buff_write_idx].operacao = OPERACAO_DEBITAR;
      buff_write_idx++;
      if (buff_write_idx>5)
		buff_write_idx = 0;
      if(pthread_mutex_unlock(&trinco)!=0) abort();
      sem_post(&sharedSem); 
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
      
      sem_wait(&sharedSemBuffer);
      
      if(pthread_mutex_lock(&trinco)!=0) abort();
      
      cmd_buffer[buff_write_idx].idConta = idConta;
      cmd_buffer[buff_write_idx].valor = valor;
      cmd_buffer[buff_write_idx].operacao = OPERACAO_CREDITAR;
      buff_write_idx++;
      if (buff_write_idx>5)
		buff_write_idx = 0;
      if(pthread_mutex_unlock(&trinco)!=0) abort();
      sem_post(&sharedSem);
    }

    /* Ler Saldo */
    else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {

      int idConta;

      if (numargs < 2) {

	       printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
         continue;
      }

      idConta = atoi(args[1]);
      
      sem_wait(&sharedSemBuffer);

	  if(pthread_mutex_lock(&trinco)!=0) abort();

      cmd_buffer[buff_write_idx].idConta = idConta;
      cmd_buffer[buff_write_idx].valor = 0;
      cmd_buffer[buff_write_idx].operacao = OPERACAO_LER_SALDO;
      buff_write_idx++;
      if (buff_write_idx>5)
		buff_write_idx = 0;
      if(pthread_mutex_unlock(&trinco)!=0) abort();
      sem_post(&sharedSem);      
    }
    
    /* stats */
    else if (strcmp(args[0], COMANDO_STATS) == 0) {

      if (numargs < 1) {

	       printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_STATS);
         continue;
      }
      
      sem_wait(&sharedSemBuffer);

	  if(pthread_mutex_lock(&trinco)!=0) abort();

      cmd_buffer[buff_write_idx].idConta = 0;
      cmd_buffer[buff_write_idx].valor = 0;
      cmd_buffer[buff_write_idx].operacao = OPERACAO_STATS;
      buff_write_idx++;
      if (buff_write_idx>5)
		buff_write_idx = 0;
      if(pthread_mutex_unlock(&trinco)!=0) abort();
      sem_post(&sharedSem);      
    }

    /* Simular */
    else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {

      int numAnos;
      int pid;

      if (numFilhos >= MAXFILHOS) {

	       printf("%s: Atingido o numero maximo de processos filho a criar.\n",\
          COMANDO_SIMULAR);
         continue;
      }

      if (numargs < 2) {
	       printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
         continue;
      }

      numAnos = atoi(args[1]);

      pid = fork();

      if (pid < 0) {
	       perror("Failed to create new process.");
         exit(EXIT_FAILURE);
      }

      if (pid > 0) {
	       pidFilhos[numFilhos] = pid;

         numFilhos ++;

         printf("%s(%d): Simulacao iniciada em background.\n\n",\
          COMANDO_SIMULAR, numAnos);
         continue;
      }
      else {
        simular(numAnos);
        exit(EXIT_SUCCESS);
      }
    }

    else
      printf("Comando desconhecido. Tente de novo.\n");

  }
}
void *consome() {
	while(1) {
	sem_wait(&sharedSem);
	comando_t comando;
	if(pthread_mutex_lock(&trinco)!=0) abort();
	comando.operacao= cmd_buffer[buff_read_idx].operacao;
	comando.idConta= cmd_buffer[buff_read_idx].idConta;
	comando.valor = cmd_buffer[buff_read_idx].valor;
    buff_read_idx++;
	if (buff_read_idx>5)
		buff_read_idx = 0;
    if(pthread_mutex_unlock(&trinco)!=0) abort();
    if(!((comando.idConta-1 > 10) || (comando.idConta-1 < 0) || (comando.operacao == OPERACAO_TERMINAR) ||comando.operacao == OPERACAO_STATS))
		pthread_mutex_lock(&trincocontas[comando.idConta -1 ]);
	if(comando.operacao == OPERACAO_CREDITAR) {
		creditarM(comando.idConta, comando.valor);
	}
	else if (comando.operacao == OPERACAO_DEBITAR) {
		debitarM(comando.idConta, comando.valor);
	}
	else if(comando.operacao == OPERACAO_LER_SALDO) {
		lerSaldoM(comando.idConta);
	}
	else if(comando.operacao == OPERACAO_STATS) {
		statsM();
	}
	else if(comando.operacao == OPERACAO_TERMINAR) {
		return NULL; /*pthread_exit causava memleak pelo que se optou por return*/
	}
	if(!((comando.idConta-1 > 10) || (comando.idConta-1 < 0) || (comando.operacao == OPERACAO_TERMINAR) || comando.operacao == OPERACAO_STATS))
		if(pthread_mutex_unlock(&trincocontas[comando.idConta-1])!=0) abort();
	sem_post(&sharedSemBuffer);
	}	
}

void creditarM(int idConta, int valor) {
	if (creditar (idConta, valor) < 0)
	       printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
    else
	       printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
}
void debitarM(int idConta, int valor) {
	if (debitar (idConta, valor) < 0)
		printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
    else
		printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
}
void lerSaldoM(int idConta) {
	int saldo = lerSaldo (idConta);
	
     if (saldo < 0)
		printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
     else
	    printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
}
void statsM() {
	 for(int k=0 ; k<NUM_CONTAS; k++)
		pthread_mutex_lock(&trincocontas[k]);
     if (stats() < 0)
		printf("%s: Erro.\n\n", COMANDO_STATS);
	 for(int k=0 ; k<NUM_CONTAS; k++)
		pthread_mutex_unlock(&trincocontas[k]);
}
    
