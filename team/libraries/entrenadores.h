/*
 * entrenadores.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_ENTRENADORES_H_
#define LIBRARIES_ENTRENADORES_H_

#include "libreriascomunes.h"

//#include "planificacion.h"




t_list* lista_objetivo;



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
void moverEntrenador(t_entrenador* entrenador, int xDestino, int yDestino);
void analizarCaptura(t_entrenador* entrenador);
void realizarIntercambio(t_entrenador* entrenador, t_entrenador* entrenadorDeadlock);
void contar_ciclos_entrenador(t_entrenador * entrenador, int ciclos);
bool entrenador_tiene_menor_rafaga(t_entrenador* entrenador, int *sizeAntes);


#endif /* LIBRARIES_ENTRENADORES_H_ */
