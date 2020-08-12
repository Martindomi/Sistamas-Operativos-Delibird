#include "utils.h"

op_code vectorDeColas[]={NEW_POKEMON,CATCH_POKEMON,GET_POKEMON};
int conexiones[3];

bool suscribirse_a_colas_gameboy(char* path){

	bool conexionOK=false;
	int i=0;
	char* mensaje;
	pthread_t hiloEscucha;

	op_code cola;

	while(vectorDeColas[i]!=NULL){

		socketSuscripcion=crear_conexion(informacion->ipBroker,informacion->puertoBroker);

		if(socketSuscripcion!= -1){
			conexionOK=true;
			pthread_create(&hiloEscucha,NULL,(void*)crear_hilo_escucha_suscripcion,socketSuscripcion);
			pthread_detach(hiloEscucha);


			cola = vectorDeColas[i];
			enviar_mensaje_suscribir_con_id(cola, informacion->idGamecard, socketSuscripcion, -1);
			sleep(1);
			//printf("envio suscipcion\n");
			conexiones[i]=socketSuscripcion;
			i++;

		}else{

		//log_info(logger, "OPERACION POR DEFAULT; SUSCRIPCION-> 'Intento de reconexion y suscripcion'");
		break;

		}

	}

	//config_destroy(config);
	//log_destroy(logger);
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

		bool conexionOK = false;
		int count = 0;

		log_info(logger,"RECONEXION; Inicio de proceso de reintento de comunicacion con el Broker");

		while(!conexionOK){
			if(count != 0){
				log_info(logger,"RECONEXION; FALLIDA, se realiza un nuevo intento");
			}


			sleep(tiempo_de_reintento_conexion);
			conexionOK =suscribirse_a_colas_gameboy(path);
			++count ;
		}
		log_info(logger,"RECONEXION; EXITOSA, cantidad de intentos: %d", count);
		//config_destroy(config);
		//log_destroy(logger);
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
