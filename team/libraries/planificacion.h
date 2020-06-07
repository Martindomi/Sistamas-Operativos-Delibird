/*
 * planificacion.h
 *
 *  Created on: 6 jun. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_PLANIFICACION_H_
#define LIBRARIES_PLANIFICACION_H_

#include "entrenadores.h"


typedef struct {
	int x,y;
	char* especie;
} t_pokemon;



typedef struct {
	t_pokemon* pokemon;
	t_entrenador* entrenador;
	double distancia;
} t_distancia;


int planificacionFifo(t_list* colaReady);
void *main_entrenador(t_entrenador*);
void moverColas(t_list* origen, t_list* destino, t_entrenador* entrenador);
void agregarAColas(t_list* lista, t_entrenador* entrenador);
t_distancia* entrenadorMasCerca(t_pokemon* pokemonNuevo,t_list* lista);
double calcularDistancia(t_entrenador* entrenador, t_pokemon* pokemon);


#endif /* LIBRARIES_PLANIFICACION_H_ */
