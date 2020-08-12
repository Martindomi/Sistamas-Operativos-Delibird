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
sem_t sem_deadlcok;
sem_t sem_exit;
sem_t sem_fin;
sem_t sem_entrenador_disponible;
sem_t sem_localized_appeared;
sem_t mutex_objetivo;
sem_t mutex_lista_ids;
sem_t mutex_colas;

int movimientoTime;

void main_entrenador(t_entrenador*);
void main_planificacion_caught();
void main_planificacion_recibidos();
void main_deadlock();
void main_exit(int);
void moverColas(t_list* origen, t_list* destino, t_entrenador* entrenador);
void agregarAColas(t_list* lista, t_entrenador* entrenador);
t_distancia* entrenadorMasCerca(t_pokemon* pokemonNuevo,t_list* listaEntrenadores);
double calcularDistancia(t_entrenador* entrenador, t_pokemon* pokemon);
void calcular_rafaga(t_entrenador* entrenador);
t_entrenador* planificacionFifo(t_list* colaReady);
t_entrenador* planificacionRR(t_list* colaReady);
t_entrenador* planificacionSJFSD(t_list* colaReady);
t_entrenador* planificacionSJFCD(t_list* colaReady);
t_list *buscar_entrenadores_new_disponibles();
t_list *buscar_entrenadores_bloqueados_disponibles();
t_list *buscar_entrenadores_bloqueados_NOdisponibles();
void main_planificacion_corto_plazo() ;
bool tiene_otro_pokemon(t_entrenador * entrenador);
t_list* crear_lista_deadlock(t_list* lista);
void limpiar_lista_char(t_list* lista);
char* sacar_pokemon_de_mas(t_entrenador* entrenador);
char* pokemon_de_mas(t_entrenador* entrenador);
bool todos_bloqueados();
bool todos_sin_espacio();
bool todos_terminados();
t_entrenador* busca_entrenador_que_necesita(char* pokemon);
void detectar_deadlock();
void mover_bloqueados_a_exit();
void finalizar_y_liberar();
int finalizar();
void mover_entrenador_bloqueado_a_exit(t_entrenador* enternador);

#endif /* LIBRARIES_PLANIFICACION_H_ */
