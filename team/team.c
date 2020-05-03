
#include "team.h"




int main(int argc, char *argv[]){

	t_config *config = config_create("./team.config");
	t_list * entrenadores_list = list_create();
	t_list *lista = list_create();
	t_entrenador unEntrenador;

/*	TODO
 * primero se tiene que conectar con el broker -> ver como serializar y deseralizar mensajes que envia y recibe
 *
 */

	inicializar_entrenadores(config, unEntrenador, entrenadores_list);


/* TODO
 * se deben hacer threads por cada entrenador
 *
 */

/*TODO
 * se debe realizar planificaion	1°FIFO
 * 									2°RR
 * 									3°SJF
 *
 */


}



