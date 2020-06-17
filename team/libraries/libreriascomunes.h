/*
 * libreriascomunes.h
 *
 *  Created on: 8 jun. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_LIBRERIASCOMUNES_H_
#define LIBRARIES_LIBRERIASCOMUNES_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <conexiones.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <semaphore.h>


t_list *cola_NEW;
t_list *cola_READY;
t_list *cola_EXEC;
t_list *cola_BLOQUED;
t_list *cola_EXIT;

typedef struct {
	int x,y;
	char* especie;
} t_pokemon;

typedef struct {

	char** posicionesEntrenadores;
	char** pokemonesEntrenadores;
	char** objetivosEntrenadores;
	int tiempoReconexion;
	int retardoCicloCPU;
	char* algoritmoPlanificacion;
	int quantum;
	float alpha;
	float estmacionInicial;
	char* ipBroker;
	char* puertoBroker;
	char* logFile;
	char* ipTeam;
	char* puertoTeam;
	//char* id;

} data_config;

typedef data_config* puntero_data_config;

t_log* loggerTEAM;
t_config* configTEAM;
puntero_data_config configData;

typedef enum
{

	NEW=1,
	READY= 2,
	EXEC= 3,
	BLOCK= 4,
	EXIT= 5

}t_estado;

typedef enum
{

	APPEAR=1,
	BROKEROFF = 2

}t_mensajeTeam;

typedef struct {
	int id;
	int x;
	int y;
	t_list* pokemonesCapturados;
	t_list* pokemonesObjetivo;
	int espacioLibre;
	t_estado estado;
	pthread_t th;
	t_pokemon* pokemonCapturando;
	sem_t sem_entrenador;
	int id_catch;

}t_entrenador;


typedef struct{
	char* pokemon;
	int cantidad;

}t_pokemonObjetivo;



typedef struct {
	t_pokemon* pokemon;
	t_entrenador* entrenador;
	double distancia;
} t_distancia;


typedef enum {
	FAIL=0,
	OK=1
} resultado_catch;

typedef struct {
	uint32_t idCorrelativo;
	resultado_catch atrapado;
} t_caught;


#endif /* LIBRARIES_LIBRERIASCOMUNES_H_ */
