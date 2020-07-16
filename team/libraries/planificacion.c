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
			sem_wait(&sem_entrenador_disponible);
			distancia_new = entrenadorMasCerca(pokemon, buscar_entrenadores_new_disponibles());
			distancia_bloqued = entrenadorMasCerca(pokemon,buscar_entrenadores_bloqueados_disponibles());

			if(distancia_new->distancia < distancia_bloqued->distancia){
				if(esSJF()){
					calcular_rafaga(distancia_new->entrenador);
					}
				moverColas(cola_NEW,cola_READY,distancia_new->entrenador);
				log_info(loggerTEAM,"CAMBIO DE COLA; Entrenador %d: NEW -> READY. Motivo: Entrenador se prepara para moverse a la posicion del pokemon a capturar", distancia_new->entrenador->id);
				//printf("voy a buscar un pokemon\n");
				t_pokemonObjetivo *pokemonsito= lista_objetivo->head->data;
				//pokemonsito->cantidad=pokemonsito->cantidad -1; // debe hacerse cuando lo atrapa
				distancia_new->entrenador->pokemonCapturando = pokemon;
			}else{
				if(esSJF()){
					calcular_rafaga(distancia_bloqued->entrenador);
					}
				moverColas(cola_BLOQUED,cola_READY,distancia_bloqued->entrenador);
				distancia_bloqued->entrenador->pokemonCapturando = pokemon;
				log_info(loggerTEAM,"CAMBIO DE COLA; Entrenador %d: BLOCK -> READY. Motivo: Entrenador se prepara para moverse a la posicion del pokemon a capturar", distancia_bloqued->entrenador->id);
			}
			sem_post(&(sem_colas_no_vacias));

			free(distancia_new);
			free(distancia_bloqued);
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
		printf("se encontre entrenador %d esperando caught\n", entrenador->id);
		if(entrenador!=NULL){
			entrenador->id_catch=0;
			if(caught->atrapado==OK){
				printf("cantidad de pokemones capturados %d\n",list_size(entrenador->pokemonesCapturados));
				list_add(entrenador->pokemonesCapturados,entrenador->pokemonCapturando->especie);
				pokemonCaputrado = list_find(lista_objetivo,(void*)_filterPokemon);
				pokemonCaputrado->cantidad--;
				entrenador->espacioLibre--;
				if(entrenador->espacioLibre!=0){
					sem_post(&sem_entrenador_disponible);
				}
				printf("\nPokemon capturado!\n");
				printf("cantidad de pokemones capturados %d\n",list_size(entrenador->pokemonesCapturados));
				log_info(loggerTEAM,"CAPTURA; Entrenador %d:  Captura pokemon: %s en la posicion: X = %d Y = %d", entrenador->id, entrenador->pokemonCapturando->especie, entrenador->x, entrenador->y);
				if(list_is_empty(buscar_entrenadores_bloqueados_NOdisponibles(cola_BLOQUED))){
					sem_post(&sem_deadlcok);
				}
				mover_entrenador_bloqueado_a_exit(entrenador);
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
		if(esSJFsinDesalojo()){
			printf("caclulo entrenador por SJF SIN desalojo \n");
			entrenadorACorrer = planificacionSJFSD(cola_READY);
		}else if(esSJFconDesalojo()){
			printf("caclulo entrenador por SJF CON desalojo \n");
			entrenadorACorrer = planificacionSJFCD(cola_READY);
		}else if(!esRR()) {
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

void calcular_rafaga(t_entrenador * entrenador){
	float a = configData->alpha;
	entrenador->estimacion = (entrenador->rafagaReal)*a + (entrenador->estimacion)*(1-a);
	entrenador->estimacionRestante = entrenador->estimacion;
	printf("-----------------------entrenador:%d rafaga: %f\n",entrenador->id,entrenador->estimacion);
}


t_entrenador* buscar_entrenador_con_rafaga_mas_corta(){

		t_entrenador* proximoEntrenador;

		bool comparar_rafagas(t_entrenador * entrenador, t_entrenador * entrenador2){

			return entrenador->estimacionRestante <= entrenador2->estimacionRestante;
		}
		if(list_size(cola_READY)>1){
			list_sort(cola_READY, comparar_rafagas);
		}
		proximoEntrenador = list_get(cola_READY,0);
		printf("--------------------------------------------------------------entrenador %d que tiene estimacion de %f\n", proximoEntrenador->id, proximoEntrenador->estimacion);

		return proximoEntrenador;

}
/*
void desalojar_por_rafaga_mas_corta(t_entrenador* entrenadorReady){
	t_entrenador *entrenadorExec = list_get(cola_EXEC,0);


	if(entrenadorReady->estimacion < entrenadorExec->estimacionRestante){
		moverColas(cola_EXEC,cola_READY,entrenadorExec);
		moverColas(cola_READY,cola_EXEC,entrenadorReady);

	}

}
*/

void finalizarEntrenadorLuegoDeCaptura(t_entrenador* entrenador){



}


t_entrenador* planificacionSJFSD(t_list* colaReady){

	return buscar_entrenador_con_rafaga_mas_corta();

}

t_entrenador* planificacionSJFCD(t_list* colaReady){

	return buscar_entrenador_con_rafaga_mas_corta();

}
/*
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
*/
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

void detectar_deadlock(){
	while(1){
		printf("esperando deadlock\n");
		sem_wait(&sem_deadlcok);
		printf("analizando deadlock\n");
		if(todos_bloqueados() && todos_sin_espacio()){
			printf("estan bloqueados\n");
			log_info(loggerTEAM,"DEADLOCK; Inicio de algoritmo de deteccion de Deadlock");
			if(list_any_satisfy(cola_BLOQUED, tiene_otro_pokemon)){
				contar_deadlock_producido();
				log_info(loggerTEAM,"DEADLOCK; Se detecta un deadlock y se procede a resolverlo");
				main_deadlock();
			}else{
				log_info(loggerTEAM,"DEADLOCK; NO se detectan deadlocks");
				mover_bloqueados_a_exit();
			}
		}
	}
}

void mover_bloqueados_a_exit(){
	int size = list_size(cola_BLOQUED);
	t_entrenador * entrenador;
	for(int i = 0; i < size; i ++){
		entrenador = list_get(cola_BLOQUED,0);
		moverColas(cola_BLOQUED,cola_EXIT,entrenador);
		printf("entrenador %d se fue a EXIT \n", entrenador->id);
		log_info(loggerTEAM,"CAMBIO DE COLA; Entrenador %d: BLOCKED -> EXIT. Motivo: Entrenador cumple su objetivo!", entrenador->id);
		sem_post(&sem_exit);

	}
}

void mover_entrenador_bloqueado_a_exit(t_entrenador* enternador){

	if(enternador->espacioLibre==0){
		if(!tiene_otro_pokemon(enternador)){
			moverColas(cola_BLOQUED,cola_EXIT,enternador);
			log_info(loggerTEAM,"CAMBIO DE COLA; Entrenador %d: BLOCKED -> EXIT. Motivo: Entrenador cumple su objetivo!", enternador->id);
			sem_post(&sem_exit);
		}
	}


}

bool todos_bloqueados(){
	return list_is_empty(cola_READY) && list_is_empty(cola_NEW) && !list_is_empty(cola_BLOQUED) && list_is_empty(cola_EXEC);
}

bool todos_sin_espacio(){
	int size = list_size(cola_BLOQUED);
	t_entrenador * entrenador ;

	for(int i=0; i<size;i++){
		entrenador = list_get(cola_BLOQUED,i);
		if(entrenador->espacioLibre != 0)
			return false;
	}

	return true;
}

bool tiene_otro_pokemon(t_entrenador * entrenador){

	t_list* pokemonesObjetivos = crear_lista_deadlock(entrenador->pokemonesObjetivo);
	char* pokemonCapturado;
	int size = list_size(entrenador->pokemonesCapturados);
	bool result = false;
	bool _filterPokemonParaSacar(char* element){
			int resultado = !strcmp(element,pokemonCapturado);
			return resultado;
		}

	for(int i = 0; i < size; i++){
		pokemonCapturado = list_get(entrenador->pokemonesCapturados, i);
		list_remove_by_condition(pokemonesObjetivos,_filterPokemonParaSacar);


	}


	//!list_is_empty(pokemonesObjetivos)

	return !list_is_empty(pokemonesObjetivos) && entrenador->espacioLibre == 0;


}

t_list* crear_lista_deadlock(t_list* lista){

	t_list* listaNueva = list_create();
	int size = list_size(lista);
	for (int i = 0; i < size; i++){
		char* poke = malloc(strlen(list_get(lista,i))+1);
		poke = (char*)list_get(lista,i);
		list_add(listaNueva,poke);
	}

	return listaNueva;
}

char* pokemon_de_mas(t_entrenador* entrenador){
	char* pokemonObjetivo, *pokemon ;
	int size = list_size(entrenador->pokemonesObjetivo);


	bool _filterPokemon(char* element){
			return strcmp(element,pokemonObjetivo);
		}

	for(int i = 0; i < size; i++){
		pokemonObjetivo= list_get(entrenador->pokemonesObjetivo, i);
		pokemon = list_find(entrenador->pokemonesCapturados,_filterPokemon);

		if(pokemon != NULL){
			break;
		}
	}

	return pokemon;
}
char* sacar_pokemon_de_mas(t_entrenador* entrenador){
	char* pokemonObjetivo, *pokemon ;
	int size = list_size(entrenador->pokemonesObjetivo);


	bool _filterPokemon(char* element){
			return strcmp(element,pokemonObjetivo);
		}

	for(int i = 0; i < size; i++){
		pokemonObjetivo= list_get(entrenador->pokemonesObjetivo, i);
		pokemon = list_remove_by_condition(entrenador->pokemonesCapturados,_filterPokemon);

		if(pokemon != NULL){
			break;
		}
	}

	return pokemon;
}


void main_deadlock(){
	//sem_wait(&sem_deadlcok);
	t_entrenador* unEntrenador,*otroEntrenador;
	unEntrenador = list_get(cola_BLOQUED,0);
	char* pokemonDeMas = pokemon_de_mas(unEntrenador);
	otroEntrenador = busca_entrenador_que_necesita(pokemonDeMas);
	unEntrenador->entrenadorDeadlock = otroEntrenador;
	moverColas(cola_BLOQUED,cola_READY,unEntrenador);
	log_info(loggerTEAM,"CAMBIO DE COLA; Entrenador %d: BLOCKED -> READY. Motivo: Entrenador se prepara para moverse a la posicion del entrenador %d para realizar intercambio", unEntrenador->id, otroEntrenador->id);

	sem_post(&unEntrenador->sem_entrenador);

}

bool necesita_pokemon(t_entrenador * entrenador, char* pokemon){
	t_list* pokemonesObjetivos = crear_lista_deadlock(entrenador->pokemonesObjetivo);
	char* pokemonCapturado;
	int size = list_size(entrenador->pokemonesCapturados);


	bool _filterPokemon(char* element){
			return !strcmp(element,pokemonCapturado);
		}

	for(int i = 0; i < size; i++){
		pokemonCapturado = list_get(entrenador->pokemonesCapturados, i);
		list_remove_by_condition(pokemonesObjetivos,_filterPokemon);
	}

	pokemonCapturado = pokemon;
	return list_any_satisfy(pokemonesObjetivos,_filterPokemon);

	//limpiar_lista_deadlock(pokemonesObjetivos);



}

t_entrenador* busca_entrenador_que_necesita(char* pokemon){

	int size= list_size(cola_BLOQUED);
	t_entrenador* entrenador;
	for(int i = 0; i < size; i++){

		entrenador = list_get(cola_BLOQUED,i);
		if(necesita_pokemon(entrenador,pokemon))
			break;

	}

	return entrenador;
}

void main_exit(){

	while(1){
		sem_wait(&sem_exit);
		if(todos_terminados()){
			finalizar();
		}
	}


}

bool todos_terminados(){
	bool ready = list_is_empty(cola_READY);
	bool new = list_is_empty(cola_NEW);
	bool bloqued = list_is_empty(cola_BLOQUED);
	bool exec = list_is_empty(cola_EXEC);
	bool exit = !list_is_empty(cola_EXIT);

	return ready && new && bloqued && exec && exit;
}

void finalizar(){
	sem_wait(&sem_fin);
	t_entrenador * entrenador;

	log_info(loggerTEAM,"");
	log_info(loggerTEAM,"=================================================================");
	log_info(loggerTEAM,"");
	log_info(loggerTEAM,"					HAS ATRAPADO A TODOS LOS POKEMONES!!!");
	log_info(loggerTEAM,"");
	log_info(loggerTEAM,"=================================================================");
	log_info(loggerTEAM,"=================================================================");
	log_info(loggerTEAM,"");
	log_info(loggerTEAM,"	RESULTADO DEL TEAM:");
	log_info(loggerTEAM,"");
	log_info(loggerTEAM,"		- Cantidad de ciclos de CPU totales: %d", ciclosCPU);
	log_info(loggerTEAM,"		- Cantidad de cambios de contexto realizados: %d", contextSwitch);
	log_info(loggerTEAM,"		- Cantidad de ciclos de CPU realizados por entrenador:");
	for(int i = 0; i < list_size(cola_EXIT); i++){
		entrenador = list_get(cola_EXIT,i);
		log_info(loggerTEAM,"     		· Entrenador %d => Ciclos: %d", entrenador->id, entrenador->ciclos);
	}
	log_info(loggerTEAM,"		- Cantidad de Deadlocks producidos: %d", deadlocksProducidos);
	log_info(loggerTEAM,"		- Cantidad de Deadlocks resueltos: %d", deadlocksResueltos);
	log_info(loggerTEAM,"");
	log_info(loggerTEAM,"=================================================================");

	finalizar_y_liberar();
}



