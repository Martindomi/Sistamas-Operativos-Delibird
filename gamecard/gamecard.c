#include "gamecard.h"
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>

//#include "TALL-GRASS.c"


int main (int argc, char *argv[]) {

	informacion = malloc(sizeof(char)*5);
	config = config_create("/home/utnso/tp-2020-1c-Elite-Four/gamecard/gamecard.config");

	int ipBrokerSize = strlen(config_get_string_value(config, "IP_BROKER"))+1;
	informacion->ipBroker = malloc(ipBrokerSize);
	memcpy(informacion->ipBroker, config_get_string_value(config, "IP_BROKER"), ipBrokerSize);

	int puertoBrokerSize = strlen(config_get_string_value(config, "PUERTO_BROKER"))+1;
	informacion->puertoBroker = malloc(puertoBrokerSize);
	memcpy(informacion->puertoBroker, config_get_string_value(config, "PUERTO_BROKER"), puertoBrokerSize);

	int ipGamecardSize = strlen(config_get_string_value(config, "IP_GAMECARD"))+1;
	informacion->ipGamecard = malloc(ipGamecardSize);
	memcpy(informacion->ipGamecard, config_get_string_value(config, "IP_GAMECARD"), ipGamecardSize);

	int puertoGamecardSize = strlen(config_get_string_value(config, "PUERTO_GAMECARD"))+1;
	informacion->puertoGamecard = malloc(puertoGamecardSize);
	memcpy(informacion->puertoGamecard, config_get_string_value(config, "PUERTO_GAMECARD"), puertoGamecardSize);

	int idGamecardSize = strlen(config_get_string_value(config, "ID"))+1;
	informacion->idGamecard = malloc(idGamecardSize);
	memcpy(informacion->idGamecard, config_get_string_value(config, "ID"), idGamecardSize);

	if((logger= log_create("/home/utnso/tp-2020-1c-Elite-Four/gamecard/gamecard.log", informacion->idGamecard, true, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear log\n");
		exit(3);
	}

	tiempo_de_reintento_conexion = config_get_int_value(config,"TIEMPO_DE_REINTENTO_CONEXION");
	tiempo_de_reintento_operacion = config_get_int_value(config,"TIEMPO_DE_REINTENTO_OPERACION");
	tiempo_retardo_operacion = config_get_int_value(config,"TIEMPO_RETARDO_OPERACION");

	t_configFS* configTG= crear_config(argc,argv);
	ptoMontaje = configTG->ptoMontaje;
	block_size = configTG->block_size;
	blocks = configTG->blocks;
	/*ptoMontaje = config_get_int_value(config,"PUNTO_MONTAJE_TALLGRASS");
	block_size = config_get_int_value(config,"BLOCK_SIZE");
	blocks = config_get_int_value(config,"BLOCKS");*/

	iniciar_filesystem();

		/*sem_init(&(sem_colas_no_vacias),0,0);
		sem_init(&(sem_cpu),0,1);
		sem_init(&(sem_caught),0,0);
		sem_init(&(sem_recibidos),0,0);
		sem_init(&(mutex_caught),0,1);
		sem_init(&(mutex_recibidos),0,1);
		sem_init(&mutex_mov_colas_time,0,1);
		sem_init(&sem_exit,0,0);*/
	sem_init(&(mutex_reconexion),0,1);
		/*sem_init(&(sem_fin),0,1);
		sem_init(&sem_deadlcok,0,0);*/
	sem_init(&mutex_boolReconexion,0,1);
		/*sem_init((&mutex_ciclos),0,1);
		sem_init((&mutex_deadlockProd),0,1);
		sem_init((&mutex_deadlockRes),0,1);
		sem_init((&mutex_conSwitch),0,1);
		sem_init((&esperaSuscripcion),0,1);*/
	sem_init(&mutex_suscripcion,0,0);
		//sem_init(&sem_entrenador_disponible,0,0);

	sem_wait(&mutex_boolReconexion);
	seCreoHiloReconexion=false;
	sem_post(&mutex_boolReconexion);

	ACK="ACK";

	socketEscucha = crear_hilo_escucha(informacion->ipGamecard,informacion->puertoGamecard);
	bool conexionOk = suscribirse_a_colas_gameboy("/home/utnso/tp-2020-1c-Elite-Four/gamecard/gamecard.config");
	if(!conexionOk){
		crear_hilo_reconexion("/home/utnso/tp-2020-1c-Elite-Four/gamecard/gamecard.config");
	}



	log_destroy(logger);
	//liberar_conexion(conexion);
	sleep(50000);
}

int aplica_funcion_escucha(int * socket){

	printf("recibe mensaje del broker\n");
	op_code cod_op;
	char *msj;
	int recv_data;

	recv_data = recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);
	if(recv_data==-1){
		return -1;
	}
	printf("recibio cod op \n");
	devolver_mensaje(ACK, strlen(ACK) + 1, *socket);

	puntero_mensaje mensajeRecibido;
	uint32_t size;
	bool encontre;
	char* mensaje;
	bool encuentra_mensaje_propio(void* elemento) {
		char* el = (char*) elemento;
		return strcmp(el, string_itoa(mensajeRecibido->id_correlativo)) == 0;
	}
	int conexion = crear_conexion(informacion->ipBroker, informacion->puertoBroker);

	switch(cod_op){
	case MESSAGE:

		msj = client_recibir_mensaje_SIN_CODEOP(*socket);
		log_info(logger,"MENSAJE RECIBIDO; Tipo: MENSAJE. Contenido: %s", msj);
		free(msj);
		break;

	case NEW_POKEMON:
		mensajeRecibido = recibir_new_pokemon(*socket, size);
		puntero_mensaje_new_pokemon newRecibido = mensajeRecibido->mensaje_cuerpo;
		tratar_mensaje_NEW_POKEMON(newRecibido->pos_x,newRecibido->pos_y,newRecibido->quant_pokemon,newRecibido->name_pokemon);
		send_message_appeared_pokemon(newRecibido->name_pokemon,newRecibido->pos_x,newRecibido->pos_y,0,mensajeRecibido->id,conexion);
		mensaje = client_recibir_mensaje(conexion);
		free(mensajeRecibido);
		free(mensaje);
		break;


	case CATCH_POKEMON:
		mensajeRecibido = recibir_catch_pokemon(*socket, size);
		puntero_mensaje_catch_pokemon catchRecibido = mensajeRecibido->mensaje_cuerpo;
		char* respuesta =tratar_mensaje_CATCH_POKEMON(catchRecibido->pos_x,catchRecibido->pos_y,catchRecibido->name_pokemon);
		printf("respuesta %s\n", respuesta);
		send_message_caught_pokemon(respuesta,0,mensajeRecibido->id,conexion);
		mensaje = client_recibir_mensaje(conexion);
		free(mensajeRecibido);
		free(mensaje);
		break;

	case GET_POKEMON:

		mensajeRecibido = recibir_get_pokemon(*socket, size);
		puntero_mensaje_get_pokemon getRecibido = mensajeRecibido->mensaje_cuerpo;
		t_list* listadoPosiciones = tratar_mensaje_GET_POKEMON(getRecibido->name_pokemon);
		for(int i = 0; i<list_size(listadoPosiciones); i++ ) {
			printf("ITEM %d\n", list_get(listadoPosiciones, i));
		}
		uint32_t cantidadPos = (list_size(listadoPosiciones)/2);
		send_message_localized_pokemon(getRecibido->name_pokemon,cantidadPos,listadoPosiciones,0,mensajeRecibido->id,conexion);
		mensaje = client_recibir_mensaje(conexion);
		free(mensajeRecibido);
		free(mensaje);
	    free(listadoPosiciones);

		break;
	}

	liberar_conexion(conexion);


	return 0;



	//liberar_conexion(*socket);
	//free(mensajeRecibido);
	// TODO esto esta para hacer loop infinito con un mensaje que tiene de id correlativo al primer mensaje enviado.
	/*sleep(10);
	enviar_mensaje_appeared_pokemon2(logger, ip_broker, puerto_broker);*/
}

