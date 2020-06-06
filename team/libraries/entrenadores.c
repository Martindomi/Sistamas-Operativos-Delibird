
#include "entrenadores.h"


void inicializar_entrenadores (t_config *config, t_list* entrenadores_list){

	int i=0;
	int cant_objetivo, cant_capturado;
	char ** posiciones;
	t_entrenador *unEntrenador;

	char** read_posiciones= config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** read_pokemones= config_get_array_value(config,"POKEMON_ENTRENADORES");
	char** read_objetivos= config_get_array_value(config,"OBJETIVOS_ENTRENADORES");
	int id = 0;
	crearListaObjetivo();

	while(read_posiciones[i]!= NULL){
		id++;
		printf("cargando entrenador %d\n",id);
		unEntrenador = malloc(sizeof(t_entrenador));
		unEntrenador->estado= NEW;
		unEntrenador->id = id;
		posiciones = string_split(read_posiciones[i], "|");
		unEntrenador->x = atoi(*(posiciones));
		unEntrenador->y = atoi(*(posiciones+1));

		//printf("%d\n",unEntrenador->x);
		//printf("%d\n",unEntrenador->y);

		if(strcmp(read_pokemones[i],"")==0){
			//printf("NO HAY\n");
		}
		else
		{
		unEntrenador->pokemonesCapturados= string_split(read_pokemones[i], "|");
		}
		//printf("%s\n",*unEntrenador.pokemonesCapturados);
		//puts(*(unEntrenador.pokemonesCapturados +1));
		//puts(*(unEntrenador.pokemonesCapturados+2));


		unEntrenador->pokemonesObjetivo= string_split(read_objetivos[i], "|");
		cargarObjetivosGlobales(unEntrenador->pokemonesObjetivo);

		//puts(*(unEntrenador.pokemonesObjetivo));
		//puts(*(unEntrenador.pokemonesObjetivo +1));
		//puts(*(unEntrenador.pokemonesObjetivo +2));
		//puts(*(unEntrenador.pokemonesObjetivo +3));


		cant_objetivo = calcularCantidadLista(unEntrenador->pokemonesObjetivo);
		cant_capturado = calcularCantidadLista(unEntrenador->pokemonesCapturados);

		unEntrenador->espacioLibre = cant_objetivo - cant_capturado;

		sem_init(&(unEntrenador->sem_entrenador),0,0);
		//printf("%d\n",unEntrenador.espacioLibre);

		//list_add(entrenadores_list,unEntrenador);
		agregarAColas(entrenadores_list,unEntrenador);

		i++;

		//t_entrenador *entrenador = list_get(entrenadores_list,0);
		//printf("%d\n", entrenador->x);


		liberarArrayDeStrings(posiciones);

	}

	imprimirListaObjetivo();


	quitarPokemonesDeListaObjetivo(entrenadores_list);

	imprimirListaObjetivo();



	liberarArrayDeStrings(read_objetivos);
	liberarArrayDeStrings(read_pokemones);
	liberarArrayDeStrings(read_posiciones);



}


void liberarArrayDeStrings(char** options){
	int j=0;
	while(options[j]!=NULL){
		free(options[j]);
		j++;
	}free(options);
}


int calcularCantidadLista(char **lista){

	int i = 0;
	char** aux = lista;

	while(*(aux + i) != NULL){
		i++;
	}

	//printf("%d\n",i);
	return i;

}

void imprimirLista(t_list* entrenadores_list){

	int largoLista = list_size(entrenadores_list);

	for (int i = 0; i < largoLista; i++ ) {

	t_entrenador *entrenador = list_get(entrenadores_list,i);
	printf("ID: %d\n", entrenador->id);
	printf("Espacio libre %d\n", entrenador->espacioLibre);
	printf("ESTADO: %d\n", entrenador->estado);
	printf("X: %d\n", entrenador->x);
	printf("Y: %d\n", entrenador->y);
	printf("EL entrenador capturo el pokemon %s \n",entrenador->pokemonesCapturados[0]);
	printf("El entrenador necesita un %s \n", entrenador->pokemonesObjetivo[1]);
	puts("");

	}
}

void imprimirListaObjetivo(){

	printf("------------\n");
	for(int i = 0; i < list_size(lista_objetivo);i++){

		t_pokemonObjetivo *poke = list_get(lista_objetivo,i);
		printf("lista %s, %d\n",poke->pokemon,poke->cantidad);

	}

	printf("------------\n");
}

void imprimerEntrenador(t_entrenador* entrenador) {
	printf("ID: %d\n", entrenador->id);
	printf("Espacio libre %d\n", entrenador->espacioLibre);
	printf("ESTADO: %d\n", entrenador->estado);
	printf("X: %d\n", entrenador->x);
	printf("Y: %d\n", entrenador->y);
	printf("EL entrenador capturo el pokemon %s \n",entrenador->pokemonesCapturados[0]);
	printf("El entrenador necesita un %s \n", entrenador->pokemonesObjetivo[1]);
	puts("");
}

void *main_entrenador(t_entrenador* entrenador){

	printf("Posicion entrenador: %d %d\n", entrenador->x, entrenador->y);
	sem_wait(&(entrenador->sem_entrenador));
	printf("Se desbloque esta cosa como dijo el sino no va andar\n");
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


void crearListaObjetivo(){

	if(lista_objetivo==NULL){
		printf("crea lista\n");
		lista_objetivo=list_create();
	}
	printf("lista craeda\n");



}

void cargarObjetivosGlobales(char ** pokemones){

	int i = 0;
	while(pokemones[i]!=NULL){

		agregarPokemonALista(pokemones[i]);
		i++;

	}
}



void agregarPokemonALista(char* pokemon){

	t_pokemonObjetivo *pokemonObjetivo=malloc(sizeof(pokemonObjetivo));

	//strcpy(pokemonObjetivo->pokemon,pokemon);

	t_pokemonObjetivo *pokemonBuscado = buscarPokemon(pokemon);



	if(pokemonBuscado==NULL){

		pokemonObjetivo->pokemon=malloc(strlen(pokemon)+1);
		strcpy(pokemonObjetivo->pokemon,pokemon);
		//pokemonObjetivo->pokemon = pokemon;
		pokemonObjetivo->cantidad = 1;
		list_add(lista_objetivo,pokemonObjetivo);


	}else{

		pokemonBuscado->cantidad = pokemonBuscado->cantidad +1;

	}



}

void quitarPokemonesDeListaObjetivo(t_list* entrenadores_list){

	for(int i = 0 ; i< list_size(entrenadores_list) ; i++){


	t_entrenador *entrenador = list_get(entrenadores_list, i);

	int j=0;
	while(entrenador->pokemonesCapturados[j]!=NULL){
		quitarPokemonDeLista((entrenador->pokemonesCapturados[j]));
		j++;
	}

}

}
void quitarPokemonDeLista(char* pokemon){

	t_pokemonObjetivo *pokemonBuscado = buscarPokemon(pokemon);

	pokemonBuscado->cantidad = pokemonBuscado->cantidad -1;
}

t_pokemonObjetivo *buscarPokemon(char* pokemon)
{


	bool _filterPokemon(t_pokemonObjetivo *element){
		return !strcmp(element->pokemon,pokemon);
	}

	t_pokemonObjetivo *poke =list_find(lista_objetivo,(void*)_filterPokemon);
	 return poke;


}




