
#include "team.h"




int main(int argc, char *argv[]){

	t_config *config = config_create("./team.config");
	t_list * entrenadores_list = list_create();
	t_list *lista = list_create();
	cola_NEW=list_create();
	cola_READY=list_create();
	cola_EXEC=list_create();
	cola_EXIT=list_create();

/*	TODO
 * primero se tiene que conectar con el broker -> ver como serializar y deseralizar mensajes que envia y recibe
 *
 */

	inicializar_entrenadores(config, entrenadores_list);
	imprimirLista(entrenadores_list);

/* TODO
 * se deben hacer threads por cada entrenador
 *
 */

	int cant_entrenadores = list_size(entrenadores_list);
	int i=0;
	while(i<cant_entrenadores){

		t_entrenador *unEntrenador = list_get(entrenadores_list,i);


		pthread_create(&(unEntrenador->th),NULL,&main_entrenador,unEntrenador);
		//pthread_join((unEntrenador->th), NULL);
		i++;


	}

	list_add_all(cola_NEW,entrenadores_list);
	imprimirLista(cola_NEW);

	list_add_all(cola_READY,cola_NEW);
	list_clean(cola_NEW);

	while(list_size(cola_EXIT)!=cant_entrenadores){
		printf("antes sleep: %d \n",list_size(cola_READY));
		sleep(2);
		printf("Esperando que terminen los entrenadores \n");
		t_entrenador *unEntrenador = list_get(cola_READY,0);
		sem_post(&(unEntrenador->sem_entrenador));
		sleep(2);
		printf("despues sleep: %d \n",list_size(cola_READY));

	}

	imprimirLista(cola_EXIT);

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

	list_destroy(entrenadores_list);// AGREGAR DESTRUCTOR DE ELEMENTOS
	list_destroy(lista);
	config_destroy(config);

}




