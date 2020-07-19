#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include<conexiones.h>
#include <semaphore.h>

int socketSuscripcion;
int socketEscucha;

bool seCreoHiloReconexion;

sem_t mutex_boolReconexion; // 	sem_init(&mutex_boolReconexion,0,1);
sem_t mutex_reconexion;// 	sem_init(&(mutex_reconexion),0,1);
sem_t mutex_suscripcion; //	sem_init(&mutex_suscripcion,0,0);

void _reintentar_conexion(char* path);
