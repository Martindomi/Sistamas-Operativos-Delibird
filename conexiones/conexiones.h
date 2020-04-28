/*
 * conexiones.h
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include <commons/config.h>
#include<string.h>
#include<pthread.h>



typedef enum
{
	MENSAJE=1,
	NEW_POKEMON = 2,
	APPEARED_POKEMON = 3,
	CATCH_POKEMON = 4,
	CAUGHT_POKEMON = 5,
	GET_POKEMON = 6,
	LOCALIZED_POKEMON = 7,
	SUSCRIBE = 8
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code header;
	t_buffer* buffer;
} t_package;


pthread_t thread;

// Server

void* recibir_buffer(int*, int);
void iniciar_servidor(char* path_config, char* ip_config, char* port_config);
void esperar_cliente(int);
void* server_recibir_mensaje(int socket_cliente, int* size);
int recibir_operacion(int);
void process_request(int cod_op, int cliente_fd);
void serve_client(int *socket);
void* serializar_paquete(t_package* paquete, int *bytes);
void devolver_mensaje(void* payload, int size, int socket_cliente);

// Client

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
char* client_recibir_mensaje(int socket_cliente);
void eliminar_paquete(t_package* paquete);
void liberar_conexion(int socket_cliente);


#endif /* CONEXIONES_H_ */
