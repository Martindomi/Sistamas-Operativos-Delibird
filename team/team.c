#include "team.h"
#include "libraries/planificacion.h"
#include "libraries/libreriascomunes.h"




int main(int argc, char *argv[]){

	//t_config *config = config_create("./team2.config");
	t_list * lista_entrenadores = list_create();
	int andaBroker = 1;

	cola_NEW=list_create();
	cola_READY=list_create();
	cola_EXEC=list_create();
	cola_EXIT=list_create();
	char* path_log;
	char* ip_broker;
	char* puerto_broker;
	char* ip_team;
	char* puerto_team;


	sem_init(&(sem_cpu),0,1);

	initListaPokemonsNecesitados();
	inicializar_entrenadores(lista_entrenadores);
	crear_hilo_entrenadores(lista_entrenadores);

	//Espero un appear o broker desconecta entonces cierro


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
	suscribirse_a_colas();
	crear_hilo_escucha(ip_team,puerto_team);
	bool conexionOK = suscribirse_a_colas();
	if(!conexionOK){
		hilo_reconexion();
	}
	sleep(10);
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
	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	sleep(5);
	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	//list_destroy(lista_entrenadores);// AGREGAR DESTRUCTOR DE ELEMENTOS

	sleep(10);

	printf("el programa sigue por aqui \n");
	//sleep(100000);
	//config_destroy(config);

	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	enviar_mensaje_appeared_pokemon(logger, ip_broker, puerto_broker);
	sleep(15);
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

void aparecio_pokemon() {

	//Esto seguro se separa, en la escucha y en la planificacion. Dos threads
	t_pokemon* pokeAppear = simularLlegadaPokemon();
	if(pokeAppear!=NULL) {
		t_distancia* resultado = entrenadorMasCerca(pokeAppear,cola_NEW);
		resultado->entrenador->pokemonCapturando = pokeAppear;
		moverColas(cola_NEW,cola_READY,resultado->entrenador);
	}

	pokeAppear = simularLlegadaPokemon();
	if(pokeAppear!=NULL) {
		t_distancia* resultado = entrenadorMasCerca(pokeAppear,cola_NEW);
		resultado->entrenador->pokemonCapturando = pokeAppear;
		moverColas(cola_NEW,cola_READY,resultado->entrenador);
	}

	void _algo(t_entrenador* entre) {
		printf("%d\n" , entre->id);
	}
	list_iterate(cola_READY,(void*) _algo);

	sem_wait(&(sem_cpu));
	t_entrenador* aEjecutar = planificacionFifo(cola_READY);
	//DESDE AK, DEBERIA HACERLO EL PLANIFICADOR CORTO PLAZO
	sem_post(&(aEjecutar->sem_entrenador));
	moverColas(cola_READY,cola_EXEC,aEjecutar);
}

void crear_hilo_entrenadores(t_list* lista_entrenadores) {
	int cant_entrenadores = list_size(lista_entrenadores);
	int i=0;
	while(i<cant_entrenadores){
		t_entrenador *unEntrenador = list_get(lista_entrenadores,i);

		pthread_create(&(unEntrenador->th),NULL,&main_entrenador,unEntrenador);
		//pthread_join((unEntrenador->th), NULL);
		i++;
	}

	list_add_all(cola_NEW,lista_entrenadores);
}

void aplica_funcion_escucha(int * socket){
	printf("recibe mensaje del broker\n");
	op_code cod_op;
	recv(*socket, &cod_op, sizeof(op_code), MSG_WAITALL);
	printf("%d\n", cod_op);
	printf("%d\n",*socket);

	devolver_mensaje(ACK, strlen(ACK) + 1, *socket);

	puntero_mensaje mensajeRecibido;
	uint32_t size;

	mensajeRecibido = recibir_appeared_pokemon(*socket, &size);
	printf("el mensaje es : %s", mensajeRecibido->mensaje_cuerpo);
	printf("el id es: %d\n", mensajeRecibido->id_correlativo);

	bool encuentra_mensaje_propio(void* elemento) {
		return strcmp((char*)elemento, string_itoa(mensajeRecibido->id_correlativo)) == 0;
	}
	// TODO ver si no es mejor utilizar los ACK en vez de los enviados.
	bool encontre = list_any_satisfy(ids_mensajes_enviados, (void*)encuentra_mensaje_propio);

	if(encontre) {
		printf("id:%d\n", mensajeRecibido->id_correlativo);
	}

	liberar_conexion(*socket);
	//free(mensajeRecibido);
	// TODO esto esta para hacer loop infinito con un mensaje que tiene de id correlativo al primer mensaje enviado.
	/*sleep(10);
	enviar_mensaje_appeared_pokemon2(logger, ip_broker, puerto_broker);*/
}
