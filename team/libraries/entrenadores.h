/*
 * entrenadores.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_ENTRENADORES_H_
#define LIBRARIES_ENTRENADORES_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <conexiones.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <semaphore.h>


typedef enum
{

	NEW=1,
	READY= 2,
	EXEC= 3,
	BLOCK= 4,
	EXIT= 5

}t_estado;

typedef struct {
	int id;
	int x;
	int y;
	char** pokemonesCapturados;
	char** pokemonesObjetivo;
	int espacioLibre;
	t_estado estado;
	pthread_t th;
	sem_t sem_entrenador;


}t_entrenador;


typedef struct{
	char* pokemon;
	int cantidad;

}t_pokemonObjetivo;

t_list* lista_objetivo;


t_list *cola_NEW;
t_list *cola_READY;
t_list *cola_EXEC;
t_list *cola_BLOQUED_terminadoOK;
//t_list *cola_BLOQUED_terminadoDeadlock; Y esto?
//t_list *cola_BLOQUED_noTerminado; Y esto?
t_list *cola_EXIT;
/*


1) elegir entrenador de ready para entrenar (fifio y rr es el primero de la lista)

2) ejecutar (mover ((para caputrar o intercambiar)), capturar ) ->	if(strcmp(planificacion,"FIFO")){hacer fifo}
																	else if(stcmp(planificacion,"RR")){hacer round robbins}
																	else if(stcmp(planificacion,"SJB")){hacer shortest job firs}

3) finalizar ejecucion (con o sin desalojo) -> pasa a bloqueado si: se movio hasta el pokemon (espera caught)
											-> pasa a exit si: captura a todos los pokemones
											-> ready (con desalojo)
*/





int sizeVectorString(char**);
void liberarArrayDeStrings(char**);

/*
 *  ENTRENADOR
 */

void inicializar_entrenadores (t_list* );
void imprimirListaEntrenadores(t_list*);
void imprimirEntrenador(t_entrenador* entrenador);


/*
 * LISTA OBJETIVO
 */

void imprimirListaObjetivo();
void crearListaObjetivo();
void cargarObjetivosGlobales();
void agregarPokemonALista(char* pokemon);
t_pokemonObjetivo* buscarPokemon(char* pokemon);
void quitarPokemonesDeListaObjetivo(t_list* entrenadores_list);
void quitarPokemonDeLista(char* pokemon);





#endif /* LIBRARIES_ENTRENADORES_H_ */
