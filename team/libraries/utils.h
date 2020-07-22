/*
 * utils.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_UTILS_H_
#define LIBRARIES_UTILS_H_

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
#include "entrenadores.h"
#include "../team.h"

//--------------------------------------------------------SUSCRIPCION Y RECONEXION GENERICA
bool seCreoHiloReconexion;
sem_t mutex_boolReconexion; // 	sem_init(&mutex_boolReconexion,0,1);
sem_t mutex_reconexion;// 	sem_init(&(mutex_reconexion),0,1);
sem_t mutex_suscripcion; //	sem_init(&mutex_suscripcion,0,0);
// op_code vectorDeColas[]={ APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON }; --> lo puse en main.c

// cada uno debe vrear el suyo de todo lo que esta arriba

bool suscribirse_a_colas(char* path);
void enviar_mensaje_suscribir_con_id(op_code codigo_operacion, char* id, int socket,int tiempo);
void crear_hilo_escucha_suscripcion(int conexion);
void crear_hilo_reconexion(char* path);
void _reintentar_conexion(char* path);

//--------------------------------------------------------
//bool suscribirse_a_colas();
void hilo_reconexion();
//void reintentar_conexion(char* path);
void funcionGenerica(int *socket);
void tarda(int ciclos);
void contar_deadlock_producido();
void contar_deadlock_resuelto();
void contar_context_switch();


//void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char *ip_team, char* puerto_team, t_log* logger);
void enviar_get_objetivos();
void enviar_mensaje_get_pokemon(char* especiePokemon);
void enviar_mensaje_catch_pokemon(t_entrenador * entrenador,char* especiePokemon, int posX, int posY);
void caught_destroyer(t_caught*);
void esperar_mensajes_broker(int socket_servidor);
void recibe_mensaje_broker(int* socket);
void enviar_menssaje_new_pokemon(t_log* logger, char* ip, char* puerto);
//int suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* ip_puerto_team, t_log* logger);
int suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* ip_puerto_team);
void liberar_config_data();
void inicializar_config_data();
void inicializar_log_team();
void inicializar_config_team(char* pathConfig);
bool esRR();
bool esSJFconDesalojo();
bool esSJFsinDesalojo();
bool esSJF();
void comprobarConexion();
void mensaje_destroyer(char* mensaje);

#endif /* LIBRARIES_UTILS_H_ */
