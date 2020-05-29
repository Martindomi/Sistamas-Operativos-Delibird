
#include "team.h"




int main(int argc, char *argv[]){

	t_config *config = config_create("./team.config");
	t_list * entrenadores_list = list_create();
	t_list *lista = list_create();
	cola_NEW=list_create();
	cola_READY=list_create();
	cola_EXEC=list_create();
	cola_EXIT=list_create();

/*	TODO
 * primero se tiene que conectar con el broker -> ver como serializar y deseralizar mensajes que envia y recibe
 *
 */

	char* ip_broker;
	char* puerto_broker;


	t_log* logger;

	logger = log_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.log", "TEAM", true, LOG_LEVEL_INFO);

	//Loggear "soy un log"

	//config = config_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");

	//enviar_mensaje_new_pokemon(logger, ip_broker, puerto_broker);
	char* puerto_thread_team = "55010";
	suscribirse_cola(APPEARED_POKEMON, ip_broker, puerto_broker, puerto_thread_team, logger);
	//config_destroy(config);--> COMENTADO YA QUE MAS ADELANTE SE USA EL CONFIG, se cierra al final del main



/* TODO
 * se deben hacer threads por cada entrenador
 *
 */
	inicializar_entrenadores(config, entrenadores_list);
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

	imprimirLista(cola_EXIT);

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


		list_destroy(entrenadores_list);// AGREGAR DESTRUCTOR DE ELEMENTOS
		list_destroy(lista);
		config_destroy(config);
}

void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger) {
	char* ip_team = "127.0.0.1";
	//crear_thread_suscripcion(ip_team, puerto_thread_team);

	suscribir(codigo_cola, ip_broker, puerto_broker, puerto_thread_team, logger);
}

void suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger) {
	char* mensaje;
	int conexion;
	//crear conexion
	conexion = crear_conexion(ip_broker, puerto_broker);
	//enviar mensaje
	log_info(logger, "conexion creada - suscripcion");

	enviar_mensaje_suscribir(codigo_operacion, puerto_thread_team, conexion);
	//recibir mensaje
	log_info(logger, "suscripcion enviado");
	mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "suscripcion recibido");
	log_info(logger, mensaje);

	free(mensaje);
	log_destroy(logger);
	liberar_conexion(conexion);
}

void crear_thread_suscripcion(char* ip, char* port)
{
	pthread_t thread_team;

	int socket_servidor;
    struct addrinfo hints, *servinfo, *p;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, port, &hints, &servinfo);

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

    while(1){

    	struct sockaddr_in dir_cliente;

		socklen_t tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

		pthread_create(&thread_team,NULL,(void*)recibe_mensaje_broker, &socket_cliente);
		pthread_detach(thread_team);

    }

}

void recibe_mensaje_broker(int* socket) {
	op_code cod_op;
	recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);
	//hace algo
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
		send_message_new_pokemon(nombre, posx, posy, quant, conexion);
		//recibir mensaje
		log_info(logger, "mensaje enviado");
		mensaje = client_recibir_mensaje(conexion);
		//loguear mensaje recibido
		log_info(logger, "mensaje recibido");
		log_info(logger, mensaje);

		free(mensaje);
		log_destroy(logger);
		liberar_conexion(conexion);
}



