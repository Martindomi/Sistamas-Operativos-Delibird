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
t_list* listaPokemonesCaught;

sem_t sem_recibidos;
sem_t mutex_recibidos;
sem_t sem_caught;
sem_t mutex_caught;
sem_t sem_colas_no_vacias;
sem_t mutex_mov_colas_time;

int movimientoTime;

void main_entrenador(t_entrenador*);
void main_planificacion_caught();
void main_planificacion_recibidos();
void moverColas(t_list* origen, t_list* destino, t_entrenador* entrenador);
void agregarAColas(t_list* lista, t_entrenador* entrenador);
t_distancia* entrenadorMasCerca(t_pokemon* pokemonNuevo,t_list* listaEntrenadores);
double calcularDistancia(t_entrenador* entrenador, t_pokemon* pokemon);
t_entrenador* planificacionFifo(t_list* colaReady);
t_entrenador* planificacionRR(t_list* colaReady);
t_list *buscar_entrenadores_bloqueados_disponibles();
t_list *buscar_entrenadores_bloqueados_NOdisponibles();
void main_planificacion_corto_plazo() ;



#endif /* LIBRARIES_PLANIFICACION_H_ */
