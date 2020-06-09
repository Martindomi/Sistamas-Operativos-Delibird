
#include "team.h"




int main(int argc, char *argv[]){

	t_config *config = config_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	/*t_list * entrenadores_list = list_create();
	t_list *lista = list_create();
	cola_NEW=list_create();
	cola_READY=list_create();
	cola_EXEC=list_create();
	cola_EXIT=list_create();*/

/*	TODO
 * primero se tiene que conectar con el broker -> ver como serializar y deseralizar mensajes que envia y recibe
 *
 */

	char* ip_broker;
	char* puerto_broker;


	t_log* logger;

	logger = log_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.log", "TEAM", true, LOG_LEVEL_INFO);


/* TODO
 * se deben hacer threads por cada entrenador
 *
 */
	/*inicializar_entrenadores(config, entrenadores_list);
	imprimirLista(entrenadores_list);

	int cant_entrenadores = list_size(entrenadores_list);
	int i=0;
	while(i<cant_entrenadores){

		t_entrenador *unEntrenador = list_get(entrenadores_list,i);


		pthread_create(&(unEntrenador->th),NULL,&main_entrenador,unEntrenador);
		//pthread_join((unEntrenador->th), NULL);
		i++;


	}

	list_add_all(cola_NEW,entrenadores_list);
	imprimirLista(cola_NEW);

	list_add_all(cola_READY,cola_NEW);
	list_clean(cola_NEW);

	while(list_size(cola_EXIT)!=cant_entrenadores){
		printf("antes sleep: %d \n",list_size(cola_READY));
		sleep(2);
		printf("Esperando que terminen los entrenadores \n");
		t_entrenador *unEntrenador = list_get(cola_READY,0);
		sem_post(&(unEntrenador->sem_entrenador));
		sleep(2);
		printf("despues sleep: %d \n",list_size(cola_READY));

	}

	imprimirLista(cola_EXIT);*/

/*TODO
 * se debe realizar planificaion	1°FIFO
 * 									2°RR
 * 									3°SJF
 *
 */

/*TODO
 *ver como liberar memoria de los entrenadores -> los char** y los entrenadores en si
 *
 *
 */

		ids_mensajes_enviados = list_create();
		ACK = "ACK";

		//Loggear "soy un log"

		config = config_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
		ip_broker = config_get_string_value(config, "IP_BROKER");
		puerto_broker = config_get_string_value(config, "PUERTO_BROKER");

		//enviar_mensaje_new_pokemon(logger, ip_broker, puerto_broker);
		char* puerto_thread_team = "55010";
		crear_thread_suscripcion(NEW_POKEMON, ip_broker, puerto_broker, puerto_thread_team, logger);

		/*list_destroy(entrenadores_list);// AGREGAR DESTRUCTOR DE ELEMENTOS
		list_destroy(lista);*/
		config_destroy(config);
}

void suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* ip_puerto_team, t_log* logger) {
	char* mensaje;
	int conexion;
	//crear conexion
	conexion = crear_conexion(ip_broker, puerto_broker);
	//enviar mensaje
	log_info(logger, "conexion creada - suscripcion");

	enviar_mensaje_suscribir(codigo_operacion, ip_puerto_team, conexion);
	//recibir mensaje
	log_info(logger, "suscripcion enviado");
	mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "suscripcion recibido");
	log_info(logger, mensaje);

	free(mensaje);
	liberar_conexion(conexion);
}

void crear_thread_suscripcion(op_code op_code, char* ip_broker, char* port_broker, char* port_team, t_log* logger)
{
	pthread_t thread_team;
	char* ip_team = "127.0.0.2";

	int socket_servidor;
    struct addrinfo hints, *servinfo, *p;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip_team, port_team, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
    	socket_servidor = guard(socket(p->ai_family, p->ai_socktype, p->ai_protocol), "could not create TCP listening socket");

        if (socket_servidor == -1)
            continue;

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

    // TODO ver como unir puerto e ip del team
    char* ip_puerto_team = "127.0.0.2:55010";

	suscribir(op_code, ip_broker, port_broker, ip_puerto_team, logger);
	enviar_mensaje_new_pokemon(logger, ip_broker, port_broker);

	while(1){

	    	struct sockaddr_in dir_cliente;

			socklen_t tam_direccion = sizeof(struct sockaddr_in);

			int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
			if (socket_cliente == -1) {
			  if (errno == EWOULDBLOCK) {
				printf("No pending connections; sleeping for one second.\n");
				sleep(1);
			  } else {
				perror("error when accepting connection");
				exit(1);
			  }
			} else {
				int socket = socket_cliente;
				printf("Got a connection; writing 'hello' then closing.\n");
				pthread_create(&thread_team,NULL,(void*)recibe_mensaje_broker, &socket);
				pthread_detach(thread_team);
			}

	    }


}


void recibe_mensaje_broker(int* socket) {
	op_code cod_op;
	recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);
	printf("%d\n", cod_op);
	printf("%d\n",*socket);

	puntero_mensaje mensajeRecibido;
	uint32_t size;

	mensajeRecibido = recibir_new_pokemon(*socket, &size);

	printf("%d\n", mensajeRecibido->id_correlativo);

	bool encuentra_mensaje_propio(void* elemento) {
		return strcmp((char*)elemento, string_itoa(mensajeRecibido->id_correlativo)) == 0;
	}
	// TODO ver si no es mejor utilizar los ACK en vez de los enviados.
	bool encontre = list_any_satisfy(ids_mensajes_enviados, (void*)encuentra_mensaje_propio);

	if(encontre) {
		printf("%d\n", mensajeRecibido->id_correlativo);
	}

	devolver_mensaje(ACK, strlen(ACK) + 1, *socket);

	//free(mensajeRecibido);
	// TODO esto esta para hacer loop infinito con un mensaje que tiene de id correlativo al primer mensaje enviado.
	sleep(10);
	enviar_mensaje_new_pokemon2(logger, ip_broker, puerto_broker);
}

void enviar_mensaje_new_pokemon(t_log* logger, char* ip, char* puerto) {
		char* mensaje;
		int conexion;

		//crear conexion
		conexion = crear_conexion(ip, puerto);
		//enviar mensaje
		log_info(logger, "conexion creada");
		char* nombre = "pikachu";
		uint32_t posx = 2;
		uint32_t posy = 3;
		uint32_t quant = 8;
		send_message_new_pokemon(nombre, posx, posy, quant, 0, 0, conexion);
		//recibir mensaje
		log_info(logger, "mensaje enviado");
		mensaje = client_recibir_mensaje(conexion);
		//loguear mensaje recibido
		log_info(logger, "mensaje recibido");
		log_info(logger, mensaje);

		list_add(ids_mensajes_enviados, mensaje);

		free(mensaje);
		liberar_conexion(conexion);
}

void enviar_mensaje_new_pokemon2(t_log* logger, char* ip, char* puerto) {
		char* mensaje;
		int conexion;

		//crear conexion
		conexion = crear_conexion(ip, puerto);
		//enviar mensaje
		log_info(logger, "conexion creada");
		char* nombre = "pikachu";
		uint32_t posx = 2;
		uint32_t posy = 3;
		uint32_t quant = 8;
		send_message_new_pokemon(nombre, posx, posy, quant, 0, 1, conexion);
		//recibir mensaje
		log_info(logger, "mensaje enviado");
		mensaje = client_recibir_mensaje(conexion);
		//loguear mensaje recibido
		log_info(logger, "mensaje recibido");
		log_info(logger, mensaje);

		list_add(ids_mensajes_enviados, mensaje);

		free(mensaje);
		liberar_conexion(conexion);
}

