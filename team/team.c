#include "team.h"
#include "libraries/planificacion.h"
#include "libraries/libreriascomunes.h"



 int main(int argc, char *argv[]){

	sem_t esperaSuscripcion;
	inicializar_config_data();

	op_code vectorDeColas[]={ APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON };

	lista_entrenadores = list_create();
	listaPokemonsRecibidos = list_create();
	listaPokemonesCaught= list_create();
	ids_mensajes_enviados = list_create();
	ACK = "ACK";

	cola_NEW=list_create();
	cola_READY=list_create();
	cola_EXEC=list_create();
	cola_EXIT=list_create();
	cola_BLOQUED=list_create();

	sem_init(&(sem_colas_no_vacias),0,0);
	sem_init(&(sem_cpu),0,1);
	sem_init(&(sem_caught),0,0);
	sem_init(&(sem_recibidos),0,0);
	sem_init(&(mutex_caught),0,1);
	sem_init(&(mutex_recibidos),0,1);
	sem_init(&mutex_mov_colas_time,0,1);
	sem_init(&sem_exit,0,0);
	sem_init(&(mutex_reconexion),0,1);
	sem_init(&(sem_fin),0,1);
	sem_init(&sem_deadlcok,0,0);
	sem_init(&mutex_boolReconexion,0,1);
	sem_init((&mutex_ciclos),0,1);
	sem_init((&mutex_deadlockProd),0,1);
	sem_init((&mutex_deadlockRes),0,1);
	sem_init((&mutex_conSwitch),0,1);
	sem_init((&esperaSuscripcion),0,1);
	sem_init(&mutex_suscripcion,0,0);
	sem_init(&sem_entrenador_disponible,0,0);

	sem_wait(&mutex_boolReconexion);
	seCreoHiloReconexion=false;
	sem_post(&mutex_boolReconexion);
	ciclosCPU = 0;
	deadlocksProducidos= 0;
	deadlocksResueltos= 0;
	contextSwitch= 0;

	//movimientoTime = 0;//sirve como timestamp de los movs de las colas

	pthread_t hiloRecibidos, hiloCaught, hiloCortoPlazo, hiloDeadlock, hiloExit;
	pthread_create(&hiloRecibidos,NULL,(void*)main_planificacion_recibidos,NULL);
	pthread_create(&hiloCaught,NULL,(void*)main_planificacion_caught,NULL);
	pthread_create(&hiloCortoPlazo,NULL,(void*)main_planificacion_corto_plazo,NULL);
	pthread_create(&hiloDeadlock,NULL,(void*)detectar_deadlock,NULL);
	int termino=0;
	pthread_create(&hiloExit,NULL,(void*)main_exit,termino);


	inicializar_entrenadores(lista_entrenadores);

	crear_hilo_entrenadores(lista_entrenadores);

	socketEscucha = crear_hilo_escucha(configData->ipTeam,configData->puertoTeam);
	bool conexionOK =suscribirse_a_colas("../team.config");
	if(!conexionOK){
		crear_hilo_reconexion("../team.config");
	}

	enviar_get_objetivos();

	//esperar_finalizacion_team();
	pthread_detach(hiloRecibidos);
	pthread_detach(hiloCaught);
	pthread_detach(hiloCortoPlazo);
	pthread_detach(hiloDeadlock);
	pthread_join(hiloExit,NULL);

	return EXIT_SUCCESS;
}
/*
t_mensajeTeam esperoMensaje() {
	printf("Esperando mensaje\n");
	int i = 0;
	sleep(1);
	if(i == 0 || list_size(cola_READY)>0) {
		i++;
		return APPEAR;
	}else {
		return BROKEROFF;
	}

}
*/

void crear_hilo_entrenadores(t_list* lista_entrenadores) {
	int cant_entrenadores = list_size(lista_entrenadores);
	int i=0;
	list_add_all(cola_NEW,lista_entrenadores);

	while(i<cant_entrenadores){
		t_entrenador *unEntrenador = list_get(lista_entrenadores,i);

		mover_entrenador_new_sin_espacio(unEntrenador);

		pthread_create(&(unEntrenador->th),NULL,(void*)main_entrenador,unEntrenador);
		pthread_detach((unEntrenador->th));
		i++;
	}
}

void mover_entrenador_new_sin_espacio(t_entrenador* enternador){

	if(enternador->espacioLibre==0){
		if(!tiene_otro_pokemon(enternador)){
			moverColas(cola_NEW,cola_EXIT,enternador);
			log_info(loggerTEAM,"CAMBIO DE COLA; Entrenador %d: NEW-> EXIT. Motivo: Entrenador con todos los pokemones!", enternador->id);
			sem_post(&sem_exit);
		}else{
			moverColas(cola_NEW,cola_BLOQUED,enternador);
			log_info(loggerTEAM,"CAMBIO DE COLA; Entrenador %d: NEW-> BLOQUED. Motivo: Entrenador sin espacio y otro pokemon", enternador->id);
			enternador->movsDisponibles=configData->quantum;
			sem_post(&sem_deadlcok);

		}
	}else{
		sem_post(&sem_entrenador_disponible);
	}


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

	bool encuentra_mensaje_propio(void* elemento) {
		char* el = (char*) elemento;
		return strcmp(el, string_itoa(mensajeRecibido->id_correlativo)) == 0;
	}

	switch(cod_op){
	case MESSAGE:

		msj = client_recibir_mensaje_SIN_CODEOP(*socket);
		log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: MENSAJE. Contenido: %s", msj);
		free(msj);
		break;

	case APPEARED_POKEMON:
		mensajeRecibido = recibir_appeared_pokemon(*socket, size);
		puntero_mensaje_appeared_pokemon appearedRecibido = mensajeRecibido->mensaje_cuerpo;
		free(mensajeRecibido);
		procesar_appeared(appearedRecibido);

		break;


	case CAUGHT_POKEMON:
		mensajeRecibido = recibir_caught_pokemon(*socket, size);
		puntero_mensaje_caught_pokemon caughtRecibido = mensajeRecibido->mensaje_cuerpo;
		free(mensajeRecibido);
		printf("RECIBO CAUGHT CON VALOR: %d\n", caughtRecibido->caughtResult);
		encontre = list_any_satisfy(ids_mensajes_enviados, (void*)encuentra_mensaje_propio);

			// TODO aca me fijo si es un mensaje que me interesa y acciono en consecuencia
			if(encontre) {
				printf("id:%d\n", mensajeRecibido->id_correlativo);
				procesar_caught(caughtRecibido, mensajeRecibido ->id_correlativo);

			}

		break;

	case LOCALIZED_POKEMON:

		mensajeRecibido = recibir_localized_pokemon(*socket, size);
		puntero_mensaje_localized_pokemon localizedRecibido = mensajeRecibido->mensaje_cuerpo;
		free(mensajeRecibido);
		encontre = list_any_satisfy(ids_mensajes_enviados, (void*)encuentra_mensaje_propio);

			// TODO aca me fijo si es un mensaje que me interesa y acciono en consecuencia
			if(encontre) {
				printf("id:%d\n", mensajeRecibido->id_correlativo);

				procesar_localized(localizedRecibido, mensajeRecibido->id_correlativo);

			}

		break;
	}

	return 0;



	//liberar_conexion(*socket);
	//free(mensajeRecibido);
	// TODO esto esta para hacer loop infinito con un mensaje que tiene de id correlativo al primer mensaje enviado.
	/*sleep(10);
	enviar_mensaje_appeared_pokemon2(logger, ip_broker, puerto_broker);*/
}


void procesar_localized(puntero_mensaje_localized_pokemon localizedRecibido, uint32_t id_correlativo){
	int cantidad = localizedRecibido->quant_pokemon * 2;
	t_pokemonObjetivo *poke = buscarPokemon(localizedRecibido->name_pokemon);
	int j = 0;
	for(int i=0;i<cantidad && j<poke->cantidad;i=i+2){

		t_pokemon *pokemon = malloc(sizeof(t_pokemon));
		pokemon->especie=malloc(sizeof(char)*localizedRecibido->name_size);
		memcpy(pokemon->especie,localizedRecibido->name_pokemon,localizedRecibido->name_size);
		pokemon->x=*((int*)(list_get(localizedRecibido->coords,i)));
		pokemon->y=*((int*)(list_get(localizedRecibido->coords,i+1)));
		printf("adentro de localized\n");
		log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: LOCALIZED. Contenido: ID Correalitvo=%u Posicion X=%d Posicion Y=%d",id_correlativo, pokemon->x, pokemon->y);



		sem_wait(&mutex_recibidos);
		list_add(listaPokemonsRecibidos,pokemon);
		sem_post(&mutex_recibidos);
		sem_post(&sem_recibidos);
		j++;
	}

	free(localizedRecibido->name_pokemon);
	list_destroy(localizedRecibido->coords);
	free(localizedRecibido);

}

void procesar_caught(puntero_mensaje_caught_pokemon caughtRecibido, uint32_t idCorrelativo){

	t_caught *caughts=malloc(sizeof(t_caught));
	caughts->idCorrelativo = idCorrelativo;
	switch(caughtRecibido->caughtResult){
	case 0:
		caughts->atrapado=OK;
		log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: CAUGHT. Contenido: ID Correalitvo=%d Atrapado=OK",idCorrelativo);

		break;
	case 1:
		caughts->atrapado=FAIL;
		log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: CAUGHT. Contenido: ID Correalitvo=%d Atrapado=FAIL",idCorrelativo);

		break;
	}

	free(caughtRecibido);
	sem_wait(&mutex_caught);
	list_add(listaPokemonesCaught,caughts);
	sem_post(&mutex_caught);
	sem_post(&sem_caught);
}

void procesar_appeared(puntero_mensaje_appeared_pokemon appearedRecibido){

	t_pokemon *pokemon = malloc(sizeof(t_pokemon));
	pokemon->especie=malloc(appearedRecibido->name_size);
	memcpy(pokemon->especie,appearedRecibido->name_pokemon,appearedRecibido->name_size);
	pokemon->x=appearedRecibido->pos_x;
	pokemon->y=appearedRecibido->pos_y;
	log_info(loggerTEAM,"MENSAJE RECIBIDO; Tipo: APPEARED. Contenido: Especie=%s, Posicion X=%d, Posicion Y=%d", pokemon->especie, pokemon->x, pokemon ->y);
	t_pokemonObjetivo *poke = buscarPokemon(appearedRecibido->name_pokemon);
	if(poke->cantidad <= 0){
		return;
	}
	free(appearedRecibido->name_pokemon);
	free(appearedRecibido);
	sem_wait(&mutex_recibidos);
	list_add(listaPokemonsRecibidos,pokemon);
	sem_post(&mutex_recibidos);
	sem_post(&sem_recibidos);
}

