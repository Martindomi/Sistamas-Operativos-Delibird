/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"

//TODO
/*
 * Recibe un paquete a serializar, y un puntero a un int en el que dejar
 * el tamaño del stream de bytes serializados que devuelve
 */
void* serializar_paquete(t_paquete* paquete, int *bytes)
{
	int tamanio_ser = sizeof(op_code_server) + sizeof(int) + paquete->buffer->size;
	void * buffer = malloc(tamanio_ser);
	int desplazamiento = 0;

	memcpy(buffer + desplazamiento, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	desplazamiento+= sizeof(paquete->codigo_operacion);

	memcpy(buffer + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);

	memcpy(buffer + desplazamiento, paquete->buffer->stream, paquete->buffer->size);

	(*bytes) = tamanio_ser;

	return buffer;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

//TODO
void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->stream = mensaje;
	paquete->buffer->size = strlen(mensaje) + 1;

	int bytes;

	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer);
	free(paquete);
}

//TODO
char* recibir_mensaje(int socket_cliente)
{
	op_code_server operacion;
	recv(socket_cliente, &operacion, sizeof(operacion), 0);
	int buffer_size;
	recv(socket_cliente, &buffer_size, sizeof(buffer_size), 0);
	char * buffer = malloc(buffer_size);
	recv(socket_cliente, buffer, buffer_size, 0);

	return buffer;

}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
