#include <conexiones.h>
#include "broker.h"

int main(int argc, char *argv[]){

	char * configPath = "../broker.config";
	char * ipconfig= "IP_BROKER";
	char * puertocofing= "PUERTO_BROKER";
	const int lista_colas[6] = {NEW_POKEMON, APPEARED_POKEMON, CATCH_POKEMON, CAUGHT_POKEMON, GET_POKEMON, LOCALIZED_POKEMON };

	inicializar_datos();

	creacion_hilos_distribucion(lista_colas);



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

	config = config_create("/home/utnso/tp-2020-1c-Elite-Four/broker/broker.config");

	ip = config_get_string_value(config, ip_config);
	puerto = config_get_string_value(config, port_config);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

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
			sem_wait(&mutexLista[NEW_POKEMON]);

			mensaje_completo->mensaje = recibir_new_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			list_add(new_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[NEW_POKEMON]);
			break;
		}
		case APPEARED_POKEMON: {
			sem_wait(&mutexLista[APPEARED_POKEMON]);

			mensaje_completo->mensaje = recibir_appeared_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->mensaje->id_correlativo == 0 || !fue_respondido(mensaje_completo, appeared_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				list_add(appeared_pokemon->mensajes, mensaje_completo);
			} else {
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			sem_post(&mutexLista[APPEARED_POKEMON]);
			break;
		}
		case CATCH_POKEMON: {
			sem_wait(&mutexLista[CATCH_POKEMON]);

			mensaje_completo->mensaje = recibir_catch_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			list_add(catch_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[CATCH_POKEMON]);
			break;
		}
		case CAUGHT_POKEMON: {
			sem_wait(&mutexLista[CAUGHT_POKEMON]);

			mensaje_completo->mensaje = recibir_caught_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->mensaje->id_correlativo == 0 || !fue_respondido(mensaje_completo, caught_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				list_add(caught_pokemon->mensajes, mensaje_completo);
			} else {
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			sem_post(&mutexLista[CAUGHT_POKEMON]);
			break;
		}
		case GET_POKEMON: {
			sem_wait(&mutexLista[GET_POKEMON]);

			mensaje_completo->mensaje = recibir_get_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			list_add(get_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[GET_POKEMON]);
			break;
		}
		case LOCALIZED_POKEMON: {
			sem_wait(&mutexLista[LOCALIZED_POKEMON]);

			mensaje_completo->mensaje = recibir_localized_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->mensaje->id_correlativo == 0 || !fue_respondido(mensaje_completo, localized_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				list_add(localized_pokemon->mensajes, mensaje_completo);
			} else {
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			sem_post(&mutexLista[LOCALIZED_POKEMON]);
			break;
		}
		case SUSCRIBE: {
			puntero_suscripcion_cola mensaje_suscripcion;

			mensaje_suscripcion = recibir_suscripcion(socket, &size, logger_broker);

			sem_wait(&mutexLista[mensaje_suscripcion->cola]);
			agregar_suscriptor_cola(mensaje_suscripcion);
			sem_post(&mutexLista[mensaje_suscripcion->cola]);

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
		sem_wait(&mutexDistribucion);
		sem_wait(&mutexLista[cola]);

		distribuir_mensajes_cola(cola);

		sem_post(&mutexLista[cola]);
		sem_post(&mutexDistribucion);
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

	// RECORRO TODOS LOS MENSAJES DE LA COLA
	for(int i = 0; i < list_size(cola_mensajes->mensajes); i++) {
		puntero_mensaje_completo = list_get(cola_mensajes->mensajes, i);

		// OBTENGO CADA SUSCRIPTOR DE UN MENSAJE
		for(int j = 0; j < list_size(cola_mensajes->suscriptores) ;j++) {
			suscriptor = list_get(cola_mensajes->suscriptores, j);

			bool encuentra_suscriptor(void* elemento) {
				return strcmp((char*)elemento, suscriptor) == 0;
			}
			// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
			bool encontre = list_any_satisfy(puntero_mensaje_completo->suscriptores_ack, (void*)encuentra_suscriptor);
			// SI NO ESTA EN LA LISTA DE LOS ACK, LE ENVIO EL MENSAJE
			if (!encontre) {
				distribuir_mensaje_sin_enviar_a(suscriptor, cola, puntero_mensaje_completo);
				// MIRO SI ESTA EN LA LISTA DE LOS ENVIADOS,
				bool enviado = list_any_satisfy(puntero_mensaje_completo->suscriptores_enviados, (void*)encuentra_suscriptor);
				// SI NO ESTA, LO AGREGO
				if(!enviado) {
					list_add(puntero_mensaje_completo->suscriptores_enviados, suscriptor);
				}
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
	char* aux;

	// Separa el "suscriptor" (IP:PUERTO) en IP y PUERTO
	aux = strtok(suscriptor, ":");
	ip_suscriptor = aux;
	aux = strtok(NULL, ":");
	puerto_suscriptor = aux;

	conexion = crear_conexion(ip_suscriptor, puerto_suscriptor);
	uint32_t id = puntero_mensaje_completo->mensaje->id;
	uint32_t id_correlativo = puntero_mensaje_completo->mensaje->id_correlativo;
	switch(cola) {
		case NEW_POKEMON: {
			puntero_mensaje_new_pokemon puntero_mensaje = ((puntero_mensaje_new_pokemon*)puntero_mensaje_completo->mensaje->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;
			uint32_t quant = puntero_mensaje->quant_pokemon;
			send_message_new_pokemon(nombre, posx, posy, quant, id, id_correlativo, conexion);

			break;
		}
		case LOCALIZED_POKEMON: {
			puntero_mensaje_localized_pokemon puntero_mensaje = ((puntero_mensaje_localized_pokemon*)puntero_mensaje_completo->mensaje->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			t_list* coords = puntero_mensaje->coords;
			uint32_t quant = puntero_mensaje->quant_pokemon;
			send_message_localized_pokemon(nombre, quant, coords, id, id_correlativo, conexion);

			break;
		}
		case GET_POKEMON: {
			puntero_mensaje_get_pokemon puntero_mensaje = ((puntero_mensaje_get_pokemon*)puntero_mensaje_completo->mensaje->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;

			send_message_get_pokemon(nombre, id, id_correlativo, conexion);

			break;
		}
		case APPEARED_POKEMON: {
			puntero_mensaje_appeared_pokemon puntero_mensaje = ((puntero_mensaje_appeared_pokemon*)puntero_mensaje_completo->mensaje->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;

			send_message_appeared_pokemon(nombre, posx, posy, id, id_correlativo, conexion);

			break;
		}
		case CATCH_POKEMON: {
			puntero_mensaje_catch_pokemon puntero_mensaje = ((puntero_mensaje_catch_pokemon*)puntero_mensaje_completo->mensaje->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;

			send_message_catch_pokemon(nombre, posx, posy, id, id_correlativo, conexion);

			break;
		}
		case CAUGHT_POKEMON: {
			puntero_mensaje_caught_pokemon puntero_mensaje = ((puntero_mensaje_caught_pokemon*)puntero_mensaje_completo->mensaje->mensaje_cuerpo);

			char* caughtPokemon = puntero_mensaje->caught_pokemon;

			send_message_caught_pokemon(caughtPokemon, id, id_correlativo, conexion);

			break;
		}

	}

	mensaje_recibido = client_recibir_mensaje(conexion);

	// TODO que hago si no recibo el ACK?
	if(strcmp(mensaje_recibido, "ACK") == 0) {
		list_add(puntero_mensaje_completo->suscriptores_ack, suscriptor);
	}

	free(mensaje_recibido);
	close(conexion);
}

void inicializar_datos() {

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

	for(int i = 0; i <= SEM_POOL; i++) {
		sem_t semaforo;
		sem_init(&semaforo, 0, 1);
		mutexLista[i] = semaforo;
	}

	sem_init(&mutexDistribucion, 0, 1);
	sem_init(&mutexIds, 0, 1);

	cantidad_mensajes = 1;

	logger_broker =log_create("/home/utnso/tp-2020-1c-Elite-Four/broker/broker.log", "BROKER", false, LOG_LEVEL_INFO);
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

void creacion_hilos_distribucion(int lista_colas[]) {
    for(int j= 0; j < THREAD_POOL; j++) {
    	int* puntero_cola = malloc(sizeof(puntero_cola));
    	*puntero_cola = lista_colas[j];
		pthread_create(&thread_pool[j], NULL, distribuir_mensajes, puntero_cola);
    }
}

void asignar_y_devolver_id(t_mensaje_completo* mensaje_completo, int socket) {
	sem_wait(&mutexIds);

	mensaje_completo->mensaje->id = cantidad_mensajes;
	char* id_mensaje = string_itoa(cantidad_mensajes);
	devolver_mensaje(id_mensaje, strlen(id_mensaje) + 1, socket);

	aumentar_cantidad_mensajes();

	sem_post(&mutexIds);
}

bool fue_respondido(t_mensaje_completo* mensaje_completo, t_cola_mensaje* cola_mensaje) {
	uint32_t id_correlativo = mensaje_completo->mensaje->id_correlativo;
	bool encuentra_id_correlativo(void* elemento) {
		t_mensaje_completo* mensaje = (t_mensaje_completo*)elemento;

		return mensaje->mensaje->id_correlativo == id_correlativo;
	}
	// CHEQUEO SI ESTA EL ID CORRELATIVO DEL MENSAJE RECIBIDO EN LA COLA DE MENSAJES
	return list_any_satisfy(cola_mensaje->mensajes, (void*)encuentra_id_correlativo);
}
