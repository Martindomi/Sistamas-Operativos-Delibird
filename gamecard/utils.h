#ifndef UTILS_H_
#define UTILS_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include<conexiones.h>
#include <semaphore.h>

typedef struct {
	char* ipBroker;
	char* puertoBroker;
	char* ipGamecard;
	char* puertoGamecard;
	char* idGamecard;
} t_info;
t_info* informacion;

char* ACK;
char* ptoMontaje;
int block_size;
int blocks;
int tiempo_de_reintento_conexion;
int tiempo_de_reintento_operacion;
int tiempo_retardo_operacion;

t_log *logger;
t_config* config;

int socketSuscripcion;
int socketEscucha;

bool seCreoHiloReconexion;

sem_t mutex_boolReconexion; // 	sem_init(&mutex_boolReconexion,0,1);
sem_t mutex_reconexion;// 	sem_init(&(mutex_reconexion),0,1);
sem_t mutex_suscripcion; //	sem_init(&mutex_suscripcion,0,0);

void _reintentar_conexion(char* path);
void crear_hilo_escucha_suscripcion(int conexion);
bool suscribirse_a_colas(char* path);

#endif
