
#include "utils.h"



bool suscribirse_a_colas() {

	bool conexionOK=false;
/*	t_config *config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	char* ip_team= config_get_string_value(config, "IP_TEAM");
	char* puerto_team= config_get_string_value(config, "PUERTO_TEAM");
	char* log_path= config_get_string_value(config, "LOG_FILE");
	t_log* logger_asd = log_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.log", "TEAM", false, LOG_LEVEL_INFO);
*/	//TODO ARREGLAR ESTO
	char* ip_puerto_team = "127.0.0.2:55002";
	/*strcat(ip_puerto_team, ip_team);
	strcat(ip_puerto_team, ":");
	strcat(ip_puerto_team, puerto_team);*/

	op_code vectorCodigo[] = {APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON};

	int conexion, i=0;

	while(conexion!= -1 && i<3){

//		conexion = suscribir(vectorCodigo[i],ip_broker,puerto_broker,ip_puerto_team,logger_asd);
		conexion = suscribir(vectorCodigo[i],configData->ipBroker,configData->puertoBroker,ip_puerto_team); //borro parametro log

		i++;

	}

	if(i==3 && conexion!= -1){
		conexionOK = true;
		//log_info(logger_asd, "Conexion Broker: true");
		//log_info(loggerTEAM,"Conexion Broker; Resultado: OK, Conexion por: SUSCRIPCION"); -> NO LO PIDE
	}

//	log_destroy(logger_asd);
//	config_destroy(config);

	return conexionOK;

}


int suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* ip_puerto_team/*, t_log* logger*/) {
	char* mensaje;
	int conexion;

	//crear conexion
	conexion = crear_conexion(ip_broker, puerto_broker);
	if(conexion == -1){
		//log_info(logger, "Conexion Broker: FALSE");
		log_info(loggerTEAM, "Conexion Broker; Resultado: FAIL, Conexion por: SUSCRIPCION, Default: Intento de reconexion y suscripcion");
		return conexion;
	}
	//log_info(logger, "conexion creada -> SUSCRIPCION ; CONEXION: %d",conexion);
	enviar_mensaje_suscribir(codigo_operacion, ip_puerto_team, conexion);
	//log_info(logger, "suscripcion enviado");
	//recibir mensaje
	mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	//log_info(logger, "suscripcion recibido %d", codigo_operacion);
	log_info(loggerTEAM,"Mensaje recibido; Tipo: MENSAJE. Contenido: %s", mensaje); // no estoy seguro que sea mensaje.
	//log_info(logger, mensaje);
	free(mensaje);

	close(conexion);
	return 1;
}



void hilo_reconexion(){

	//t_config * config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");

	pthread_t th_reconexion;

	int tiempo = configData->tiempoReconexion;//atoi(config_get_string_value(config,"TIEMPO_RECONEXION"));

	pthread_create(&th_reconexion,NULL,(void*)reintentar_conexion,tiempo);


}


void reintentar_conexion(int tiempo){

	//t_config* config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	//t_log *logger= inicializar_log("/home/utnso/tp-2020-1c-Elite-Four/team/team.config","TEAM");
	bool conexionOK = false;
	int count = 0;

	//log_info(logger,"Inicio de proceso de reintento de comunicacion con el Broker");
	log_info(loggerTEAM,"Inicio de proceso de reintento de comunicacion con el Broker");
	while(!conexionOK){
		if(count != 0){
			//log_info(logger,"Reintento de comunicacion con el broker: FALLIDO; intento numero: %d", (count+1));
			log_info(loggerTEAM,"Reintento de comunicacion con el broke FALLIDO, cantidad de intentos: %d. Se realiza un nuevo intento", (count+1));
		}


		sleep(tiempo);
		conexionOK = suscribirse_a_colas();
		++count ;
	}

//	log_info(logger,"Reintento de comunicacion con el broker: EXITO; cantidad de intentos: %d", count);
	log_info(loggerTEAM,"Reintento de comunicacion con el broker EXITOSO, cantidad de intentos: %d", count);

	//config_destroy(config);
	//log_destroy(logger);
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
		list_add(ids_mensajes_enviados, mensaje);

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
		list_add(ids_mensajes_enviados, mensaje);

		liberar_conexion(conexion);
}

void enviar_mensaje_get_pokemon(char* especiePokemon){

	char*mensaje;
	int conexion;

	conexion = crear_conexion(configData->ipBroker, configData->puertoBroker);
	if(conexion==-1){
		// accion default (loggear)
		hilo_reconexion();
	}else{
	send_message_get_pokemon(especiePokemon,0,0,conexion);
	mensaje=client_recibir_mensaje(conexion);
	log_info(loggerTEAM,"Mensaje recibido; Tipo: MENSAJE, Contenido: id del mensaje enviado es: %d",mensaje);
	list_add(ids_mensajes_enviados, mensaje);

	liberar_conexion(conexion);

	}

}

void enviar_mensaje_catch_pokemon(char* especiePokemon, int posX, int posY){

	char*mensaje;
	int conexion;

	conexion = crear_conexion(configData->ipBroker, configData->puertoBroker);
	if(conexion==-1){
		// accion default (loggear) -> podria ser que lo atrapa igual
		hilo_reconexion();
	}else{
		send_message_catch_pokemon(especiePokemon,posX,posY,0,0,conexion);
		mensaje=client_recibir_mensaje(conexion);
		log_info(loggerTEAM,"Mensaje recibido; Tipo: MENSAJE, Contenido: id del mensaje enviado es: %d",mensaje);
		list_add(ids_mensajes_enviados, mensaje);

		liberar_conexion(conexion);

	}

}

void inicializar_config_team(char* pathConfig){
	if((configTEAM = config_create(pathConfig))==NULL){
		printf("No se pudo crear config\n");
		exit(2);
	}


}

void inicializar_log_team(){


	if((loggerTEAM= log_create(configData->logFile, "TEAM", true, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear log\n");
		exit(3);
	}

}
void inicializar_config_data(){

	inicializar_config_team("./team.config");
	configData = malloc(sizeof(data_config));

	configData->alpha=config_get_double_value(configTEAM,"ALPHA");
	configData->algoritmoPlanificacion = config_get_string_value(configTEAM,"ALGORITMO_PLANIFICACION");
	//puts(configData->algoritmoPlanificacion);
	configData->posicionesEntrenadores= config_get_array_value(configTEAM,"POSICIONES_ENTRENADORES");
	puts(configData->posicionesEntrenadores[0]);
	configData->pokemonesEntrenadores= config_get_array_value(configTEAM,"POKEMON_ENTRENADORES");
	configData->objetivosEntrenadores= config_get_array_value(configTEAM,"OBJETIVOS_ENTRENADORES");
	configData->estmacionInicial=config_get_double_value(configTEAM,"ESTIMACION_INICIAL");
	configData->ipBroker=config_get_string_value(configTEAM,"IP_BROKER");
	configData->ipTeam=config_get_string_value(configTEAM,"IP_TEAM");
	configData->logFile=config_get_string_value(configTEAM,"LOG_FILE");
	configData->puertoBroker=config_get_string_value(configTEAM, "PUERTO_BROKER");
	configData->puertoTeam=config_get_string_value(configTEAM, "PUERTO_TEAM");
	configData->quantum=config_get_int_value(configTEAM,"QUANTUM");
	configData->retardoCicloCPU=config_get_int_value(configTEAM,"RETARDO_CICLO_CPU");
	configData->tiempoReconexion=config_get_int_value(configTEAM,"TIEMPO_RECONEXION");

	inicializar_log_team();
	config_destroy(configTEAM);
}
// no se si sirve
void liberar_config_data(){

	int i =0;
	free(configData->algoritmoPlanificacion);
	free(configData->ipBroker);
	free(configData->ipTeam);
	free(configData->logFile);
	while(configData->objetivosEntrenadores[i]!=NULL){
		free(configData->objetivosEntrenadores[i]);
		i++;
	}
	i = 0;
	while(configData->pokemonesEntrenadores[i]!=NULL){
		free(configData->pokemonesEntrenadores[i]);
		i++;
	}
	i = 0;
	while(configData->posicionesEntrenadores[i]!=NULL){
		free(configData->posicionesEntrenadores[i]);
		i++;
	}
	free(configData->puertoBroker);
	free(configData->puertoTeam);
	free(configData);
	i = 0;

}

