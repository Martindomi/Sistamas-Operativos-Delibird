
#include "utils.h"



bool suscribirse_a_colas() {

	t_config *config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	bool conexionOK=false;
	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	char* ip_team= config_get_string_value(config, "IP_TEAM");
	char* puerto_team= config_get_string_value(config, "PUERTO_TEAM");
	char* log_path= config_get_string_value(config, "LOG_FILE");
	t_log* logger_asd = log_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.log", "TEAM", false, LOG_LEVEL_INFO);
	//TODO ARREGLAR ESTO
	char* ip_puerto_team = "127.0.0.2:55002";
	/*strcat(ip_puerto_team, ip_team);
	strcat(ip_puerto_team, ":");
	strcat(ip_puerto_team, puerto_team);*/

	op_code vectorCodigo[] = {APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON};

	int conexion, i=0;

	while(conexion!= -1 && i<3){

		conexion = suscribir(vectorCodigo[i],ip_broker,puerto_broker,ip_puerto_team,logger_asd);

		i++;

	}

	if(i==3 && conexion!= -1){
		conexionOK = true;
		log_info(logger_asd, "Conexion Broker: true");
	}

	log_destroy(logger_asd);
	config_destroy(config);

	return conexionOK;

}


int suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* ip_puerto_team, t_log* logger) {
	char* mensaje;
	int conexion;

	//crear conexion
	conexion = crear_conexion(ip_broker, puerto_broker);
	if(conexion == -1){
		log_info(logger, "Conexion Broker: false");
		return conexion;
	}
	log_info(logger, "conexion creada -> SUSCRIPCION ; CONEXION: %d",conexion);
	enviar_mensaje_suscribir(codigo_operacion, ip_puerto_team, conexion);
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

	t_config * config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");

	pthread_t th_reconexion;

	int tiempo = atoi(config_get_string_value(config,"TIEMPO_RECONEXION"));

	pthread_create(&th_reconexion,NULL,&reintentar_conexion,tiempo);


}


void reintentar_conexion(int tiempo){

	t_config* config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	t_log *logger= inicializar_log("/home/utnso/tp-2020-1c-Elite-Four/team/team.config","TEAM");
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

void recibe_mensaje_broker(int* socket) {

	if(socket== -1)return;
	char* ack = "ACK";
	op_code cod_op;
	send(*socket,ack,sizeof(char)*4,MSG_WAITALL);
	recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);

}

void enviar_mensaje_appeared_pokemon(t_log* logger, char* ip, char* puerto) {
		char* mensaje;
		int conexion;

		//crear conexion
		conexion = crear_conexion(ip, puerto);
		//enviar mensaje
		log_info(logger, "conexion creada");
		char* nombre = "pikachu";
		uint32_t posx = 2;
		uint32_t posy = 3;
		send_message_appeared_pokemon(nombre, posx, posy, 0, 0, conexion);
		//recibir mensaje
		log_info(logger, "mensaje enviado");
		mensaje = client_recibir_mensaje(conexion);
		//loguear mensaje recibido
		log_info(logger, "mensaje recibido: %s",mensaje);

		free(mensaje);
		liberar_conexion(conexion);
}
void enviar_mensaje_appeared_pokemon2(t_log* logger, char* ip, char* puerto) {
		char* mensaje;
		int conexion;

		//crear conexion
		conexion = crear_conexion(ip, puerto);
		//enviar mensaje
		log_info(logger, "conexion creada");
		char* nombre = "pikachu";
		uint32_t posx = 2;
		uint32_t posy = 3;
		send_message_appeared_pokemon(nombre, posx, posy, 0, 1, conexion);
		//recibir mensaje
		log_info(logger, "mensaje enviado");
		mensaje = client_recibir_mensaje(conexion);
		//loguear mensaje recibido
		log_info(logger, "mensaje recibido");
		log_info(logger, mensaje);

		free(mensaje);
		liberar_conexion(conexion);
}

