/*
 * planificacion.c
 *
 *  Created on: 6 jun. 2020
 *      Author: utnso
 */

#include "planificacion.h"
#include "../team.h"


void main_planificacion_recibidos(){
	int i = 0;
	t_pokemonObjetivo *pokemonsObjetivo ;
	t_pokemon *pokemon;
	t_entrenador *entrenador;
	t_distancia *distancia_new, *distancia_bloqued;

	bool _filterPokemon(t_pokemonObjetivo *element){
		return !strcmp(element->pokemon,pokemon->especie);
	}

	while(1){
		printf("esperando localized\n");
		sem_wait(&sem_recibidos);
		pokemon = list_get(listaPokemonsRecibidos,i);
		pokemonsObjetivo =list_find(lista_objetivo,(void*)_filterPokemon);
		if(pokemonsObjetivo->cantidad==NULL){

		}else if(pokemonsObjetivo->cantidad==0){

		}else{
			distancia_new = entrenadorMasCerca(pokemon, buscar_entrenadores_new_disponibles());
			distancia_bloqued = entrenadorMasCerca(pokemon,buscar_entrenadores_bloqueados_disponibles());

			if(distancia_new->distancia < distancia_bloqued->distancia){
				moverColas(cola_NEW,cola_READY,distancia_new->entrenador);
				log_info(loggerTEAM,"Entrenador %d; Cambio de cola: NEW -> READY. Motivo: Listo para movilizarse hacia ubicacion de pokemon a atrapar", distancia_new->entrenador->id);
				//printf("voy a buscar un pokemon\n");
				t_pokemonObjetivo *pokemonsito= lista_objetivo->head->data;
				pokemonsito->cantidad=pokemonsito->cantidad -1; // debe hacerse cuando lo atrapa
				distancia_new->entrenador->pokemonCapturando = pokemon;
			}else{
				moverColas(cola_BLOQUED,cola_READY,distancia_bloqued->entrenador);
				distancia_bloqued->entrenador->pokemonCapturando = pokemon;
				log_info(loggerTEAM,"Entrenador %d; Cambio de cola: BLOCK -> READY. Motivo: Listo para movilizarse hacia ubicacion de pokemon a atrapar", distancia_bloqued->entrenador->id);
			}
			sem_post(&(sem_colas_no_vacias));
		}

		list_remove(listaPokemonsRecibidos,i);
	}
return;
}

void main_planificacion_caught(){

	t_list* entrenadores_esperando_caught;
	t_caught *caught;
	t_entrenador *entrenador;
	t_pokemonObjetivo *pokemonCaputrado;
	bool _filterEntrenadorCaught(t_entrenador *entrenador){
		return caught->idCorrelativo == entrenador->id_catch;
	}
	bool _filterPokemon(t_pokemonObjetivo *element){
		return !strcmp(element->pokemon,entrenador->pokemonCapturando->especie);
	}
	while(1){
		sem_wait(&sem_caught);
		entrenadores_esperando_caught = buscar_entrenadores_bloqueados_NOdisponibles();
		caught = list_get(listaPokemonesCaught,0);
		list_remove(listaPokemonesCaught,0);
		entrenador = list_find(entrenadores_esperando_caught,(void*)_filterEntrenadorCaught);
		if(entrenador!=NULL){
			entrenador->id_catch=0;
			if(caught->atrapado==OK){
				list_add(entrenador->pokemonesCapturados,entrenador->pokemonCapturando->especie);
				pokemonCaputrado = list_find(lista_objetivo,(void*)_filterPokemon);
				pokemonCaputrado->cantidad=pokemonCaputrado->cantidad -1;
			}
		}

	}
}

void main_planificacion_corto_plazo() {

	//1 semaforo espera recibidos
	//1 semaforo espera q el entrenador lo desbloquee
	while(1) {
		sem_wait(&(sem_cpu));
		sem_wait(&(sem_colas_no_vacias));
		t_entrenador* entrenadorACorrer ;
		if(!esRR()) {
			printf("calculo entrenador por fifo\n");
			entrenadorACorrer = planificacionFifo(cola_READY);
		}else {
			printf("calculo entrenador por RR\n");
			entrenadorACorrer = planificacionRR(cola_READY);
		}

		sem_post(&entrenadorACorrer->sem_entrenador);
	}

}

t_list *buscar_entrenadores_new_disponibles(){

	t_list *lista_libres;

		bool filtrado_NEW(t_entrenador *entrenador){
			return entrenador->id_catch == 0 && entrenador->espacioLibre > 0;
		}
	lista_libres = list_filter(cola_NEW,(void*)filtrado_NEW);
	return lista_libres;
}

t_list *buscar_entrenadores_bloqueados_disponibles(){

	t_list *lista_libres;

		bool filtrado_bloqueados(t_entrenador *entrenador){
			return entrenador->id_catch == 0 && entrenador->espacioLibre > 0;
		}
	lista_libres = list_filter(cola_BLOQUED,(void*)filtrado_bloqueados);
	return lista_libres;
}

t_list *buscar_entrenadores_bloqueados_NOdisponibles(){

	t_list *lista_libres;

		bool filtrado_bloqueados(t_entrenador *entrenador){
			return entrenador->id_catch != 0;
		}
	lista_libres = list_filter(cola_BLOQUED,(void*)filtrado_bloqueados);
	return lista_libres;
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
	sem_wait(&mutex_mov_colas_time);
	movimientoTime++;
	entrenador->seMovioEnTime = movimientoTime;
	sem_post(&mutex_mov_colas_time);
	list_add(destino, entrenador);

}

void agregarAColas(t_list* lista, t_entrenador* entrenador) {
	list_add(lista, entrenador);
}

t_entrenador* planificacionFifo(t_list* colaReady){
	t_entrenador* proximoAEjecutarReady = list_get(colaReady,0);

	return proximoAEjecutarReady;
}

t_entrenador* planificacionRR(t_list* colaReady){
	t_entrenador* proximoAEjecutar = planificacionFifo(colaReady); //la lista la saca igual q fifo

	proximoAEjecutar->movsDisponibles = configData->quantum;
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

	if(cantidadLista==0){
		distanciaResultado->entrenador=NULL;
		return distanciaResultado;
	}

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
