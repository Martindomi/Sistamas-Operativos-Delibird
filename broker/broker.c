#include <conexiones.h>
#include "broker.h"

int main(int argc, char *argv[]){
	// Creo lista de colas para utilizar en la creacion de los hilos de distribucion de mensajes
	const int lista_colas[6] = {NEW_POKEMON, APPEARED_POKEMON, CATCH_POKEMON, CAUGHT_POKEMON, GET_POKEMON, LOCALIZED_POKEMON };

	// Leo el archivo de configuracion del broker
	leer_archivo_config();

	// Inicializo los datos correspondientes
	inicializar_datos();

	// Creo los hilos para la distribucion de mensajes
	creacion_hilos_distribucion(lista_colas);

	// Creo hilo de escucha de mensajes nuevos en la ip y puerto del broker
	iniciar_servidor();

	return EXIT_SUCCESS;

}

void iniciar_servidor()
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ipBroker, puertoBroker, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        guard((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)), "Failed to create socket");

        guard(bind(socket_servidor, p->ai_addr, p->ai_addrlen), "Bind failed");

        break;
    }

	guard(listen(socket_servidor, SOMAXCONN), "Listen failed");

    freeaddrinfo(servinfo);

    while(1)
    	esperar_cliente(socket_servidor);
}

void esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = guard(accept(socket_servidor, (void*) &dir_cliente, &tam_direccion), "Accept failed");

	// Crea un hilo de escucha de nuevos mensajes cuando recibe una conexion por parte cliente
	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_detach(thread);

}

void serve_client(int* socket)
{
	// Recibo el codigo de operacion del mensaje entrante
	op_code cod_op;
	log_info(loggerBroker,"Se conecto un proceso al broker");
	if(recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL) == -1)
		cod_op = -1;
	// Envio codigo de operacion para procesar mensaje
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int socket) {
	uint32_t size;
	t_mensaje* mensaje_completo = malloc(sizeof(t_mensaje));
	// Proceso la request segun el codigo de operacion
	switch (cod_op) {
		case MESSAGE: {
			// En caso de ser un mensaje, lo recibe y devuelve el ack
			void* msg;
			msg = server_recibir_mensaje(socket, &size);
			msg = "ACK";
			size = sizeof(4);
			devolver_mensaje(msg, size, socket);
			free(msg);
			break;
		}
		case NEW_POKEMON: {
			mensaje_completo = recibir_new_pokemon(socket, &size);

			log_info(loggerBroker, "Llega un mensaje a la cola de NEW_POKEMON");

			asignar_y_devolver_id(mensaje_completo, socket);

			asignar_memoria(mensaje_completo, cod_op);

			// Agrega el mensaje a la cola de mensajes NEW_POKEMON
			list_add(new_pokemon->mensajes, mensaje_completo);

			// Aumenta el semaforo para permitir la distribucion de mensajes de esta cola
			sem_post(&mutexLista[NEW_POKEMON]);

			break;
		}
		case APPEARED_POKEMON: {
			mensaje_completo = recibir_appeared_pokemon(socket, &size);

			log_info(loggerBroker,"Llega un mensaje a la cola de APPEARED_POKEMON");

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, appeared_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				asignar_memoria(mensaje_completo, cod_op);

				// Agrega el mensaje a la cola de mensajes APPEARED_POKEMON
				list_add(appeared_pokemon->mensajes, mensaje_completo);


			} else {
				// Si el mensaje ya fue respondido, le informo al cliente que ya tengo una respuesta
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			// Aumenta el semaforo para permitir la distribucion de mensajes de esta cola
			sem_post(&mutexLista[APPEARED_POKEMON]);
			break;
		}
		case CATCH_POKEMON: {

			mensaje_completo = recibir_catch_pokemon(socket, &size);

			log_info(loggerBroker,"Llega un mensaje a la cola de CATCH_POKEMON");

			asignar_y_devolver_id(mensaje_completo, socket);

			asignar_memoria(mensaje_completo, cod_op);

			// Agrega el mensaje a la cola de mensajes CATCH_POKEMON
			list_add(catch_pokemon->mensajes, mensaje_completo);

			// Aumenta el semaforo para permitir la distribucion de mensajes de esta cola
			sem_post(&mutexLista[CATCH_POKEMON]);
			break;
		}
		case CAUGHT_POKEMON: {

			mensaje_completo = recibir_caught_pokemon(socket, &size);

			log_info(loggerBroker,"Llega un mensaje a la cola de CAUGHT_POKEMON");

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, caught_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				asignar_memoria(mensaje_completo, cod_op);

				// Agrega el mensaje a la cola de mensajes CAUGHT_POKEMON
				list_add(caught_pokemon->mensajes, mensaje_completo);

			} else {
				// Si el mensaje ya fue respondido, le informo al cliente que ya tengo una respuesta
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			// Aumenta el semaforo para permitir la distribucion de mensajes de esta cola
			sem_post(&mutexLista[CAUGHT_POKEMON]);
			break;
		}
		case GET_POKEMON: {

			mensaje_completo = recibir_get_pokemon(socket, &size);

			log_info(loggerBroker,"Llega un mensaje a la cola de GET_POKEMON");

			asignar_y_devolver_id(mensaje_completo, socket);

			asignar_memoria(mensaje_completo, cod_op);

			// Agrega el mensaje a la cola de mensajes GET_POKEMON
			list_add(get_pokemon->mensajes, mensaje_completo);

			// Aumenta el semaforo para permitir la distribucion de mensajes de esta cola
			sem_post(&mutexLista[GET_POKEMON]);

			break;
		}
		case LOCALIZED_POKEMON: {

			mensaje_completo = recibir_localized_pokemon(socket, &size);

			log_info(loggerBroker,"Llega un mensaje a la cola de LOCALIZED_POKEMON");

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, localized_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				asignar_memoria(mensaje_completo, cod_op);

				// Agrega el mensaje a la cola de mensajes LOCALIZED_POKEMON
				list_add(localized_pokemon->mensajes, mensaje_completo);

			} else {
				// Si el mensaje ya fue respondido, le informo al cliente que ya tengo una respuesta
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			// Aumenta el semaforo para permitir la distribucion de mensajes de esta cola
			sem_post(&mutexLista[LOCALIZED_POKEMON]);
			break;
		}
		case SUSCRIBE: {
			puntero_suscripcion_cola mensaje_suscripcion;

			mensaje_suscripcion = recibir_suscripcion(socket, &size, loggerBroker);

			log_info(loggerBroker, "El proceso %s se suscribe a la cola %s", mensaje_suscripcion->cliente, nombre_cola(mensaje_suscripcion->cola));

			agregar_suscriptor_cola(mensaje_suscripcion, socket);

			// Devuelve al cliente que se suscribio con exito
			char* suscripcion_aceptada = "SUSCRIPCION COMPLETADA";
			devolver_mensaje(suscripcion_aceptada, strlen(suscripcion_aceptada) + 1, socket);

			// Espera que nadie este usando memoria
			sem_wait(&mutexAsignarMemoria);
			enviar_mensajes_memoria(mensaje_suscripcion, socket);
			sem_post(&mutexAsignarMemoria);

			//sem_post(&mutexLista[mensaje_suscripcion->cola]);

			// Libera el mensaje si no esta suscripto por un tiempo determinado
			if(mensaje_suscripcion->tiempo == -1) {
				free(mensaje_suscripcion);
			}
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
	// Aumento la cantidad de mensajes recibidos (actua como id de mensajes)
	cantidad_mensajes ++;
}

void agregar_suscriptor_cola(puntero_suscripcion_cola mensaje_suscripcion, int socket){
	// Busco la cola a la que se quiere suscribir
	t_cola_mensaje* cola_mensaje = selecciono_cola(mensaje_suscripcion->cola);

	// Creo estructura auxiliar para el manejo de hilo
	puntero_suscriptor suscriptor = malloc(sizeof(t_suscriptor));
	suscriptor->cliente = mensaje_suscripcion->cliente;
	suscriptor->socket = socket;

	// Agrego a la lista de suscriptores de la cola de mensajes al suscriptor
	list_add(cola_mensaje->suscriptores, suscriptor);

	// Si el tiempo que recibo es distinto de -1
	pthread_t threadSuscripcion;
	if(mensaje_suscripcion->tiempo != -1) {
		// Creo un hilo para desuscribir al suscriptor luego de un tiempo
		pthread_create(&threadSuscripcion,NULL,(void*)desuscribir_cliente, mensaje_suscripcion);
		pthread_detach(threadSuscripcion);
	}
}

void desuscribir_cliente(puntero_suscripcion_cola mensaje) {
	// Espera X cantidad de tiempo
	sleep(mensaje->tiempo);

	// Busca la cola de donde debe desuscribir al cliente
	t_cola_mensaje* cola_mensaje = selecciono_cola(mensaje->cola);

	// Devuelve true si encuentra al suscriptor
	bool encontrar_suscriptor(void* elemento) {
		puntero_suscriptor suscriptor = (puntero_suscriptor) elemento;
		return strcmp(suscriptor->cliente, mensaje->cliente) == 0;
	}

	// Libera al suscriptor eliminado de la cola
	void free_suscriptor(void* elemento) {
		puntero_suscriptor suscriptor = (puntero_suscriptor) elemento;
		//free(suscriptor->cliente);
		free(suscriptor);
	}

	// Busca al suscriptor en la cola de mensajes y lo elimina
	list_remove_and_destroy_by_condition(cola_mensaje->suscriptores, encontrar_suscriptor, free_suscriptor);
}

void* distribuir_mensajes(void* puntero_cola) {
	// Distribuye los mensajes de las diferentes colas a sus suscriptores
	while(1) {

		int cola = *((int*) puntero_cola);

		// Espero que me llegue un mensaje de una cola para seguir el flujo
		sem_wait(&mutexLista[cola]);
		// Espero que no haya nadie mas distribuyendo mensajes
		sem_wait(&mutexDistribucion);
		// Distribuyo los mensajes de una cola especifica
		distribuir_mensajes_cola(cola);

		sem_post(&mutexDistribucion);
		// Espero 15 segundos para repetir proceso
		sleep(15);
	}
}

void distribuir_mensajes_cola(int cola) {
	// Distribucion de los mensajes en colas de mensajes no enviados a sus suscriptores
	puntero_mensaje puntero_mensaje;
	t_cola_mensaje* cola_mensajes = selecciono_cola(cola);
	// TODO Mejorar manejo de error
	if (cola_mensajes == -1) {
		pthread_exit(NULL);
	}

	// RECORRO TODOS LOS MENSAJES DE LA COLA
	for(int i = 0; i < list_size(cola_mensajes->mensajes); i++) {
		puntero_mensaje = list_get(cola_mensajes->mensajes, i);
		envio_mensaje(puntero_mensaje, cola, cola_mensajes);
	}
}

void envio_mensaje(puntero_mensaje puntero_mensaje, int cola, t_cola_mensaje* cola_mensajes) {
	puntero_suscriptor suscriptor;

	// OBTENGO CADA SUSCRIPTOR DE UN MENSAJE
	for(int j = 0; j < list_size(cola_mensajes->suscriptores) ;j++) {
		suscriptor = list_get(cola_mensajes->suscriptores, j);

		punteroParticion punteroParticionMensaje = buscar_particion_mensaje(puntero_mensaje->id);

		bool encuentra_suscriptor(void* elemento) {
			return strcmp((char*)elemento, suscriptor->cliente) == 0;
		}
		// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
		bool encontre = list_any_satisfy(punteroParticionMensaje->suscriptores_ack, (void*)encuentra_suscriptor);
		// SI NO ESTA EN LA LISTA DE LOS ACK, LE ENVIO EL MENSAJE
		if (!encontre) {
			distribuir_mensaje_sin_enviar_a(suscriptor, cola, puntero_mensaje, NULL);
			// MIRO SI ESTA EN LA LISTA DE LOS ENVIADOS,
			bool enviado = list_any_satisfy(punteroParticionMensaje->suscriptores_enviados, (void*)encuentra_suscriptor);
			// SI NO ESTA, LO AGREGO
			if(!enviado) {
				list_add(punteroParticionMensaje->suscriptores_enviados, suscriptor->cliente);
			}
		}
	}
}

punteroParticion buscar_particion_mensaje(uint32_t idMensaje) {
	bool obtener_particion_id(void* elemento) {
		punteroParticion punteroParticionElemento = (punteroParticion) elemento;
		return punteroParticionElemento->id == idMensaje;
	}
	// Busco la particion del mensaje a la que le corresponde el id
	return list_find(particiones, (void*)obtener_particion_id);
}

void distribuir_mensaje_sin_enviar_a(puntero_suscriptor suscriptor, int cola, puntero_mensaje puntero_mensaje_completo, punteroParticion punteroParticion) {
	int conexion;
	uint32_t id;
	uint32_t id_correlativo;

	// Obtengo el socket del cliente suscripto
	conexion = suscriptor->socket;
	// Si el punteroParticion es null, significa que esta distribuyendo un mensaje que tengo en la cola de mensajes
	if(punteroParticion == NULL) {
		id = puntero_mensaje_completo->id;
		id_correlativo = puntero_mensaje_completo->id_correlativo;
	} else {
		// Si no es null, significa que estoy distribuyendo mensajes de memoria
		id = punteroParticion->id;
		id_correlativo = punteroParticion->idCorrelativo;
	}

	// Me fijo a que cola de mensajes corresponde la distribucion y lo envio
	char* stringId = string_itoa(id);
	log_info(loggerBroker,"Se le envia al suscriptor %s un mensaje de la cola %s y su ID es: %s", suscriptor->cliente,nombre_cola(cola), stringId);
	free(stringId);

	switch(cola) {
		case NEW_POKEMON: {
			puntero_mensaje_new_pokemon puntero_mensaje = ((puntero_mensaje_new_pokemon*)puntero_mensaje_completo->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;
			uint32_t quant = puntero_mensaje->quant_pokemon;
			send_message_new_pokemon(nombre, posx, posy, quant, id, id_correlativo, conexion);

			break;
		}
		case LOCALIZED_POKEMON: {
			puntero_mensaje_localized_pokemon puntero_mensaje = ((puntero_mensaje_localized_pokemon*)puntero_mensaje_completo->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			t_list* coords = puntero_mensaje->coords;
			uint32_t quant = puntero_mensaje->quant_pokemon;
			send_message_localized_pokemon(nombre, quant, coords, id, id_correlativo, conexion);

			break;
		}
		case GET_POKEMON: {
			puntero_mensaje_get_pokemon puntero_mensaje = ((puntero_mensaje_get_pokemon*)puntero_mensaje_completo->mensaje_cuerpo);
			char* nombre = puntero_mensaje->name_pokemon;
			send_message_get_pokemon(nombre, id, id_correlativo, conexion);

			break;
		}
		case APPEARED_POKEMON: {
			puntero_mensaje_appeared_pokemon puntero_mensaje = ((puntero_mensaje_appeared_pokemon*)puntero_mensaje_completo->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;
			send_message_appeared_pokemon(nombre, posx, posy, id, id_correlativo, conexion);

			break;
		}
		case CATCH_POKEMON: {
			puntero_mensaje_catch_pokemon puntero_mensaje = ((puntero_mensaje_catch_pokemon*)puntero_mensaje_completo->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;

			send_message_catch_pokemon(nombre, posx, posy, id, id_correlativo, conexion);

			break;
		}
		case CAUGHT_POKEMON: {

			puntero_mensaje_caught_pokemon puntero_mensaje = ((puntero_mensaje_caught_pokemon*)puntero_mensaje_completo->mensaje_cuerpo);

			uint32_t caughtResult = puntero_mensaje->caughtResult;

			char* caughtPokemon = caughtResult == 0 ? "OK" : "FAIL";

			send_message_caught_pokemon(caughtPokemon, id, id_correlativo, conexion);

			break;
		}

	}
	// Creo un hilo para recibir el ACK por parte del cliente asi no corta el flujo de ejecucion
	pthread_t threadACK;
	puntero_ack punteroAck = malloc(sizeof(t_ack));
	punteroAck->conexion = conexion;
	punteroAck->idMensaje = id;
	punteroAck->suscriptor = suscriptor->cliente;
	pthread_create(&threadACK, NULL, esperar_mensaje_ack, punteroAck);
	pthread_detach(threadACK);

}

void esperar_mensaje_ack(puntero_ack punteroAck) {
	char* mensaje_recibido;

	mensaje_recibido = client_recibir_mensaje(punteroAck->conexion);
	if(strcmp(mensaje_recibido, "ACK") == 0) {

		char* idACK = string_itoa(punteroAck->idMensaje);
		log_info(loggerBroker, "El proceso %s envia el %s del mensaje con ID: %s", punteroAck->suscriptor, mensaje_recibido, idACK);
		free(idACK);

		punteroParticion punteroParticionEncontrado = buscar_particion_mensaje(punteroAck->idMensaje);

		if(punteroParticionEncontrado != NULL) {
			// Agrego al suscriptor como ACK del mensaje
			list_add(punteroParticionEncontrado->suscriptores_ack, punteroAck->suscriptor);
		}

	}

	free(mensaje_recibido);
	free(punteroAck);
}

void inicializar_datos() {
	printf("Process id %d\n", getpid());

	// Manejo de señal SIGUSR1 para realizar el dump de cache
	signal(SIGUSR1, manejo_dump_cache);
	signal(SIGINT, manejo_end);
	// Pedido de memoria de todas las colas de mensajes
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

	// Semaforos de colas de mensajes
	for(int i = 0; i <= SEM_POOL; i++) {
		sem_t semaforo;
		sem_init(&semaforo, 0, 0);
		mutexLista[i] = semaforo;
	}

	// Semaforo de distribucion mensajes nuevos
	sem_init(&mutexDistribucion, 0, 1);
	// Semaforo de ids
	sem_init(&mutexIds, 0, 1);
	// Semaforo asignacion de memoria
	sem_init(&mutexAsignarMemoria, 0 ,1);

	// Variable que representa el ID de cada mensaje
	cantidad_mensajes = 1;

	// Creacion de memoria principal
	punteroMemoriaPrincipal = malloc(tamanoMemoria);
	particiones = list_create();
	punteroParticion particionInicial = malloc(sizeof(t_particion));
	particionInicial->colaMensaje = NULL;
	particionInicial->id = NULL;
	particionInicial->idCorrelativo = NULL;
	particionInicial->ocupada = false;
	particionInicial->izq = false;
	particionInicial->der = false;
	particionInicial->historicoBuddy = list_create();
	list_add(particionInicial->historicoBuddy, "C");
	particionInicial->punteroMemoria = punteroMemoriaPrincipal;
	particionInicial->tamanoMensaje = calcular_tamano(tamanoMemoria, 0);
	particionInicial->suscriptores_ack = list_create();
	particionInicial->suscriptores_enviados = list_create();
	particionInicial->lruHora = obtener_milisegundos();
	list_add(particiones, particionInicial);
	punteroMemoriaFinal = (char*)punteroMemoriaPrincipal + calcular_tamano(tamanoMemoria, 0);
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

char* nombre_cola(int cola){
	switch(cola) {
		case NEW_POKEMON: {
			return "NEW_POKEMON";
		}
		case APPEARED_POKEMON: {
			return "APPEARED_POKEMON";
		}
		case GET_POKEMON: {
			return "GET_POKEMON";
		}
		case LOCALIZED_POKEMON: {
			return "LOCALIZED_POKEMON";
		}
		case CATCH_POKEMON: {
			return "CATCH_POKEMON";
		}
		case CAUGHT_POKEMON: {
			return "CAUGHT_POKEMON";
		}
		default: return "NO_ASIGNADA";
	}
}

void creacion_hilos_distribucion(int lista_colas[]) {
	// Creo un hilo con la funcion de distribucion por cada cola de mensajes
    for(int j= 0; j < THREAD_POOL; j++) {
    	int* puntero_cola = malloc(sizeof(puntero_cola));
    	*puntero_cola = lista_colas[j];
		pthread_create(&thread_pool[j], NULL, distribuir_mensajes, puntero_cola);
		pthread_detach(thread_pool[j]);

    }
}

void asignar_y_devolver_id(t_mensaje* mensaje_completo, int socket) {
	// Espera que no haya nadie aumentando la cantidad de mensajes recibidos (ID)
	sem_wait(&mutexIds);

	// Asigna el id al mensaje y se lo devuelve al cliente
	mensaje_completo->id = cantidad_mensajes;
	char* id_mensaje = string_itoa(cantidad_mensajes);
	devolver_mensaje(id_mensaje, strlen(id_mensaje) + 1, socket);
	free(id_mensaje);
	// Aumenta la cantidad de mensajes recibidos (ID)
	aumentar_cantidad_mensajes();

	sem_post(&mutexIds);
}

bool fue_respondido(t_mensaje* mensaje_completo, t_cola_mensaje* cola_mensaje) {
	uint32_t id_correlativo = mensaje_completo->id_correlativo;
	bool encuentra_id_correlativo(void* elemento) {
		t_mensaje* mensaje = (t_mensaje*)elemento;

		return mensaje->id_correlativo == id_correlativo;
	}
	// CHEQUEO SI ESTA EL ID CORRELATIVO DEL MENSAJE RECIBIDO EN LA COLA DE MENSAJES
	return list_any_satisfy(cola_mensaje->mensajes, (void*)encuentra_id_correlativo);
}

void aplica_funcion_escucha(int * socket){
	// Declarada para que no rompa el hilo de escucha
}

void leer_archivo_config() {
	// Lee y guarda la configuracion ingresada en el archivo .config
	configBroker = guard_lectura_string_config(config_create("/home/utnso/tp-2020-1c-Elite-Four/broker/broker.config"));

	ipBroker = obtener_string_config(configBroker, "IP_BROKER");
	puertoBroker = obtener_string_config(configBroker, "PUERTO_BROKER");
	logFile = obtener_string_config(configBroker, "LOG_FILE");
	tamanoMemoria = obtener_int_config(configBroker, "TAMANO_MEMORIA");
	tamanoMinimoParticion = obtener_int_config(configBroker, "TAMANO_MINIMO_PARTICION");
	algoritmoMemoria = validar_string_binario(obtener_string_config(configBroker, "ALGORITMO_MEMORIA"), "PARTICIONES", "BS");
	algoritmoReemplazo = validar_string_binario(obtener_string_config(configBroker, "ALGORITMO_REEMPLAZO"), "FIFO", "LRU");
	algoritmoParticionLibre = validar_string_binario(obtener_string_config(configBroker, "ALGORITMO_PARTICION_LIBRE"), "FF", "BF");
	frecuenciaCompactacion = obtener_int_config(configBroker, "FRECUENCIA_COMPACTACION");

	loggerBroker =log_create(logFile, "BROKER", true, LOG_LEVEL_INFO);

}

void asignar_memoria(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	// Se fija que no haya otro mensaje siendo asignado en memoria
	sem_wait(&mutexAsignarMemoria);

	// Busca el algoritmo de particiones segun lo cargado en config
	if(strcmp(algoritmoMemoria, "PARTICIONES") == 0) {
		// Entra en el algoritmo de particiones
		asignar_memoria_pd(mensajeCompleto, colaMensaje);
	} else if(strcmp(algoritmoMemoria, "BS") == 0) {
		// Entra en el algoritmo de Buddy System
		asignar_memoria_bs(mensajeCompleto, colaMensaje);
	}
	sem_post(&mutexAsignarMemoria);
}

void asignar_memoria_pd(t_mensaje* mensajeCompleto, int colaMensaje) {
	// Comprueba que el mensaje que intenta guardar no sea mayor que el tamaño total de la memoria asignada al broker
	if(mensajeCompleto->size_mensaje_cuerpo <= tamanoMemoria) {

		// Busca una posicion libre de memoria para asignarle el mensaje
		void* posMemoria = pd_memoria_libre(mensajeCompleto, (uint32_t) colaMensaje);

		// Si no encuentra una posicion de memoria vacia
		while(posMemoria == NULL) {
			// Aumento la cantidad de fallos de busquedas que hubo
			cantidadBusquedasFallidas ++;
			if(frecuenciaCompactacion == 0) {
				// Si frecuencia de compactacion es 0, compacta y vuelve a buscar
				compactar_memoria();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else if(frecuenciaCompactacion == -1) {
				// Si frecuencia de compactacion es -1, vacia la memoria, consolida y vuelve a buscar
				vaciar_memoria();
				consolidar(NULL);
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else if(frecuenciaCompactacion == 1) {
				// Si frecuencia de compactacion es 1, elimina una particion, consolida, compacta y vuelve a buscar
				eliminar_particion();
				compactar_memoria();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else if(cantidadBusquedasFallidas % frecuenciaCompactacion == 0) {
				// Si la cantidad de veces que fallo en encontrar memoria libre es multiplo de la
					//frecuencia de compactacion, compacta y vuelve a buscar
				compactar_memoria();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else {
				// Caso contrario a lo anterior, elimina una particion y vuelve a buscar
				eliminar_particion();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			}
		}

		// A este punto llega si encuentra una posicion de memoria libre para alojar el mensaje
			//y lo guarda en esa posicion
		guardar_mensaje_memoria(mensajeCompleto, posMemoria, (uint32_t) colaMensaje);
		char* posInicioParticionMemoria = string_itoa((char*)posMemoria - (char*)punteroMemoriaPrincipal);
		log_info(loggerBroker, "Se ingresa un mensaje con ID: %d con posicion relativa: %s", mensajeCompleto->id, posInicioParticionMemoria);
		free(posInicioParticionMemoria);

	} else {
		// En caso de que el mensaje exceda el peso total de la memoria, corto el flujo
		guard(-1, "Mensaje excede el limite permitido.");
	}
}

void* pd_memoria_libre(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	// Entra al algoritmo de particiones y comprueba si la lista de memoria esta llena
	if(!lista_llena(particiones)){
		// Si no esta llena, se fija en las configuraciones que algoritmo debe usar para ubicar el nuevo mensaje
		if(strcmp(algoritmoParticionLibre, "FF") == 0) {
			// Utiliza First Fit
			return buscar_memoria_libre_first_fit(mensajeCompleto, colaMensaje);
		} else if (strcmp(algoritmoParticionLibre, "BF") == 0) {
			// Utiliza Best Fit
			return buscar_memoria_libre_best_fit(mensajeCompleto, colaMensaje);
		}
	} else {
		// Si la lista esta llena, devuelvo que no encontro posicion libre
		return NULL;
	}
}

void* buscar_memoria_libre_first_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	// RECORRE LAS PARTICIONES EN BUSCA DE UNA LIBRE Y DONDE ENTRE EL MENSAJE
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionObtenido = list_get(particiones, i);
		if(!punteroParticionObtenido->ocupada){
			// En caso de que la particion que encontre libre tiene un tamaño mayor al del mensaje que quiero guardar,
				// entonces debo crear una nueva particion
			if(punteroParticionObtenido->tamanoMensaje
					> calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0)) {
				// Genero una nueva particion vacia del tamaño restante
				punteroParticion nuevaParticion = malloc(sizeof(t_particion));
				nuevaParticion->colaMensaje = NULL;
				nuevaParticion->id = NULL;
				nuevaParticion->idCorrelativo = NULL;
				nuevaParticion->ocupada = false;
				nuevaParticion->punteroMemoria = ((char*)punteroParticionObtenido->punteroMemoria)
						+ calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0);
				// TODO revisar que hacer si la particion nueva tiene un tamaño menor al permitido por variable
				nuevaParticion->tamanoMensaje = punteroParticionObtenido->tamanoMensaje -
						calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0);
				nuevaParticion->suscriptores_ack = list_create();
				nuevaParticion->suscriptores_enviados = list_create();
				nuevaParticion->lruHora = obtener_milisegundos();
				list_add(particiones, nuevaParticion);
			}
			// En caso de que tenga el mismo tamaño o uno menor
			if(punteroParticionObtenido->tamanoMensaje >= mensajeCompleto->size_mensaje_cuerpo) {
				// A la particion libre encontrada le asigno la info del mensaje
				punteroParticionObtenido->colaMensaje = colaMensaje;
				punteroParticionObtenido->id = mensajeCompleto->id;
				punteroParticionObtenido->idCorrelativo = mensajeCompleto->id_correlativo;
				punteroParticionObtenido->ocupada = true;
				punteroParticionObtenido->tamanoMensaje = calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0);
				punteroParticionObtenido->lruHora = obtener_milisegundos();
				list_clean(punteroParticionObtenido->suscriptores_ack);
				list_clean(punteroParticionObtenido->suscriptores_enviados);

				// Retorno la posicion donde guarde el mensaje
				return punteroParticionObtenido->punteroMemoria;
			}
		}
	}
	// En caso de no encontrar ninguna posicion libre, retorna null
	return NULL;
}

void* buscar_memoria_libre_best_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	// Busca una particion desocupada
	punteroParticion punteroMejorParticion = list_find(particiones, primer_puntero_desocupado);
	// Si encuentra una desocupada
	if(punteroMejorParticion != NULL) {
		// BUSCA LA PARTICION OPTIMA EN TAMAÑO PARA EL MENSAJE
		for(int i = 0; i < list_size(particiones); i++) {
			punteroParticion punteroParticionObtenido = list_get(particiones, i);
			// SI ESTA DESOCUPADA, EL TAMAÑO DEL MENSAJE ENTRA Y TIENE UN TAMAÑO MENOR AL DE LA MEJOR
			if(!punteroParticionObtenido->ocupada &&
					punteroParticionObtenido->tamanoMensaje >=
						calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0) &&
							punteroParticionObtenido->tamanoMensaje <
								punteroMejorParticion->tamanoMensaje ){
				punteroMejorParticion = punteroParticionObtenido;
			}
		}
		// Una vez que obtiene la mejor particion donde entra el mensaje
		// Se fija si el tamaño de la particion es mayor al del mensaje a guardar
		if(punteroMejorParticion->tamanoMensaje
							> calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0)) {
			// En caso de serlo, crea una nueva particion vacia con el tamano restante
				//y la agrega a la lista de particiones
			punteroParticion nuevaParticion = malloc(sizeof(t_particion));
			nuevaParticion->colaMensaje = NULL;
			nuevaParticion->id = NULL;
			nuevaParticion->idCorrelativo = NULL;
			nuevaParticion->ocupada = false;
			nuevaParticion->punteroMemoria = ((char*)punteroMejorParticion->punteroMemoria)
					+ calcular_tamano((char*) mensajeCompleto->size_mensaje_cuerpo, 0);
			// TODO revisar que hacer si la particion nueva tiene un tamaño menor al permitido por variable
			nuevaParticion->tamanoMensaje = punteroMejorParticion->tamanoMensaje-
					calcular_tamano((char*) mensajeCompleto->size_mensaje_cuerpo, 0);
			nuevaParticion->suscriptores_ack = list_create();
			nuevaParticion->suscriptores_enviados = list_create();
			nuevaParticion->lruHora = obtener_milisegundos();
			list_add(particiones, nuevaParticion);
		}
		// Pregunta si el tamaño de la mejor particion es mayor o igual que el tamaño del mensaje que quiere guardar
		if(punteroMejorParticion->tamanoMensaje >= mensajeCompleto->size_mensaje_cuerpo) {
			// Si esto ocurre, guardo en la mejor particion el contenido del mensaje nuevo
			punteroMejorParticion->colaMensaje = colaMensaje;
			punteroMejorParticion->id = mensajeCompleto->id;
			punteroMejorParticion->idCorrelativo = mensajeCompleto->id_correlativo;
			punteroMejorParticion->ocupada = true;
			punteroMejorParticion->tamanoMensaje = calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0);
			punteroMejorParticion->lruHora = obtener_milisegundos();
			list_clean(punteroMejorParticion->suscriptores_ack);
			list_clean(punteroMejorParticion->suscriptores_enviados);

			// Retorno la posicion de memoria donde se guardo el mensaje
			return punteroMejorParticion->punteroMemoria;
		}
	}
	// En caso de no encontrar memoria libre retorna null
	return NULL;
}

void compactar_memoria() {
	// Compactar memoria
	punteroParticion punteroParticionDesocupada = NULL;
	punteroParticion punteroParticionOcupada = NULL;

	// Ordena la lista por posiciones de memoria
	list_sort(particiones, ordernar_particiones_memoria);

	// Recorra la lista de particiones
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionRecorrer = list_get(particiones, i);
		// Agarra la primer particion desocupada
		if(punteroParticionDesocupada == NULL){
			if(!punteroParticionRecorrer->ocupada) {
				punteroParticionDesocupada = list_get(particiones, i);
			}
		} else {
			// Si ya tiene una particion desocupada, busca una ocupada que le siga luego en el orden
			if(punteroParticionRecorrer->ocupada) {
				punteroParticionOcupada = list_get(particiones, i);
				break;
			}
		}
	}

	// Si pudo obtener ambas particiones, una desocupada y una ocupada que le siga a la desocupada
	if(punteroParticionDesocupada != NULL && punteroParticionOcupada != NULL) {
		// Procede a realizar el intercambio de estas particiones
		intercambio_particiones(punteroParticionDesocupada, punteroParticionOcupada);
		// Llamado recursivo para volver a aplicar el mismo proceso
		log_info(loggerBroker,"Compactación de particiones ...");
		compactar_memoria();
	} else {
		// En caso de no encontrar ninguna libre o no encontrar una ocupada luego de una libre, consolida
		consolidar(NULL);
		return;
	}

}

void eliminar_particion() {
	// Elimina una particion segun algoritmo
	int indexEliminar;
	// Obtiene segun el algoritmo el indice de la particion a eliminar dentro de la lista de particiones
	if(strcmp(algoritmoReemplazo, "FIFO") == 0) {
		// Utiliza algoritmo FIFO
		indexEliminar = eliminar_particion_fifo();
	} else if (strcmp(algoritmoReemplazo, "LRU") == 0) {
		// Utiliza algoritmo LRU
		indexEliminar = eliminar_particion_lru();
	}

	// Si obtiene un indice, elimina la particion en tal indice
	if(indexEliminar != -1) {
		eliminar_particion_seleccionada(indexEliminar);
		consolidar(indexEliminar);
	} else {
		// Si no obtiene indice, consolida si hay al menos una particion en la lista
		if(list_size(particiones) > 0) {
			consolidar(0);
		}
	}
}

int eliminar_particion_fifo() {
	// Eliminar una particion por FIFO
	punteroParticion punteroParticionMenorId;
	// Busca la primera particion ocupada
	punteroParticionMenorId = list_find(particiones, (void*)primer_puntero_ocupado);
	int index = -1;
	// BUSCO PARTICION CON MENOR ID => MENSAJE MAS VIEJO EN MEMORIA
	if(punteroParticionMenorId != NULL) {
		for(int i = 0; i < list_size(particiones); i++) {
			punteroParticion punteroParticionEncontrado = list_get(particiones, i);
			if(punteroParticionMenorId->id >= punteroParticionEncontrado->id && punteroParticionEncontrado->ocupada) {
				punteroParticionMenorId = punteroParticionEncontrado;
				index = i;
			}
		}
	}
	// Retorno el indice de la particion mas vieja
	return index;
}

int eliminar_particion_lru() {
	// Eliminar particion por LRU
	int index = -1;
	// Agarra la primer particion
	punteroParticion punteroParticionLru = list_get(particiones, 0);
	// BUSCO EL PUNTERO CON EL TIEMPO DE USO MAS LEJANO
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticion = list_get(particiones, i);
		if(punteroParticion->ocupada) {
			if(punteroParticion->lruHora <= punteroParticionLru->lruHora) {
				punteroParticionLru = punteroParticion;
				index = i;
			}
		}
	}
	// Retorno el indice de la particion con el tiempo mas viejo
	return index;
}

void eliminar_particion_seleccionada(int index) {
	// Elimina una particion en el index especificado
	punteroParticion punteroParticionEliminar = list_get(particiones, index);

	// Setea la particion en desocupada
	punteroParticionEliminar->ocupada = false;

	t_cola_mensaje* cola = selecciono_cola(punteroParticionEliminar->colaMensaje);

	// Busca en la cola de mensajes el mensaje a eliminar y lo borra de ahi tambien
	for(int j = 0 ; j< list_size(cola->mensajes); j++) {
		puntero_mensaje punteroMensaje = list_get(cola->mensajes, j);

		if(punteroParticionEliminar->id == punteroMensaje->id) {

			sem_wait(&mutexDistribucion);
			envio_mensaje(punteroMensaje, punteroParticionEliminar->colaMensaje, cola);
			sem_post(&mutexDistribucion);

			free(list_get(cola->mensajes, j));
			list_remove(cola->mensajes, j);
			break;
		}
	}

	char* posInicioMemoriaR = string_itoa((char*)punteroParticionEliminar->punteroMemoria - (char*)punteroMemoriaPrincipal);
	char* idString = string_itoa(punteroParticionEliminar->id);
	log_info(loggerBroker,"Se elimina el mensaje de ID: %s y su posicion en memoria era: %s", idString, posInicioMemoriaR);
	free(posInicioMemoriaR);
	free(idString);

}

void intercambio_particiones(punteroParticion punteroParticionDesocupada,
		punteroParticion punteroParticionOcupada) {

	// Busca el mensaje en la memoria principal
	puntero_mensaje punteroMensaje = obtener_mensaje_memoria(punteroParticionOcupada);

	// Guarda el mensaje obtenido en la posicion de la particion desocupada
	guardar_mensaje_memoria(punteroMensaje, punteroParticionDesocupada->punteroMemoria, punteroParticionOcupada->colaMensaje);

	// Cambio a donde apunta la particion de memoria ocupada para que ahora
		//apunte a la posicion de la particion desocupada
	punteroParticionOcupada->punteroMemoria = punteroParticionDesocupada->punteroMemoria;

	// Cambio a donde apunta la particion desocupada para que ahora apunte a donde apuntaba mas el tamaño del mensaje recibido
	punteroParticionDesocupada->punteroMemoria = (char*)punteroParticionDesocupada->punteroMemoria
			+ punteroParticionOcupada->tamanoMensaje;

}

void consolidar(int indexEliminado) {
	// Empieza a consolidar a partir del index de particion eliminada
	punteroParticion punteroParticionEliminada;
	// El indice es nulo cuando se consolida sin eliminar
	if(indexEliminado == NULL) {
		// Busca el indice de la primer particion desocupada que encuentre
		punteroParticionEliminada = list_find(particiones, primer_puntero_desocupado);
		indexEliminado = obtener_index_particion(punteroParticionEliminada->punteroMemoria);
	} else {
		// Aca busca la particion especifica segun su index
		punteroParticionEliminada = list_get(particiones, indexEliminado);
	}

	// Analiza si existe alguna particion tal que la suma de su posicion de memoria
		//mas el tamano que ocupa, da la posicion de la particion a eliminar.
			// En otras palabras, busca una particion a izquierda
	bool encuentro_particion_anterior(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return (char*)particion->punteroMemoria + particion->tamanoMensaje
				== punteroParticionEliminada->punteroMemoria;
	}
	// COMPRUEBO SI TIENE ALGUNA PARTICION A IZQUIERA DESOCUPADA PARA UNIRLA
	punteroParticion particionAnterior = list_find(particiones, (void*)encuentro_particion_anterior);
	// Pregunta si encuentra y esta desocupada
	if(particionAnterior != NULL && !particionAnterior->ocupada) {
		// En caso de encontrarla y de que este desocupada
		// Le aumento el tamaño por la cantidad que ocupa la particion eliminada
		particionAnterior->tamanoMensaje += punteroParticionEliminada->tamanoMensaje;
		// Remuevo la particion ya que no me sirve mas
		list_remove(particiones, indexEliminado);
		// Repito proceso pero con el indice de la particion de la izquierda
		consolidar(guard(obtener_index_particion(particionAnterior->punteroMemoria), "No se encontro memoria anterior\n"));
	} else {
		// Si no encuentra particion libre a izquierda, busco a derecha

		// Analiza si existe una particion tal que su posicion de memoria sea igual
			// a la suma de la posicion de la particion a eliminar mas su tamano.
				// En otras palabras, busco una particion a derecha
		bool encuentro_particion_posterior(void* elemento) {
			punteroParticion particion = (punteroParticion*)elemento;
			return particion->punteroMemoria
					== (char*)punteroParticionEliminada->punteroMemoria + punteroParticionEliminada->tamanoMensaje;
		}
		// Busco la particion a derecha
		punteroParticion particionPosterior = list_find(particiones, (void*)encuentro_particion_posterior);
		// Si encuentro la particion a derecha y esta libre
		if(particionPosterior != NULL && !particionPosterior->ocupada) {
			// Aumento el tamano de la particion de la derecha por el tamano del mensaje a eliminar
			particionPosterior->tamanoMensaje += punteroParticionEliminada->tamanoMensaje;
			// Hago que ahora la particion de la derecha apunte a donde apuntaba la que se elimina
			particionPosterior->punteroMemoria = punteroParticionEliminada->punteroMemoria;
			// Saco de la lista la particion
			list_remove(particiones, indexEliminado);
			// Repito proceso desde la posicion de la particion derecha
			consolidar(guard(obtener_index_particion(particionPosterior->punteroMemoria), "No se encontro memoria posterior\n"));
		}
	}
}

void enviar_mensajes_memoria(puntero_suscripcion_cola mensajeSuscripcion, int socket) {
	// Envia los mensajes que estan en memoria a los distintos suscriptores

	// Busca las particiones que pertenecen a la cola de mensajes a la que se suscribieron
	bool misma_cola(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return particion->colaMensaje == mensajeSuscripcion->cola;
	}
	t_list* particionesCola = list_filter(particiones, misma_cola);

	// Recorro la lista de particiones filtradas para enviar los mensajes
	for(int i = 0; i < list_size(particionesCola); i++ ) {
		punteroParticion punteroParticionMensaje = list_get(particionesCola, i);
		// Busco que este ocupada la particion
		if(punteroParticionMensaje->ocupada) {
			// Busco un cliente que coincida con el que se acaba de suscribir
			bool encuentra_suscriptor(void* elemento) {
				char* sus = (char*) elemento;
				return strcmp(sus, mensajeSuscripcion->cliente) == 0;
			}
			// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
			bool encontre = list_any_satisfy(punteroParticionMensaje->suscriptores_ack, (void*)encuentra_suscriptor);
			// SI NO ESTA EN LA LISTA DE LOS ACK, LE ENVIO EL MENSAJE
			if (!encontre) {
				// Busca el mensaje en memoria
				puntero_mensaje punteroMensaje = obtener_mensaje_memoria(punteroParticionMensaje);
				puntero_suscriptor suscriptor = malloc(sizeof(t_suscriptor));
				suscriptor->cliente = mensajeSuscripcion->cliente;
				suscriptor->socket = socket;

				// Distribuye el mensaje
				distribuir_mensaje_sin_enviar_a(suscriptor, mensajeSuscripcion->cola, punteroMensaje, punteroParticionMensaje);

				// Actualiza el tiempo de LRU de la particion
				actualizar_lru_mensaje(punteroParticionMensaje->id);

				// MIRO SI ESTA EN LA LISTA DE LOS ENVIADOS,
				bool enviado = list_any_satisfy(punteroParticionMensaje->suscriptores_enviados, (void*)encuentra_suscriptor);
				// SI NO ESTA, LO AGREGO
				if(!enviado) {
					list_add(punteroParticionMensaje->suscriptores_enviados, mensajeSuscripcion->cliente);
				}
				free(suscriptor);
			}
		}
	}
	list_destroy(particionesCola);
}

void actualizar_lru_mensaje(uint32_t idMensaje) {
	// Busca por id la particion del mensaje
	bool encuentra_mensaje_con_id(void* elemento) {
		punteroParticion particion = (punteroParticion) elemento;
		return particion->id == idMensaje;
	}
	punteroParticion particionEncontrada = list_find(particiones, encuentra_mensaje_con_id);
	// Actualiza el LRU de la particion
	particionEncontrada->lruHora = obtener_milisegundos();
}

puntero_mensaje obtener_mensaje_memoria(punteroParticion particion) {
	// Busca el mensaje en memoria segun la posicion de la particion dada
	switch(particion->colaMensaje) {
		case NEW_POKEMON: {
			return obtener_mensaje_new_memoria(particion->punteroMemoria);
		}
		case APPEARED_POKEMON: {
			return obtener_mensaje_appeared_memoria(particion->punteroMemoria);
		}
		case GET_POKEMON: {
			return obtener_mensaje_get_memoria(particion->punteroMemoria);
		}
		case LOCALIZED_POKEMON: {
			return obtener_mensaje_localized_memoria(particion->punteroMemoria);
		}
		case CAUGHT_POKEMON: {
			return obtener_mensaje_caught_memoria(particion->punteroMemoria);
		}
		case CATCH_POKEMON: {
			return obtener_mensaje_catch_memoria(particion->punteroMemoria);
		}
		case -1: exit(-1);
	}
}

void guardar_mensaje_memoria(t_mensaje* mensajeCompleto, void* posMemoria, uint32_t colaMensaje) {
	// Guarda el mensaje en memoria en la posicion especificada
	uint32_t id = mensajeCompleto->id;
	uint32_t idCorrelativo = mensajeCompleto->id_correlativo;

	switch(colaMensaje) {
		case NEW_POKEMON: {
			puntero_mensaje_new_pokemon punteroMensajeNew = (puntero_mensaje_new_pokemon) mensajeCompleto->mensaje_cuerpo;
			guardar_mensaje_new_memoria(posMemoria, punteroMensajeNew->name_pokemon, punteroMensajeNew->pos_x, punteroMensajeNew->pos_y,
					punteroMensajeNew->quant_pokemon);
			break;
		}
		case GET_POKEMON: {
			puntero_mensaje_get_pokemon punteroMensajeGet = (puntero_mensaje_get_pokemon) mensajeCompleto->mensaje_cuerpo;
			guardar_mensaje_get_memoria(posMemoria, punteroMensajeGet->name_pokemon);
			break;
		}
		case LOCALIZED_POKEMON: {
			puntero_mensaje_localized_pokemon punteroMensajeLocalized = (puntero_mensaje_localized_pokemon) mensajeCompleto->mensaje_cuerpo;
			guardar_mensaje_localized_memoria(posMemoria, punteroMensajeLocalized->name_pokemon, punteroMensajeLocalized->quant_pokemon,
					punteroMensajeLocalized->coords);
			break;
		}
		case APPEARED_POKEMON: {
			puntero_mensaje_appeared_pokemon punteroMensajeAppeared = (puntero_mensaje_appeared_pokemon) mensajeCompleto->mensaje_cuerpo;
			guardar_mensaje_appeared_memoria(posMemoria, punteroMensajeAppeared->name_pokemon, punteroMensajeAppeared->pos_x, punteroMensajeAppeared->pos_y);
			break;
		}
		case CAUGHT_POKEMON: {
			puntero_mensaje_caught_pokemon punteroMensajeCaught = (puntero_mensaje_caught_pokemon) mensajeCompleto->mensaje_cuerpo;
			char* caughtPokemon = punteroMensajeCaught->caughtResult == 0 ? "OK" : "FAIL";

			guardar_mensaje_caught_memoria(posMemoria, caughtPokemon);
			break;
		}
		case CATCH_POKEMON: {
			puntero_mensaje_catch_pokemon punteroMensajeCatch = (puntero_mensaje_catch_pokemon) mensajeCompleto->mensaje_cuerpo;
			guardar_mensaje_catch_memoria(posMemoria, punteroMensajeCatch->name_pokemon, punteroMensajeCatch->pos_x, punteroMensajeCatch->pos_y);
			break;
		}
		case -1: exit(-1);
		default: exit(-1);
	}

	//log_info(loggerBroker,"Se ingresa un mensaje en la memoria");
}

int obtener_index_particion(int* punteroMemoria) {
	// Obtiene el indice de una particion segun la posicion dada, si no lo encuentra, retorna -1
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion puntero = list_get(particiones, i);
		if(puntero->punteroMemoria == punteroMemoria) {
			return i;
		}
	}
	return -1;
}

void asignar_memoria_bs(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	// Valida que el mensaje a asignar no supere la memoria total del broker
	if(mensajeCompleto->size_mensaje_cuerpo <= tamanoMemoria) {

		// Busca memoria libre segun algoritmo de asignacion
		void* posMemoria = bs_segun_algoritmo(mensajeCompleto, (uint32_t) colaMensaje);

		// En caso de no encontrar una posicion libre
		while(posMemoria == NULL) {
			// Primero se debe eliminar segun algoritmo y despues consolidar
			bs_eliminar_particion();
			bs_consolidar();
			// Busca nuevamente
			posMemoria = bs_segun_algoritmo(mensajeCompleto, colaMensaje);
		}

		// Guarda el mensaje en memoria
		guardar_mensaje_memoria(mensajeCompleto, posMemoria, (uint32_t) colaMensaje);
		char* posInicioParticionMemoria = string_itoa((char*)posMemoria - (char*)punteroMemoriaPrincipal);
		log_info(loggerBroker, "Se ingresa un mensaje con ID: %d con posicion relativa: %s", mensajeCompleto->id, posInicioParticionMemoria);
		free(posInicioParticionMemoria);
	} else {
		// El mensaje excede el tamaño disponible de memoria
		guard(-1, "Mensaje excede el limite permitido.");
	}
}

void* bs_segun_algoritmo(t_mensaje* mensajeCompleto, uint32_t colaMensaje){
	// Elije el algoritmo de ubicacion segun el config
	if(strcmp(algoritmoParticionLibre, "FF") == 0) {
		// Selecciona First Fit
		return bs_first_fit(mensajeCompleto, colaMensaje);
	} else if (strcmp(algoritmoParticionLibre, "BF") == 0) {
		// Selecciona Best Fit
		return bs_best_fit(mensajeCompleto, colaMensaje);
	}
}

void* bs_first_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje){

	// Obtiene la potencia de 2 mas cercana al tamaño del mensaje
	int tamanioNecesario = potencia_de_dos_cercana(mensajeCompleto->size_mensaje_cuerpo);

	// Valida que no sea menor que el minimo de particion
	if(tamanioNecesario < tamanoMinimoParticion){
		tamanioNecesario = tamanoMinimoParticion;
	}

	// Recorre la lista de particiones
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionObtenido = list_get(particiones, i);
		if(!punteroParticionObtenido->ocupada){
			if(punteroParticionObtenido->tamanoMensaje > tamanioNecesario){
				// Modifico la particion original para que tenga la mitad de tamaño
					// y despues hago una nueva con la otra mitad que faltaba
					// Ej: si tengo una particion con tamaño de 32MB lo
						//divido en 2 de 16MB etc hasta que mi mensaje pueda asignarse
				dividir_particiones(punteroParticionObtenido,i,tamanioNecesario);
				return bs_segun_algoritmo(mensajeCompleto, colaMensaje);
			}else{
				if(punteroParticionObtenido->tamanoMensaje == tamanioNecesario){
					// Si el tamano de la particion encontrada es igual al tamano del mensaje,
						//entonces guarda los datos del mensaje en la particion
					punteroParticionObtenido->colaMensaje = colaMensaje;
					punteroParticionObtenido->id = mensajeCompleto->id;
					punteroParticionObtenido->idCorrelativo = mensajeCompleto->id_correlativo;
					punteroParticionObtenido->ocupada = true;
					//punteroParticionObtenido->tamanoMensaje = mensajeCompleto->size_mensaje_cuerpo;
					punteroParticionObtenido->tamanoMensaje = tamanioNecesario;
					punteroParticionObtenido->lruHora = obtener_milisegundos();
					list_clean(punteroParticionObtenido->suscriptores_ack);
					list_clean(punteroParticionObtenido->suscriptores_enviados);

					// Retorna la posicion de memoria donde guarda el mensaje
					return punteroParticionObtenido->punteroMemoria;
				}
			}
		}
	}
	// Retorna null si no encuentra posicion libre
	return NULL;
}

void* bs_best_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje){
	// Utiliza Best Fit

	// Obtiene la potencia de 2 mas cercana al tamaño del mensaje
	int tamanioNecesario = potencia_de_dos_cercana(mensajeCompleto->size_mensaje_cuerpo);

	// Compruebo que no sea menor al tamano minimo de particion
	if(tamanioNecesario < tamanoMinimoParticion){
		tamanioNecesario = tamanoMinimoParticion;
	}

	int nuevoTamanioNecesario = tamanioNecesario;

	// Analizo toda la lista para buscar la particion
		// mas conveniente para guardar el mensaje
	bool primer_puntero_desocupado(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return !particion->ocupada;
	}

	punteroParticion particionMasChica = NULL;
	int indexParticionMasChica = -1;

	// Si la lista no esta llena
	if(!lista_llena(particiones)){
		bool entraMensaje = false;
		do{
			// Busco la particion mas chica y obtengo su id
			for(int i = 0; i< list_size(particiones);i++){
				punteroParticion particionObtenida = list_get(particiones,i);
				if(!particionObtenida->ocupada){
					if(particionObtenida->tamanoMensaje == nuevoTamanioNecesario){
						particionMasChica = particionObtenida;
						indexParticionMasChica = i;
						entraMensaje = true;
						break;
					}
				}
			}

			// Si no encontre particion mas chica de ese tamano
			if(!entraMensaje){
				if(nuevoTamanioNecesario < tamanoMemoria) {
					// Busco el tamaño inmediatamente superior
					nuevoTamanioNecesario *= 2;
				} else {
					entraMensaje = true;
				}
			}
		}while(!entraMensaje);
	}

	// Se fija si encuentro la particion mas chica donde entre el mensaje
	if(particionMasChica != NULL){
		if(particionMasChica->tamanoMensaje > tamanioNecesario){
			// Modifico la particion original para que tenga la mitad de tamaño
				// y despues hago una nueva con la otra mitad que faltaba
			// Ej: si tengo una particion con tamaño de 32MB lo divido en 2 de 16MB etc hasta
				//que mi mensaje pueda asignarse
			dividir_particiones(particionMasChica,indexParticionMasChica,tamanioNecesario);
			return bs_segun_algoritmo(mensajeCompleto, colaMensaje);
		}else{
			if(particionMasChica->tamanoMensaje == tamanioNecesario){
				// Si son del mismo tamano, ocupa la particion
				particionMasChica->colaMensaje = colaMensaje;
				particionMasChica->id = mensajeCompleto->id;
				particionMasChica->idCorrelativo = mensajeCompleto->id_correlativo;
				particionMasChica->ocupada = true;
				//particionMasChica->tamanoMensaje = mensajeCompleto->size_mensaje_cuerpo;
				particionMasChica->tamanoMensaje = tamanioNecesario;
				particionMasChica->lruHora = obtener_milisegundos();
				list_clean(particionMasChica->suscriptores_ack);
				list_clean(particionMasChica->suscriptores_enviados);

				// Retorna la posicion de memoria de la particion mas chica hallada
				return particionMasChica->punteroMemoria;
			}
		}
	}
	// Si no encuentra libre, retorna null
	return NULL;
}

int potencia_de_dos_cercana(uint32_t tamanioMensaje){
	// Retorna la potencia de dos mas cercana al tamano del mensaje
	// 2 elevado a 1
	int tamanioMininoNecesario = 2;
	while(tamanioMininoNecesario < tamanioMensaje){
		// Multiplica por 2 hasta llegar a un numero mayor o igual al que necesita
		tamanioMininoNecesario *= 2;
	}
	return tamanioMininoNecesario;
}

void dividir_particiones(punteroParticion particionInicial,int index ,uint32_t tamanioNecesario){
	// Divide una particion segun el tamano requerido

	// Se fija si tiene que seguir dividiendo o no
	if(particionInicial->tamanoMensaje == tamanioNecesario){
		// No hace falta realizar más divisiones
		return;
	} else {
		/* "Creo dos particiones nuevas", ambas con el tamaño de la original
		 * dividido 2.
		*/
		// particion izquierda (Es la original pero con tamaño a la mitad)
		particionInicial->tamanoMensaje = particionInicial->tamanoMensaje / 2;
		particionInicial->izq = true;
		particionInicial->der = false;

		particionInicial->lruHora = obtener_milisegundos();

		// particion derecha
		punteroParticion nuevaParticion = malloc(sizeof(t_particion));
		nuevaParticion->id = NULL;
		nuevaParticion->idCorrelativo = NULL;
		nuevaParticion->colaMensaje = NULL;
		nuevaParticion->ocupada = false;
		nuevaParticion->izq = false;
		nuevaParticion->der = true;
		nuevaParticion->lruHora = obtener_milisegundos();
		nuevaParticion->historicoBuddy = list_duplicate(particionInicial->historicoBuddy);
		list_add(nuevaParticion->historicoBuddy,"D");

		list_add(particionInicial->historicoBuddy, "I");

		nuevaParticion->punteroMemoria = (char*)particionInicial->punteroMemoria
				+ (particionInicial->tamanoMensaje);
		nuevaParticion->tamanoMensaje = particionInicial->tamanoMensaje;
		nuevaParticion->suscriptores_ack = list_create();
		nuevaParticion->suscriptores_enviados = list_create();
		list_add_in_index(particiones,index + 1, nuevaParticion);

		//ver_estado_memoria();

		// vuelvo a dividir la particion izquierda
		dividir_particiones(particionInicial, index ,tamanioNecesario);
	}
}

void bs_consolidar(){
	// Consolidacion
	// Se repite la consolidacion hasta que no haya mas cambios
	int cambios;
	do{
		cambios = 0;
		for(int i = 0; i < list_size(particiones); i++) {
			int indexBuddyIzq = i;
			int indexBuddyDer = i+1;
			punteroParticion buddyIzq = list_get(particiones, indexBuddyIzq);
			punteroParticion buddyDer = NULL;
			if(indexBuddyDer <= list_size(particiones)){
				buddyDer = list_get(particiones, indexBuddyDer);
			}

			uint32_t tamanioBuddyIzq = buddyIzq->tamanoMensaje;
			uint32_t tamanioBuddyDer = buddyDer->tamanoMensaje;

			if(!buddyIzq->ocupada){
				// Chequeo si las dos particiones que tomo son buddys
				if(buddyDer != NULL){
					if((buddyIzq->izq == true) && (buddyIzq->izq == buddyDer->der) && (!buddyDer->ocupada) && (tamanioBuddyIzq == tamanioBuddyDer)){

						char* buddyIzqString = string_itoa((char*)buddyIzq->punteroMemoria - (char*)punteroMemoriaPrincipal);
						char* buddyDerString = string_itoa((char*)buddyDer->punteroMemoria - (char*)punteroMemoriaPrincipal);
						log_info(loggerBroker, "Consolidacion de buddys, el buddy Izq con posicion: %s y el buddy Der con posicion: %s", buddyIzqString, buddyDerString);
						free(buddyIzqString);
						free(buddyDerString);
						// "UNIFICO" los buddys
						buddyIzq->tamanoMensaje += buddyDer->tamanoMensaje;
						buddyIzq->lruHora = obtener_milisegundos();
						buddyIzq->id = NULL;
						buddyIzq->colaMensaje = NULL;
						// analizo si el buddy que consolidé
						// es un buddy derecho o izquierdo

						// EJEMPLO:
						// historicoBuddyIzq = {"C","I","I","I"}
						// tamHistoricoBI = 4
						// list_get(historicoBuddyIzq,tamHistoricoBI - 2) = "I"
						// En este ejemplo, el buddy actual es el Izquierdo y cuando se consolide
						// con el derecho, pasa a ser un buddy izquierdo con el doble de tamaño

						t_list* historicoBuddyIzq = buddyIzq->historicoBuddy;
						int tamHistoricoBI = list_size(historicoBuddyIzq);
						// cuando se consolida, me fijo el tipo que era el buddy "padre"
						if(strcmp(list_get(historicoBuddyIzq,tamHistoricoBI -2), "D") == 0){
							// en este caso es un buddy derecho
							buddyIzq->izq = false;
							buddyIzq->der = true;
						}else{
							if(strcmp(list_get(historicoBuddyIzq,tamHistoricoBI -2), "I") == 0){
								// en este caso es un buddy izquierdo
								buddyIzq->izq = true;
								buddyIzq->der = false;
							}
						}

						list_remove(buddyIzq->historicoBuddy, tamHistoricoBI -1); // elimino el estado actual y vuelvo a uno anterior
						list_remove(particiones, indexBuddyDer); // elimino el buddy derecho de la lista
						cambios++;
						//ver_estado_memoria();
						break; // detiene el for para que no se buguee la lista
					}// fin if
				}
			}
		} // fin for
	}while(cambios != 0);
}

void bs_eliminar_particion(){
	// Eliminar particion segun algoritmo
	if(strcmp(algoritmoReemplazo, "FIFO") == 0) {
		// Utiliza FIFO
		bs_eliminar_particion_fifo();
	} else if (strcmp(algoritmoReemplazo, "LRU") == 0) {
		// Utiliza LRU
		bs_eliminar_particion_lru();
	}
}

void bs_eliminar_particion_fifo(){
	// Utiliza algoritmo FIFO para eliminar una particion
	punteroParticion punteroParticionMenorId;

	// Busca la primer particion ocupada y la toma como la de menor ID
	bool primer_puntero_ocupado(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return particion->ocupada;
	}
	punteroParticionMenorId = list_find(particiones, (void*)primer_puntero_ocupado);

	int index = -1;
	// BUSCO PARTICION CON MENOR ID => MENSAJE MAS VIEJO EN MEMORIA
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionEncontrado = list_get(particiones, i);
		if(punteroParticionMenorId->id >= punteroParticionEncontrado->id && punteroParticionEncontrado->ocupada) {
			punteroParticionMenorId = punteroParticionEncontrado;
			index = i;
		}
	}

	// Encuentra una particion para eliminar
	if(index != -1) {
		// Desocupa la particion
		punteroParticionMenorId->ocupada = false;

		// Busca el mensaje en la cola de mensaje y lo elimina
		t_cola_mensaje* cola = selecciono_cola(punteroParticionMenorId->colaMensaje);
		for(int j = 0 ; j< list_size(cola->mensajes); j++) {
			bool mensaje_con_id(void* elemento) {
				puntero_mensaje mensaje = (puntero_mensaje*)elemento;
				return mensaje->id == punteroParticionMenorId->id;
			}
			puntero_mensaje punteroMensaje = list_find(cola->mensajes, (void*)mensaje_con_id);
			if(punteroMensaje != NULL) {
				sem_wait(&mutexDistribucion);
				envio_mensaje(punteroMensaje, punteroParticionMenorId->colaMensaje, cola);
				sem_post(&mutexDistribucion);

				free(list_get(cola->mensajes, j));
				list_remove(cola->mensajes, j);
			}
		}
		// Vacia la particion
		char* posInicioMemoriaR = string_itoa((char*)punteroParticionMenorId->punteroMemoria - (char*)punteroMemoriaPrincipal);
		char* idString = string_itoa(punteroParticionMenorId->id);
		log_info(loggerBroker, "Se elimina el mensaje de ID: %s y su posicion relativa era: %s", idString, posInicioMemoriaR);
		free(posInicioMemoriaR);
		free(idString);
		punteroParticionMenorId->id = NULL;
		punteroParticionMenorId->colaMensaje = NULL;
	}
}

int bs_primer_puntero_desocupado(){
	// Retorna la posicion de la primera particion desocupada
	punteroParticion primerDesocupado = list_get(particiones, list_size(particiones) - 1);
	int indexPrimerDesocupado = list_size(particiones) - 1;
	for(int i = 0; i < list_size(particiones); i++){
		punteroParticion particionObtenida = list_get(particiones, i);
		if(!particionObtenida->ocupada){
			if(indexPrimerDesocupado > i){
				primerDesocupado = particionObtenida;
				indexPrimerDesocupado = i;
			}
		}
	}
	return indexPrimerDesocupado;
}

void bs_eliminar_particion_lru(){
	// Eliminar particion por LRU
	int index = -1;
	// Busca la primer particion desocupada
	punteroParticion punteroParticionLru = list_find(particiones, (void*)primer_puntero_ocupado);
	// BUSCO EL PUNTERO CON EL TIEMPO DE USO MAS LEJANO
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticion = list_get(particiones, i);
		if(punteroParticion->ocupada) {
			//printf("seconds: %f\n",seconds);
			if(punteroParticion->lruHora <= punteroParticionLru->lruHora) {
				punteroParticionLru = punteroParticion;
				index = i;
			}
		}
	}

	// Verifica si encontro la particion mas vieja
	if(index != -1){
		// Desocupa la particion que encontro
		punteroParticionLru->ocupada = false;

		// Borra el mensaje de la cola de mensajes
		t_cola_mensaje* cola = selecciono_cola(punteroParticionLru->colaMensaje);
		for(int j = 0 ; j< list_size(cola->mensajes); j++) {
			bool mensaje_con_id(void* elemento) {
				puntero_mensaje mensaje = (puntero_mensaje*)elemento;
				return mensaje->id == punteroParticionLru->id;
			}
			puntero_mensaje punteroMensaje = list_find(cola->mensajes, (void*)mensaje_con_id);
			if(punteroMensaje != NULL) {
				sem_wait(&mutexDistribucion);
				envio_mensaje(punteroMensaje, punteroParticionLru->colaMensaje, cola);
				sem_post(&mutexDistribucion);

				free(list_get(cola->mensajes, j));
				list_remove(cola->mensajes, j);
			}
		}
		// Vacia la data de la particion
		char* posInicioMemoriaR = string_itoa((char*)punteroParticionLru->punteroMemoria - (char*)punteroMemoriaPrincipal);
		char* idString = string_itoa(punteroParticionLru->id);
		log_info(loggerBroker, "Se elimina el mensaje de ID: %s y su posicion relativa era: %s", idString, posInicioMemoriaR);
		free(posInicioMemoriaR);
		free(idString);

		punteroParticionLru->id = NULL;
		punteroParticionLru->colaMensaje = NULL;
	}
}

bool lista_llena(t_list* particiones){
	// Retorna si la lista de particiones tiene todas sus particiones ocupadas o no
	int cantidadDeOcupados = 0;
	for(int i=0; i< list_size(particiones);i++){
		punteroParticion particion = list_get(particiones,i);
		if(particion->ocupada){
			cantidadDeOcupados++;
		}
	}
	return(cantidadDeOcupados == list_size(particiones));
}

bool primer_puntero_ocupado(void* elemento) {
	punteroParticion particion = (punteroParticion*)elemento;
	return particion->ocupada;
}

bool primer_puntero_desocupado(void* elemento) {
	punteroParticion particion = (punteroParticion*)elemento;
	return !particion->ocupada;
}

void desocupar_particion(void* elemento) {
	punteroParticion particion = (punteroParticion*)elemento;
	particion->ocupada = false;
}

void vaciar_memoria() {
	// Desocupa todas las particiones
	list_iterate(particiones, desocupar_particion);
}

/*char* hora_actual() {
	time_t ahora;
	char* ahora_string;

	ahora = guard(time(NULL), "Failed to obtain current date");

	ahora_string = ctime(&ahora);

	if(ahora_string == NULL) {
		exit(-1);
	}
	return ahora_string;
}*/

uint32_t calcular_tamano(char* memoriaActual, char* memoriaNueva) {
	uint32_t valor = memoriaActual - memoriaNueva;
	return  valor > tamanoMinimoParticion ? valor : tamanoMinimoParticion;
}

bool ordernar_particiones_memoria(void* puntero1, void* puntero2) {
	punteroParticion particion1 = (punteroParticion) puntero1;
	punteroParticion particion2 = (punteroParticion) puntero2;
	return particion1->punteroMemoria < particion2->punteroMemoria;
}

void ver_estado_memoria() {
	// Imprime por pantalla el estado de todas las particiones
	for(int j = 0; j < list_size(particiones); j++) {
		punteroParticion asd = list_get(particiones, j);
		printf("Puntero %p. Estado %d\n", asd->punteroMemoria, asd->ocupada);
		printf("Tamano de mensaje %d\n", asd->tamanoMensaje);
	}
}

uint32_t obtener_milisegundos() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint32_t a1 = (int64_t)(tv.tv_sec) * 1000;
	uint32_t a2 = (tv.tv_usec);
	return a1 + a2;
}

void guardar_estado_memoria(FILE* file) {
	// Funcion para acomodar los datos de las particiones para mostrar en el dump de cache
	t_list* listaParticiones = list_sorted(particiones, ordernar_particiones_memoria);
	for(int j = 0; j < list_size(listaParticiones); j++) {
		punteroParticion particion = list_get(listaParticiones, j);

		char* ocupada = particion->ocupada ? "X" : "L";
		char* cola;
		switch(particion->colaMensaje) {
			case NEW_POKEMON: cola = "NEW_POKEMON"; break;
			case APPEARED_POKEMON: cola = "APPEARED_POKEMON"; break;
			case GET_POKEMON: cola = "GET_POKEMON"; break;
			case LOCALIZED_POKEMON: cola = "LOCALIZED_POKEMON"; break;
			case CATCH_POKEMON: cola = "CATCH_POKEMON"; break;
			case CAUGHT_POKEMON: cola = "CAUGHT_POKEMON"; break;
			default: cola = "NO_ASIGNADA"; break;
		}
		fprintf(file, string_from_format("Particion %d: %p - %p.	[%s]	Size: %db	LRU: %d	Cola: %s	ID: %d\n", j, particion->punteroMemoria,
				(char*)particion->punteroMemoria + particion->tamanoMensaje - 1, ocupada, particion->tamanoMensaje, particion->lruHora, cola, particion->id));
	}
}

void manejo_dump_cache(int num) {
	// Realiza el dump de cache en el archivo dump.log
	log_info(loggerBroker, "Se solicito la ejecucion del dump de la cache");

	FILE* dump = fopen("../dump.log", "a");
	char fecha[50];
	time_t hoy;
	struct tm* timeinfo;
	time(&hoy);
	timeinfo = localtime(&hoy);
	strftime(fecha, 50, "%d/%m/%Y %T", timeinfo);

	char* infoHeader = malloc(500);
	infoHeader = "Dump: ";

	fprintf(dump, "----------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf(dump, infoHeader);
	fprintf(dump, fecha);
	fprintf(dump, "\n");

	guardar_estado_memoria(dump);

	fprintf(dump, "----------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

	fclose(dump);
}

void manejo_end() {
	list_destroy(new_pokemon->suscriptores);
	list_destroy(new_pokemon->mensajes);
	free(new_pokemon);
	list_destroy(get_pokemon->suscriptores);
	list_destroy(get_pokemon->mensajes);
	free(get_pokemon);
	list_destroy(localized_pokemon->suscriptores);
	list_destroy(localized_pokemon->mensajes);
	free(localized_pokemon);
	list_destroy(appeared_pokemon->suscriptores);
	list_destroy(appeared_pokemon->mensajes);
	free(appeared_pokemon);
	list_destroy(catch_pokemon->suscriptores);
	list_destroy(catch_pokemon->mensajes);
	free(catch_pokemon);
	list_destroy(caught_pokemon->suscriptores);
	list_destroy(caught_pokemon->mensajes);
	free(caught_pokemon);

	void liberar_particion(void* part) {
		punteroParticion particion = (punteroParticion) part;
		list_destroy(particion->suscriptores_enviados);
		list_destroy(particion->suscriptores_ack);
		//list_destroy(particion->historicoBuddy);
		free(particion);
	}
	list_destroy_and_destroy_elements(particiones, liberar_particion);
	free(punteroMemoriaPrincipal);
	exit(0);
}
