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

	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
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

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
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

	return buffer;

}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

// -------------------------------------------------NEW POKEMON--------------------------------------------------

void send_message_new_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t quant, uint32_t id, uint32_t id_correlativo, int socket_cliente)
{
	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = NEW_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 6 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

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

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;
	mensaje_recibido->quant_pokemon = quant_pokemon;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->mensaje_cuerpo = mensaje_recibido;

	free(buffer);
	return mensaje;
}

//----------------------------------APPEARED_POKEMON-------------------------------------------------

void send_message_appeared_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = APPEARED_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 5 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

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

puntero_mensaje recibir_appeared_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);
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

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->mensaje_cuerpo = mensaje_recibido;

	free(buffer);
	return mensaje;
}

//----------------------------------CATCH_POKEMON-------------------------------------------------

void send_message_catch_pokemon(char* nombre, uint32_t posx, uint32_t posy, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = CATCH_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 5 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

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

puntero_mensaje recibir_catch_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);
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

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;
	mensaje_recibido->pos_x = pos_x;
	mensaje_recibido->pos_y = pos_y;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->mensaje_cuerpo = mensaje_recibido;

	free(buffer);
	return mensaje;
}

//----------------------------------CAUGHT_POKEMON-------------------------------------------------

void send_message_caught_pokemon(char* caught_pokemon, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = CAUGHT_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	uint32_t caught_size = strlen(caught_pokemon) + 1;
	buffer->size = caught_size + 3 * sizeof(uint32_t);
	void* stream = malloc(buffer->size);

	int tamanio = 0;
	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream, &(caught_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream, caught_pokemon, caught_size);

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

puntero_mensaje recibir_caught_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);
	puntero_mensaje mensaje = malloc(sizeof(t_mensaje));
	uint32_t id;
	uint32_t id_correlacional;
	puntero_mensaje_caught_pokemon mensaje_recibido = malloc(sizeof(t_mensaje_caught_pokemon));

	uint32_t caught_size;
	char* caught_pokemon;

	int desplazamiento = 0;
	memcpy(&id, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&id_correlacional, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&caught_size, buffer, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	caught_pokemon = malloc(caught_size);
	memcpy(caught_pokemon, buffer + desplazamiento, caught_size);

	mensaje_recibido->caught_size = caught_size;
	mensaje_recibido->caught_pokemon = caught_pokemon;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->mensaje_cuerpo = mensaje_recibido;

	free(buffer);
	return mensaje;
}

//----------------------------------GET_POKEMON-------------------------------------------------

void send_message_get_pokemon(char* nombre, uint32_t id, uint32_t id_correlativo, int socket_cliente){

	t_package* paquete = malloc(sizeof(t_package));

	paquete->header = GET_POKEMON;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 3 + strlen(nombre) + 1;
	void* stream = malloc(buffer->size);

	uint32_t name_size = strlen(nombre) + 1;

	int tamanio = 0;

	memcpy(stream + tamanio, &id, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &id_correlativo, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, &(name_size), sizeof(uint32_t));
	tamanio += sizeof(uint32_t);

	memcpy(stream + tamanio, nombre, strlen(nombre) + 1);

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

puntero_mensaje recibir_get_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);
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

	mensaje_recibido->name_size = name_size;
	mensaje_recibido->name_pokemon = name_pokemon;

	mensaje->id = id;
	mensaje->id_correlativo = id_correlacional;
	mensaje->mensaje_cuerpo = mensaje_recibido;

	free(buffer);
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

	for(int i=0; i<quant_pokemon;i++){

		uint32_t* coord = list_get(coords,i);

		memcpy(stream + tamanio, &(coord), sizeof(uint32_t));
		tamanio += sizeof(uint32_t);
	}

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

puntero_mensaje recibir_localized_pokemon( int socket, uint32_t* paquete_size){

	void * buffer = server_recibir_mensaje(socket, &paquete_size);
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

	for(int i=0; i<quant_pokemon;i++){

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
	mensaje->mensaje_cuerpo = mensaje_recibido;

	free(buffer);
	return mensaje;
}



//----------------------------------SUSCRIPCION-------------------------------------------------
void enviar_mensaje_suscribir(op_code codigo_operacion, char* ip_puerto_cliente, int socket){
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
}

puntero_suscripcion_cola recibir_suscripcion( int socket, uint32_t* paquete_size, t_log* logger_broker){
	void * buffer = server_recibir_mensaje(socket, &paquete_size);
	log_info(logger_broker, "buffer suscripcion pasa");
	puntero_suscripcion_cola mensaje_recibido = malloc(sizeof(puntero_suscripcion_cola));
	int puerto_size;
	char* puerto;
	op_code codigo_cola;

	log_info(logger_broker, "PASA MALLOC");
	log_info(logger_broker, &buffer);
	log_info(logger_broker, "muestra buffer");

	int desplazamiento = 0;
	memcpy(&codigo_cola, buffer + desplazamiento, sizeof(op_code));
	desplazamiento += sizeof(op_code);

	memcpy(&puerto_size, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	puerto = malloc(puerto_size);
	memcpy(puerto, buffer + desplazamiento, puerto_size);

	mensaje_recibido->cola = codigo_cola;
	mensaje_recibido->cliente = puerto;

	free(buffer);
	return mensaje_recibido;
}

int guard(int n, char * err) { if (n == -1) { perror(err); exit(1); } return n; }

