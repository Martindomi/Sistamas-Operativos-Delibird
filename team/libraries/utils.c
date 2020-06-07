
#include "utils.h"


void comprobarConexion(){
		char* ip;
		char* puerto;
		t_log* logger;
		t_config* config;

		logger = log_create("team.log", "TEAM", 0, LOG_LEVEL_INFO);
		config = config_create("team.config");
		ip = config_get_string_value(config, "IP_BROKER");
		puerto = config_get_string_value(config, "PUERTO_BROKER");

		log_info(logger, "ESTOY LOGEANDO");
		log_info(logger, ip);
		log_info(logger, puerto);

		int conexion = crear_conexion(ip, puerto);

		enviar_mensaje("hola", conexion);
		//recibir mensaje
		log_info(logger, "mensaje enviado");
		char*mensaje = client_recibir_mensaje(conexion);
		//loguear mensaje recibido
		log_info(logger, "mensaje recibido");
		log_info(logger, mensaje);

		log_info(logger, "ESTOY LOGEANDO");
		log_info(logger, ip);
		log_info(logger, puerto);

		log_destroy(logger);
		liberar_conexion(conexion);

}

void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger) {
	char* ip_team = "127.0.0.2";
	crear_thread_suscripcion(ip_team, puerto_thread_team);

	suscribir(codigo_cola, ip_broker, puerto_broker, puerto_thread_team, logger);
}

void suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger) {
	char* mensaje;
	int conexion;
	//crear conexion
	conexion = crear_conexion(ip_broker, puerto_broker);
	//enviar mensaje
	log_info(logger, "conexion creada - suscripcion: %d",conexion);

	enviar_mensaje_suscribir(codigo_operacion, puerto_thread_team, conexion);
	//recibir mensaje
	log_info(logger, "suscripcion enviado");
	mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "suscripcion recibido");
	log_info(logger, mensaje);

	free(mensaje);
	log_destroy(logger);
	close(conexion);
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

    pthread_create(&thread_team,NULL,(void*)hilo_escucha, &socket_servidor);
    pthread_detach(thread_team);
/*
    while(1){

    	struct sockaddr_in dir_cliente;

		socklen_t tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

		pthread_create(&thread_team,NULL,(void*)recibe_mensaje_broker, &socket_cliente);
		pthread_detach(thread_team);

    }

 */

}

void* hilo_escucha(int socket_servidor){

	while(1){

		struct sockaddr_in dir_cliente;

		socklen_t tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

		//char* mensaje = client_recibir_mensaje(socket_cliente);
		recibe_mensaje_broker(&socket_cliente);



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


