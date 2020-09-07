#include "contas.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define atrasar() sleep(ATRASO)

int deveTerminar = 0;
		     
int contasSaldos[NUM_CONTAS];


pthread_mutex_t account_ctrl[NUM_CONTAS];

void initTrincosContas() {
  int rc;

  for(int i=0; i<NUM_CONTAS; i++) {
    rc = pthread_mutex_init(&account_ctrl[i], NULL);
    
    if (rc != 0) {
      errno = rc;
      perror("Error in pthread_mutex_init");
      exit(EXIT_FAILURE);
    }
  }
}

void lock_account(int account) {
  int rc;

  assert(contaExiste(account));
  rc = pthread_mutex_lock(&account_ctrl[account-1]);

  /* confirmar que antes foi verificado que se trata de uma conta válida */
  if (rc != 0) {
    errno = rc;
    perror("Error in pthread_mutex_lock");
    exit(EXIT_FAILURE);
  }
}

void unlock_account(int account) {
  int rc;

  assert(contaExiste(account));
  rc = pthread_mutex_unlock(&account_ctrl[account-1]);

  /* confirmar que antes foi verificado que se trata de uma conta válida */
  if (rc != 0) {
    errno = rc;
    perror("Error in pthread_mutex_unlock");
    exit(EXIT_FAILURE);
  }
}

int contaExiste(int idConta) {
  return (idConta > 0 && idConta <= NUM_CONTAS);
}

void inicializarContas() {
  int i;
  for (i=0; i<NUM_CONTAS; i++)
    contasSaldos[i] = 0;
}

int debitar(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;

  lock_account(idConta);

  if (contasSaldos[idConta - 1] < valor) {
    unlock_account(idConta);
    return -1;
  }
  atrasar();
  contasSaldos[idConta - 1] -= valor;

  unlock_account(idConta);
  return 0;
}

int creditar(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;

  lock_account(idConta);
  contasSaldos[idConta - 1] += valor;
  unlock_account(idConta);
  return 0;
}

int lerSaldo(int idConta) {
  int saldo;

  atrasar();
  if (!contaExiste(idConta))
    return -1;

  lock_account(idConta);
  saldo = contasSaldos[idConta - 1];
  unlock_account(idConta);
  return saldo;
}

int transferirClandestino(int idContaOrigem) {
  int idContaMenor = 1;
  int idContaMaior = idContaOrigem;
  int valor;

  atrasar();
  if (idContaOrigem == idContaMenor) {
	printf("Conta 1 : %d\n",lerSaldo(idContaOrigem));
	return 0;
  }
  else if (!contaExiste(idContaOrigem))
    return -1;

  lock_account(idContaMenor);
  lock_account(idContaMaior);
  atrasar();
  
  valor = contasSaldos[idContaOrigem - 1];
  
  valor = valor * 0.1;
  
  contasSaldos[idContaOrigem - 1] -= valor;

  contasSaldos[idContaMenor - 1] += valor;

  unlock_account(idContaMaior);
  unlock_account(idContaMenor);
  return 0;
}

int transferir(int idContaOrigem, int idContaDestino, int valor) {
  int idContaMenor, idContaMaior;

  atrasar();
  if (!contaExiste(idContaOrigem) || !contaExiste(idContaDestino) || idContaOrigem == idContaDestino)
    return -1;

  if (idContaOrigem > idContaDestino)
  {
    idContaMenor = idContaDestino;
    idContaMaior = idContaOrigem;
  }
  else
  {
    idContaMenor = idContaOrigem;
    idContaMaior = idContaDestino;
  }

  lock_account(idContaMenor);
  lock_account(idContaMaior);

  if (contasSaldos[idContaOrigem - 1] < valor) {
    unlock_account(idContaMaior);
    unlock_account(idContaMenor);
    return -1;
  }
  atrasar();
  contasSaldos[idContaOrigem - 1] -= valor;

  contasSaldos[idContaDestino - 1] += valor;

  unlock_account(idContaMaior);
  unlock_account(idContaMenor);
  return 0;
}

void trataSignal(int sig) {
  (void)sig;
  deveTerminar = 1;
}

void simular(int numAnos) {
  int id, saldo;
  int ano = 0;
  
  for (ano = 0; 
       ano <= numAnos && !deveTerminar;
       ano++) {
    
    printf("SIMULAÇÃO: Ano %d\n=================\n", ano);
    for (id = 1; id<=NUM_CONTAS; id++) {
      if (ano > 0) {
        saldo = lerSaldo(id);
        creditar(id, saldo * TAXAJURO);
        saldo = lerSaldo(id);
        debitar(id, (CUSTOMANUTENCAO > saldo) ? saldo : CUSTOMANUTENCAO);
      }
      saldo = lerSaldo(id);
      /* A funcao printf pode ser interrompida pelo SIGUSR1,
	 retornando -1 e colocando errno=EINTR.
	 Para lidar com esse caso, repetimos o printf sempre
	 que essa situacao se verifique. 
	 Nota: este aspeto e' de nivel muito avancado, logo
	 so' foi exigido a projetos com nota maxima
      */
      while (printf("Conta %5d,\t Saldo %10d\n", id, saldo) < 0) {
        if (errno == EINTR)
          continue;
        else
          break;
      }
    }
  }
  
  if (deveTerminar)
    printf("Simulacao terminada por signal\n");
    //printf("Simulacao terminada por ordem do processo pai\n");
  
}

void acc_mutex_destroyer() {
  int rc;

  for(int i=0; i<NUM_CONTAS; i++) {
    rc = pthread_mutex_destroy(&account_ctrl[i]);
    
    if (rc != 0) {
      errno = rc;
      perror("Error in pthread_mutex_destroy");
      exit(EXIT_FAILURE);
    }
  }
}
