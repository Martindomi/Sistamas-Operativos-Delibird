#include "utils.h"

op_code vectorDeColas[]={NEW_POKEMON,CATCH_POKEMON,GET_POKEMON};
int conexiones[3];

bool suscribirse_a_colas(char* path){

	bool conexionOK=false;
	int i=0;
	char* mensaje;
	pthread_t hiloEscucha;

	op_code cola;
	t_config *config = config_create(path);

	char* logPath = config_get_string_value(config,"LOG_FILE");
	char* ipBroker = config_get_string_value(config, "IP_BROKER");
	char* puertoBroker = config_get_string_value(config, "PUERTO_BROKER");
	char* id = config_get_string_value(config,"ID");

	t_log *logger = log_create(logPath,id,true,LOG_LEVEL_INFO);

	while(vectorDeColas[i]!=NULL){

		socketSuscripcion=crear_conexion(ipBroker,puertoBroker);

		if(socketSuscripcion!= -1){
			conexionOK=true;
			pthread_create(&hiloEscucha,NULL,(void*)crear_hilo_escucha_suscripcion,socketSuscripcion);
			pthread_detach(hiloEscucha);


			cola = vectorDeColas[i];
			enviar_mensaje_suscribir_con_id(cola, id, socketSuscripcion, -1);
			printf("envio suscipcion\n");
			tarda(1);
			conexiones[i]=socketSuscripcion;
			i++;

		}else{

		log_info(logger, "OPERACION POR DEFAULT; SUSCRIPCION-> 'Intento de reconexion y suscripcion'");
		break;

		}

	}

	config_destroy(config);
	log_destroy(logger);
	return conexionOK;
}

void crear_hilo_reconexion(char* path){
	sem_wait(&mutex_reconexion);
	if(!seCreoHiloReconexion){
		sem_wait(&mutex_boolReconexion);
		seCreoHiloReconexion=true;
		sem_post(&mutex_boolReconexion);
		sem_post(&mutex_reconexion);
		pthread_t th_reconexion;
		pthread_create(&th_reconexion,NULL,(void*)_reintentar_conexion,path);
		pthread_detach(th_reconexion);
	}else{
		sem_post(&mutex_reconexion);
	}
}

void _reintentar_conexion(char* path){


		t_config *config = config_create(path);
		int tiempo = config_get_int_value(config,"TIEMPO_DE_REINTENTO_CONEXION");
		char* logPath = config_get_string_value(config, "LOG_FILE");
		char *programeName= config_get_string_value(config, "ID");
		char *ip= config_get_string_value(config, "IP_BROKER");
		char *puerto= config_get_string_value(config, "PUERTO_BROKER");
		t_log *logger= log_create(logPath,programeName,true,LOG_LEVEL_INFO);
		bool conexionOK = false;
		int count = 0;

		log_info(logger,"RECONEXION; Inicio de proceso de reintento de comunicacion con el Broker");

		while(!conexionOK){
			if(count != 0){
				log_info(logger,"RECONEXION; FALLIDA, se realiza un nuevo intento");
			}


			sleep(tiempo);
			conexionOK =suscribirse_a_colas(path);
			++count ;
		}
		log_info(logger,"RECONEXION; EXITOSA, cantidad de intentos: %d", count);
		config_destroy(config);
		log_destroy(logger);
		sem_wait(&mutex_boolReconexion);
		seCreoHiloReconexion=false;
		sem_post(&mutex_boolReconexion);
		sem_post(&mutex_suscripcion);

}

void crear_hilo_escucha_suscripcion(int conexion){

	int result_recv=0;
	while(1){
		result_recv = aplica_funcion_escucha(&conexion);
		if(result_recv == -1){
			liberar_conexion(conexion);
			crear_hilo_reconexion("../gamecard.config");
			return;
		}
	}

}
