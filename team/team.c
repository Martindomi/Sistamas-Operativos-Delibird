
#include "team.h"




int main(int argc, char *argv[]){

	//t_config *config = config_create("./team2.config");
	t_list * lista_entrenadores = list_create();

	cola_NEW=list_create();
	cola_READY=list_create();
	cola_EXEC=list_create();
	cola_EXIT=list_create();
	char* path_log;
	char* ip_broker;
	char* puerto_broker;
	char* ip_team;
	char* puerto_team;

	t_pokemon* pika1 = malloc(sizeof(t_pokemon));
			pika1->x = 2;
			pika1->y = 3;

			//distancia1:2
			//distancia2:17
	t_pokemon* pika2 = malloc(sizeof(t_pokemon));
			pika2->x = 2;
			pika2->y = 3;



/* TODO
 * se deben hacer threads por cada entrenador
 *
 */
	inicializar_entrenadores(lista_entrenadores);
	//printf("Imprimo e list\n");
	//imprimirLista(entrenadores_list);

	int cant_entrenadores = list_size(lista_entrenadores);
	int i=0;
	while(i<cant_entrenadores){
		t_entrenador *unEntrenador = list_get(lista_entrenadores,i);

		pthread_create(&(unEntrenador->th),NULL,&main_entrenador,unEntrenador);
		//pthread_join((unEntrenador->th), NULL);
		i++;
	}

	list_add_all(cola_NEW,lista_entrenadores);
	//printf("Imprimo NEW");
	//imprimirLista(cola_NEW);


/*	TODO
 * primero se tiene que conectar con el broker -> ver como serializar y deseralizar mensajes que envia y recibe
 *
 */





	t_config *config = config_creatxe("./team.config");
	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	ip_team = config_get_string_value(config, "IP_TAEM");
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
/*	suscribirse_cola(LOCALIZED_POKEMON, ip_broker, puerto_broker, ip_team, puerto_team, logger);
	suscribirse_cola(CAUGHT_POKEMON, ip_broker, puerto_broker, ip_team, puerto_team, logger);
	suscribirse_cola(APPEARED_POKEMON, ip_broker, puerto_broker, ip_team, puerto_team, logger);
*/
	config_destroy(config);
	log_destroy(logger);




/*TODO
 * se debe realizar planificaion	1°FIFO
 * 									2°RR
 * 									3°SJF
 *
 */

	t_distancia* resultado = entrenadorMasCerca(pika1,cola_NEW);
	moverColas(cola_NEW, cola_READY, resultado->entrenador);
	resultado = entrenadorMasCerca(pika2,cola_NEW);
	moverColas(cola_NEW, cola_READY, resultado->entrenador);
	printf("Imprimo NEW\n");
	imprimirListaEntrenadores(cola_NEW);
	printf("Imprimo Ready\n");
	imprimirListaEntrenadores(cola_READY);

/*
	while(list_size(cola_EXIT)!=cant_entrenadores){
		printf("antes sleep: %d \n",list_size(cola_READY));
		sleep(2);
		printf("Esperando que terminen los entrenadores \n");
		t_entrenador *unEntrenador = list_get(cola_READY,0);
		sem_post(&(unEntrenador->sem_entrenador));
		sleep(2);
		printf("despues sleep: %d \n",list_size(cola_READY));

	}*/

	//imprimirLista(cola_EXIT);



/*TODO
 *ver como liberar memoria de los entrenadores -> los char** y los entrenadores en si
 *
 *
 */


		list_destroy(lista_entrenadores);// AGREGAR DESTRUCTOR DE ELEMENTOS

		//config_destroy(config);
}


void aplica_funcion_escucha(int * socket){
	//printf("ARMA LA FUNCION \n");
}

