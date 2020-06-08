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



bool suscribirse_a_colas();
int suscribir2(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger);
void hilo_reconexion();
void reintentar_conexion(int tiempo);
void funcionGenerica(int *socket);



void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char *ip_team, char* puerto_team, t_log* logger);
void suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger);
void esperar_mensajes_broker(int socket_servidor);
void recibe_mensaje_broker(int* socket);
void enviar_menssaje_new_pokemon(t_log* logger, char* ip, char* puerto);


void comprobarConexion();


#endif /* LIBRARIES_UTILS_H_ */
