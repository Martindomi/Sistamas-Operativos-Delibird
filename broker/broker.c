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
	t_mensaje* mensaje_completo = malloc(sizeof(t_mensaje));
	switch (cod_op) {
		case MESSAGE: {
			void* msg;
			msg = server_recibir_mensaje(socket, &size);
			msg = "1";
			size = sizeof(4);
			devolver_mensaje(msg, size, socket);
			log_info(loggerBroker, "MESSAGE");
			log_info(loggerBroker, msg);
			free(msg);
			break;
		}
		case NEW_POKEMON: {

			mensaje_completo = recibir_new_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			//asignar_memoria(mensaje_completo);

			list_add(new_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[NEW_POKEMON]);
			break;
		}
		case APPEARED_POKEMON: {

			mensaje_completo = recibir_appeared_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, appeared_pokemon)) {
				asignar_y_devolver_id(mensaje_completo, socket);

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

			list_add(catch_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[CATCH_POKEMON]);
			break;
		}
		case CAUGHT_POKEMON: {

			mensaje_completo = recibir_caught_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, caught_pokemon)) {
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

			mensaje_completo = recibir_get_pokemon(socket, &size);

			asignar_y_devolver_id(mensaje_completo, socket);

			list_add(get_pokemon->mensajes, mensaje_completo);

			sem_post(&mutexLista[GET_POKEMON]);
			break;
		}
		case LOCALIZED_POKEMON: {

			mensaje_completo = recibir_localized_pokemon(socket, &size);

			// SI NO ENCUENTRO EL ID CORRELATIVO EN LA COLA DE MENSAJES LO GUARDO, SINO LO IGNORO
			if(mensaje_completo->id_correlativo == 0 || !fue_respondido(mensaje_completo, localized_pokemon)) {
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

			mensaje_suscripcion = recibir_suscripcion(socket, &size, loggerBroker);

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

		sem_wait(&mutexLista[cola]);
		sem_wait(&mutexDistribucion);

		distribuir_mensajes_cola(cola);

		sem_post(&mutexDistribucion);
		sleep(10);
	}
}

void distribuir_mensajes_cola(int cola) {
	puntero_mensaje puntero_mensaje;
	t_cola_mensaje* cola_mensajes = selecciono_cola(cola);
	char* suscriptor;
	// TODO Mejorar manejo de error
	if (cola_mensajes == -1) {
		pthread_exit(NULL);
	}

	// RECORRO TODOS LOS MENSAJES DE LA COLA
	for(int i = 0; i < list_size(cola_mensajes->mensajes); i++) {
		puntero_mensaje = list_get(cola_mensajes->mensajes, i);

		// OBTENGO CADA SUSCRIPTOR DE UN MENSAJE
		for(int j = 0; j < list_size(cola_mensajes->suscriptores) ;j++) {
			suscriptor = list_get(cola_mensajes->suscriptores, j);

			punteroParticion punteroParticionMensaje = buscar_particion_mensaje(puntero_mensaje->id);

			bool encuentra_suscriptor(void* elemento) {
				return strcmp((char*)elemento, suscriptor) == 0;
			}
			// ME FIJO SI EL SUSCRIPTOR ESTA EN LA LISTA DE SUSCRIPTORES ACK DEL MENSAJE
			bool encontre = list_any_satisfy(punteroParticionMensaje->suscriptores_ack, (void*)encuentra_suscriptor);
			// SI NO ESTA EN LA LISTA DE LOS ACK, LE ENVIO EL MENSAJE
			if (!encontre) {
				printf("Distribucion a %s\n", suscriptor);
				distribuir_mensaje_sin_enviar_a(suscriptor, cola, puntero_mensaje);
				// MIRO SI ESTA EN LA LISTA DE LOS ENVIADOS,
				bool enviado = list_any_satisfy(punteroParticionMensaje->suscriptores_enviados, (void*)encuentra_suscriptor);
				// SI NO ESTA, LO AGREGO
				if(!enviado) {
					list_add(punteroParticionMensaje->suscriptores_enviados, suscriptor);
				}
				break;
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

void distribuir_mensaje_sin_enviar_a(char* suscriptor, int cola, puntero_mensaje puntero_mensaje_completo) {
	int conexion;
	char* ip_suscriptor;
	char* puerto_suscriptor;
	char* mensaje_recibido;
	char** aux;
	aux = string_split(suscriptor, ":");
	ip_suscriptor = aux[0];
	puerto_suscriptor = aux[1];

	printf("Suscriptor %s\n", suscriptor);
	conexion = crear_conexion(ip_suscriptor, puerto_suscriptor);
	uint32_t id = puntero_mensaje_completo->id;
	uint32_t id_correlativo = puntero_mensaje_completo->id_correlativo;
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
			printf("APPEARED conex %d\n", conexion);
			printf("APPEARED id %d\n", id);
			printf("APPEARED idCorre %d\n", id_correlativo);
			printf("APPEARED posx %d\n", posx);
			printf("APPEARED posy %d\n", posy);
			printf("APPEARED nombre %s\n", nombre);
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

			char* caughtPokemon = puntero_mensaje->caught_pokemon;

			send_message_caught_pokemon(caughtPokemon, id, id_correlativo, conexion);

			break;
		}

	}

	mensaje_recibido = client_recibir_mensaje(conexion);
	printf("RECIBE %s\n", mensaje_recibido);
	// TODO que hago si no recibo el ACK?
	if(strcmp(mensaje_recibido, "ACK") == 0) {
		punteroParticion punteroParticionEncontrado = buscar_particion_mensaje(puntero_mensaje_completo->id);
		list_add(punteroParticionEncontrado->suscriptores_ack, suscriptor);
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
		sem_init(&semaforo, 0, 0);
		mutexLista[i] = semaforo;
	}

	sem_init(&mutexDistribucion, 0, 1);
	sem_init(&mutexIds, 0, 1);

	cantidad_mensajes = 1;

	punteroMemoriaPrincipal = malloc(tamanoMemoria);
	printf("Direccion de memoria del puntero: %p\n", punteroMemoriaPrincipal);
	particiones = list_create();
	punteroParticion particionInicial = malloc(sizeof(t_particion));
	particionInicial->colaMensaje = NULL;
	particionInicial->id = NULL;
	particionInicial->idCorrelativo = NULL;
	particionInicial->ocupada = false;
	particionInicial->punteroMemoria = punteroMemoriaPrincipal;
	particionInicial->tamanoMensaje = tamanoMemoria;
	particionInicial->suscriptores_ack = list_create();
	particionInicial->suscriptores_enviados = list_create();
	list_add(particiones, particionInicial);
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

	loggerBroker =log_create(logFile, "BROKER", false, LOG_LEVEL_INFO);

}

void asignar_memoria(t_mensaje* mensajeCompleto, int colaMensaje) {
	printf("Asignar Memoria\n");
	void* posMemoria = buscar_memoria_libre(mensajeCompleto, (uint32_t) colaMensaje);

	// SI NO ENCUENTRA UNA QUE CUMPLA CON LO ANTERIOR
	while(posMemoria == NULL) {
		cantidadBusquedasFallidas ++;
		printf("Cantidad de busquedas fallidas %d de %d\n", cantidadBusquedasFallidas, frecuenciaCompactacion);
		if(frecuenciaCompactacion == 0) {
			compactar_memoria();
			posMemoria = buscar_memoria_libre(mensajeCompleto, colaMensaje);
		} else if(frecuenciaCompactacion == -1) {
			vaciar_memoria();
			compactar_memoria();
			posMemoria = buscar_memoria_libre(mensajeCompleto, colaMensaje);
		} else if(frecuenciaCompactacion == 1) {
			eliminar_particion();
			compactar_memoria();
			posMemoria = buscar_memoria_libre(mensajeCompleto, colaMensaje);
		} else if(cantidadBusquedasFallidas % frecuenciaCompactacion == 0) {
			compactar_memoria();
			posMemoria = buscar_memoria_libre(mensajeCompleto, colaMensaje);
		} else {
			eliminar_particion();
			posMemoria = buscar_memoria_libre(mensajeCompleto, colaMensaje);
		}
	}

	printf("Encontro memoria\n");

	memcpy(posMemoria, mensajeCompleto->mensaje_cuerpo, mensajeCompleto->size_mensaje_cuerpo);
	printf("Asigno memoria!!!!!\n");
}

void* buscar_memoria_libre(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	if(strcmp(algoritmoParticionLibre, "FF") == 0) {
		return buscar_memoria_libre_first_fit(mensajeCompleto, colaMensaje);
	} else if (strcmp(algoritmoParticionLibre, "BF") == 0) {
		return buscar_memoria_libre_best_fit(mensajeCompleto, colaMensaje);
	}
}

void* buscar_memoria_libre_first_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	printf("First fit\n");
	// RECORRE LAS PARTICIONES EN BUSCA DE UNA LIBRE Y DONDE ENTRE EL MENSAJE
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionObtenido = list_get(particiones, i);
		if(!punteroParticionObtenido->ocupada){
			if(punteroParticionObtenido->tamanoMensaje
					> (mensajeCompleto->size_mensaje_cuerpo)) {
				punteroParticion nuevaParticion = malloc(sizeof(t_particion));
				nuevaParticion->colaMensaje = NULL;
				nuevaParticion->id = NULL;
				nuevaParticion->idCorrelativo = NULL;
				nuevaParticion->ocupada = false;
				nuevaParticion->punteroMemoria = punteroParticionObtenido->punteroMemoria
						+ mensajeCompleto->size_mensaje_cuerpo + 1;
				nuevaParticion->tamanoMensaje = punteroParticionObtenido->tamanoMensaje
						- mensajeCompleto->size_mensaje_cuerpo;
				nuevaParticion->suscriptores_ack = list_create();
				nuevaParticion->suscriptores_enviados = list_create();
				list_add(particiones, nuevaParticion);
				printf("Posicion particion nueva: %p\n", nuevaParticion->punteroMemoria);
				printf("Memoria libre restante: %d\n", nuevaParticion->tamanoMensaje);
			}
			if(punteroParticionObtenido->tamanoMensaje >= mensajeCompleto->size_mensaje_cuerpo) {
				punteroParticionObtenido->colaMensaje = colaMensaje;
				punteroParticionObtenido->id = mensajeCompleto->id;
				punteroParticionObtenido->idCorrelativo = mensajeCompleto->id_correlativo;
				punteroParticionObtenido->ocupada = true;
				punteroParticionObtenido->tamanoMensaje = mensajeCompleto->size_mensaje_cuerpo;
				punteroParticionObtenido->lruHora = time(NULL);

				printf("Posicion Mensaje: %p\n", punteroParticionObtenido->punteroMemoria);
				printf("Tamanio Mensaje: %d\n", punteroParticionObtenido->tamanoMensaje);
				return punteroParticionObtenido->punteroMemoria;
			}
		}
	}
	printf("No encontro memoria libre\n");
	return NULL;
}

void* buscar_memoria_libre_best_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje) {
	printf("Best fit\n");

	punteroParticion punteroMejorParticion = list_find(particiones, primer_puntero_desocupado);
	// SI ENCUENTRO NINGUNO DESOCUPADO
	if(punteroMejorParticion != NULL) {
		// BUSCA LA PARTICION OPTIMA EN TAMAÑO PARA EL MENSAJE
		for(int i = 0; i < list_size(particiones); i++) {
			punteroParticion punteroParticionObtenido = list_get(particiones, i);
			// SI ESTA DESOCUPADA, EL TAMAÑO DEL MENSAJE ENTRA Y TIENE UN TAMAÑO MENOR AL DE LA MEJOR
			if(!punteroParticionObtenido->ocupada &&
					punteroParticionObtenido->tamanoMensaje >=
						mensajeCompleto->size_mensaje_cuerpo &&
							punteroParticionObtenido->tamanoMensaje <
								punteroMejorParticion->tamanoMensaje ){
				punteroMejorParticion = punteroParticionObtenido;
			}
		}

		if(punteroMejorParticion->tamanoMensaje
							> (mensajeCompleto->size_mensaje_cuerpo)) {
			punteroParticion nuevaParticion = malloc(sizeof(t_particion));
			nuevaParticion->colaMensaje = NULL;
			nuevaParticion->id = NULL;
			nuevaParticion->idCorrelativo = NULL;
			nuevaParticion->ocupada = false;
			nuevaParticion->punteroMemoria = punteroMejorParticion->punteroMemoria
					+ mensajeCompleto->size_mensaje_cuerpo + 1;
			nuevaParticion->tamanoMensaje = punteroMejorParticion->tamanoMensaje
					- mensajeCompleto->size_mensaje_cuerpo;
			nuevaParticion->suscriptores_ack = list_create();
			nuevaParticion->suscriptores_enviados = list_create();
			list_add(particiones, nuevaParticion);
			printf("Posicion particion nueva: %p\n", nuevaParticion->punteroMemoria);
			printf("Memoria libre restante: %d\n", nuevaParticion->tamanoMensaje);
		}
		if(punteroMejorParticion->tamanoMensaje >= mensajeCompleto->size_mensaje_cuerpo) {
			punteroMejorParticion->colaMensaje = colaMensaje;
			punteroMejorParticion->id = mensajeCompleto->id;
			punteroMejorParticion->idCorrelativo = mensajeCompleto->id_correlativo;
			punteroMejorParticion->ocupada = true;
			punteroMejorParticion->tamanoMensaje = mensajeCompleto->size_mensaje_cuerpo;
			punteroMejorParticion->lruHora = time(NULL);

			printf("Posicion Mensaje: %p\n", punteroMejorParticion->punteroMemoria);
			printf("Tamanio Mensaje: %d\n", punteroMejorParticion->tamanoMensaje);
			return punteroMejorParticion->punteroMemoria;
		}
	}

	printf("No encontro memoria libre\n");
	return NULL;
}

void compactar_memoria() {
	printf("Compacta memoria\n");
	punteroParticion punteroParticionDesocupada;
	punteroParticion punteroParticionOcupada;

	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticionRecorrer = list_get(particiones, i);
		if(punteroParticionDesocupada != NULL){
			if(!punteroParticionRecorrer->ocupada) {
				printf("Encuentra particion desocupada\n");
				punteroParticionDesocupada = list_get(particiones, i);
			}
		} else {
			if(punteroParticionRecorrer->ocupada) {
				printf("Encuentra particion ocupada\n");
				punteroParticionOcupada = list_get(particiones, i);
				break;
			}
		}
	}

	if(punteroParticionDesocupada != NULL && punteroParticionOcupada != NULL) {
		printf("Encontro ambas particiones para intercambiar\n");
		intercambio_particiones(punteroParticionDesocupada, punteroParticionOcupada);
		compactar_memoria();
	} else {
		printf("No encontro particiones para compactar\n");
		return;
	}

}

void eliminar_particion() {
	printf("Eliminar particion\n");
	int indexEliminar;
	if(strcmp(algoritmoReemplazo, "FIFO") == 0) {
		indexEliminar = guard(eliminar_particion_fifo(), "Problema al eliminar una particion\n");
	} else if (strcmp(algoritmoReemplazo, "LRU") == 0) {
		indexEliminar = guard(eliminar_particion_lru(), "Problema al eliminar una particion\n");
	}
	eliminar_particion_seleccionada(indexEliminar);
	consolidar(indexEliminar);
}

int eliminar_particion_fifo() {
	punteroParticion punteroParticionMenorId;

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

	return index;
}

int eliminar_particion_lru() {
	int index = -1;
	punteroParticion punteroParticionLru = list_get(particiones, 0);
	// BUSCO EL PUNTERO CON EL TIEMPO DE USO MAS LEJANO
	for(int i = 0; i < list_size(particiones); i++) {
		punteroParticion punteroParticion = list_get(particiones, i);
		if(punteroParticion->ocupada) {
			double seconds = difftime(punteroParticion->lruHora, punteroParticionLru->lruHora);
			if(seconds <= 0) {
				punteroParticionLru = punteroParticion;
				index = i;
			}
		}
	}
	return index;
}

void eliminar_particion_seleccionada(int index) {
	printf("Encuentra una particion para eliminar\n");
	punteroParticion punteroParticionEliminar = list_get(particiones, index);
	punteroParticionEliminar->ocupada = false;

	t_cola_mensaje* cola = selecciono_cola(punteroParticionEliminar->colaMensaje);
	for(int j = 0 ; j< list_size(cola->mensajes); j++) {
		puntero_mensaje punteroMensaje = list_get(cola->mensajes, j);

		if(punteroParticionEliminar->id == punteroMensaje->id) {
			free(list_get(cola->mensajes, j));
			list_remove(cola->mensajes, j);
			break;
		}
	}
}

void intercambio_particiones(punteroParticion punteroParticionDesocupada,
		punteroParticion punteroParticionOcupada) {
	memcpy(punteroParticionDesocupada->punteroMemoria,
			punteroParticionOcupada->punteroMemoria, punteroParticionOcupada->tamanoMensaje);

	punteroParticionOcupada->punteroMemoria = punteroParticionDesocupada->punteroMemoria;

	punteroParticionDesocupada->punteroMemoria = punteroParticionDesocupada->punteroMemoria
			+ punteroParticionOcupada->tamanoMensaje;

	printf("Realiza el intercambio de las particiones\n");

}

void consolidar(int indexEliminado) {
	printf("Entra consolidar\n");
	punteroParticion punteroParticionEliminada = list_get(particiones, indexEliminado);

	bool encuentro_particion_anterior(void* elemento) {
		punteroParticion particion = (punteroParticion*)elemento;
		return particion->punteroMemoria + particion->tamanoMensaje
				== punteroParticionEliminada->punteroMemoria;
	}
	// COMPRUEBO SI TIENE ALGUNA PARTICION A IZQUIERA DESOCUPADA PARA UNIRLA
	punteroParticion particionAnterior = list_find(particiones, (void*)encuentro_particion_anterior);
	if(particionAnterior != NULL) {
		printf("Encontro una particion libre a izquierda\n");
		if(!particionAnterior->ocupada) {
			particionAnterior->tamanoMensaje += punteroParticionEliminada->tamanoMensaje;
			list_remove(particiones, indexEliminado);
			consolidar(guard(obtener_index_particion(particionAnterior->punteroMemoria), "No se encontro memoria anterior\n"));
		}
	} else {
		// SI NO TIENE A IZQUIERDA, BUSCO A DERECHA
		bool encuentro_particion_posterior(void* elemento) {
			punteroParticion particion = (punteroParticion*)elemento;
			return particion->punteroMemoria
					== punteroParticionEliminada->punteroMemoria + punteroParticionEliminada->tamanoMensaje + 1;
		}
		printf("Entro a derecha\n");
		punteroParticion particionPosterior = list_find(particiones, (void*)encuentro_particion_posterior);
		if(particionPosterior != NULL) {

			if(!particionPosterior->ocupada) {
				printf("Encontro una particion libre a derecha\n");

				particionPosterior->tamanoMensaje += punteroParticionEliminada->tamanoMensaje;
				particionPosterior->punteroMemoria = punteroParticionEliminada->punteroMemoria;
				list_remove(particiones, indexEliminado);
				consolidar(guard(obtener_index_particion(particionPosterior->punteroMemoria), "No se encontro memoria posterior\n"));
			}
		}
	}
	printf("No hay mas particiones libres ni a izq ni a der\n");
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
