#include "contas.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define atrasar() sleep(ATRASO)
		     
int contasSaldos[NUM_CONTAS];
int sigFlag = INATIVA;


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
  atrasar();
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

void simular(int numAnos) {
	int i,j;
	/*criacao dum vetor com tamanho igual ao no. de contas*/
	int saldoNovo[NUM_CONTAS];
	for(i=0; i	<= numAnos; i++) {
		printf("SIMULACAO: Ano %d\n", i);
		printf("=================\n");
		for(j=0; j< NUM_CONTAS; j++) {
			/* caso ano 0: apenas e copiado para o vetor saldoNovo o
			 * saldo atual de cada conta*/
			if( i == 0 )
				saldoNovo[j] = lerSaldo(j+1);
			/* caso geral: o saldo no vetor saldoNovo e alterado tendo
			 * em conta a taxa de manutencao e a taxa de juro.*/
			else {
		/*1*/	saldoNovo[j] = (int)((((saldoNovo[j])*(1 +TAXAJURO))-CUSTOMANUTENCAO));
				if (saldoNovo[j]<0)
					saldoNovo[j] = 0;
			}
			printf("Conta %d, Saldo %d \n", (j+1), saldoNovo[j]);		
		}
		/*verificacao de flag: interrompe processo se flag ativa*/
		if (sigFlag== ATIVA) {
			puts("Simulacao interrompida por signal");
			exit(EXIT_SUCCESS);
		}
	}
}

/* Existia a possibilidade de na funcao simular utilizar o vetor contas
 * e as funcoes debitar e creditar (alterar diretamente no vetor inicial
 * visto que as variaveis nao sao partilhadas entre processos) em vez de
 * criar um novo vetor. Contudo, este metodo e mais lento visto que
 * utiliza duas funcoes e duas clausulas if que podem ser simplesmente
 * substituidas pela linha 1*/
 
/*void simular(int numAnos) {
	int i,j, valorAdicionar, valorDiferenca;
	for(i=0; i	<= numAnos; i++) {
		printf("SIMULACAO: Ano %d\n", i);
		printf("=================\n");
		for(j=0; j< NUM_CONTAS; j++) {
			if(i!=0) {
				valorAdicionar = (lerSaldo(j+1)*(TAXAJURO))-CUSTOMANUTENCAO;
				if (valorAdicionar<0) {
					if((valorDiferenca= lerSaldo(j+1) + valorAdicionar) <0 )
						valorAdicionar -= valorDiferenca;
					debitar((j+1),(-valorAdicionar));
				}
				else
					creditar((j+1),valorAdicionar);
			}
			printf("Conta %d, Saldo %d \n", (j+1), lerSaldo(j+1));		
		}
		if (sigFlag== ATIVA) {
			puts("Simulacao interrompida por signal");
			exit(EXIT_SUCCESS);
		}
	}
} */
 
void apanhaSignal() {
	sigFlag = ATIVA;
}
