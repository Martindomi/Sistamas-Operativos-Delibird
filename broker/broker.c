#include <conexiones.h>
#include "broker.h"

int main(int argc, char *argv[]){

	const int lista_colas[6] = {NEW_POKEMON, APPEARED_POKEMON, CATCH_POKEMON, CAUGHT_POKEMON, GET_POKEMON, LOCALIZED_POKEMON };

	leer_archivo_config();

	inicializar_datos();

	creacion_hilos_distribucion(lista_colas);

	iniciar_servidor();

	return EXIT_SUCCESS;

}

void iniciar_servidor()
{
	int socket_servidor = -1;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ipBroker, puertoBroker, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        guard(socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol), "Failed to create socket");

        guard(bind(socket_servidor, p->ai_addr, p->ai_addrlen), "Bind failed");

        break;
    }

	guard(listen(socket_servidor, SOMAXCONN), "Listen failed");

    freeaddrinfo(servinfo);

	esperar_cliente(socket_servidor);
}

void esperar_cliente(int socket_servidor) {
	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

    while(1){
		int socket_cliente = guard(accept(socket_servidor, (void*) &dir_cliente, &tam_direccion), "Accept failed");

		pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
		pthread_detach(thread);
    }
}

void serve_client(int* socket)
{
	op_code cod_op = NO_ASIGNADA;
	log_info(loggerBroker,"Se conecto un proceso al broker");

	int sad = recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);
	if( sad == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int socket) {
	uint32_t size;
	t_mensaje* mensaje_completo = malloc(sizeof(t_mensaje));
	switch (cod_op) {
		case MESSAGE: {
			void* msg;
			msg = server_recibir_mensaje(socket, &size);
			msg = "1";
			size = sizeof(4);
			devolver_mensaje(msg, size, socket);
			//log_info(loggerBroker, "MESSAGE");
			//log_info(loggerBroker, msg);
			free(msg);
			break;
		}
		case NEW_POKEMON: {

			mensaje_completo = recibir_new_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			log_info(loggerBroker, "Llega un mensaje a la cola de NEW_POKEMON, con nombre %s (ID: %d)", ((puntero_mensaje_new_pokemon)mensaje_completo->mensaje_cuerpo)->name_pokemon, mensaje_completo->id);

			asignar_memoria(mensaje_completo, cod_op);

			list_add(new_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[NEW_POKEMON]);
			break;
		}
		case APPEARED_POKEMON: {

			mensaje_completo = recibir_appeared_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, appeared_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				log_info(loggerBroker,"Llega un mensaje a la cola de APPEARED_POKEMON, con nombre %s (ID: %d)", ((puntero_mensaje_appeared_pokemon)mensaje_completo->mensaje_cuerpo)->name_pokemon, mensaje_completo->id);

				asignar_memoria(mensaje_completo, cod_op);

				list_add(appeared_pokemon->mensajes, mensaje_completo);
			} else {
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			sem_post(&mutexLista[APPEARED_POKEMON]);
			break;
		}
		case CATCH_POKEMON: {

			mensaje_completo = recibir_catch_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			log_info(loggerBroker,"Llega un mensaje a la cola de CATCH_POKEMON, con nombre %s (ID: %d)", ((puntero_mensaje_catch_pokemon)mensaje_completo->mensaje_cuerpo)->name_pokemon, mensaje_completo->id);

			asignar_memoria(mensaje_completo, cod_op);

			list_add(catch_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[CATCH_POKEMON]);
			break;
		}
		case CAUGHT_POKEMON: {

			mensaje_completo = recibir_caught_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, caught_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				log_info(loggerBroker,"Llega un mensaje a la cola de CAUGHT_POKEMON, con resultado %d (ID: %d)", ((puntero_mensaje_caught_pokemon)mensaje_completo->mensaje_cuerpo)->caughtResult, mensaje_completo->id);

				asignar_memoria(mensaje_completo, cod_op);

				list_add(caught_pokemon->mensajes, mensaje_completo);
			} else {
				char* mensaje_ya_respondido = "MENSAJE YA RESPONDIDO";
				devolver_mensaje(mensaje_ya_respondido, strlen(mensaje_ya_respondido) + 1, socket);
			}

			sem_post(&mutexLista[CAUGHT_POKEMON]);
			break;
		}
		case GET_POKEMON: {

			mensaje_completo = recibir_get_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			log_info(loggerBroker,"Llega un mensaje a la cola de GET_POKEMON, con nombre %s (ID: %d)", ((puntero_mensaje_get_pokemon)mensaje_completo->mensaje_cuerpo)->name_pokemon, mensaje_completo->id);

			asignar_memoria(mensaje_completo, cod_op);

			list_add(get_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[GET_POKEMON]);
			break;
		}
		case LOCALIZED_POKEMON: {

			mensaje_completo = recibir_localized_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, localized_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

				log_info(loggerBroker,"Llega un mensaje a la cola de LOCALIZED_POKEMON, con nombre %s (ID: %d)", ((puntero_mensaje_localized_pokemon)mensaje_completo->mensaje_cuerpo)->name_pokemon, mensaje_completo->id);

				asignar_memoria(mensaje_completo, cod_op);

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

			mensaje_suscripcion = recibir_suscripcion(socket, &size, loggerBroker);

			//printf("Recibe una suscripcion\n");
			log_info(loggerBroker, "Proceso suscripto: %s a la cola %s", mensaje_suscripcion->cliente, obtener_nombre_cola(mensaje_suscripcion->cola));

			agregar_suscriptor_cola(mensaje_suscripcion, socket);

			char* suscripcion_aceptada = "SUSCRIPCION COMPLETADA";
			devolver_mensaje(suscripcion_aceptada, strlen(suscripcion_aceptada) + 1, socket);

			sem_wait(&mutexAsignarMemoria);
			enviar_mensajes_memoria(mensaje_suscripcion, socket);
			sem_post(&mutexAsignarMemoria);

			//sem_post(&mutexLista[mensaje_suscripcion->cola]);

			if(mensaje_suscripcion->tiempo == -1) {
				free(mensaje_suscripcion);
			}
			break;
		}
		case NO_ASIGNADA:
			return;
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

void agregar_suscriptor_cola(puntero_suscripcion_cola mensaje_suscripcion, int socket){
	t_cola_mensaje* cola_mensaje = selecciono_cola(mensaje_suscripcion->cola);

	puntero_suscriptor suscriptor = malloc(sizeof(t_suscriptor));
	suscriptor->cliente = malloc(mensaje_suscripcion->tamanoCliente);
	suscriptor->cliente = mensaje_suscripcion->cliente;
	suscriptor->socket = socket;

	list_add(cola_mensaje->suscriptores, suscriptor);
	pthread_t threadSuscripcion;
	if(mensaje_suscripcion->tiempo != -1) {
		pthread_create(&threadSuscripcion,NULL,(void*)desuscribir_cliente, mensaje_suscripcion);
		pthread_detach(threadSuscripcion);
	}
}

void desuscribir_cliente(puntero_suscripcion_cola mensaje) {
	//printf("Entro al hilo\n");
	sleep(mensaje->tiempo);
	t_cola_mensaje* cola_mensaje = selecciono_cola(mensaje->cola);
	bool encontrar_suscriptor(void* elemento) {
		puntero_suscriptor suscriptor = (puntero_suscriptor) elemento;
		//printf("%s y %s\n", suscriptor->cliente, mensaje->cliente);
		return strcmp(suscriptor->cliente, mensaje->cliente) == 0;
	}
	void free_suscriptor(void* elemento) {
		puntero_suscriptor suscriptor = (puntero_suscriptor) elemento;
		free(suscriptor->cliente);
		free(suscriptor);
	}
	list_remove_and_destroy_by_condition(cola_mensaje->suscriptores, encontrar_suscriptor, free_suscriptor);
	free(mensaje);
	//printf("Remueve cliente\n");
}

void* distribuir_mensajes(void* puntero_cola) {
	while(1) {
		// ENVIA MENSAJES A SUSCRIPTORES
		int cola = *((int*) puntero_cola);
		//sleep(15);

		sem_wait(&mutexLista[cola]);
		sem_wait(&mutexDistribucion);

		distribuir_mensajes_cola(cola);

		sem_post(&mutexDistribucion);
	}
}

void distribuir_mensajes_cola(int cola) {
	puntero_mensaje puntero_mensaje;
	t_cola_mensaje* cola_mensajes = selecciono_cola(cola);
	// TODO Mejorar manejo de error
	if (cola_mensajes == -1) {
		pthread_exit(NULL);
	}
	//printf("COLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
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
		//ver_estado_memoria();
		punteroParticion punteroParticionMensaje = buscar_particion_mensaje(puntero_mensaje->id);
		//printf("Puntero buscado %d\n", punteroParticionMensaje->id);
		bool encuentra_suscriptor(void* elemento) {
			return strcmp((char*)elemento, suscriptor->cliente) == 0;
		}
		if(punteroParticionMensaje != NULL) {
			//printf("Entro al distinto de null\n");
			// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
			bool encontre = list_any_satisfy(punteroParticionMensaje->suscriptores_ack, (void*)encuentra_suscriptor);
			// SI NO ESTA EN LA LISTA DE LOS ACK, LE ENVIO EL MENSAJE
			bool enviado = list_any_satisfy(punteroParticionMensaje->suscriptores_enviados, (void*)encuentra_suscriptor);

			if(!encontre && !enviado) {
				distribuir_mensaje_sin_enviar_a(suscriptor, cola, puntero_mensaje, NULL);

				list_add(punteroParticionMensaje->suscriptores_enviados, suscriptor->cliente);
			} else if (!encontre && enviado){
				sem_post(&mutexLista[cola]);
			}
		}
	}
}

punteroParticion buscar_particion_mensaje(uint32_t idMensaje) {
	bool obtener_particion_id(void* elemento) {
		punteroParticion punteroParticionElemento = (punteroParticion) elemento;
		return punteroParticionElemento->id == idMensaje;
	}
	// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
	return list_find(particiones, (void*)obtener_particion_id);
}

void distribuir_mensaje_sin_enviar_a(puntero_suscriptor suscriptor, int cola, puntero_mensaje puntero_mensaje_completo, punteroParticion punteroParticion) {
	int conexion;
	uint32_t id;
	uint32_t id_correlativo;

	conexion = suscriptor->socket;
	if(punteroParticion == NULL) {
		id = puntero_mensaje_completo->id;
		id_correlativo = puntero_mensaje_completo->id_correlativo;
	} else {
		id = punteroParticion->id;
		id_correlativo = punteroParticion->idCorrelativo;
	}
	log_info(loggerBroker,"Envio de mensaje con ID: %d a %s", id, suscriptor->cliente);

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
			//printf("pokemon------------> %s\n", puntero_mensaje->name_pokemon);
			char* nombre = puntero_mensaje->name_pokemon;
			//printf("No envia mensaje\n");
			send_message_get_pokemon(nombre, id, id_correlativo, conexion);
			//printf("Envia mensaje\n");
			break;
		}
		case APPEARED_POKEMON: {
			puntero_mensaje_appeared_pokemon puntero_mensaje = ((puntero_mensaje_appeared_pokemon*)puntero_mensaje_completo->mensaje_cuerpo);

			char* nombre = puntero_mensaje->name_pokemon;
			uint32_t posx = puntero_mensaje->pos_x;
			uint32_t posy = puntero_mensaje->pos_y;
			/*printf("APPEARED conex %d\n", conexion);
			printf("APPEARED id %d\n", id);
			printf("APPEARED idCorre %d\n", id_correlativo);
			printf("APPEARED posx %d\n", posx);
			printf("APPEARED posy %d\n", posy);
			printf("APPEARED nombre %s\n", nombre);*/
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
	pthread_t threadACK;
	puntero_ack punteroAck = malloc(sizeof(t_ack));
	punteroAck->conexion = conexion;
	punteroAck->idMensaje = id;
	punteroAck->suscriptor = suscriptor->cliente;
	pthread_create(&threadACK, NULL, esperar_mensaje_ack, punteroAck);
	pthread_detach(threadACK);

	//close(conexion);
}

void esperar_mensaje_ack(puntero_ack punteroAck) {
	char* mensaje_recibido;

	mensaje_recibido = client_recibir_mensaje(punteroAck->conexion);
	//printf("RECIBE %s\n", mensaje_recibido);
	log_info(loggerBroker, "ACK Recibido con ID: %d del proceso %s", punteroAck->idMensaje, punteroAck->suscriptor);
	if(strcmp(mensaje_recibido, "ACK") == 0) {
		punteroParticion punteroParticionEncontrado = buscar_particion_mensaje(punteroAck->idMensaje);
		if(punteroParticionEncontrado != NULL) {
			list_add(punteroParticionEncontrado->suscriptores_ack, punteroAck->suscriptor);
		}
	}

	free(mensaje_recibido);
	free(punteroAck);
}

void inicializar_datos() {
	//printf("Process id %d\n", getpid());

	signal(SIGUSR1, manejo_dump_cache);
	signal(SIGINT, manejo_end);

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
	//Semaforo asignacion de memoria
	sem_init(&mutexAsignarMemoria, 0 ,1);

	cantidad_mensajes = 1;

	punteroMemoriaPrincipal = malloc(tamanoMemoria);
	//printf("Direccion de memoria inicial: %p\n", punteroMemoriaPrincipal);
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
	//printf("Direccion de memoria final: %p\n", punteroMemoriaFinal);
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
		pthread_detach(thread_pool[j]);

    }
}

void asignar_y_devolver_id(t_mensaje* mensaje_completo, int socket) {
	sem_wait(&mutexIds);

	mensaje_completo->id = cantidad_mensajes;
	char* id_mensaje = string_itoa(cantidad_mensajes);
	devolver_mensaje(id_mensaje, strlen(id_mensaje) + 1, socket);
	free(id_mensaje);
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

}

void leer_archivo_config() {
	configBroker = guard_lectura_string_config(config_create("../broker.config"));

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
	sem_wait(&mutexAsignarMemoria);
	if(strcmp(algoritmoMemoria, "PARTICIONES") == 0) {
		//printf("Particiones\n");
		asignar_memoria_pd(mensajeCompleto, colaMensaje);
	} else if(strcmp(algoritmoMemoria, "BS") == 0) {
		//printf("Buddy System\n");
		asignar_memoria_bs(mensajeCompleto, colaMensaje);
	}
	sem_post(&mutexAsignarMemoria);
}

void asignar_memoria_pd(t_mensaje* mensajeCompleto, int colaMensaje) {
	if(mensajeCompleto->size_mensaje_cuerpo <= tamanoMemoria) {
		//printf("Asignar Memoria\n");

		void* posMemoria = pd_memoria_libre(mensajeCompleto, (uint32_t) colaMensaje);

		// SI NO ENCUENTRA UNA QUE CUMPLA CON LO ANTERIOR
		while(posMemoria == NULL) {
			cantidadBusquedasFallidas ++;
			//printf("Cantidad de busquedas fallidas %d de %d\n", cantidadBusquedasFallidas, frecuenciaCompactacion);
			if(frecuenciaCompactacion == 0) {
				compactar_memoria();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else if(frecuenciaCompactacion == -1) {
				// TODO revisar si es necesario usar el algoritmo de reemplazo hasta q todas las particiones queden vacias
				vaciar_memoria();
				consolidar(NULL);
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else if(frecuenciaCompactacion == 1) {
				eliminar_particion();
				compactar_memoria();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else if(cantidadBusquedasFallidas % frecuenciaCompactacion == 0) {
				compactar_memoria();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			} else {
				eliminar_particion();
				posMemoria = pd_memoria_libre(mensajeCompleto, colaMensaje);
			}
		}

		//printf("Encontro memoria\n");

		guardar_mensaje_memoria(mensajeCompleto, posMemoria, (uint32_t) colaMensaje);

		log_info(loggerBroker,"Se guarda un mensaje en memoria en posicion %d con ID: %d", (char*)posMemoria - (char*)punteroMemoriaPrincipal , mensajeCompleto->id);

		//printf("Asigno memoria!!!!!\n");
	} else {
		guard(-1, "Mensaje excede el limite permitido.");
	}
}

void* pd_memoria_libre(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	//printf("la lista esta llena %d\n", lista_llena(particiones));

	if(!lista_llena(particiones)){
		if(strcmp(algoritmoParticionLibre, "FF") == 0) {
			return buscar_memoria_libre_first_fit(mensajeCompleto, colaMensaje);
		} else if (strcmp(algoritmoParticionLibre, "BF") == 0) {
			return buscar_memoria_libre_best_fit(mensajeCompleto, colaMensaje);
		}
	} else {
		return NULL;
	}
}

void* buscar_memoria_libre_first_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	//printf("First fit\n");
	// RECORRE LAS PARTICIONES EN BUSCA DE UNA LIBRE Y DONDE ENTRE EL MENSAJE
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionObtenido = list_get(particiones, i);
		if(!punteroParticionObtenido->ocupada){
			if(punteroParticionObtenido->tamanoMensaje
					> calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0)) {
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
				nuevaParticion->historicoBuddy = list_create();
				list_add(particiones, nuevaParticion);
				//printf("Puntero particion nueva: %p\n", nuevaParticion->punteroMemoria);
				//printf("Posicion particion nueva: %d\n", (char*)nuevaParticion->punteroMemoria - (char*)punteroMemoriaPrincipal);
				//printf("Memoria libre restante: %d\n", nuevaParticion->tamanoMensaje);
			}
			if(punteroParticionObtenido->tamanoMensaje >= mensajeCompleto->size_mensaje_cuerpo) {
				punteroParticionObtenido->colaMensaje = colaMensaje;
				punteroParticionObtenido->id = mensajeCompleto->id;
				punteroParticionObtenido->idCorrelativo = mensajeCompleto->id_correlativo;
				punteroParticionObtenido->ocupada = true;
				punteroParticionObtenido->tamanoMensaje = calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0);
				punteroParticionObtenido->lruHora = obtener_milisegundos();
				list_clean(punteroParticionObtenido->suscriptores_ack);
				list_clean(punteroParticionObtenido->suscriptores_enviados);

				//printf("Puntero Mensaje: %p\n", punteroParticionObtenido->punteroMemoria);
				//printf("Posicion Mensaje: %d\n", (char*)punteroParticionObtenido->punteroMemoria - (char*)punteroMemoriaPrincipal);
				//printf("Tamanio Mensaje: %d\n", punteroParticionObtenido->tamanoMensaje);
				//ver_estado_memoria();
				return punteroParticionObtenido->punteroMemoria;
			}
		}
	}
	//printf("No encontro memoria libre\n");
	return NULL;
}

void* buscar_memoria_libre_best_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	//printf("Best fit\n");

	punteroParticion punteroMejorParticion = list_find(particiones, primer_puntero_desocupado);
	// SI ENCUENTRO NINGUNO DESOCUPADO
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

		if(punteroMejorParticion->tamanoMensaje
							> calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0)) {
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
			nuevaParticion->historicoBuddy = list_create();
			list_add(particiones, nuevaParticion);
			//printf("Puntero Mensaje: %p\n", nuevaParticion->punteroMemoria);
			//printf("Posicion particion nueva: %d\n", (char*)nuevaParticion->punteroMemoria - (char*)punteroMemoriaPrincipal);
			//printf("Memoria libre restante: %d\n", nuevaParticion->tamanoMensaje);
		}
		if(punteroMejorParticion->tamanoMensaje >= mensajeCompleto->size_mensaje_cuerpo) {
			punteroMejorParticion->colaMensaje = colaMensaje;
			punteroMejorParticion->id = mensajeCompleto->id;
			punteroMejorParticion->idCorrelativo = mensajeCompleto->id_correlativo;
			punteroMejorParticion->ocupada = true;
			punteroMejorParticion->tamanoMensaje = calcular_tamano((char*)mensajeCompleto->size_mensaje_cuerpo, 0);
			punteroMejorParticion->lruHora = obtener_milisegundos();
			list_clean(punteroMejorParticion->suscriptores_ack);
			list_clean(punteroMejorParticion->suscriptores_enviados);

			//printf("Puntero Mensaje: %p\n", punteroMejorParticion->punteroMemoria);
			//printf("Posicion Mensaje: %d\n", (char*)punteroMejorParticion->punteroMemoria - (char*)punteroMemoriaPrincipal);
			//printf("Tamanio Mensaje: %d\n", punteroMejorParticion->tamanoMensaje);
			return punteroMejorParticion->punteroMemoria;
		}
	}

	//printf("No encontro memoria libre\n");
	return NULL;
}

void compactar_memoria() {
	//printf("Compacta memoria\n");
	punteroParticion punteroParticionDesocupada = NULL;
	punteroParticion punteroParticionOcupada = NULL;

	list_sort(particiones, ordernar_particiones_memoria);

	//ver_estado_memoria();

	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionRecorrer = list_get(particiones, i);
		if(punteroParticionDesocupada == NULL){
			if(!punteroParticionRecorrer->ocupada) {
				//printf("Encuentra particion desocupada\n");
				punteroParticionDesocupada = list_get(particiones, i);
			}
		} else {
			if(punteroParticionRecorrer->ocupada) {
				//printf("Encuentra particion ocupada\n");
				punteroParticionOcupada = list_get(particiones, i);
				break;
			}
		}
	}

	if(punteroParticionDesocupada != NULL && punteroParticionOcupada != NULL) {
		//printf("Encontro ambas particiones para intercambiar\n");
		intercambio_particiones(punteroParticionDesocupada, punteroParticionOcupada);
		log_info(loggerBroker,"Compactacion de particiones ...");
		compactar_memoria();
	} else {
		//printf("No encontro particiones para compactar\n");
		consolidar(NULL);
		return;
	}

}

void eliminar_particion() {
	//printf("Eliminar particion\n");
	int indexEliminar;
	if(strcmp(algoritmoReemplazo, "FIFO") == 0) {
		indexEliminar = eliminar_particion_fifo();
	} else if (strcmp(algoritmoReemplazo, "LRU") == 0) {
		indexEliminar = eliminar_particion_lru();
	}
	//printf("Indice a eliminar %d\n", indexEliminar);

	if(indexEliminar != -1) {
		eliminar_particion_seleccionada(indexEliminar);
		consolidar(indexEliminar);
	} else {
		if(list_size(particiones) > 0) {
			consolidar(0);
		}
	}
}

int eliminar_particion_fifo() {
	punteroParticion punteroParticionMenorId;

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

	return index;
}

int eliminar_particion_lru() {
	int index = -1;
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
	return index;
}

void eliminar_particion_seleccionada(int index) {
	//printf("Encuentra una particion para eliminar\n");
	punteroParticion punteroParticionEliminar = list_get(particiones, index);
	//printf("Particion eliminada %p con mensaje con id %d\n", punteroParticionEliminar->punteroMemoria, punteroParticionEliminar->id);
	punteroParticionEliminar->ocupada = false;

	t_cola_mensaje* cola = selecciono_cola(punteroParticionEliminar->colaMensaje);
	for(int j = 0 ; j< list_size(cola->mensajes); j++) {
		puntero_mensaje punteroMensaje = list_get(cola->mensajes, j);

		if(punteroParticionEliminar->id == punteroMensaje->id) {

			sem_wait(&mutexDistribucion);
			envio_mensaje(punteroMensaje, punteroParticionEliminar->colaMensaje, cola);

			log_info(loggerBroker,"Elimina mensaje con ID: %d en posicion %d", punteroMensaje->id, (char*)punteroParticionEliminar->punteroMemoria - (char*)punteroMemoriaPrincipal);

			puntero_mensaje mens = list_remove(cola->mensajes, j);
			liberar(punteroParticionEliminar->colaMensaje, mens);

			sem_post(&mutexDistribucion);

			break;
		}
	}

}

void liberar(int cola, puntero_mensaje mensaje) {
	switch(cola){
		case NEW_POKEMON:
			liberar_mensajes_new(mensaje);
			break;
		case APPEARED_POKEMON:
			liberar_mensajes_appeared(mensaje);
			break;
		case GET_POKEMON:
			liberar_mensajes_get(mensaje);
			break;
		case LOCALIZED_POKEMON:
			liberar_mensajes_localized(mensaje);
			break;
		case CAUGHT_POKEMON:
			liberar_mensajes_caught(mensaje);
			break;
		case CATCH_POKEMON:
			liberar_mensajes_catch(mensaje);
			break;
		default: return;
	}
}

void intercambio_particiones(punteroParticion punteroParticionDesocupada,
		punteroParticion punteroParticionOcupada) {

	/*puntero_mensaje punteroMensaje = obtener_mensaje_memoria(punteroParticionOcupada);
	guardar_mensaje_memoria(punteroMensaje, punteroParticionDesocupada->punteroMemoria, punteroParticionOcupada->colaMensaje);
	free(punteroMensaje->mensaje_cuerpo);
	free(punteroMensaje);*/
	memcpy(punteroParticionDesocupada->punteroMemoria, punteroParticionOcupada->punteroMemoria, punteroParticionOcupada->tamanoMensaje);

	punteroParticionOcupada->punteroMemoria = punteroParticionDesocupada->punteroMemoria;

	punteroParticionDesocupada->punteroMemoria = (char*)punteroParticionDesocupada->punteroMemoria
			+ punteroParticionOcupada->tamanoMensaje;

	//printf("Realiza el intercambio de las particiones\n");

}

void consolidar(int indexEliminado) {
	//printf("Entra consolidar\n");
	punteroParticion punteroParticionEliminada;
	if(indexEliminado == NULL) {
		punteroParticionEliminada = list_find(particiones, primer_puntero_desocupado);
		indexEliminado = obtener_index_particion(punteroParticionEliminada->punteroMemoria);
	} else {
		punteroParticionEliminada = list_get(particiones, indexEliminado);
	}
	bool encuentro_particion_anterior(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return (char*)particion->punteroMemoria + particion->tamanoMensaje
				== punteroParticionEliminada->punteroMemoria;
	}
	// COMPRUEBO SI TIENE ALGUNA PARTICION A IZQUIERA DESOCUPADA PARA UNIRLA
	punteroParticion particionAnterior = list_find(particiones, (void*)encuentro_particion_anterior);
	if(particionAnterior != NULL && !particionAnterior->ocupada) {
			//printf("Encontro una particion libre a izquierda\n");

			particionAnterior->tamanoMensaje += punteroParticionEliminada->tamanoMensaje;
			punteroParticion punteroEliminado = (punteroParticion) list_remove(particiones, indexEliminado);
			liberar_particion(punteroEliminado);
			consolidar(guard(obtener_index_particion(particionAnterior->punteroMemoria), "No se encontro memoria anterior\n"));
	} else {
		// SI NO TIENE A IZQUIERDA, BUSCO A DERECHA
		bool encuentro_particion_posterior(void* elemento) {
			punteroParticion particion = (punteroParticion*)elemento;
			return particion->punteroMemoria
					== (char*)punteroParticionEliminada->punteroMemoria + punteroParticionEliminada->tamanoMensaje;
		}
		//printf("Entro a derecha\n");
		punteroParticion particionPosterior = list_find(particiones, (void*)encuentro_particion_posterior);
		if(particionPosterior != NULL && !particionPosterior->ocupada) {
				//printf("Encontro una particion libre a derecha\n");

				particionPosterior->tamanoMensaje += punteroParticionEliminada->tamanoMensaje;
				particionPosterior->punteroMemoria = punteroParticionEliminada->punteroMemoria;
				punteroParticion punteroEliminado = (punteroParticion) list_remove(particiones, indexEliminado);
				liberar_particion(punteroEliminado);
				consolidar(guard(obtener_index_particion(particionPosterior->punteroMemoria), "No se encontro memoria posterior\n"));
		}
	}
	//printf("No hay mas particiones libres ni a izq ni a der\n");
}

void enviar_mensajes_memoria(puntero_suscripcion_cola mensajeSuscripcion, int socket) {

	bool misma_cola(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return particion->colaMensaje == mensajeSuscripcion->cola;
	}
	t_list* particionesCola = list_filter(particiones, misma_cola);
	//printf("MEMORIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
	for(int i = 0; i < list_size(particionesCola); i++ ) {
		punteroParticion punteroParticionMensaje = list_get(particionesCola, i);
		if(punteroParticionMensaje != NULL) {

			bool encuentra_suscriptor(void* elemento) {
				char* sus = (char*) elemento;
				return strcmp(sus, mensajeSuscripcion->cliente) == 0;
			}
			// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
			// SI NO ESTA EN LA LISTA DE LOS ACK, LE ENVIO EL MENSAJE
			//printf("SUSCRIPTOR MENSAJE AHORA %s\n", mensajeSuscripcion->cliente);
			//printf("ENCONTRO O NO %d\n", encontre);
			if(punteroParticionMensaje->ocupada) {
				//printf("Entro al distinto de null\n");
				// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
				bool encontre = list_any_satisfy(punteroParticionMensaje->suscriptores_ack, (void*)encuentra_suscriptor);
				// SI NO ESTA EN LA LISTA DE LOS ACK, LE ENVIO EL MENSAJE
				bool enviado = list_any_satisfy(punteroParticionMensaje->suscriptores_enviados, (void*)encuentra_suscriptor);

				if(!encontre && !enviado) {
					puntero_mensaje punteroMensaje = obtener_mensaje_memoria(punteroParticionMensaje);
					puntero_suscriptor suscriptor = malloc(sizeof(t_suscriptor));
					suscriptor->cliente = mensajeSuscripcion->cliente;
					suscriptor->socket = socket;
					distribuir_mensaje_sin_enviar_a(suscriptor, mensajeSuscripcion->cola, punteroMensaje, punteroParticionMensaje);
					actualizar_lru_mensaje(punteroParticionMensaje->id);

					list_add(punteroParticionMensaje->suscriptores_enviados, mensajeSuscripcion->cliente);
					free(suscriptor);
					liberar(punteroParticionMensaje->colaMensaje, punteroMensaje);
				} else if (!encontre && enviado){
					enviar_mensajes_memoria(mensajeSuscripcion, socket);
				}
			}
		}
	}
	list_destroy(particionesCola);
}

void actualizar_lru_mensaje(uint32_t idMensaje) {
	bool encuentra_mensaje_con_id(void* elemento) {
		punteroParticion particion = (punteroParticion) elemento;
		return particion->id == idMensaje;
	}
	punteroParticion particionEncontrada = list_find(particiones, encuentra_mensaje_con_id);
	particionEncontrada->lruHora = obtener_milisegundos();
}

puntero_mensaje obtener_mensaje_memoria(punteroParticion particion) {
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
}

int obtener_index_particion(int* punteroMemoria) {
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion puntero = list_get(particiones, i);
		if(puntero->punteroMemoria == punteroMemoria) {
			return i;
		}
	}
	return -1;
}

void asignar_memoria_bs(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	if(mensajeCompleto->size_mensaje_cuerpo <= tamanoMemoria) {
		//printf("Asignar Memoria\n");

		void* posMemoria = bs_segun_algoritmo(mensajeCompleto, (uint32_t) colaMensaje);

		while(posMemoria == NULL) {
			// Primero se debe eliminar segun algoritmo y despues consolidar
			bs_eliminar_particion();
			bs_consolidar();
			posMemoria = bs_segun_algoritmo(mensajeCompleto, colaMensaje); // intento asignar nuevamente
		}

		//printf("Encontro memoria\n");

		guardar_mensaje_memoria(mensajeCompleto, posMemoria, (uint32_t) colaMensaje);

		log_info(loggerBroker,"Se guarda un mensaje en memoria en posicion %d con ID: %d", (char*)posMemoria - (char*)punteroMemoriaPrincipal, mensajeCompleto->id);

		//printf("Asigno memoria!!!!!\n");
	} else {
		guard(-1, "Mensaje excede el limite permitido.");
	}
}

void* bs_segun_algoritmo(t_mensaje* mensajeCompleto, uint32_t colaMensaje){
	if(strcmp(algoritmoParticionLibre, "FF") == 0) {
		return bs_first_fit(mensajeCompleto, colaMensaje);
	} else if (strcmp(algoritmoParticionLibre, "BF") == 0) {
		return bs_best_fit(mensajeCompleto, colaMensaje);
	}
}

void* bs_first_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje){
	//printf("First fit\n");
	int tamanioNecesario = potencia_de_dos_cercana(mensajeCompleto->size_mensaje_cuerpo); // tengo que obtener la potencia de 2 mas cercana al tamaño del mensaje

	if(tamanioNecesario < tamanoMinimoParticion){
		tamanioNecesario = tamanoMinimoParticion;
	}

	//printf("potencia %d\n", tamanioNecesario);
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionObtenido = list_get(particiones, i);
		if(!punteroParticionObtenido->ocupada){
			if(punteroParticionObtenido->tamanoMensaje > tamanioNecesario){
				// Modifico la particion original para que tenga la mitad de tamaño
				// y despues hago una nueva con la otra mitad que faltaba
				dividir_particiones(punteroParticionObtenido,i,tamanioNecesario); // Ej: si tengo una particion con tamaño de 32MB lo divido en 2 de 16MB etc hasta que mi mensaje pueda asignarse
				return bs_segun_algoritmo(mensajeCompleto, colaMensaje);
			}else{
				if(punteroParticionObtenido->tamanoMensaje == tamanioNecesario){
					punteroParticionObtenido->colaMensaje = colaMensaje;
					punteroParticionObtenido->id = mensajeCompleto->id;
					punteroParticionObtenido->idCorrelativo = mensajeCompleto->id_correlativo;
					punteroParticionObtenido->ocupada = true;
					//punteroParticionObtenido->tamanoMensaje = mensajeCompleto->size_mensaje_cuerpo;
					punteroParticionObtenido->tamanoMensaje = tamanioNecesario;
					punteroParticionObtenido->lruHora = obtener_milisegundos();
					list_clean(punteroParticionObtenido->suscriptores_ack);
					list_clean(punteroParticionObtenido->suscriptores_enviados);

					//printf("Puntero Mensaje: %p\n", punteroParticionObtenido->punteroMemoria);
					//printf("Posicion Mensaje: %d\n", (char*)punteroParticionObtenido->punteroMemoria - (char*)punteroMemoriaPrincipal);
					//printf("Encontro memoria libre: %d\n", punteroParticionObtenido->tamanoMensaje);
					//ver_estado_memoria();
					return punteroParticionObtenido->punteroMemoria;
				}
			}
		}
	}
	//printf("No encontro memoria libre\n");
	return NULL;
}

void* bs_best_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje){
	//printf("Best fit\n");
	// tengo que obtener la potencia de 2 mas cercana al tamaño del mensaje
	int tamanioNecesario = potencia_de_dos_cercana(mensajeCompleto->size_mensaje_cuerpo);

	if(tamanioNecesario < tamanoMinimoParticion){
			tamanioNecesario = tamanoMinimoParticion;
	}

	int nuevoTamanioNecesario = tamanioNecesario;
	//printf("El tamaño necesario para el mensaje es: %d\n",tamanioNecesario);

	// Analizo toda la lista para buscar la particion
	// mas conveniente para guardar el mensaje

	bool primer_puntero_desocupado(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return !particion->ocupada;
	}

	punteroParticion particionMasChica = NULL;
	int indexParticionMasChica = -1;

	if(!lista_llena(particiones)){
		bool entraMensaje = false;
		do{
			for(int i = 0; i< list_size(particiones);i++){
				punteroParticion particionObtenida = list_get(particiones,i);
				if(!particionObtenida->ocupada){
					if(particionObtenida->tamanoMensaje == nuevoTamanioNecesario){
						particionMasChica = particionObtenida;
						indexParticionMasChica = i;
						entraMensaje = true;
						//printf("La particion mas conveniente es la de: %dB\n", particionMasChica->tamanoMensaje);
						//printf("Su posicion en la lista es: %d\n", indexParticionMasChica);
						break;
					}
				}
			}
			//printf("entra mensaje %d\n", entraMensaje);
			//printf("nuevo tamanio %d\n", nuevoTamanioNecesario);
			//printf("tamano memoria %d\n", tamanoMemoria);
			if(!entraMensaje){
				if(nuevoTamanioNecesario < tamanoMemoria) {
					//busco el tamaño inmediatamente superior
					nuevoTamanioNecesario *= 2;
					//printf("Como no encontré un mensaje de %d, pruebo con uno de %d\n",tamanioNecesario, nuevoTamanioNecesario);
				} else {
					entraMensaje = true;
				}
			}
			//printf("OLA");
		}while(!entraMensaje);
	}

	if(particionMasChica != NULL){
		if(particionMasChica->tamanoMensaje > tamanioNecesario){
			// Modifico la particion original para que tenga la mitad de tamaño
			// y despues hago una nueva con la otra mitad que faltaba
			dividir_particiones(particionMasChica,indexParticionMasChica,tamanioNecesario); // Ej: si tengo una particion con tamaño de 32MB lo divido en 2 de 16MB etc hasta que mi mensaje pueda asignarse
			return bs_segun_algoritmo(mensajeCompleto, colaMensaje);
		}else{
			if(particionMasChica->tamanoMensaje == tamanioNecesario){
				particionMasChica->colaMensaje = colaMensaje;
				particionMasChica->id = mensajeCompleto->id;
				particionMasChica->idCorrelativo = mensajeCompleto->id_correlativo;
				particionMasChica->ocupada = true;
				//particionMasChica->tamanoMensaje = mensajeCompleto->size_mensaje_cuerpo;
				particionMasChica->tamanoMensaje = tamanioNecesario;
				particionMasChica->lruHora = obtener_milisegundos();
				list_clean(particionMasChica->suscriptores_ack);
				list_clean(particionMasChica->suscriptores_enviados);

				//printf("Puntero Mensaje: %p\n", particionMasChica->punteroMemoria);
				//printf("Posicion Mensaje: %d\n", (char*)particionMasChica->punteroMemoria - (char*)punteroMemoriaPrincipal);
				//printf("Encontro memoria libre: %d\n", particionMasChica->tamanoMensaje);
				//ver_estado_memoria();
				return particionMasChica->punteroMemoria;
			}
		}
	}
	//printf("CHAU");
	return NULL;
}

int potencia_de_dos_cercana(uint32_t tamanioMensaje){
	int tamanioMininoNecesario = 2; // 2 elevado a 1
	while(tamanioMininoNecesario < tamanioMensaje){
		tamanioMininoNecesario *= 2; // multiplico por 2 hasta llegar a un numero mayor o igual al que necesito
	}
	return tamanioMininoNecesario;
}

void dividir_particiones(punteroParticion particionInicial,int index ,uint32_t tamanioNecesario){
	//printf("Dividir particiones\n");
	if(particionInicial->tamanoMensaje == tamanioNecesario){
		//printf("Deja de dividir\n");
		return;
		// no hace falta realizar más divisiones
	}else{

		/* "Creo dos particiones nuevas", ambas con el tamaño de la original
		 * dividido 2.
		*/
		//printf("Entra a dividir\n");
		//printf("La particion izquierda está en el indice %d\n", index);
		// particion izquierda (Es la original pero con tamaño a la mitad)
		particionInicial->tamanoMensaje = particionInicial->tamanoMensaje / 2;
		particionInicial->izq = true;
		particionInicial->der = false;
		particionInicial->lruHora = obtener_milisegundos();
		//printf("Crea particion izquierda\n");

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

		//printf("Crea particion derecha\n");


		nuevaParticion->punteroMemoria = (char*)particionInicial->punteroMemoria
				+ (particionInicial->tamanoMensaje);
		nuevaParticion->tamanoMensaje = particionInicial->tamanoMensaje;
		nuevaParticion->suscriptores_ack = list_create();
		nuevaParticion->suscriptores_enviados = list_create();
		list_add_in_index(particiones,index + 1, nuevaParticion);
		//printf("La particion derecha está en el indice %d\n", index + 1);
		//printf("Divide\n");
		//ver_estado_memoria();
		// vuelvo a dividir la particion izquierda
		dividir_particiones(particionInicial, index ,tamanioNecesario);
	}
}

void bs_consolidar(){
	//printf("Consolidar\n");
	// voy a repetir la consolidacion hasta que
	// no haya mas cambios
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

						//printf("Hay dos buddys de tamaño: %d\n", buddyIzq->tamanoMensaje);
						log_info(loggerBroker,"Consolida Buddys, Izquierdo en posicion %d y Derecho en posicion %d", (char*)buddyIzq->punteroMemoria - (char*)punteroMemoriaPrincipal, (char*)buddyDer->punteroMemoria - (char*)punteroMemoriaPrincipal);

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
						punteroParticion puntero = (punteroParticion) list_remove(particiones, indexBuddyDer); // elimino el buddy derecho de la lista
						liberar_particion(puntero);
						//printf("Ahora hay un buddy de tamaño: %d\n", buddyIzq->tamanoMensaje);
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
	//printf("Eliminar particion\n");
		//int indexEliminado;
		if(strcmp(algoritmoReemplazo, "FIFO") == 0) {
			// que es guard?
			bs_eliminar_particion_fifo();
		} else if (strcmp(algoritmoReemplazo, "LRU") == 0) {
			bs_eliminar_particion_lru();
		}
}

void bs_eliminar_particion_fifo(){

	punteroParticion punteroParticionMenorId;

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
	if(index != -1) {
		//printf("Encuentra una particion para eliminar\n");
		punteroParticionMenorId->ocupada = false;

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

				log_info(loggerBroker,"Elimina mensaje con ID: %d en posicion %d", punteroParticionMenorId->id, (char*)punteroParticionMenorId->punteroMemoria - (char*)punteroMemoriaPrincipal);

				puntero_mensaje mens = list_remove(cola->mensajes, j);
				liberar(punteroParticionMenorId->colaMensaje, mens);

				punteroParticionMenorId->id = NULL;
				punteroParticionMenorId->colaMensaje = NULL;
				sem_post(&mutexDistribucion);

			}
		}


		//printf("Elimina una particion de: %d de id: %d\n",punteroParticionMenorId->tamanoMensaje, punteroParticionMenorId->id);

	}
}

int bs_primer_puntero_desocupado(){
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
	//printf("Entra a LRU\n");
	int index = -1;
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

	//printf("Puntero seleccionado en la posición: %d\n",index);

	if(index != -1){
		//printf("Encuentra una particion para eliminar\n");
		punteroParticionLru->ocupada = false;

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

				log_info(loggerBroker,"Elimina mensaje con ID: %d en posicion %d", punteroParticionLru->id, (char*)punteroParticionLru->punteroMemoria - (char*)punteroMemoriaPrincipal);

				puntero_mensaje mens = list_remove(cola->mensajes, j);
				liberar(punteroParticionLru->colaMensaje, mens);

				punteroParticionLru->id = NULL;
				punteroParticionLru->colaMensaje = NULL;
				sem_post(&mutexDistribucion);

			}
		}

	}
	//printf("Sale de LRU\n");
}

bool lista_llena(t_list* particiones){
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
	list_iterate(particiones, desocupar_particion);
}

char* hora_actual() {
	time_t ahora;
	char* ahora_string;

	ahora = guard(time(NULL), "Failed to obtain current date");

	ahora_string = ctime(&ahora);

	if(ahora_string == NULL) {
		exit(-1);
	}
	return ahora_string;
}

uint32_t calcular_tamano(char* memoriaActual, char* memoriaNueva) {
	uint32_t valor = memoriaActual - memoriaNueva;
	return  valor > tamanoMinimoParticion ? valor : tamanoMinimoParticion;
}

uint32_t convertir_decimal(uint32_t decimal) {
	int resto, remainder;
	int i, j = 0;
	char hexadecimal[100];
	resto = decimal;
	while(resto != 0) {
		remainder = resto % 16;
		if(remainder < 10)
			hexadecimal[j++] = 48 + remainder;
		else hexadecimal[j++] = 55 + remainder;
		resto = resto / 16;
	}
	for(i = j; i >= 0; i--)
		printf("%c", hexadecimal[i]);
	return atoi(hexadecimal);
}

uint32_t convertir_hexadecimal_decimal(char* hexadecimal) {
	return atoi(hexadecimal);
}

bool ordernar_particiones_memoria(void* puntero1, void* puntero2) {
	punteroParticion particion1 = (punteroParticion) puntero1;
	punteroParticion particion2 = (punteroParticion) puntero2;
	return particion1->punteroMemoria < particion2->punteroMemoria;
}

void ver_estado_memoria() {
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
		char* linea = string_from_format("Particion %d: %p - %p.	[%s]	Size: %db	LRU: %d	Cola: %s	ID: %d\n", j, particion->punteroMemoria,
				(char*)particion->punteroMemoria + particion->tamanoMensaje - 1, ocupada, particion->tamanoMensaje, particion->lruHora, cola, particion->id);
		fprintf(file, linea);
		free(linea);
	}
	list_destroy(listaParticiones);
}

void manejo_dump_cache(int num) {
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
void liberar_mensajes_new(void* men) {
	t_mensaje* mensaje = (t_mensaje*) men;
	puntero_mensaje_new_pokemon newPokemon = (puntero_mensaje_new_pokemon) mensaje->mensaje_cuerpo;
	free(newPokemon->name_pokemon);
	free(newPokemon);
	free(mensaje);
}
void liberar_mensajes_get(void* men) {
	t_mensaje* mensaje = (t_mensaje*) men;
	puntero_mensaje_get_pokemon getPokemon = (puntero_mensaje_get_pokemon) mensaje->mensaje_cuerpo;
	free(getPokemon->name_pokemon);
	free(getPokemon);
	free(mensaje);
}
void liberar_mensajes_localized(void* men) {
	t_mensaje* mensaje = (t_mensaje*) men;
	puntero_mensaje_localized_pokemon localizedPokemon = (puntero_mensaje_localized_pokemon) mensaje->mensaje_cuerpo;
	free(localizedPokemon->name_pokemon);
	list_destroy(localizedPokemon->coords);
	free(localizedPokemon);
	free(mensaje);
}
void liberar_mensajes_appeared(void* men) {
	t_mensaje* mensaje = (t_mensaje*) men;
	puntero_mensaje_appeared_pokemon appearedPokemon = (puntero_mensaje_appeared_pokemon) mensaje->mensaje_cuerpo;
	free(appearedPokemon->name_pokemon);
	free(appearedPokemon);
	free(mensaje);
}
void liberar_mensajes_catch(void* men) {
	t_mensaje* mensaje = (t_mensaje*) men;
	puntero_mensaje_catch_pokemon catchPokemon = (puntero_mensaje_catch_pokemon) mensaje->mensaje_cuerpo;
	free(catchPokemon->name_pokemon);
	free(catchPokemon);
	free(mensaje);
}
void liberar_mensajes_caught(void* men) {
	t_mensaje* mensaje = (t_mensaje*) men;
	puntero_mensaje_caught_pokemon caughtPokemon = (puntero_mensaje_caught_pokemon) mensaje->mensaje_cuerpo;
	free(caughtPokemon);
	free(mensaje);
}
void liberar_particion(void* part) {
	punteroParticion particion = (punteroParticion) part;
	list_destroy(particion->suscriptores_enviados);
	list_destroy(particion->suscriptores_ack);
	list_destroy(particion->historicoBuddy);
	free(particion);
}
void manejo_end() {
	void liberar_suscriptores(void* sus) {
		puntero_suscriptor suscriptor = (puntero_suscriptor) sus;
		free(suscriptor->cliente);
		free(suscriptor);
	}
	//----------------NEW POKEMON ------------------------------

	list_destroy_and_destroy_elements(new_pokemon->suscriptores, liberar_suscriptores);
	list_destroy_and_destroy_elements(new_pokemon->mensajes, liberar_mensajes_new);
	free(new_pokemon);
	//----------------------------------------------------------
	//-------------GET POKEMON ----------------------------------

	list_destroy_and_destroy_elements(get_pokemon->suscriptores, liberar_suscriptores);
	list_destroy_and_destroy_elements(get_pokemon->mensajes, liberar_mensajes_get);
	free(get_pokemon);
	//-------------------------------------------------------------
	//---------------- LOCALIZED POKEMON ---------------------------

	list_destroy_and_destroy_elements(localized_pokemon->suscriptores, liberar_suscriptores);
	list_destroy_and_destroy_elements(localized_pokemon->mensajes, liberar_mensajes_localized);
	free(localized_pokemon);
	//-------------------------------------------------------------
	//----------------- APPEARED POKEMON --------------------------

	list_destroy_and_destroy_elements(appeared_pokemon->suscriptores, liberar_suscriptores);
	list_destroy_and_destroy_elements(appeared_pokemon->mensajes, liberar_mensajes_appeared);
	free(appeared_pokemon);
	//---------------------------------------------------------------
	//----------------- CATCH POKEMON -------------------------------

	list_destroy_and_destroy_elements(catch_pokemon->suscriptores, liberar_suscriptores);
	list_destroy_and_destroy_elements(catch_pokemon->mensajes, liberar_mensajes_catch);
	free(catch_pokemon);
	//----------------------------------------------------------------
	//--------------------CAUGHT POKEMON -----------------------------

	list_destroy_and_destroy_elements(caught_pokemon->suscriptores,liberar_suscriptores);
	list_destroy_and_destroy_elements(caught_pokemon->mensajes, liberar_mensajes_caught);
	free(caught_pokemon);
	//---------------------------------------------------------------------


	list_destroy_and_destroy_elements(particiones, liberar_particion);
	free(punteroMemoriaPrincipal);
	exit(0);
}

char* obtener_nombre_cola(int cola) {
	switch(cola) {
		case NEW_POKEMON: return "NEW_POKEMON";
		case GET_POKEMON: return "GET_POKEMON";
		case APPEARED_POKEMON: return "APPEARED_POKEMON";
		case LOCALIZED_POKEMON: return "LOCALIZED_POKEMON";
		case CAUGHT_POKEMON: return "CAUGHT_POKEMON";
		case CATCH_POKEMON: return "CATCH_POKEMON";
	}
	return "";
}
