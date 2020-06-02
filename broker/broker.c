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

	sem_init(&mutex_sem, 0, 1);
	sem_init(&mutex_envio, 0, 0);

	cantidad_mensajes = 0;

	logger_broker =log_create("/home/utnso/tp-2020-1c-Elite-Four/broker/broker.log", "BROKER", false, LOG_LEVEL_INFO);
	logger_global = log_create("/home/utnso/tp-2020-1c-Elite-Four/broker/global.log", "GLOBAL", false, LOG_LEVEL_INFO);
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
	const int lista_colas[5] = {NEW_POKEMON, APPEARED_POKEMON, CATCH_POKEMON, CAUGHT_POKEMON, GET_POKEMON, LOCALIZED_POKEMON };

	config = config_create("/home/utnso/tp-2020-1c-Elite-Four/broker/broker.config");

	ip = config_get_string_value(config, ip_config);
	puerto = config_get_string_value(config, port_config);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    //for(int j= 0; j < 6; j++) {
    	int* puntero_cola = malloc(sizeof(puntero_cola));
    	*puntero_cola = lista_colas[0];
		pthread_create(&thread_pool[0], NULL, distribuir_mensajes, puntero_cola);
    //}

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        guard((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)), "Failed to create socket");

        guard(bind(socket_servidor, p->ai_addr, p->ai_addrlen), "Bind failed");

        break;
    }

	guard(listen(socket_servidor, SOMAXCONN), "Listen failed");

    freeaddrinfo(servinfo);

	//config_destroy(config);

    while(1)
    	esperar_cliente(socket_servidor);
}

void esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = guard(accept(socket_servidor, (void*) &dir_cliente, &tam_direccion), "Accept failed");

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
	mensaje->suscriptores_ack = list_create();
	mensaje->suscriptores_enviados = list_create();
		switch (cod_op) {
		case MESSAGE:
			msg = server_recibir_mensaje(socket, &size);
			msg = "1";
			size = sizeof(4);
			devolver_mensaje(msg, size, socket);
			log_info(logger_broker, "MESSAGE");
			log_info(logger_broker, msg);
			break;
		case NEW_POKEMON:
			sem_wait(&mutex_sem);
			log_info(logger_broker, "ENTRA");

			mensajeRecibido = recibir_new_pokemon(socket, &size, logger_broker);
			log_info(logger_broker, "SALE");

			mensaje->id = cantidad_mensajes;
			mensaje->mensaje = mensajeRecibido;
			mensaje->suscriptores_ack = list_create();
			mensaje->suscriptores_enviados = list_create();

			list_add(new_pokemon->mensajes, mensaje);

			char* id_mensaje = string_itoa(cantidad_mensajes);
			devolver_mensaje(id_mensaje, strlen(id_mensaje) + 1, socket);

			aumentar_cantidad_mensajes();

			log_info(logger_broker, "NEWPOKEMON");

			free(mensajeRecibido);
			sem_post(&mutex_sem);

			break;
		case SUSCRIBE:
			sem_wait(&mutex_sem);
			log_info(logger_broker, "ENTRA SUSCRIPCION");
			puntero_suscripcion_cola mensaje_suscripcion;
			mensaje_suscripcion = recibir_suscripcion(socket, &size, logger_broker);
			log_info(logger_broker, "SALE SUSCRIPCION");

			agregar_suscriptor_cola(mensaje_suscripcion);

			char* suscripcion_aceptada = "SUSCRIPCION COMPLETADA";
			devolver_mensaje(suscripcion_aceptada, strlen(suscripcion_aceptada) + 1, socket);
			sem_post(&mutex_sem);
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
	sem_post(&mutex_envio);
	free(mensaje->suscriptores_ack);
	free(mensaje->suscriptores_enviados);
	free(mensaje);
}

void aumentar_cantidad_mensajes(){
	cantidad_mensajes ++;
}

void agregar_suscriptor_cola(puntero_suscripcion_cola mensaje_suscripcion){
	switch(mensaje_suscripcion->cola) {
		case APPEARED_POKEMON:
			list_add(appeared_pokemon->suscriptores, mensaje_suscripcion->cliente);
			break;
		case NEW_POKEMON:
			list_add(new_pokemon->suscriptores, mensaje_suscripcion->cliente);
			break;
	}
}

void* distribuir_mensajes(void* puntero_cola) {
	while(1) {
		// ENVIA MENSAJES A SUSCRIPTORES
		int cola = *((int*) puntero_cola);
		int cant_mensajes_nuevos;
		sem_wait(&mutex_envio);
		sem_wait(&mutex_sem);
		cant_mensajes_nuevos = mensajes_nuevos(cola);

		if(cant_mensajes_nuevos > 0) {
			distribuir_mensaje(cola);
		}
		sem_post(&mutex_sem);
		sleep(10);
	}
}

int mensajes_nuevos(cola) {
	int retorno;
	puntero_mensaje puntero;
	t_cola_mensaje* cola_mensajes;
	switch(cola) {
		case NEW_POKEMON: {
			cola_mensajes = new_pokemon;
			break;
		}
		case APPEARED_POKEMON: {
			cola_mensajes = appeared_pokemon;
			break;
		}
		default: {
			//TODO SACAR ESTO Y PONER LSA OTRAS COLAS
			cola_mensajes = new_pokemon;
		}
	}
	for(int i = 0; i < list_size(cola_mensajes->mensajes); i++) {
		puntero = list_get(cola_mensajes->mensajes, i);
		//TODO asi no se calcula la cantidad de mensajes nuevos
		if(list_size(cola_mensajes->suscriptores) == list_size(puntero->suscriptores_enviados)) {
			retorno = 0;
		} else {
			retorno = 1;
			break;
		}
	}
	return retorno;
}

void* distribuir_mensaje(cola) {
	int conexion;
	conexion = crear_conexion("127.0.0.2", "55010");


	// TODO completar con las demas colas
	switch(cola) {
		case NEW_POKEMON: {
			for(int i = 0; i < list_size(new_pokemon->mensajes); i++){
				t_mensaje* mensaje = (t_mensaje*) list_get(new_pokemon->mensajes, i);
				puntero_mensaje_new_pokemon mensaje_envio = mensaje->mensaje;
				char* nombre = mensaje_envio->name_pokemon;
				uint32_t posx = mensaje_envio->pos_x;
				uint32_t posy = mensaje_envio->pos_y;
				uint32_t quant = mensaje_envio->quant_pokemon;
				send_message_new_pokemon(nombre, posx, posy, quant, conexion);
				list_add(mensaje->suscriptores_enviados, nombre);
			}

		}
	}
}
