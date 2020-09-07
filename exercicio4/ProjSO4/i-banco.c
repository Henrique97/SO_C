/*
// Projeto SO - exercicio 2, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#include "contas.h"
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

#define COMANDO_DEBITAR     "debitar"
#define COMANDO_CREDITAR    "creditar"
#define COMANDO_LER_SALDO     "lerSaldo"
#define COMANDO_SIMULAR     "simular"
#define COMANDO_SAIR      "sair"
#define COMANDO_ARG_SAIR_AGORA  "agora"
#define COMANDO_TRANSFERIR    "transferir"

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

#define MAXFILHOS   20


typedef struct
{
  char path[200];
  int operacao;
  int idConta;
  int idContaDestino;
  int valor;
} comando_t;


#define NUM_TRABALHADORAS  3
#define CMD_BUFFER_DIM     (NUM_TRABALHADORAS * 2)

comando_t cmd_buffer[CMD_BUFFER_DIM];

int buff_write_idx = 0, buff_read_idx = 0, active_threads = 0;


pthread_mutex_t buffer_ctrl, active_threads_ctrl;
sem_t sem_read_ctrl, sem_write_ctrl;

pthread_t thread_id[NUM_TRABALHADORAS];

pthread_cond_t simular_ctrl;

int t_num[NUM_TRABALHADORAS]; /*##aux*/


void wait_on_read_sem(void) {
  while (sem_wait(&sem_read_ctrl) != 0) {
    if (errno == EINTR)
      continue;

    perror("Error waiting at semaphore \"sem_read_ctrl\"");
    exit(EXIT_FAILURE);
  }
}
void post_to_read_sem(void) {
  if(sem_post(&sem_read_ctrl) != 0) {
    perror("Error posting at semaphore \"sem_read_ctrl\"");
    exit(EXIT_FAILURE);
  }
}

void wait_on_write_sem(void) {
  while (sem_wait(&sem_write_ctrl) != 0) {
    if (errno == EINTR)
      continue;

    perror("Error waiting at semaphore \"sem_write_ctrl\"");
    exit(EXIT_FAILURE);
  }
}
void post_to_write_sem(void) {
  if(sem_post(&sem_write_ctrl) != 0) {
    perror("Error posting at semaphore \"sem_write_ctrl\"");
    exit(EXIT_FAILURE);
  }
}


void lock_cmd_buffer(void) {
  int rc;

  if ((rc = pthread_mutex_lock(&buffer_ctrl)) != 0) {
    errno = rc;
    perror("Error in pthread_mutex_lock");
    exit(EXIT_FAILURE);
  }
}

void unlock_cmd_buffer(void) {
  int rc;

  if ((rc = pthread_mutex_unlock(&buffer_ctrl)) != 0) {
    errno = rc;
    perror("Error in pthread_mutex_unlock");
    exit(EXIT_FAILURE);
  }
}

void lock_active_threads_ctrl(void) {
  int rc;

  if ((rc = pthread_mutex_lock(&active_threads_ctrl)) != 0) {
    errno = rc;
    perror("Error in pthread_mutex_lock");
    exit(EXIT_FAILURE);
  }
}

void unlock_active_threads_ctrl(void) {
  int rc;

  if ((rc = pthread_mutex_unlock(&active_threads_ctrl)) != 0) {
    errno = rc;
    perror("Error in pthread_mutex_unlock");
    exit(EXIT_FAILURE);
  }
}

void put_cmd(comando_t *cmd) {
  cmd_buffer[buff_write_idx] = *cmd;
  buff_write_idx = (buff_write_idx+1) % CMD_BUFFER_DIM;
}

void get_cmd(comando_t *cmd) {
  *cmd = cmd_buffer[buff_read_idx];
  buff_read_idx = (buff_read_idx+1) % CMD_BUFFER_DIM;
}

void *thread_main(void *arg_ptr) {
  int t_num, fcli;
  comando_t cmd;
  char path[200];
  char message[200];

  t_num = *((int *)arg_ptr);


  while(1) {
    wait_on_read_sem();

    lock_active_threads_ctrl();
    active_threads++;
    unlock_active_threads_ctrl();
	
    lock_cmd_buffer();
    get_cmd(&cmd);
    unlock_cmd_buffer();

    post_to_write_sem();

	strcpy(path,cmd.path);
    
    if((fcli = open(path,O_WRONLY))<0)
		exit(-1);
    
    if(cmd.operacao == OP_LER_SALDO)
    {
      int saldo;

      saldo = lerSaldo (cmd.idConta);
      if (saldo < 0)
        //printf("Erro ao ler saldo da conta %d.\n", cmd.idConta);
        sprintf(message,"Erro ao ler saldo da conta %d.\n\n", cmd.idConta);
      else
        sprintf(message,"%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, cmd.idConta, saldo);
        //printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, cmd.idConta, saldo);
    }

    else if(cmd.operacao == OP_CREDITAR)
    {
      if (creditar (cmd.idConta, cmd.valor) < 0)
        sprintf(message,"Erro ao creditar %d na conta %d.\n\n", cmd.valor, cmd.idConta);
      else
        sprintf(message,"%s(%d, %d): OK\n\n", COMANDO_CREDITAR, cmd.idConta, cmd.valor);
    }

    else if(cmd.operacao == OP_DEBITAR)
    {
      if (debitar (cmd.idConta, cmd.valor) < 0)
        sprintf(message,"Erro ao debitar %d na conta %d.\n\n", cmd.valor, cmd.idConta);
      else
        sprintf(message,"%s(%d, %d): OK\n\n", COMANDO_DEBITAR, cmd.idConta, cmd.valor);
    }

    else if(cmd.operacao == OP_TRANSFERIR)
    {
      if(transferir(cmd.idConta, cmd.idContaDestino, cmd.valor) < 0)
        sprintf(message,"Erro ao transferir %d da conta %d para a conta %d.\n\n", cmd.valor, cmd.idConta, cmd.idContaDestino);
      else
        sprintf(message,"%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, cmd.idConta, cmd.idContaDestino, cmd.valor);
    }

    else if(cmd.operacao == OP_SAIR)
    {
      //printf("Thread %d terminated!\n", t_num);
      return NULL;
    }

    else /* unknown command; ignore */
    {
      sprintf(message,"Thread %d: Unknown command: %d\n\n", t_num, cmd.operacao);
      /* continue; O continue é retirado porque o contador das threads tem de ser decrementado. */
    }


    if(write(fcli,message,sizeof(message))<0)
		printf("Erro ao enviar mensagem a i-banco-terminal.\n");
	
    lock_active_threads_ctrl();
    active_threads--;
    pthread_cond_signal(&simular_ctrl);
    unlock_active_threads_ctrl();
  }
}

void start_threads(void) {
  int i, rc;

  for(i=0; i<NUM_TRABALHADORAS; ++i) {
//##    if (pthread_create(&thread_id[i], NULL, thread_main, NULL) != 0) {
    t_num[i] = i;
    if ((rc = pthread_create(&thread_id[i],
                             NULL,
                             thread_main,
                             (void *)&t_num[i])) != 0) {
      errno = rc;
      perror("Error in pthread_create");
      exit(EXIT_FAILURE);
    }
  }
}

void sem_destroyer() {
	if(sem_destroy(&sem_read_ctrl) != 0) {
		perror("Could not destroy semaphore \"sem_read_ctrl\"");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sem_write_ctrl) != 0) {
		perror("Could not destroy semaphore \"sem_read_ctrl\"");
		exit(EXIT_FAILURE);
	}
  	
}

void mutex_destroyer() {
	if(pthread_mutex_destroy(&active_threads_ctrl) != 0) {
		perror("Could not destroy semaphore \"sem_read_ctrl\"");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_destroy(&buffer_ctrl) != 0) {
		perror("Could not destroy semaphore \"sem_read_ctrl\"");
		exit(EXIT_FAILURE);
	}
}

int main (int argc, char** argv) {

  comando_t cmd;
  int numFilhos = 0;
  pid_t pidFilhos[MAXFILHOS];
  int numTerminaisAtivos = 0;
  
  
  inicializarContas();
  initTrincosContas();  
  /* Associa o signal SIGUSR1 'a funcao trataSignal.
     Esta associacao e' herdada pelos processos filho que venham a ser criados.
     Alternativa: cada processo filho fazer esta associacao no inicio da
     funcao simular; o que se perderia com essa solucao?
     Nota: este aspeto e' de nivel muito avancado, logo
     so' foi exigido a projetos com nota maxima  
  */
  if (signal(SIGUSR1, trataSignal) == SIG_ERR) {
    perror("Erro ao definir signal.");
    exit(EXIT_FAILURE);
  }


  pthread_mutex_init(&buffer_ctrl, NULL); /* this func allways return 0 (ok) */
  pthread_mutex_init(&active_threads_ctrl,NULL); /* this func allways return 0 (ok) */

  if(sem_init(&sem_read_ctrl, 0, 0) != 0) {
    perror("Could not initialize semaphore \"sem_read_ctrl\"");
    exit(EXIT_FAILURE);
  }
  if(sem_init(&sem_write_ctrl, 0, CMD_BUFFER_DIM) != 0) {
    perror("Could not initialize semaphore \"sem_read_ctrl\"");
    exit(EXIT_FAILURE);
  }

  start_threads();
  logger();

  
  /*###############Pipes##############*/

  int fserv, fd;
  unlink("i-banco-pipe");
  
  if(mkfifo("i-banco-pipe",0777)<0)
	exit(-1);
  
  printf("Bem-vinda/o ao i-banco\n\n");
	while (1) {
		
		if(numTerminaisAtivos==0) {
			if((fserv = open("i-banco-pipe",O_RDONLY))<0)
				exit(EXIT_FAILURE);
		}
		
		if ((fd =read(fserv, &cmd,sizeof(comando_t)))<0)
			exit(EXIT_FAILURE);


		if(cmd.operacao==OP_SAIR_AGORA) {
			for (int i=0; i<numFilhos; i++)		
				kill(pidFilhos[i], SIGUSR1);
			cmd.operacao= OP_SAIR;
		}
		
		if(cmd.operacao==OP_SAIR) {
			int rc;
			char message[200];
			int fcli;
			char path[200];
			printf("i-banco vai terminar.\n--\n");

			strcpy(path,cmd.path);
    
			if((fcli = open(path,O_WRONLY))<0)
				exit(-1);
			
			for(int i=0; i < NUM_TRABALHADORAS; ++i) {
			wait_on_write_sem();
			put_cmd(&cmd);
			post_to_read_sem();
			}
			
			/* Espera pela terminação de cada thread */
			for(int i=0; i < NUM_TRABALHADORAS; ++i) {         
				if ((rc = pthread_join(thread_id[i], NULL)) != 0) {
				errno = rc;
				perror("Error in pthread_join");
				exit(EXIT_FAILURE);
				}
			}	
			/* Espera pela terminacao de processos filho */
			while (numFilhos > 0) {
				int status;
				pid_t childpid;
	
				childpid = wait(&status);
				if (childpid < 0) {
					if (errno == EINTR) {
					/* Este codigo de erro significa que chegou signal que interrompeu a espera
					pela terminacao de filho; logo voltamos a esperar */
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
      
		  sem_destroyer();
		  mutex_destroyer();
	  	  acc_mutex_destroyer();
      
		  printf("--\ni-banco terminou.\n");
		  sprintf(message,"--\ni-banco terminou.\n");
		  if(write(fcli,message,sizeof(message))<0)
			printf("Erro ao enviar mensagem a i-banco-terminal.\n");
		  closelogger();
		  close(fserv);
		  exit(EXIT_SUCCESS);
		}
		
		else if(cmd.operacao== OP_ADI_TERMINAL) {
			numTerminaisAtivos++;
		}
		else if(cmd.operacao== OP_REM_TERMINAL) {
			numTerminaisAtivos--;
		}
		
		else if(cmd.operacao== OP_SIMULAR) {
			int numAnos;
			int pid;
			char message[200];
			int fcli;
			char path[200];
			
			strcpy(path,cmd.path);
    
			if((fcli = open(path,O_WRONLY))<0)
				exit(-1);
				
				
				
			if (numFilhos >= MAXFILHOS) {
				sprintf(message,"%s: Atingido o numero maximo de processos filho a criar.\n", COMANDO_SIMULAR);
				continue;
			}
      
			numAnos = cmd.idConta;

			lock_active_threads_ctrl();
			while(active_threads != 0) 
			pthread_cond_wait(&simular_ctrl, &active_threads_ctrl);

			pid = fork();
			unlock_active_threads_ctrl();
      
			if (pid < 0) {
				perror("Failed to create new process.");
				exit(EXIT_FAILURE);
			}
      
			if (pid > 0) {   
				pidFilhos[numFilhos] = pid;
				numFilhos ++;
				sprintf(message,"%s(%d): Simulacao iniciada em background.\n\n", COMANDO_SIMULAR, numAnos);
				if(write(fcli,message,sizeof(message))<0)
					printf("Erro ao enviar mensagem a i-banco-terminal.\n");
				continue;
			}
			else {
				char * s = malloc(snprintf(NULL, 0, "i-banco-sim-%d.txt",getpid()) + 1);
				sprintf(s, "i-banco-sim-%d.txt", getpid());	
				int fdchild= open(s, O_CREAT|O_TRUNC|O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR|S_IXUSR|S_IRWXG|S_IRGRP|S_IWGRP|S_IXGRP|S_IRWXO|S_IROTH|S_IWOTH|S_IXOTH);	
				free(s);
				dup2(fdchild,1);
				int args[1];
				args[0] = numAnos;
				logfunction((int)pthread_self(), "simular", args, 1);
				closelogger();
				simular(numAnos);
				exit(EXIT_SUCCESS);
			}
		}
		
		else {
			wait_on_write_sem();
			if (fd < 0) break;
			put_cmd(&cmd);
			post_to_read_sem();
		}
	}
}
