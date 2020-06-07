/*
 * planificacion.c
 *
 *  Created on: 6 jun. 2020
 *      Author: utnso
 */

#include "planificacion.h"

void* main_entrenador(t_entrenador* entrenador){

	printf("Posicion entrenador: %d %d\n", entrenador->x, entrenador->y);
	sem_wait(&(entrenador->sem_entrenador));
	printf("SE DESBLOQUEA ENTRENADOR, ESTA EJECUTANDO\n");
	moverColas(cola_READY,cola_EXIT, entrenador);
}


void moverColas(t_list* origen, t_list* destino, t_entrenador* entrenador) {
	int tamanio = list_size(origen);
	for(int i = 0; i<tamanio;i++){
		t_entrenador* comp =  list_get(origen,i);
		if(comp->id == entrenador->id) {
			list_remove(origen,i);
			break;
		}
	}
	list_add(destino, entrenador);
}

void agregarAColas(t_list* lista, t_entrenador* entrenador) {
	list_add(lista, entrenador);
}


int planificacionFifo(t_list* colaReady){
	return 0;
}

t_distancia* entrenadorMasCerca(t_pokemon* pokemonNuevo,t_list* lista) {
	//busco cual es el entrenador mas cerca
	//y lo devuelvo
	int recorrido = 0;
	int cantidadLista =  list_size(lista);
	t_distancia* distanciaResultado = malloc(sizeof(t_distancia));
	distanciaResultado->distancia = 9999999;
	distanciaResultado->pokemon = pokemonNuevo;

	for(recorrido = 0;recorrido<cantidadLista;recorrido++) {
		t_entrenador* entrenador = list_get(lista,recorrido);
		double distanciaCalculada = calcularDistancia(entrenador,pokemonNuevo);
		if(distanciaCalculada<distanciaResultado->distancia) {
			distanciaResultado->entrenador = entrenador;
			distanciaResultado->distancia = distanciaCalculada;
		}
	}
	//log despues sacar o modificar
	printf("Distancia Menor: %f \n",distanciaResultado->distancia);

	return distanciaResultado;
}

double calcularDistancia(t_entrenador* entrenador, t_pokemon* pokemon) {
	double distanciaX2 = pow(entrenador->x - pokemon->x,2);
	double distanciaY2 = pow(entrenador->y - pokemon->y,2);
	return distanciaX2 + distanciaY2;
}
