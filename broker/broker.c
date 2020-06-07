#include <conexiones.h>
#include "broker.h"

int main(int argc, char *argv[]){

	char * configPath = "../broker.config";
	char * ipconfig= "IP_BROKER";
	char * puertocofing= "PUERTO_BROKER";

	inicializar_colas();

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

    // TODO crear hilos para todas las colas
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
	t_mensaje_completo* mensaje_completo = malloc(sizeof(t_mensaje_completo));
	mensaje_completo->suscriptores_ack = list_create();
	mensaje_completo->suscriptores_enviados = list_create();
	sem_wait(&mutex_sem);
	switch (cod_op) {
		case MESSAGE: {
			void* msg;
			msg = server_recibir_mensaje(socket, &size);
			msg = "1";
			size = sizeof(4);
			devolver_mensaje(msg, size, socket);
			log_info(logger_broker, "MESSAGE");
			log_info(logger_broker, msg);
			free(msg);
			break;
		}
		case NEW_POKEMON: {
			mensaje_completo->mensaje = recibir_new_pokemon(socket, &size);

			mensaje_completo->mensaje->id = cantidad_mensajes;

			list_add(new_pokemon->mensajes, mensaje_completo);

			break;
		}
		case APPEARED_POKEMON: {
			mensaje_completo->mensaje = recibir_appeared_pokemon(socket, &size);

			mensaje_completo->mensaje->id = cantidad_mensajes;

			list_add(appeared_pokemon->mensajes, mensaje_completo);

			break;
		}
		case CATCH_POKEMON: {
			mensaje_completo->mensaje = recibir_catch_pokemon(socket, &size);

			mensaje_completo->mensaje->id = cantidad_mensajes;

			list_add(catch_pokemon->mensajes, mensaje_completo);

			break;
		}
		case CAUGHT_POKEMON: {
			mensaje_completo->mensaje = recibir_caught_pokemon(socket, &size);

			mensaje_completo->mensaje->id = cantidad_mensajes;

			list_add(caught_pokemon->mensajes, mensaje_completo);

			break;
		}
		case GET_POKEMON: {
			mensaje_completo->mensaje = recibir_get_pokemon(socket, &size);

			mensaje_completo->mensaje->id = cantidad_mensajes;

			list_add(get_pokemon->mensajes, mensaje_completo);

			break;
		}
		case LOCALIZED_POKEMON: {
			mensaje_completo->mensaje = recibir_localized_pokemon(socket, &size);

			mensaje_completo->mensaje->id = cantidad_mensajes;

			list_add(localized_pokemon->mensajes, mensaje_completo);

			break;
		}
		case SUSCRIBE: {
			puntero_suscripcion_cola mensaje_suscripcion;

			mensaje_suscripcion = recibir_suscripcion(socket, &size, logger_broker);

			agregar_suscriptor_cola(mensaje_suscripcion);

			char* suscripcion_aceptada = "SUSCRIPCION COMPLETADA";
			devolver_mensaje(suscripcion_aceptada, strlen(suscripcion_aceptada) + 1, socket);

			free(mensaje_suscripcion);
			break;
		}
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}

	if(cod_op != SUSCRIBE) {
		char* id_mensaje = string_itoa(cantidad_mensajes);
		devolver_mensaje(id_mensaje, strlen(id_mensaje) + 1, socket);

		aumentar_cantidad_mensajes();

	}

	sem_post(&mutex_sem);
	sem_post(&mutex_envio);
	/*free(mensaje->suscriptores_ack);
	free(mensaje->suscriptores_enviados);
	free(mensaje);*/
}

void aumentar_cantidad_mensajes(){
	cantidad_mensajes ++;
}

void agregar_suscriptor_cola(puntero_suscripcion_cola mensaje_suscripcion){
	t_cola_mensaje* cola_mensaje = selecciono_cola(mensaje_suscripcion->cola);
	list_add(cola_mensaje->suscriptores, mensaje_suscripcion->cliente);
}

void* distribuir_mensajes(void* puntero_cola) {
	while(1) {
		// ENVIA MENSAJES A SUSCRIPTORES
		int cola = *((int*) puntero_cola);
		sem_wait(&mutex_envio);
		sem_wait(&mutex_sem);

		distribuir_mensajes_cola(cola);

		sem_post(&mutex_sem);
		sleep(10);
	}
}

void distribuir_mensajes_cola(int cola) {
	puntero_mensaje_completo puntero_mensaje_completo;
	t_cola_mensaje* cola_mensajes = selecciono_cola(cola);
	char* suscriptor;
	// TODO Mejorar manejo de error
	if (cola_mensajes == -1) {
		pthread_exit(NULL);
	}

	for(int i = 0; i < list_size(cola_mensajes->mensajes); i++) {
		puntero_mensaje_completo = list_get(cola_mensajes->mensajes, i);
		//TODO asi no se calcula la cantidad de mensajes nuevos
		for(int j = 0; j < list_size(cola_mensajes->suscriptores) ;j++) {
			suscriptor = list_get(cola_mensajes->suscriptores, j);

			bool encuentra_suscriptor(void* elemento) {
				return (char*)elemento == suscriptor;
			}
			// TODO ver si no es mejor utilizar los ACK en vez de los enviados.
			bool encontre = list_any_satisfy(puntero_mensaje_completo->suscriptores_enviados, (void*)encuentra_suscriptor);
			if (!encontre) {
				distribuir_mensaje_sin_enviar_a(suscriptor, cola, puntero_mensaje_completo);
				list_add(puntero_mensaje_completo->suscriptores_enviados, suscriptor);
				break;
			}
		}

	}
}

void distribuir_mensaje_sin_enviar_a(char* suscriptor, int cola, puntero_mensaje_completo puntero_mensaje_completo) {
	int conexion;
	char* ip_suscriptor;
	char* puerto_suscriptor;
	char* mensaje_recibido;

	ip_suscriptor = "127.0.0.2";
	puerto_suscriptor = "55010";

	// TODO buscar una forma de separar el string suscriptor (ip:puerto)
	/*strcpy(ip_suscriptor, suscriptor);
	strtok_r(ip_suscriptor, ":", &puerto_suscriptor);*/

	conexion = crear_conexion(ip_suscriptor, puerto_suscriptor);

	switch(cola) {
		case NEW_POKEMON: {
			puntero_mensaje_new_pokemon puntero_mensaje = ((puntero_mensaje_new_pokemon*)puntero_mensaje_completo->mensaje->mensaje_cuerpo);
			uint32_t id = puntero_mensaje_completo->mensaje->id;
			uint32_t id_correlativo = puntero_mensaje_completo->mensaje->id_correlativo;
			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;
			uint32_t quant = puntero_mensaje->quant_pokemon;
			send_message_new_pokemon(nombre, posx, posy, quant, id, id_correlativo, conexion);

			mensaje_recibido = client_recibir_mensaje(conexion);

			// TODO que hago si no recibo el ACK?
			if(mensaje_recibido == "ACK") {
				list_add(puntero_mensaje_completo->suscriptores_ack, suscriptor);
			}

			free(mensaje_recibido);
			break;
		}
		// TODO hacer el envio de los demas mensajes
	}
	close(conexion);
}

void inicializar_colas() {

	new_pokemon = malloc(sizeof(t_cola_mensaje));
	(*new_pokemon).suscriptores = list_create();
	(*new_pokemon).mensajes = list_create();

	appeared_pokemon = malloc(sizeof(t_cola_mensaje));
	(*appeared_pokemon).suscriptores = list_create();
	(*appeared_pokemon).mensajes = list_create();

	get_pokemon = malloc(sizeof(t_cola_mensaje));
	(*get_pokemon).suscriptores = list_create();
	(*get_pokemon).mensajes = list_create();

	localized_pokemon = malloc(sizeof(t_cola_mensaje));
	(*localized_pokemon).suscriptores = list_create();
	(*localized_pokemon).mensajes = list_create();

	catch_pokemon = malloc(sizeof(t_cola_mensaje));
	(*catch_pokemon).suscriptores = list_create();
	(*catch_pokemon).mensajes = list_create();

	caught_pokemon = malloc(sizeof(t_cola_mensaje));
	(*caught_pokemon).suscriptores = list_create();
	(*caught_pokemon).mensajes = list_create();
}

t_cola_mensaje* selecciono_cola(int cola) {
	switch(cola) {
		case NEW_POKEMON: {
			return new_pokemon;
		}
		case APPEARED_POKEMON: {
			return appeared_pokemon;
		}
		case GET_POKEMON: {
			return get_pokemon;
		}
		case LOCALIZED_POKEMON: {
			return localized_pokemon;
		}
		case CATCH_POKEMON: {
			return catch_pokemon;
		}
		case CAUGHT_POKEMON: {
			return caught_pokemon;
		}
		default: return -1;
	}
}

