/*
 * planificacion.c
 *
 *  Created on: 6 jun. 2020
 *      Author: utnso
 */

#include "planificacion.h"
#include "../team.h"


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

t_entrenador* planificacionFifo(t_list* colaReady){
	t_entrenador* proximoAEjecutar = list_get(colaReady,0);

	return proximoAEjecutar;
}

void initListaPokemonsNecesitados() {
	//con esto simulamos el almacenamiento del gamecard
	listaPokemons = list_create();
	t_pokemon* temp = malloc(sizeof(t_pokemon));


	temp->especie = malloc(15);
	strcpy(temp->especie,"Squirtle\n");
			temp->x = 5;
			temp->y = 4;
	list_add(listaPokemons,temp);

	temp = malloc(sizeof(t_pokemon));
	temp->especie = malloc(15);
	strcpy(temp->especie,"Charmander\n");
			temp->x = 4;
			temp->y = 7;
	list_add(listaPokemons,temp);

	temp = malloc(sizeof(t_pokemon));
	temp->especie = malloc(15);
	strcpy(temp->especie,"Pikachu\n");
			temp->x = 2;
			temp->y = 3;
	list_add(listaPokemons,temp);

	temp = malloc(sizeof(t_pokemon));
	temp->especie = malloc(15);
	strcpy(temp->especie,"Pidgey\n");
			temp->x = 6;
			temp->y = 10;
	list_add(listaPokemons,temp);
}

t_pokemon* simularLlegadaPokemon() {
	return list_remove(listaPokemons,0);
}

t_distancia* entrenadorMasCerca(t_pokemon* pokemonNuevo,t_list* listaEntrenadores) {
	//busco cual es el entrenador mas cerca
	//y lo devuelvo
	int recorrido = 0;
	int cantidadLista =  list_size(listaEntrenadores);
	t_distancia* distanciaResultado = malloc(sizeof(t_distancia));
	distanciaResultado->distancia = 9999999;
	distanciaResultado->pokemon = pokemonNuevo;

	for(recorrido = 0;recorrido<cantidadLista;recorrido++) {
		t_entrenador* entrenador = list_get(listaEntrenadores,recorrido);
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
