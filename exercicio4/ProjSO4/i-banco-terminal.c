
#include "commandlinereader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define COMANDO_DEBITAR     "debitar"
#define COMANDO_CREDITAR    "creditar"
#define COMANDO_LER_SALDO     "lerSaldo"
#define COMANDO_SIMULAR     "simular"
#define COMANDO_SAIR      "sair"
#define COMANDO_ARG_SAIR_AGORA  "agora"
#define COMANDO_TRANSFERIR    "transferir"
#define COMANDO_SAIR_TERMINAL "sair-terminal"

#define OP_LER_SALDO   0
#define OP_CREDITAR    1
#define OP_DEBITAR     2
#define OP_SAIR        3
#define OP_TRANSFERIR  4
#define OP_SAIR_AGORA  5
#define OP_SIMULAR     6
#define OP_ADI_TERMINAL 7
#define OP_REM_TERMINAL 8


#define MAXARGS   4
#define BUFFER_SIZE 100

typedef struct
{
  char path[200];
  int operacao;
  int idConta;
  int idContaDestino;
  int valor;
} comando_t;
  
int fserv;
char path[200];

void initpipe(){
	sprintf(path, "%d", getpid());
	
	unlink(path);
  
	if(mkfifo(path,0777)<0)
		exit(-1);
}

void init() {
	if((fserv = open(path,O_RDONLY))<0)
	  exit(-1);
}

void send_to_pipe(int fcli,comando_t *cmd) {
  time_t ini,fim;
  
  char message[200];
  
  if(write(fcli,cmd,sizeof(comando_t))<0)
	printf("Erro ao enviar comando a i-banco.\n");
	
  if(cmd->operacao!=OP_REM_TERMINAL && cmd->operacao!=OP_ADI_TERMINAL) {
	if(fserv==0)
		init();
		time(&ini);
	if(read(fserv,message,sizeof(message))<0)
		printf("Erro ao receber mensagem de i-banco.\n");
	else {
		time(&fim);
		printf("%s",message);
		printf("tempo: %f\n\n", difftime(fim,ini));
	}
  }
}

void trataSignal(int sig) {
  printf("Erro,nao existe i-banco.\n");
  if (signal(SIGPIPE, trataSignal) == SIG_ERR) {
    perror("Erro ao definir signal.");
    exit(EXIT_FAILURE);
  }
}

int main (int argc, char** argv) {  
  char *args[MAXARGS + 1];
  char buffer[BUFFER_SIZE];
  comando_t cmd;
   int send_cmd_to_pipe;
   
   int fcli;
   
   if((fcli = open(argv[1],O_WRONLY))<0) exit(-1);
  
  printf("Bem-vinda/o ao i-banco-terminal\n\n");
  
  cmd.operacao = OP_ADI_TERMINAL;
  send_to_pipe(fcli,&cmd);
  
  initpipe();
  strcpy(cmd.path,path);

  if (signal(SIGPIPE, trataSignal) == SIG_ERR) {
    perror("Erro ao definir signal.");
    exit(EXIT_FAILURE);
  }
  
  while (1) {
    int numargs;
    
    numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

    send_cmd_to_pipe = 0; /* default is NO (do not send) */

	if (numargs < 0 || (numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0))) {
	  cmd.operacao = OP_SAIR;
      if (numargs > 1 && (strcmp(args[1], COMANDO_ARG_SAIR_AGORA) == 0)) {
		  cmd.operacao = OP_SAIR_AGORA;
      }
      send_cmd_to_pipe=1;
    }


    else if (numargs == 0)
      /* Nenhum argumento; ignora e volta a pedir */
      continue;


    /* Debitar */
    else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
      if (numargs < 3) {
        printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
        continue;
      }
      cmd.operacao = OP_DEBITAR;
      cmd.idConta = atoi(args[1]);
      cmd.valor = atoi(args[2]);

      send_cmd_to_pipe = 1;
    }
    
    /* Creditar */
    else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
      if (numargs < 3) {
        printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
        continue;
      }
      cmd.operacao = OP_CREDITAR;
      cmd.idConta = atoi(args[1]);
      cmd.valor = atoi(args[2]);

      send_cmd_to_pipe = 1;
    }

    /* Tranferir */
    else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {
      if (numargs < 4) {
        printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_TRANSFERIR);
        continue;
      }
      cmd.operacao = OP_TRANSFERIR;
      cmd.idConta = atoi(args[1]);
      cmd.idContaDestino = atoi(args[2]);
      cmd.valor = atoi(args[3]);

      send_cmd_to_pipe = 1;
    }
    
    /* Ler Saldo */
    else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
      if (numargs < 2) {
        printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
        continue;
      }
      cmd.operacao = OP_LER_SALDO;
      cmd.idConta = atoi(args[1]);

      send_cmd_to_pipe = 1;
    }
    /*Comando sair terminal*/
    else if (strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0){
		printf("--\ni-banco-terminal terminou.\n");
		cmd.operacao = OP_REM_TERMINAL;
		send_to_pipe(fcli,&cmd);
		close(fcli);
		unlink(path);
		exit(EXIT_SUCCESS);
	}
    /*Simular*/
    else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
		if (numargs < 1) {
        printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
        continue;
        }
      cmd.operacao = OP_SIMULAR;
      cmd.idConta = atoi(args[1]);
      send_cmd_to_pipe = 1;
	}
    
    else
      printf("Comando desconhecido. Tente de novo.\n");


    if(send_cmd_to_pipe) {
      send_to_pipe(fcli,&cmd);
      send_cmd_to_pipe = 0;
    }
  }
  exit(EXIT_SUCCESS);
}
