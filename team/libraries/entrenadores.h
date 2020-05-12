/*
 * entrenadores.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_ENTRENADORES_H_
#define LIBRARIES_ENTRENADORES_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

typedef enum
{

	NEW=1,
	READY= 2,
	EXEC= 3,
	BLOCK= 4,
	EXIT= 5

}t_estado;

typedef struct {

	int x;
	int y;
	char** pokemonesCapturados;
	char** pokemonesObjetivo;
	int espacioLibre;
	t_estado estado;
	pthread_t fred;

	// lo que se mueve por las colas seria el entrenador y cuando se tiene que ejecutar ahi hago el pthread_join ?

}t_entrenador;


typedef struct{

	char* pokemon;
	int cantidad;

};

t_list* objetivo; // cargar todos los objetivos para poder mandar GETS al borker ( es necesario ? )


t_list *cola_NEW;
t_list *cola_READY;
t_list *cola_EXEC;
t_list *cola_BLOQUED_terminadoOK;
//t_list *cola_BLOQUED_terminadoDeadlock; Y esto?
//t_list *cola_BLOQUED_noTerminado; Y esto?

t_list *cola_EXIT;
/*
planificacion = new -> ready -> exec -> ready   <---- no es esta -----> creo,
									 -> exit
									 -> bloqued

planificacion = ready -> exec -> siguiente estado  <------ es esta ---->

planificacion = get_config_string(....) // fifo


1) elegir entrenador de ready para entrenar (fifio y rr es el primero de la lista)

2) ejecutar (mover ((para caputrar o intercambiar)), capturar ) ->	if(strcmp(planificacion,"FIFO")){hacer fifo}
																	else if(stcmp(planificacion,"RR")){hacer round robbins}
																	else if(stcmp(planificacion,"SJB")){hacer shortest job firs}

3) finalizar ejecucion (con o sin desalojo) -> pasa a bloqueado si: se movio hasta el pokemon (espera caught)
											-> pasa a exit si: captura a todos los pokemones
											-> ready (con desalojo)
*/
void inicializar_entrenadores (t_config*,t_list* );
int calcularCantidadLista(char**);
void imprimirLista(t_list*);
void liberarArrayDeStrings(char**);


#endif /* LIBRARIES_ENTRENADORES_H_ */
