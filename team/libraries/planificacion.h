/*
 * planificacion.h
 *
 *  Created on: 6 jun. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_PLANIFICACION_H_
#define LIBRARIES_PLANIFICACION_H_


#include "libreriascomunes.h"



t_list* listaPokemons;
t_list* listaPokemonsRecibidos;

void *main_entrenador(t_entrenador*);
void moverColas(t_list* origen, t_list* destino, t_entrenador* entrenador);
void agregarAColas(t_list* lista, t_entrenador* entrenador);
t_distancia* entrenadorMasCerca(t_pokemon* pokemonNuevo,t_list* listaEntrenadores);
double calcularDistancia(t_entrenador* entrenador, t_pokemon* pokemon);
t_entrenador* planificacionFifo(t_list* colaReady);


#endif /* LIBRARIES_PLANIFICACION_H_ */
