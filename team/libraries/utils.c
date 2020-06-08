
#include "utils.h"



bool suscribirse_a_colas() {

	t_config *config = inicializar_config("./team.config");
	bool conexionOK=false;
	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	char* ip_team= config_get_string_value(config, "IP_TEAM");
	char* puerto_team= config_get_string_value(config, "PUERTO_TEAM");
	char* log_path= config_get_string_value(config, "LOG_FILE");
	t_log *logger = inicializar_log("./team.config", "TEAM");

	op_code vectorCodigo[] = {APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON };

	int conexion, i=0;

	while(conexion!= -1 && i<3){

		conexion = suscribir2(vectorCodigo[i],ip_broker,puerto_broker,puerto_team,logger);

		i++;

	}

	if(i==3 && conexion!= -1){
		conexionOK = true;
		log_info(logger, "Conexion Broker: true");
	}

	log_destroy(logger);
	config_destroy(config);

	return conexionOK;

}


int suscribir2(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger) {
	char* mensaje;
	int conexion;



	//crear conexion
	conexion = crear_conexion(ip_broker, puerto_broker);
	if(conexion == -1){
		log_info(logger, "Conexion Broker: false");
		return conexion;
	}
	log_info(logger, "conexion creada -> SUSCRIPCION ; CONEXION: %d",conexion);
	enviar_mensaje_suscribir(codigo_operacion, puerto_thread_team, conexion);
	log_info(logger, "suscripcion enviado");
	//recibir mensaje
	mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "suscripcion recibido %d", codigo_operacion);
	log_info(logger, mensaje);
	free(mensaje);

	close(conexion);
	return 1;
}



void hilo_reconexion(){

	t_config * config = inicializar_config("./team.config");

	pthread_t th_reconexion;

	int tiempo = atoi(config_get_string_value(config,"TIEMPO_RECONEXION"));

	pthread_create(&th_reconexion,NULL,&reintentar_conexion,tiempo);


}


void reintentar_conexion(int tiempo){

	t_config* config = inicializar_config("./team.config");
	t_log *logger= inicializar_log("./team.config","TEAM");
	bool conexionOK = false;
	int count = 0;

	log_info(logger,"Inicio de proceso de reintento de comunicacion con el Broker");

	while(!conexionOK){
		if(count != 0){
			log_info(logger,"Reintento de comunicacion con el broker: FALLIDO; intento numero: %d", (count+1));
		}


		sleep(tiempo);
		conexionOK = suscribirse_a_colas();
		++count ;
	}

	log_info(logger,"Reintento de comunicacion con el broker: EXITO; cantidad de intentos: %d", count);

	config_destroy(config);
	log_destroy(logger);
}















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

void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char* ip_team,  char* puerto_thread_team, t_log* logger) {
	//crear_hilo_escucha(ip_team, puerto_thread_team);
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
	log_info(logger, "suscripcion recibido %d", codigo_operacion);
	log_info(logger, mensaje);

	free(mensaje);
	//log_destroy(logger);
	close(conexion);
}

void crear_hilo_escucha(char* ip, char* port)
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

}

void* hilo_escucha(int socket_servidor){

	while(1){

		struct sockaddr_in dir_cliente;

		socklen_t tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
		sleep(2);printf("Esperando mensaje\n");

		//char* mensaje = client_recibir_mensaje(socket_cliente);
		aplica_funcion_escucha(&socket_cliente);



	}
}



/*
    while(1){

    	struct sockaddr_in dir_cliente;

		socklen_t tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

		pthread_create(&thread_team,NULL,(void*)recibe_mensaje_broker, &socket_cliente);
		pthread_detach(thread_team);

    }

 */


void recibe_mensaje_broker(int* socket) {

	if(socket== -1)return;
	char* ack = "ACK";
	op_code cod_op;
	send(*socket,ack,sizeof(char)*4,MSG_WAITALL);
	recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);

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


