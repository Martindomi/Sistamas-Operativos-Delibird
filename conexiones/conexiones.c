/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include "conexiones.h"


/*void iniciar_servidor(char* path_config, char* ip_config, char* port_config)
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

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

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
			msg = strcat(msg," correcto");
			size = size + 10;
			devolver_mensaje(msg, size, cliente_fd);
			free(msg);
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}*/

void* server_recibir_mensaje(int socket_cliente, uint32_t* size)
{
	void * buffer;

	//printf("Entra a recibir mensaje\n");
	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
	//printf("tamaño: %d\n", *size);

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

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);

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
	int tamanio_ser = sizeof(op_code) + sizeof(uint32_t) + paquete->buffer->size;
	void * buffer = malloc(tamanio_ser);
	int desplazamiento = 0;

	memcpy(buffer + desplazamiento, &(paquete->header), sizeof(paquete->header));
	desplazamiento+= sizeof(paquete->header);

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

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		freeaddrinfo(server_info);
		return -1;
	}


	freeaddrinfo(server_info);

	return socket_cliente;
}


int crear_conexion_servidor(char* ip, char* puerto)
{
	pthread_t thread_team;

	int socket_servidor;
    struct addrinfo hints, *servinfo, *p;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            return -1;

        int flags = guard(fcntl(socket_servidor, F_GETFL), "could not get flags on TCP listening socket");
        guard(fcntl(socket_servidor, F_SETFL, flags | O_NONBLOCK), "could not set TCP listening socket to be non-blocking");

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;

}
//TODO
void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = NEW_POKEMON;
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
	int buffer_size;


	recv(socket_cliente, &operacion, sizeof(operacion), 0);

	recv(socket_cliente, &buffer_size, sizeof(buffer_size), 0);

	char * buffer = malloc(buffer_size);
	recv(socket_cliente, buffer, buffer_size, 0);

	//puts(buffer);
	return buffer;

}

char* client_recibir_mensaje_SIN_CODEOP(int socket_cliente)
{
	op_code operacion;
	int buffer_size;


	recv(socket_cliente, &buffer_size, sizeof(buffer_size), 0);

	char * buffer = malloc(buffer_size);
	recv(socket_cliente, buffer, buffer_size, 0);

	//puts(buffer);
	return buffer;

}


void liberar_conexion(int socket_cliente)
{
	if(socket_cliente != -1){
		//printf("Se cierra el socket\n");
		close(socket_cliente);
	}
}

// -------------------------------------------------NEW POKEMON--------------------------------------------------

void send_message_new_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t quant, uint32_t id, uint32_t id_correlativo, int socket_cliente)
{
	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = NEW_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 6 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

	guardar_mensaje_new(stream, nombre, posx, posy, quant, id, id_correlativo);

	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

}

void guardar_mensaje_new(void* stream, char* nombre, uint32_t posx, uint32_t posy, uint32_t quant, uint32_t id, uint32_t id_correlativo) {

	uint32_t name_size = strlen(nombre) + 1;

	int tamanio = 0;

	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &name_size, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, strlen(nombre) + 1);
	tamanio += strlen(nombre) + 1;

	memcpy(stream + tamanio, &(posx), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(posy), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(quant), sizeof(uint32_t));

}

void guardar_mensaje_new_memoria(void* stream, char* nombre, uint32_t posx, uint32_t posy, uint32_t quant) {

	uint32_t name_size = strlen(nombre);

	int tamanio = 0;

	memcpy(stream + tamanio, &name_size, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, name_size);
	tamanio += name_size;

	memcpy(stream + tamanio, &(posx), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(posy), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(quant), sizeof(uint32_t));

}

void* serializar_paquete(t_package* paquete, int *bytes) {
	int tamanio = paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t);
	void* mem = malloc(tamanio);
	int desplazamiento = 0;
	memcpy(mem + desplazamiento, &paquete->header, sizeof(op_code));
	desplazamiento += sizeof(op_code);
	memcpy(mem + desplazamiento, &paquete->buffer->size, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(mem + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;
	(*bytes) = tamanio;
	return mem;
}

puntero_mensaje recibir_new_pokemon( int socket, uint32_t* paquete_size){
	void * buffer = server_recibir_mensaje(socket, &paquete_size);

	puntero_mensaje mensaje = obtener_mensaje_new(buffer);

	free(buffer);
	return mensaje;
}

puntero_mensaje obtener_mensaje_new(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_new_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_new_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t quant_pokemon;

	int desplazamiento = 0;
	memcpy(&id, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&id_correlacional, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&pos_x, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&pos_y, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&quant_pokemon, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;
	mensaje_recibido->quant_pokemon = quant_pokemon;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t)*4 + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

puntero_mensaje obtener_mensaje_new_memoria(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_new_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_new_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t quant_pokemon;

	int desplazamiento = 0;

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&pos_x, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&pos_y, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&quant_pokemon, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;
	mensaje_recibido->quant_pokemon = quant_pokemon;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t)*4 + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

//----------------------------------APPEARED_POKEMON-------------------------------------------------

void send_message_appeared_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = APPEARED_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 5 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

	guardar_mensaje_appeared(stream, nombre, posx, posy, id, id_correlativo);

	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

}

void guardar_mensaje_appeared(void* stream, char* nombre, uint32_t posx, uint32_t posy, uint32_t id, uint32_t id_correlativo) {

	uint32_t name_size = strlen(nombre) + 1;

	int tamanio = 0;

	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, strlen(nombre) + 1);
	tamanio += strlen(nombre) + 1;

	memcpy(stream + tamanio, &(posx), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(posy), sizeof(uint32_t));

}

void guardar_mensaje_appeared_memoria(void* stream, char* nombre, uint32_t posx, uint32_t posy) {

	uint32_t name_size = strlen(nombre);

	int tamanio = 0;

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, name_size);
	tamanio += name_size;

	memcpy(stream + tamanio, &(posx), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(posy), sizeof(uint32_t));

}

puntero_mensaje recibir_appeared_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);

	puntero_mensaje mensaje = obtener_mensaje_appeared(buffer);

	free(buffer);
	return mensaje;
}

puntero_mensaje obtener_mensaje_appeared(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_appeared_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_appeared_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;

	int desplazamiento = 0;
	memcpy(&id, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&id_correlacional, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&pos_x, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&pos_y, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t)*3 + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

puntero_mensaje obtener_mensaje_appeared_memoria(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_appeared_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_appeared_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;

	int desplazamiento = 0;

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&pos_x, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&pos_y, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t)*3 + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

//----------------------------------CATCH_POKEMON-------------------------------------------------

void send_message_catch_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = CATCH_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 5 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

	guardar_mensaje_catch(stream, nombre, posx, posy, id, id_correlativo);

	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

}

void guardar_mensaje_catch(void* stream, char* nombre, uint32_t posx, uint32_t posy, uint32_t id, uint32_t id_correlativo) {

	uint32_t name_size = strlen(nombre) + 1;

	int tamanio = 0;

	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, strlen(nombre) + 1);
	tamanio += strlen(nombre) + 1;

	memcpy(stream + tamanio, &(posx), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(posy), sizeof(uint32_t));

}

void guardar_mensaje_catch_memoria(void* stream, char* nombre, uint32_t posx, uint32_t posy) {

	uint32_t name_size = strlen(nombre);

	int tamanio = 0;

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, name_size);
	tamanio += name_size;

	memcpy(stream + tamanio, &(posx), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(posy), sizeof(uint32_t));

}

puntero_mensaje recibir_catch_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);

	puntero_mensaje mensaje = obtener_mensaje_catch(buffer);

	free(buffer);
	return mensaje;
}

puntero_mensaje obtener_mensaje_catch(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_catch_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_catch_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;

	int desplazamiento = 0;
	memcpy(&id, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&id_correlacional, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&pos_x, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&pos_y, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t) * 3 + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

puntero_mensaje obtener_mensaje_catch_memoria(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_catch_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_catch_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t pos_x;
	uint32_t pos_y;

	int desplazamiento = 0;

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&pos_x, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&pos_y, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t) * 3 + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

//----------------------------------CAUGHT_POKEMON-------------------------------------------------

void send_message_caught_pokemon(char* caught_pokemon, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = CAUGHT_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 3 * sizeof(uint32_t);
	void* stream = malloc(buffer->size);

	guardar_mensaje_caught(stream, caught_pokemon, id, id_correlativo);

	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

}

void guardar_mensaje_caught(void* stream, char* caught_pokemon, uint32_t id, uint32_t id_correlativo) {
	uint32_t result;
	if(strcmp(caught_pokemon, "OK") == 0) {
		result = 0;
	} else if(strcmp(caught_pokemon, "FAIL") == 0) {
		result = 1;
	}

	int tamanio = 0;
	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(result), sizeof(uint32_t));
}

void guardar_mensaje_caught_memoria(void* stream, char* caught_pokemon) {
	uint32_t result;
	if(strcmp(caught_pokemon, "OK") == 0) {
		result = 0;
	} else if(strcmp(caught_pokemon, "FAIL") == 0) {
		result = 1;
	}

	memcpy(stream, &(result), sizeof(uint32_t));
}

puntero_mensaje recibir_caught_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);

	puntero_mensaje mensaje = obtener_mensaje_caught(buffer);

	free(buffer);
	return mensaje;
}

puntero_mensaje obtener_mensaje_caught(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_caught_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_caught_pokemon));

	uint32_t caughtResult;
	char* caught_pokemon;

	int desplazamiento = 0;
	memcpy(&id, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&id_correlacional, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&caughtResult, buffer + desplazamiento, sizeof(uint32_t));

	mensaje_recibido->caughtResult = caughtResult;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

puntero_mensaje obtener_mensaje_caught_memoria(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_caught_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_caught_pokemon));

	uint32_t caughtResult;
	char* caught_pokemon;

	memcpy(&caughtResult, buffer, sizeof(uint32_t));

	mensaje_recibido->caughtResult = caughtResult;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

//----------------------------------GET_POKEMON-------------------------------------------------

void send_message_get_pokemon(char* nombre, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = GET_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 3 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

	guardar_mensaje_get(stream, nombre, id, id_correlativo);

	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	puntero_mensaje puntero= obtener_mensaje_get(stream);
	puntero_mensaje_get_pokemon puntero_get = (puntero_mensaje_get_pokemon) (puntero->mensaje_cuerpo);

	//printf("envio get de pokemon: %s \n", puntero_get->name_pokemon);

	//printf("%d\n", socket_cliente);
	send(socket_cliente, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

}

void guardar_mensaje_get(void* stream, char* nombre, uint32_t id, uint32_t id_correlativo) {
	uint32_t name_size = strlen(nombre) + 1;

	int tamanio = 0;

	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, strlen(nombre) + 1);

}

void guardar_mensaje_get_memoria(void* stream, char* nombre) {
	uint32_t name_size = strlen(nombre);

	uint32_t tamanio = 0;

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, name_size);

}

puntero_mensaje recibir_get_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);

	puntero_mensaje mensaje = obtener_mensaje_get(buffer);

	free(buffer);
	return mensaje;
}

puntero_mensaje obtener_mensaje_get(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_get_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_get_pokemon));
	uint32_t name_size;
	char* name_pokemon;

	int desplazamiento = 0;
	memcpy(&id, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&id_correlacional, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t) + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

puntero_mensaje obtener_mensaje_get_memoria(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_get_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_get_pokemon));
	uint32_t name_size;
	char* name_pokemon;

	uint32_t desplazamiento = 0;

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t) + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;
	return mensaje;
}

//----------------------------------LOCALIZED_POKEMON-------------------------------------------------
void send_message_localized_pokemon(char* nombre,uint32_t quant_pokemon,t_list* coords, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	// "pokemon" 3 x y x1 y1 x2 y2

	paquete->header = LOCALIZED_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*(2 + 2 * quant_pokemon) + strlen(nombre) + 1 + 2 * sizeof(uint32_t) ;
	void* stream = malloc(buffer->size);

	guardar_mensaje_localized(stream, nombre, quant_pokemon, coords, id, id_correlativo);

	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

}

void guardar_mensaje_localized(void* stream, char* nombre,uint32_t quant_pokemon,t_list* coords, uint32_t id, uint32_t id_correlativo) {
	uint32_t name_size = strlen(nombre) + 1;

	int tamanio = 0;

	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, strlen(nombre) + 1);
	tamanio += name_size;

	memcpy(stream + tamanio, &(quant_pokemon), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	for(int i=0; i<quant_pokemon*2;i++){

		uint32_t* coord = list_get(coords,i);

		memcpy(stream + tamanio, &(coord), sizeof(uint32_t));
		tamanio += sizeof(uint32_t);
	}
}

void guardar_mensaje_localized_memoria(void* stream, char* nombre,uint32_t quant_pokemon,t_list* coords) {
	uint32_t name_size = strlen(nombre);

	int tamanio = 0;

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, name_size);
	tamanio += name_size;

	memcpy(stream + tamanio, &(quant_pokemon), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	for(int i=0; i<quant_pokemon*2;i++){

		uint32_t* coord = list_get(coords,i);

		memcpy(stream + tamanio, &(coord), sizeof(uint32_t));
		tamanio += sizeof(uint32_t);
	}
}

puntero_mensaje recibir_localized_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);

	puntero_mensaje mensaje = obtener_mensaje_localized(buffer);

	free(buffer);
	return mensaje;
}

puntero_mensaje obtener_mensaje_localized(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));

	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_localized_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_localized_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t quant_pokemon;
	t_list* coords = list_create();

	int desplazamiento = 0;
	memcpy(&id, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&id_correlacional, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&quant_pokemon, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	int i;
	for(i=0; i<quant_pokemon*2;i++){
		uint32_t* coord;
		memcpy(&coord, buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);

		list_add(coords,coord);

	}

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->quant_pokemon = quant_pokemon;
	mensaje_recibido->coords = coords;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t)* (2+i) + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;

	return mensaje;
}

puntero_mensaje obtener_mensaje_localized_memoria(void* buffer) {
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));

	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_localized_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_localized_pokemon));
	uint32_t name_size;
	char* name_pokemon;
	uint32_t quant_pokemon;
	t_list* coords = list_create();

	int desplazamiento = 0;

	memcpy(&name_size, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	name_pokemon = malloc(name_size);
	memcpy(name_pokemon, buffer + desplazamiento, name_size);
	desplazamiento += name_size;

	memcpy(&quant_pokemon, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	int i;
	for(i=0; i<quant_pokemon*2;i++){
		uint32_t* coord;
		memcpy(&coord, buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);

		list_add(coords,coord);

	}

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->quant_pokemon = quant_pokemon;
	mensaje_recibido->coords = coords;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->size_mensaje_cuerpo = sizeof(uint32_t)* (2+i) + strlen(name_pokemon);
	mensaje->mensaje_cuerpo = mensaje_recibido;

	return mensaje;
}

//----------------------------------SUSCRIPCION-------------------------------------------------
/*void enviar_mensaje_suscribir(op_code codigo_operacion, char* ip_puerto_cliente, int socket){
	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = SUSCRIBE;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int size_cliente = strlen(ip_puerto_cliente) + 1;
	buffer->size = sizeof(op_code) + sizeof(int) + size_cliente;
	void* stream = malloc(buffer->size);

	int tamanio = 0;
	memcpy(stream + tamanio, &codigo_operacion, sizeof(op_code));
	tamanio += sizeof(op_code);

	memcpy(stream + tamanio, &size_cliente, sizeof(int));
	tamanio += sizeof(int);

	memcpy(stream + tamanio, ip_puerto_cliente, size_cliente);

	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);
}*/

void enviar_mensaje_suscribir_con_id(op_code codigo_operacion, char* id, int socket, int tiempo){
	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = SUSCRIBE;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int size_cliente = strlen(id) + 1;
	buffer->size = sizeof(op_code) + sizeof(int)*2 + size_cliente;
	void* stream = malloc(buffer->size);

	int tamanio = 0;
	memcpy(stream + tamanio, &codigo_operacion, sizeof(op_code));
	tamanio += sizeof(op_code);

	memcpy(stream + tamanio, &size_cliente, sizeof(int));
	tamanio += sizeof(int);

	memcpy(stream + tamanio, id, size_cliente);
	tamanio += size_cliente;

	memcpy(stream + tamanio, &tiempo, sizeof(int));
	buffer->stream = stream;

	paquete->buffer = buffer;

	int bytes = paquete->buffer->size + sizeof(uint32_t) + sizeof(op_code);
	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket, a_enviar, bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);
}

puntero_suscripcion_cola recibir_suscripcion( int socket, uint32_t* paquete_size, t_log* logger_broker){
	void * buffer = server_recibir_mensaje(socket, &paquete_size);
	puntero_suscripcion_cola mensaje_recibido = malloc(sizeof(t_suscripcion_cola));
	int puerto_size;
	char* puerto;
	op_code codigo_cola;
	int tiempo;

	int desplazamiento = 0;
	memcpy(&codigo_cola, buffer + desplazamiento, sizeof(op_code));
	desplazamiento += sizeof(op_code);

	memcpy(&puerto_size, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	puerto = malloc(puerto_size);
	memcpy(puerto, buffer + desplazamiento, puerto_size);
	desplazamiento += puerto_size;

	memcpy(&tiempo, buffer + desplazamiento, sizeof(int));

	mensaje_recibido->cola = codigo_cola;
	mensaje_recibido->cliente = puerto;
	mensaje_recibido->tiempo = tiempo;
	mensaje_recibido->tamanoCliente = puerto_size;

	free(buffer);
	return mensaje_recibido;
}

int guard(int n, char * err) { if (n == -1) { perror(err); exit(1); } return n; }


//----------------------------------HILO ESCUCHA-------------------------------------------------

int crear_hilo_escucha(char* ip, char* puerto)
{
	pthread_t thread_team;

	int socket_servidor;
    struct addrinfo hints, *servinfo, *p;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

//        int flags = guard(fcntl(socket_servidor, F_GETFL), "could not get flags on TCP listening socket");
//       guard(fcntl(socket_servidor, F_SETFL, flags | O_NONBLOCK), "could not set TCP listening socket to be non-blocking");

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    pthread_create(&thread_team,NULL,(void*)hilo_escucha, socket_servidor);
    pthread_detach(thread_team);
    return socket_servidor;
}

void* hilo_escucha(int socket_servidor){
	int socketser = socket_servidor;
	//printf("%d\n", socketser);
	while(1){

		struct sockaddr_in dir_cliente;

		socklen_t tam_direccion = sizeof(struct sockaddr_in);
		int socket_cliente = accept(socketser, (void*) &dir_cliente, &tam_direccion);

		if (socket_cliente == -1) {

//				printf("No pending connections; sleeping for one second.\n");

				sleep(1);

		} else {
			int socket = socket_cliente;
//			printf("Got a connection; writing 'hello' then closing.\n");
			aplica_funcion_escucha(&socket);
		}
	}
}



//---------------------------------- INICIALIZAR CONFIG/LOG -------------------------------------------------

t_config * inicializar_config(char* pathConfig){
	t_config *config;
	if((config = config_create(pathConfig))==NULL){
		printf("no se pudo leer config\n");
		exit(2);
	}

	return config;
}

t_log* inicializar_log(char* pathConfig, char* nombreModulo){
	t_config* config = inicializar_config(pathConfig);
	char* log_path = config_get_string_value(config,"LOG_FILE");
	t_log *logger ;
	if((logger= log_create(log_path, nombreModulo, true, LOG_LEVEL_INFO))==NULL){
		printf("no se pudo leer log\n");
		exit(3);
	}

	return logger;
}


char* guard_lectura_string_config(char* string) {
	if(!string) {
		printf("Archivo de config mal configurado.\n");
		exit(4);
	} else {
		return string;
	}
}

char* obtener_string_config(t_config* config, char *key) {
	return guard_lectura_string_config(config_get_string_value(config, key));
}

char* validar_string_binario(char* valorObtenido, char* opcion1, char* opcion2) {
	if(strcmp(valorObtenido, opcion1) || strcmp(valorObtenido, opcion2)) {
		return valorObtenido;
	} else {
		printf("Valor ingresado incorrecto: %s\n", valorObtenido);
		exit(5);
	}
}

int guard_lectura_int_config(int integer) {
	if(!integer) {
		printf("Archivo de config mal configurado.\n");
		exit(4);
	} else {
		return integer;
	}
}

int obtener_int_config(t_config* config, char *key) {
	return guard_lectura_int_config(config_get_int_value(config, key));
}
