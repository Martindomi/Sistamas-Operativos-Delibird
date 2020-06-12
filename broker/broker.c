#include <conexiones.h>
#include "broker.h"

int main(int argc, char *argv[]){

	char * configPath = "../broker.config";
	char * ipconfig= "IP_BROKER";
	char * puertocofing= "PUERTO_BROKER";
	t_list* suscriptores_new = malloc(sizeof(t_list));
	t_list* mensajes_new = malloc(sizeof(t_list));
	t_list* suscriptores_appeared = malloc(sizeof(t_list));
	t_list* mensajes_appeared = malloc(sizeof(t_list));
	new_pokemon = malloc(sizeof(t_cola_mensaje));
	(*new_pokemon).suscriptores = suscriptores_new;
	(*new_pokemon).mensajes = mensajes_new;

	appeared_pokemon = malloc(sizeof(t_cola_mensaje));
	(*appeared_pokemon).suscriptores = suscriptores_appeared;
	(*appeared_pokemon).mensajes = mensajes_appeared;

	cantidad_mensajes = 0;

	logger_broker =log_create("../broker.log", "BROKER", false, LOG_LEVEL_INFO);
	logger_global = log_create("../global.log", "GLOBAL", false, LOG_LEVEL_INFO);
	log_info(logger_broker, "ESTOY LOGEANDO");



	iniciar_servidor(configPath, ipconfig, puertocofing);

	return EXIT_SUCCESS;

}

void iniciar_servidor(char* path_config, char* ip_config, char* port_config)
{
	int socket_servidor;
	char* ip;
	char* puerto;
    struct addrinfo hints, *servinfo, *p;
	t_config* config;


	config = config_create("/home/utnso/Documentos/tp-2020-1c-Elite-Four/broker/broker.config");

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

	//config_destroy(config);

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
	op_code cod_op;
	if(recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int socket) {
	uint32_t size;
	void* msg;
	puntero_mensaje_new_pokemon mensajeRecibido;
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
		switch (cod_op) {
		case MESSAGE:
			msg = server_recibir_mensaje(socket, &size);
			msg = "1";
			size = sizeof(4);
			devolver_mensaje(msg, size, socket);
			log_info(logger_broker, "MESSAGE");
			log_info(logger_broker, msg);
			free(msg);
			break;
		case NEW_POKEMON:
			log_info(logger_broker, "ENTRA");

			mensajeRecibido = recibir_new_pokemon(socket, &size, logger_broker);
			log_info(logger_broker, "SALE");

			mensaje->id = cantidad_mensajes;
			mensaje->mensaje = mensajeRecibido;

			list_add(new_pokemon->mensajes, mensaje);

			char* id_mensaje = string_itoa(cantidad_mensajes);
			devolver_mensaje(id_mensaje, strlen(id_mensaje) + 1, socket);

			aumentar_cantidad_mensajes();

			log_info(logger_broker, "NEWPOKEMON");

			free(mensajeRecibido);
			free(mensaje);
			break;
		case SUSCRIBE:
			log_info(logger_broker, "ENTRA SUSCRIPCION");
			puntero_suscripcion_cola mensaje_suscripcion;
			mensaje_suscripcion = recibir_suscripcion(socket, &size, logger_broker);
			log_info(logger_broker, "SALE SUSCRIPCION");

			agregar_suscriptor_cola(mensaje_suscripcion);

			char* suscripcion_aceptada = "SUSCRIPCION COMPLETADA";
			devolver_mensaje(suscripcion_aceptada, strlen(suscripcion_aceptada) + 1, socket);

			log_info(logger_broker, "SUSCRIPCION");

			free(mensajeRecibido);
			free(mensaje);
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}

void aumentar_cantidad_mensajes(){
	cantidad_mensajes ++;
}

void agregar_suscriptor_cola(puntero_suscripcion_cola mensaje_suscripcion){
	switch(mensaje_suscripcion->cola) {
		case APPEARED_POKEMON:
			list_add(appeared_pokemon->suscriptores, mensaje_suscripcion->cliente);

			break;
	}
}

/*void* server_recibir_mensaje(int socket_cliente, uint32_t* size)
{
	void * buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}*/
