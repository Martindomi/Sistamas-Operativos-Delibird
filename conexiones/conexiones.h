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
//#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include <commons/config.h>
#include<string.h>
#include<pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

typedef enum
{
	MESSAGE=1,
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
	uint32_t size;
	void* stream;

} t_buffer;

typedef struct
{
	op_code header;
	t_buffer* buffer;

} t_package;

typedef struct {
	uint32_t id;
	uint32_t id_correlativo;
	uint32_t size_mensaje_cuerpo;
	void* mensaje_cuerpo;
} t_mensaje;
typedef t_mensaje* puntero_mensaje;

typedef struct {
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t quant_pokemon;
} t_mensaje_new_pokemon;

typedef t_mensaje_new_pokemon* puntero_mensaje_new_pokemon;

typedef struct {
	uint32_t name_size;
	char* name_pokemon;
	uint32_t quant_pokemon;
	t_list* coords; // lista ordenada de uint32_t
} t_mensaje_localized_pokemon;

typedef t_mensaje_localized_pokemon* puntero_mensaje_localized_pokemon;

typedef struct {
	uint32_t name_size;
	char* name_pokemon;
} t_mensaje_get_pokemon;

typedef t_mensaje_get_pokemon* puntero_mensaje_get_pokemon;

typedef struct {
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;
} t_mensaje_appeared_pokemon;

typedef t_mensaje_appeared_pokemon* puntero_mensaje_appeared_pokemon;


typedef struct {
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;
} t_mensaje_catch_pokemon;

typedef t_mensaje_catch_pokemon* puntero_mensaje_catch_pokemon;


typedef struct {
	uint32_t caughtResult;
} t_mensaje_caught_pokemon;

typedef t_mensaje_caught_pokemon* puntero_mensaje_caught_pokemon;

typedef struct {
	op_code cola;
	char* cliente;
} t_suscripcion_cola;
typedef t_suscripcion_cola* puntero_suscripcion_cola;

typedef struct {
	op_code cola;
	char* id;
	int tiempo;
} t_suscripcion_cola_ID;
typedef t_suscripcion_cola_ID* puntero_suscripcion_cola_ID;


t_log* logger_global;

// Server

void* recibir_buffer(int*, int);
void iniciar_servidor();
void esperar_cliente(int);
void* server_recibir_mensaje(int socket_cliente, uint32_t* size);
int recibir_operacion(int);
void process_request(int cod_op, int cliente_fd);
void serve_client(int *socket);
void* serializar_mensaje(t_package* paquete, int *bytes);
void devolver_mensaje(void* payload, int size, int socket_cliente);

// Client

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
char* client_recibir_mensaje(int socket_cliente);
void eliminar_paquete(t_package* paquete);
void liberar_conexion(int socket_cliente);
void* serializar_paquete(t_package* paquete, int* bytes);

// HILO ESCUCHA
void crear_hilo_escucha(char* ip, char* puerto);
void* hilo_escucha(int socket);

// obtener mensajes
puntero_mensaje obtener_mensaje_new(void* buffer);
puntero_mensaje obtener_mensaje_get(void* buffer);
puntero_mensaje obtener_mensaje_localized(void* buffer);
puntero_mensaje obtener_mensaje_caught(void* buffer);
puntero_mensaje obtener_mensaje_catch(void* buffer);
puntero_mensaje obtener_mensaje_appeared(void* buffer);

// INICIALIZACION CONFIG/LOGGER


//  hay que pasarle el path donde este el config. EJ: ./gameboy.config
t_config * inicializar_config(char*);

// hay que pasarle 1° el path donde esta el config y 2° el nombre del modulo para que aparezca en el log ("GAMECARD"/"BROKER"/"TEAM")
t_log* inicializar_log(char*, char*);

#endif /* CONEXIONES_H_ */
