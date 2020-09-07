#include "contas.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define atrasar() sleep(ATRASO)

int deveTerminar = 0;
int contasSaldos[NUM_CONTAS];


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

  if (contasSaldos[idConta - 1] < valor)
    return -1;

  contasSaldos[idConta - 1] -= valor;

  return 0;
}

int creditar(int idConta, int valor) {
  atrasar();

  if (!contaExiste(idConta))
    return -1;

  contasSaldos[idConta - 1] += valor;

  return 0;
}

int lerSaldo(int idConta) {
  atrasar();

  if (!contaExiste(idConta))
    return -1;

  return contasSaldos[idConta - 1];
}

void trataSignal(int sig) {
  //printf("Processo %d recebeu o signal\n", getpid());
  deveTerminar = 1;
}

void simular(int numAnos) {
  int id, saldo;
  int ano = 0;

  for (ano = 0; ano <= numAnos && !deveTerminar; ano++) {

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
int stats() {
	int k;
	int contasStats[NUM_CONTAS];
	float media = 0, variancia = 0;
	for(int k=0 ; k<NUM_CONTAS; k++) {
		contasStats[k]= contasSaldos[k];
	}
	for(k=0 ; k<NUM_CONTAS; k++) {
		media += contasStats[k];
	}
	media = media / NUM_CONTAS;
	printf("media: %0.01f \n" , media);
	for(k=0 ; k<NUM_CONTAS;k++) {
		variancia += (contasStats[k]-media)*(contasStats[k]-media);
	}
	variancia= variancia/ NUM_CONTAS;
	printf("variancia: %0.01f\n" , variancia);
	return 0;
	
}
