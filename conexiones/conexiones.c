/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include "conexiones.h"


void iniciar_servidor(char* path_config, char* ip_config, char* port_config)
{
	int socket_servidor;
	char* ip;
	char* puerto;
    struct addrinfo hints, *servinfo, *p;
	t_config* config;


	config = config_create(path_config);

	ip = config_get_string_value(config, ip_config);
	puerto = config_get_string_value(config, port_config);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

	config_destroy(config);

    while(1)
    	esperar_cliente(socket_servidor);
}

void esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_detach(thread);

}

void serve_client(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int cliente_fd) {
	int size;
	void* msg;
		switch (cod_op) {
		case MESSAGE:
			msg = server_recibir_mensaje(cliente_fd, &size);
			devolver_mensaje(msg, size, cliente_fd);
			free(msg);
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}

void* server_recibir_mensaje(int socket_cliente, int* size)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

/*
void* serializar_paquete(t_package* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->header), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}
*/

void devolver_mensaje(void* payload, int size, int socket_cliente)
{
	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = MESSAGE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = size;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, payload, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_mensaje(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


//
// **** CLIENTE ****
//

void* serializar_mensaje(t_package* paquete, int *bytes)
{
	int tamanio_ser = sizeof(op_code) + sizeof(uint32_t)*3 + paquete->buffer->size;
	void * buffer = malloc(tamanio_ser);
	int desplazamiento = 0;

	memcpy(buffer + desplazamiento, &(paquete->header), sizeof(paquete->header));
	desplazamiento+= sizeof(paquete->header);

	memcpy(buffer + desplazamiento, &(paquete->ID), sizeof(paquete->ID));
	desplazamiento+= sizeof(paquete->ID);

	memcpy(buffer + desplazamiento, &(paquete->correlativeID), sizeof(paquete->correlativeID));
	desplazamiento+= sizeof(paquete->correlativeID);

	memcpy(buffer + desplazamiento, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	desplazamiento+= sizeof(paquete->buffer->size);

	memcpy(buffer + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= sizeof(paquete->buffer->stream);

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
	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = MESSAGE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->stream = mensaje;
	paquete->buffer->size = strlen(mensaje) + 1;

	int bytes;

	void* a_enviar = serializar_mensaje(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer);
	free(paquete);
}

//TODO
char* client_recibir_mensaje(int socket_cliente)
{
	op_code operacion;
	int buffer_size, ID, correlativeID;

	//recibe codop, id , id correlativo, pero SOLO DEVUELVE BUFFER CON MENSAJE
	recv(socket_cliente, &operacion, sizeof(operacion), 0);
	recv(socket_cliente, &ID, sizeof(ID), 0);
	recv(socket_cliente, &correlativeID, sizeof(correlativeID), 0);

	recv(socket_cliente, &buffer_size, sizeof(buffer_size), 0);

	char * buffer = malloc(buffer_size);
	recv(socket_cliente, buffer, buffer_size, 0);

	return buffer;

}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

