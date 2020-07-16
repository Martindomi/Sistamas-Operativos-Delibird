
#include "utils.h"
 op_code vectorDeColas[]={ CAUGHT_POKEMON, LOCALIZED_POKEMON,APPEARED_POKEMON  };
//--------------------------------------------------------------------------------------------------------------------//

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

void crear_hilo_escucha_suscripcion(int conexion){

	int result_recv=0;
	while(1){
		result_recv = aplica_funcion_escucha(&conexion);
		if(result_recv == -1){
			liberar_conexion(conexion);
			crear_hilo_reconexion("./team.config");
			return;
		}
	}

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
	}else{
		sem_post(&mutex_reconexion);
	}


}

void _reintentar_conexion(char* path){


		t_config *config = config_create(path);
		int tiempo = config_get_int_value(config,"TIEMPO_RECONEXION");
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
//--------------------------------------------------------------------------------------------------------------------//

/*
bool suscribirse_a_colas() {

	bool conexionOK=false;
	t_config *config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	char* ip_team= config_get_string_value(config, "IP_TEAM");
	char* puerto_team= config_get_string_value(config, "PUERTO_TEAM");
	char* log_path= config_get_string_value(config, "LOG_FILE");
	t_log* logger_asd = log_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.log", "TEAM", false, LOG_LEVEL_INFO);
	//TODO ARREGLAR ESTO
	char* ip_puerto_team = "127.0.0.2:55002";
	strcat(ip_puerto_team, ip_team);
	strcat(ip_puerto_team, ":");
	strcat(ip_puerto_team, puerto_team);

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

*/
/*
int suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* ip_puerto_team, t_log* logger) {
	char* mensaje;
	int conexion;

	//crear conexion
	tarda(1);
	conexion = crear_conexion(ip_broker, puerto_broker);
	if(conexion == -1){
		//log_info(logger, "Conexion Broker: FALSE");
		log_info(loggerTEAM, "OPERACION POR DEFAULT; SUSCRIPCION-> 'Intento de reconexion y suscripcion'");
		return conexion;
	}
	//log_info(logger, "conexion creada -> SUSCRIPCION ; CONEXION: %d",conexion);
	enviar_mensaje_suscribir(codigo_operacion, ip_puerto_team, conexion);
	//log_info(logger, "suscripcion enviado");
	//recibir mensaje
	mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	//log_info(logger, "suscripcion recibido %d", codigo_operacion);
	log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: MENSAJE. Contenido: [id del mensaje enviado es] %s", mensaje); // no estoy seguro que sea mensaje.
	//log_info(logger, mensaje);
	free(mensaje);

	close(conexion);
	return 1;
}
*/

/*
void hilo_reconexion(){

	//t_config * config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");

	pthread_t th_reconexion;

	int tiempo = configData->tiempoReconexion;//atoi(config_get_string_value(config,"TIEMPO_RECONEXION"));

	pthread_create(&th_reconexion,NULL,(void*)reintentar_conexion,tiempo);


}
*/
/*
void reintentar_conexion(int tiempo){
	sem_wait(&mutex_reconexion);
	//t_config* config = inicializar_config("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	//t_log *logger= inicializar_log("/home/utnso/tp-2020-1c-Elite-Four/team/team.config","TEAM");
	bool conexionOK = false;
	int count = 0;

	//log_info(logger,"Inicio de proceso de reintento de comunicacion con el Broker");
	log_info(loggerTEAM,"RECONEXION; Inicio de proceso de reintento de comunicacion con el Broker");

	while(!conexionOK){
		if(count != 0){
			//log_info(logger,"Reintento de comunicacion con el broker: FALLIDO; intento numero: %d", (count+1));
			log_info(loggerTEAM,"RECONEXION; FALLIDA, se realiza un nuevo intento");
		}


		sleep(tiempo);
		conexionOK = suscribirse_a_colas();
		++count ;
	}

//	log_info(logger,"Reintento de comunicacion con el broker: EXITO; cantidad de intentos: %d", count);
	log_info(loggerTEAM,"RECONEXION; EXITOSA, cantidad de intentos: %d", count);
	sem_post(&mutex_reconexion);

	//config_destroy(config);
	//log_destroy(logger);
}
*/
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

void enviar_get_objetivos(){

	int size = list_size(lista_objetivo);
	t_pokemonObjetivo* especieObjetivo;

	for(int i = 0 ; i < size; i ++){

		especieObjetivo = list_get(lista_objetivo,i);

		if(especieObjetivo->cantidad > 0){
			enviar_mensaje_get_pokemon(especieObjetivo->pokemon);
		}
	}
}

void enviar_mensaje_get_pokemon(char* especiePokemon){

	char* mensaje;
	int conexion;
	tarda(1);
	conexion = crear_conexion(configData->ipBroker, configData->puertoBroker);
	if(conexion==-1){
		//op_code vectorDeColas[]= {APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON};
		log_info(loggerTEAM,"OPERACION POR DEFAULT; GET-> 'Pokemon %s sin locaciones'", especiePokemon);
		crear_hilo_reconexion("./team.config");
	}else{
	send_message_get_pokemon(especiePokemon,0,0,conexion);
	mensaje=client_recibir_mensaje(conexion);
	log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: MENSAJE, Contenido: [id del mensaje enviado es] %s",mensaje);
	list_add(ids_mensajes_enviados, mensaje);

	liberar_conexion(conexion);

	}

}



void enviar_mensaje_catch_pokemon(t_entrenador *entrenador, char* especiePokemon, int posX, int posY){

	char*mensaje;
	int conexion;
	t_pokemon *pokemon;

	bool _filterPokemon(t_pokemonObjetivo *element){
		return !strcmp(element->pokemon,pokemon->especie);
	}
	tarda(1);
	conexion = crear_conexion(configData->ipBroker, configData->puertoBroker);
	if(conexion==-1){
		//op_code vectorDeColas[]= {APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON};
			crear_hilo_reconexion("./team.config");
		sem_post(&mutex_boolReconexion);
		log_info(loggerTEAM,"OPERACION POR DEFAULT; CATCH-> 'Entrenador %d captura pokemon %s con exito'",entrenador->id,especiePokemon);
		list_add(entrenador->pokemonesCapturados,entrenador->pokemonCapturando->especie);
		entrenador->id_catch=0;
		pokemon = entrenador->pokemonCapturando;
		entrenador->espacioLibre--;
		t_pokemonObjetivo *pokemonCapturado = list_find(lista_objetivo,(void*)_filterPokemon);
		pokemonCapturado->cantidad--;
		if(entrenador->espacioLibre!=0){
			sem_post(&sem_entrenador_disponible);
		}
		t_list* entrenadores_no_disponibles = buscar_entrenadores_bloqueados_NOdisponibles();
		if(list_is_empty(entrenadores_no_disponibles)){
			sem_post(&sem_deadlcok);
		}
		list_destroy(entrenadores_no_disponibles);
		//log_info(loggerTEAM,"Mensaje Recibido; Tipo: CAUGHT, Resultado: OK (por Default)");
		log_info(loggerTEAM,"CAPTURA; Entrenador %d:  Captura pokemon: %s en la posicion: X = %d Y = %d", entrenador->id, entrenador->pokemonCapturando->especie, entrenador->x, entrenador->y);
		mover_entrenador_bloqueado_a_exit(entrenador);

	}else{
		send_message_catch_pokemon(especiePokemon,posX,posY,0,0,conexion);
		printf("Envio catch pokemon %s\n",especiePokemon);
		mensaje=client_recibir_mensaje(conexion);
		entrenador->id_catch = atoi(mensaje);
		log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: MENSAJE, Contenido: [id del mensaje enviado es] %s",mensaje);
		list_add(ids_mensajes_enviados, mensaje);

		liberar_conexion(conexion);

	}

}

void tarda(int ciclos){

	sem_wait(&mutex_ciclos);
	ciclosCPU += ciclos;
	sem_post(&mutex_ciclos);

	sleep(configData->retardoCicloCPU * ciclos);

}

void contar_deadlock_producido(){

	sem_wait(&mutex_deadlockProd);
	deadlocksProducidos += 1;
	sem_post(&mutex_deadlockProd);

}

void contar_deadlock_resuelto(){

	sem_wait(&mutex_deadlockRes);
	deadlocksResueltos += 1;
	sem_post(&mutex_deadlockRes);

}

void contar_context_switch(){

	sem_wait(&mutex_conSwitch);
	contextSwitch += 1;
	sem_post(&mutex_conSwitch);

}

void inicializar_config_team(char* pathConfig){
	if((configTEAM = config_create(pathConfig))==NULL){
		printf("No se pudo crear config\n");
		exit(2);
	}


}

void inicializar_log_team(){


	if((loggerTEAM= log_create(configData->logFile, configData->id, true, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear log\n");
		exit(3);
	}

}
void inicializar_config_data(){

	inicializar_config_team("./team.config");
	configData = malloc(sizeof(data_config));
	int sizeALG, sizeIPB, sizePB, sizeIPT, sizePT,sizeID;
	sizeALG = strlen(config_get_string_value(configTEAM,"ALGORITMO_PLANIFICACION"))+1;
	configData->algoritmoPlanificacion = malloc(sizeALG);
	sizeIPB = strlen(config_get_string_value(configTEAM,"IP_BROKER"))+1;
	configData->ipBroker = malloc(sizeIPB);
	sizePB = strlen(config_get_string_value(configTEAM,"PUERTO_BROKER"))+1;
	configData->puertoBroker = malloc(sizePB);
	sizeIPT = strlen(config_get_string_value(configTEAM,"IP_TEAM"))+1;
	configData->ipTeam= malloc(sizeIPT);
	sizePT = strlen(config_get_string_value(configTEAM,"PUERTO_TEAM"))+1;
	configData->puertoTeam= malloc(sizePT);
	sizeID = strlen(config_get_string_value(configTEAM,"ID"))+1;
	configData->id= malloc(sizeID);



	configData->alpha=config_get_double_value(configTEAM,"ALPHA");
	memcpy(configData->algoritmoPlanificacion,config_get_string_value(configTEAM,"ALGORITMO_PLANIFICACION"),sizeALG);
	memcpy(configData->id,config_get_string_value(configTEAM,"ID"),sizeID);
	//config_get_string_value(configTEAM,"ALGORITMO_PLANIFICACION");
	//puts(configData->algoritmoPlanificacion);
	configData->posicionesEntrenadores= config_get_array_value(configTEAM,"POSICIONES_ENTRENADORES");
	configData->pokemonesEntrenadores= config_get_array_value(configTEAM,"POKEMON_ENTRENADORES");
	configData->objetivosEntrenadores= config_get_array_value(configTEAM,"OBJETIVOS_ENTRENADORES");
	configData->estmacionInicial=config_get_double_value(configTEAM,"ESTIMACION_INICIAL");
	memcpy(configData->ipBroker,config_get_string_value(configTEAM,"IP_BROKER"),sizeIPB);
	memcpy(configData->ipTeam,config_get_string_value(configTEAM,"IP_TEAM"),sizeIPT);
	configData->logFile=config_get_string_value(configTEAM,"LOG_FILE");
	memcpy(configData->puertoBroker,config_get_string_value(configTEAM, "PUERTO_BROKER"),sizePB);
	memcpy(configData->puertoTeam,config_get_string_value(configTEAM, "PUERTO_TEAM"),sizePT);
	configData->quantum=config_get_int_value(configTEAM,"QUANTUM");
	configData->retardoCicloCPU=config_get_int_value(configTEAM,"RETARDO_CICLO_CPU");
	configData->tiempoReconexion=config_get_int_value(configTEAM,"TIEMPO_RECONEXION");

	inicializar_log_team();
	config_destroy(configTEAM);
}
// no se si sirve
void liberar_config_data(){


	free(configData->algoritmoPlanificacion);
	free(configData->ipBroker);
	free(configData->ipTeam);
	free(configData->puertoBroker);
	free(configData->puertoTeam);
	free(configData->id);
	free(configData);


}

void pokemonObjetivo_destroyer(t_pokemonObjetivo* pokemon){

	free(pokemon->pokemon);
	free(pokemon);
}

void liberar_lista_objetivos(){

	if(!list_is_empty(lista_objetivo)){
		list_destroy_and_destroy_elements(lista_objetivo,pokemonObjetivo_destroyer);
	}else{
		list_destroy(lista_objetivo);
	}
}


void pokemon_destroyer(t_pokemon* pokemon){

	if(pokemon!=NULL){
	free(pokemon);
	}
}

void liberar_lista_pokemons_recibidos(){

	list_destroy_and_destroy_elements(listaPokemonsRecibidos,pokemonObjetivo_destroyer);

}

void caught_destroyer(t_caught* caught){

	free(caught);
}

void liberar_lista_pokemons_caught(){

	list_destroy_and_destroy_elements(listaPokemonesCaught,pokemonObjetivo_destroyer);
}

void entrenador_destroyer(t_entrenador* entrenador){

	list_clean_and_destroy_elements(entrenador->pokemonesCapturados, mensaje_destroyer);
	list_clean_and_destroy_elements(entrenador->pokemonesObjetivo, mensaje_destroyer);
	pokemon_destroyer(entrenador->pokemonCapturando);
	free(entrenador);
}

void liberar_entrenadores_de_lista(t_list* unaLista){

	list_destroy_and_destroy_elements(unaLista,entrenador_destroyer);
}

void mensaje_destroyer(char* mensaje){
	free(mensaje);
}

void liberar_ids_mensajes_enviados(){
	list_destroy_and_destroy_elements(ids_mensajes_enviados, mensaje_destroyer);
}


void finalizar_y_liberar(){

	log_destroy(loggerTEAM);
	liberar_conexion(socketEscucha);
	liberar_conexion(socketSuscripcion);
	liberar_config_data();
	liberar_lista_objetivos();
	liberar_lista_pokemons_caught();
	liberar_lista_pokemons_recibidos();
	liberar_ids_mensajes_enviados();
	liberar_entrenadores_de_lista(cola_EXIT);
	list_destroy(cola_EXEC);
	list_destroy(cola_BLOQUED);
	list_destroy(cola_NEW);
	list_destroy(cola_READY);
	list_destroy(lista_entrenadores);

	exit(0);

}


bool esRR() {
	return strcmp(configData->algoritmoPlanificacion,"RR") == 0;
}

bool esSJFconDesalojo() {
	return strcmp(configData->algoritmoPlanificacion,"SJF-CD") == 0;
}

bool esSJFsinDesalojo() {
	return strcmp(configData->algoritmoPlanificacion,"SJF-SD") == 0;
}

bool esSJF(){
	return esSJFconDesalojo() || esSJFsinDesalojo();

}
