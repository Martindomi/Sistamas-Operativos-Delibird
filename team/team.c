#include "team.h"
#include "libraries/planificacion.h"
#include "libraries/libreriascomunes.h"


/*
 *  ANOTACIONES:
 *
 * 	*********************   CREADOS   *********************
 *
 * 	libreriascomunes 	-> data_config  (estructura para sacar todos los datos del team.config) configData (variable tipo data_config)
 * 						-> configTEAM (config global para no estar abriendo y cerrando)(solo se usa una vez para crear el configData)
 * 						-> logTEAM (log global para no estar abriendo y cerrando)
 *
 *  utils: 	-> funciones para inicializar los de arriba
 *			-> enviar mensaje GET (sin probar)(falta proceso 'default')
 *			-> enviar mensaje CATCH (sin probar)(falta proceso 'default')
 *
 *  probe inicializar entrenadores sin crear el config adentro de la funcion y pasandole los datos de configData (pruebas ok)
 *
 *  todos los config y logs fueron comentadas (por las dudas) y usados los globales
 *
 *	*********************   CONSULTAS   *********************
 *
 *	Esta bien tener el log global? el config podria no serlo ya que se abre y cierra
 *
 *	Habria que pasarle el path del config por parametro a team para que sepa cual usar? ej ./team /home/utnso/tp/team/team.config y asi poder tener varios
 *	procesos team y cada uno podra ejecutar un config diferente.
 *
 *
 *  *********************   NOTA   *********************
 *
 *  habria que probar todo bien, tambien lo que dije que ya probe :)
 */


 int main(int argc, char *argv[]){

	inicializar_config_data();

	t_list * lista_entrenadores = list_create();
	int andaBroker = 1;
	listaPokemonsRecibidos = list_create();
	listaPokemonesCaught= list_create();
	ids_mensajes_enviados = list_create();
	ACK = "ACK";
	printf("%d\n",list_is_empty(lista_entrenadores));
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

	sem_init((&mutex_ciclos),0,1);
	sem_init((&mutex_deadlockProd),0,1);
	sem_init((&mutex_deadlockRes),0,1);
	sem_init((&mutex_conSwitch),0,1);


	ciclosCPU = 0;
	deadlocksProducidos= 0;
	deadlocksResueltos= 0;
	contextSwitch= 0;

	movimientoTime = 0;//sirve como timestamp de los movs de las colas

	pthread_t hiloRecibidos, hiloCaught, hiloCortoPlazo, hiloDeadlock, hiloExit;
	pthread_create(&hiloRecibidos,NULL,(void*)main_planificacion_recibidos,NULL);
	pthread_create(&hiloCaught,NULL,(void*)main_planificacion_caught,NULL);
	pthread_create(&hiloCortoPlazo,NULL,(void*)main_planificacion_corto_plazo,NULL);
	pthread_create(&hiloDeadlock,NULL,(void*)detectar_deadlock,NULL);
	pthread_create(&hiloExit,NULL,(void*)main_exit,NULL);
	//initListaPokemonsNecesitados();
	inicializar_entrenadores(lista_entrenadores);

	printf("%d\n",list_is_empty(lista_entrenadores));
	crear_hilo_entrenadores(lista_entrenadores);

	enviar_get_objetivos();



/*	t_list *posicionesPikachu = list_create();
	int *i=malloc(sizeof(int));
	*i=6;
	list_add(posicionesPikachu,i);
	i=malloc(sizeof(int));
	*i=9;
	list_add(posicionesPikachu,i);
	i=malloc(sizeof(int));
	*i=9;
	list_add(posicionesPikachu,i);
	 i=malloc(sizeof(int));
	*i=9;
	list_add(posicionesPikachu,i);
	i=malloc(sizeof(int));
	*i=10;
	list_add(posicionesPikachu,i);
	i=malloc(sizeof(int));
	*i=1;
	list_add(posicionesPikachu,i);

	//Espero un appear o broker desconecta entonces cierro


	printf("imprimo NEW: \n");
	imprimirListaEntrenadores(cola_NEW);
	printf("Imprimo Ready: \n");
	imprimirListaEntrenadores(cola_READY);

	puntero_mensaje_localized_pokemon pokemonloco = malloc(sizeof(puntero_mensaje_localized_pokemon));
	pokemonloco->name_size=5;
	pokemonloco->name_pokemon=malloc(sizeof(char)*pokemonloco->name_size);
	memcpy(pokemonloco->name_pokemon,"Toto",5);
	pokemonloco->quant_pokemon=3;
	pokemonloco->coords = posicionesPikachu;

	procesar_localized(pokemonloco,1);
	sleep(10);*/
//	printf("imprimo NEW: \n");
//	imprimirListaEntrenadores(cola_NEW);
//	printf("Imprimo Ready: \n");
//	imprimirListaEntrenadores(cola_READY);

	//t_list* coords;

   // procesar_localized();

/*
	do {

	t_mensajeTeam queHago =	esperoMensaje();
	switch(queHago) {
		case APPEAR:
			aparecio_pokemon();
			break;
		case BROKEROFF:
			andaBroker = 0;
			printf("Se cerro el broker, chau\n");
			break;
		}

	}while(andaBroker);
*/
/*	TODO
 * primero se tiene que conectar con el broker -> ver como serializar y deseralizar mensajes que envia y recibe
 *
  Conexiones Prueba
	t_config *config = config_create("./team.config");	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	ip_team = config_get_string_value(config, "IP_TEAM");
	puerto_team = config_get_string_value(config, "PUERTO_TEAM");
	path_log = config_get_string_value(config, "LOG_FILE");
	t_log* logger;
	logger = log_create(path_log, "TEAM", true, LOG_LEVEL_INFO);
*/	suscribirse_a_colas();
	crear_hilo_escucha(configData->ipTeam,configData->puertoTeam);
/*	bool conexionOK = suscribirse_a_colas();
	if(!conexionOK){
		hilo_reconexion();
	}*/
/*	sleep(10);
	suscribirse_cola(LOCALIZED_POKEMON, ip_broker, puerto_broker, ip_team, puerto_team, logger);
	suscribirse_cola(CAUGHT_POKEMON, ip_broker, puerto_broker, ip_team, puerto_team, logger);
	suscribirse_cola(APPEARED_POKEMON, ip_broker, puerto_broker, ip_team, puerto_team, logger);
	config_destroy(config);
	log_destroy(logger);
	*/




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
/*
	t_config *config = config_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");
	ip_team = config_get_string_value(config, "IP_TEAM");
	puerto_team = config_get_string_value(config, "PUERTO_TEAM");
	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	path_log = config_get_string_value(config, "LOG_FILE");
	logger = log_create(path_log, "TEAM", true, LOG_LEVEL_INFO);
	ACK = "ACK";
	ids_mensajes_enviados = list_create();

	crear_hilo_escucha(ip_team,puerto_team);
	bool conexionOK = suscribirse_a_colas();
	if(!conexionOK){
		hilo_reconexion();
	}
	sleep(5);
	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	sleep(5);
	enviar_mensaje_appeared_pokemon2(logger, ip_broker, puerto_broker);

	sleep(5);
	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	sleep(5);
	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);*/
	//list_destroy(lista_entrenadores);// AGREGAR DESTRUCTOR DE ELEMENTOS

	//printf("el programa sigue por aqui \n");
	//sleep(100000);
	//config_destroy(config);

	// TODO sacar esto
	sleep(50000);
}

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

void crear_hilo_entrenadores(t_list* lista_entrenadores) {
	int cant_entrenadores = list_size(lista_entrenadores);
	int i=0;
	while(i<cant_entrenadores){
		t_entrenador *unEntrenador = list_get(lista_entrenadores,i);

		pthread_create(&(unEntrenador->th),NULL,(void*)main_entrenador,unEntrenador);
		//pthread_join((unEntrenador->th), NULL);
		i++;
	}

	list_add_all(cola_NEW,lista_entrenadores);
}

void aplica_funcion_escucha(int * socket){
	printf("recibe mensaje del broker\n");
	op_code cod_op;
	recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);
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
	case APPEARED_POKEMON:
		mensajeRecibido = recibir_appeared_pokemon(*socket, size);
		puntero_mensaje_appeared_pokemon appearedRecibido = mensajeRecibido->mensaje_cuerpo;
		procesar_appeared(appearedRecibido);

		break;


	case CAUGHT_POKEMON:
		mensajeRecibido = recibir_caught_pokemon(*socket, size);
		puntero_mensaje_caught_pokemon caughtRecibido = mensajeRecibido->mensaje_cuerpo;
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

		encontre = list_any_satisfy(ids_mensajes_enviados, (void*)encuentra_mensaje_propio);

			// TODO aca me fijo si es un mensaje que me interesa y acciono en consecuencia
			if(encontre) {
				printf("id:%d\n", mensajeRecibido->id_correlativo);
				procesar_localized(localizedRecibido, mensajeRecibido->id_correlativo);

			}

		break;
	}





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
		log_info(loggerTEAM,"Mensaje recibido; Tipo: LOCALIZED. Contenido: ID Correalitvo=%u Posicion X=%d Posicion Y=%d",id_correlativo, pokemon->x, pokemon->y);



		sem_wait(&mutex_recibidos);
		list_add(listaPokemonsRecibidos,pokemon);
		sem_post(&mutex_recibidos);
		sem_post(&sem_recibidos);
		j++;
	}


}

void procesar_caught(puntero_mensaje_caught_pokemon caughtRecibido, uint32_t idCorrelativo){

	t_caught *caughts=malloc(sizeof(t_caught));
	caughts->idCorrelativo = idCorrelativo;
	switch(caughtRecibido->caught_size){
	case 3:
		caughts->atrapado=OK;
		log_info(loggerTEAM,"Mensaje recibido; Tipo: CAUGHT. Contenido: ID Correalitvo=%d Atrapado=OK",idCorrelativo);

		break;
	case 5:
		caughts->atrapado=FAIL;
		log_info(loggerTEAM,"Mensaje recibido; Tipo: CAUGHT. Contenido: ID Correalitvo=%d Atrapado=FAIL",idCorrelativo);

		break;
	}
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
	log_info(loggerTEAM,"Mensaje recibido; Tipo: APPEARED. Contenido: Especie=%s, Posicion X=%d, Posicion Y=%d", pokemon->especie, pokemon->x, pokemon ->y);
	t_pokemonObjetivo *poke = buscarPokemon(appearedRecibido->name_pokemon);
	if(poke->cantidad <= 0){
		return;
	}

	sem_wait(&mutex_recibidos);
	list_add(listaPokemonsRecibidos,pokemon);
	sem_post(&mutex_recibidos);
	sem_post(&sem_recibidos);
}

