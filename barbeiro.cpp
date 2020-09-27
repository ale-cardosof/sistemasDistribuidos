#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"
#include <random>
#include <iostream>

#define CHAIRS 5                /* n�mero de cadeiras para os clientes � espera */
#define TRUE 1

sem_t customers;                /* n�mero de cliente � espera de atendimento */
sem_t barbers;                  /* n�mero de barbeiros � espera de clientes */
sem_t mutex;                    /* para exclus�o m�tua */
int waiting = 0;                /* clientes que est�o esperando (n�o est�o cortando) */

std::random_device rd; // peg aum numero random
std::mt19937 gen(rd()); 
std::uniform_int_distribution<> distr(2, 3); // define o range 20 a 30 minutos = 2 a 3 segundos
float dia = 144; //6 segundos = 1 hora //144 segundos = 24 horas
int clienteHora = 0;
int clienteTempo = 6;
int barbeiroDormiu = 0;
int numClientesAtendidos = 0;

/* prot�tipos */
void* barber(void *arg);
void* customer(void *arg);
void cut_hair();
void customer_arrived();
void get_haircut();
void giveup_haircut();
void barber_sleep();

int main() {
	sem_init(&customers, TRUE, 0);
	sem_init(&barbers, TRUE, 0);
	sem_init(&mutex, TRUE, 1);

	pthread_t b, c;

	/* criando �nico barbeiro */
	pthread_create(&b, NULL, barber, NULL);

	/* cria��o indefinida de clientes */
	while(dia >= 0) { 
		
		printf("Horas restantes do dia: %f \n", dia/6);
		customer_arrived();
		pthread_create(&c, NULL, customer, NULL);
		
		//clienteHora++;
		//clienteTempo -= 3;
		sleep(3);//dois clientes chegam em uma hora (6 seg), um cliente chega a cada 30 minutos (3 seg)
		dia -= 3;
	}

	printf("O barbeiro teve %d clientes \n", numClientesAtendidos);
	printf("O barbeiro dormiu %d \n", barbeiroDormiu);
	return 0;
}

void* barber(void *arg) {
	while(TRUE) {
		sem_wait(&customers);   /* vai dormir se o n�mero de clientes for 0 */
		sem_wait(&mutex);       /* obt�m acesso a 'waiting' */
		waiting = waiting - 1;  /*descresce de um o contador de clientes � espera */
		sem_post(&barbers);     /* um barbeiro est� agora pronto para cortar cabelo */
		sem_post(&mutex);       /* libera 'waiting' */
		cut_hair();             /* corta o cabelo (fora da regi�o cr�tica) */
		if(waiting == 0){
			barber_sleep();
		}
	}
	pthread_exit(NULL);
}

void* customer(void *arg) {
	sem_wait(&mutex);           /* entra na regi�o cr�tica */

	if(waiting < CHAIRS) {      /* se n�o houver cadeiras vazias, saia */
		//customer_arrived();
		waiting = waiting + 1;  /* incrementa o contador de clientes � espera */
		sem_post(&customers);   /* acorda o barbeiro se necess�rio */
		sem_post(&mutex);       /* libera o acesso a 'waiting' */
		sem_wait(&barbers);     /* vai dormir se o n�mero de barbeiros livres for 0 */
		get_haircut();          /* sentado e sendo servido */
	} else {
		sem_post(&mutex);       /* a barbearia est� cheia; n�o espera */
		giveup_haircut();
	}
	pthread_exit(NULL);
}

void cut_hair() {
	float tempoCorte = distr(gen);
	printf("Barbeiro esta cortando o cabelo de alguem!\n");
	dia = dia - tempoCorte;
	numClientesAtendidos++;
	printf("Horas restantes do dia: %f \n", dia/6);
	sleep(tempoCorte);	
}

void customer_arrived() {
	printf("Cliente chegou para cortar cabelo!\n");
}
void get_haircut() {
	printf("Cliente teve o cabelo cortado!\n");
}

void giveup_haircut() {
	printf("Cliente desistiu! (O salao esta muito cheio!)\n");
}

void barber_sleep(){
	printf("O barbeiro esta dormindo!\n");
	dia--; //barbeiro dorme por 10 min
	barbeiroDormiu++;
	printf("Horas restantes do dia: %f \n", dia/6);
}
