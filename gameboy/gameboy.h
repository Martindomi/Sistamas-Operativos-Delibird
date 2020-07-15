
#ifndef TP0_H_
#define TP0_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<conexiones.h>
#include<semaphore.h>

char* ip_broker;
char* puerto_broker;
char* ip_team;
char* puerto_team;
char* ip_gamecard;
char* puerto_gamecard;
char* ip_gameboy;
char* puerto_gameboy;
char* id_proceso;
char* ACK;

t_log* logger_gameboy;
uint32_t obtener_cola_mensaje(char* cola_string);

//--------------------------------------------------------SUSCRIPCION Y RECONEXION GENERICA
bool seCreoHiloReconexion;
sem_t mutex_boolReconexion; // 	sem_init(&mutex_boolReconexion,0,1);
sem_t mutex_reconexion;// 	sem_init(&(mutex_reconexion),0,1);
sem_t mutex_suscripcion; //	sem_init(&mutex_suscripcion,0,0);
// op_code vectorDeColas[]={ APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON }; --> lo puse en main.c

// cada uno debe vrear el suyo de todo lo que esta arriba

bool suscribirse_a_colas(char* path);
bool suscribirse_a_cola_gameboy(op_code cola_elegida ,int tiempo);
//--------------------------------------------------------

#endif /* TP0_H_ */

