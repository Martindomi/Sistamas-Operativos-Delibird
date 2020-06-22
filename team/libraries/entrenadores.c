
#include "entrenadores.h"
#include "../team.h"

//lista poke recibidos/P1 P2 (P3 no se anali)
//NEW/
//READY/E1
//EXEC/ E2
void main_entrenador(t_entrenador* entrenador){
	while(1) {
		sem_wait(&(entrenador->sem_entrenador));
		moverColas(cola_READY, cola_EXEC, entrenador);
		moverEntrenador(entrenador);
		analizarCaptura(entrenador);
		//TODO
		//Ak hay q comparar si tiene todos los de el, o no objs == capturados
		if(list_size(entrenador->pokemonesObjetivo)!=list_size(entrenador->pokemonesCapturados)){
			//Ak tiene q ir a block
			printf("Entrenador %d: me quedan pokemons, vuelvo a ready\n", entrenador->id);
		}else {
			moverColas(cola_READY,cola_EXIT, entrenador);
			printf("Entrenador %d: no me quedan pokemons, me cierro\n", entrenador->id);
		}
		sem_post(&(sem_cpu));
	}
}

//cuando recibe caught esto
void capturoPokemon(t_entrenador* entrenador){
	bool _filterPokemon(char* pokemonNombre){
		return !strcmp(entrenador->pokemonCapturando->especie,pokemonNombre);
	}

	t_list* pokeObj = entrenador->pokemonesObjetivo;
	t_list* pokeCap = entrenador->pokemonesCapturados;
	entrenador->espacioLibre = entrenador->espacioLibre-1;

	char* pokemonCapturado = list_filter(pokeObj,(void*)_filterPokemon);
	list_add(pokeCap,pokemonCapturado);
}

void inicializar_entrenadores (t_list* entrenadores_list){

	int i=0;
	int cant_objetivo, cant_capturado;
	char ** posiciones;
	t_entrenador *unEntrenador;

	//t_config *config = config_create("/home/utnso/tp-2020-1c-Elite-Four/team/team.config");

	char** read_posiciones= configData->posicionesEntrenadores;//config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** read_pokemones= configData->pokemonesEntrenadores;//config_get_array_value(config,"POKEMON_ENTRENADORES");
	char** read_objetivos= configData->objetivosEntrenadores;//config_get_array_value(config,"OBJETIVOS_ENTRENADORES");
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
		unEntrenador->pokemonesCapturados = list_create();
		unEntrenador->pokemonesObjetivo = list_create();
		if(strcmp(read_pokemones[i],"")==0){
			//printf("NO HAY\n");
		} else {
			char** tempCapt = string_split(read_pokemones[i], "|");
			int capturadosContador = 0;
			while(tempCapt[capturadosContador]!=NULL) {
				list_add(unEntrenador->pokemonesCapturados,tempCapt[capturadosContador]);
				capturadosContador++;
			}

		}
		//printf("%s\n",*unEntrenador.pokemonesCapturados);
		//puts(*(unEntrenador.pokemonesCapturados +1));
		//puts(*(unEntrenador.pokemonesCapturados+2));

		char** tempObjetivos = string_split(read_objetivos[i], "|");
		int objetivoContador = 0;
		while(tempObjetivos[objetivoContador]!=NULL) {
			list_add(unEntrenador->pokemonesObjetivo,tempObjetivos[objetivoContador]);
			objetivoContador++;
		}


		cargarObjetivosGlobales(unEntrenador->pokemonesObjetivo);
		//puts(*(unEntrenador.pokemonesObjetivo));
		//puts(*(unEntrenador.pokemonesObjetivo +1));
		//puts(*(unEntrenador.pokemonesObjetivo +2));
		//puts(*(unEntrenador.pokemonesObjetivo +3));


		unEntrenador->espacioLibre = list_size(unEntrenador->pokemonesObjetivo) - list_size(unEntrenador->pokemonesCapturados);

		sem_init(&(unEntrenador->sem_entrenador),0,0);
		//printf("%d\n",unEntrenador.espacioLibre);

		//list_add(entrenadores_list,unEntrenador);
		agregarAColas(entrenadores_list,unEntrenador);

		i++;

		//t_entrenador *entrenador = list_get(entrenadores_list,0);
		//printf("%d\n", entrenador->x);


		liberarArrayDeStrings(posiciones);

	}

//	imprimirListaObjetivo();


	quitarPokemonesDeListaObjetivo(entrenadores_list);

	imprimirListaObjetivo();



	liberarArrayDeStrings(read_objetivos);
	liberarArrayDeStrings(read_pokemones);
	liberarArrayDeStrings(read_posiciones);

	//config_destroy(config);

}

void liberarArrayDeStrings(char** options){
	int j=0;
	while(options[j]!=NULL){
		free(options[j]);
		j++;
	}free(options);
}

void imprimirListaEntrenadores(t_list* entrenadores_list){

	int largoLista = list_size(entrenadores_list);

	for (int i = 0; i < largoLista; i++ ) {

	t_entrenador *entrenador = list_get(entrenadores_list,i);

	imprimirEntrenador(entrenador);

	}
}

void imprimirEntrenador(t_entrenador* entrenador) {
	printf("----------\n");
	printf("ID: %d\n", entrenador->id);
	printf("Espacio libre %d\n", entrenador->espacioLibre);
	printf("ESTADO: %d\n", entrenador->estado);
	printf("X: %d\n", entrenador->x);
	printf("Y: %d\n", entrenador->y);
	printf("EL entrenador capturo el pokemon %s \n",list_get(entrenador->pokemonesCapturados,0));
	printf("El entrenador necesita un %s \n", list_get(entrenador->pokemonesObjetivo,0));
	puts("");
}

void imprimirListaObjetivo(){

	printf("------------\n");
	for(int i = 0; i < list_size(lista_objetivo);i++){

		t_pokemonObjetivo *poke = list_get(lista_objetivo,i);
		printf("lista %s, %d\n",poke->pokemon,poke->cantidad);

	}

	printf("------------\n");
}

void crearListaObjetivo(){

	if(lista_objetivo==NULL){
		printf("crea lista\n");
		lista_objetivo=list_create();
	}
	printf("lista craeda\n");



}

void cargarObjetivosGlobales(t_list* pokemones){

	list_iterate(pokemones,(void*) agregarPokemonALista);
	/*int i = 0;
	while(list_get(pokemons[i]!=NULL){

		agregarPokemonALista(pokemones[i]);
		i++;

	}*/
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
	while(list_get(entrenador->pokemonesCapturados,j)!=NULL){
		quitarPokemonDeLista(list_get(entrenador->pokemonesCapturados,j));
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

void moverEntrenador(t_entrenador* entrenador) {
	int xDestino =  entrenador->pokemonCapturando->x;
	int yDestino =  entrenador->pokemonCapturando->y;
	int cantidadAMoverseX = xDestino - entrenador->x;
	int cantidadAMoverseY = yDestino - entrenador->y;
	bool esRRo = esRR();
	int cantDeMovs;
	//Si entrenador->movsDisponibles = 0 entonces tiene movs infinitos(FIFO, SJF)
	//Si entrenador->movsDisponibles != 0 entonces es RR
	printf("Inicio Entrenador: %d, en posiciones X:%d Y:%d\n", entrenador->id, entrenador->x, entrenador->y);
	for(cantDeMovs = 0;cantDeMovs<(abs(cantidadAMoverseX) + abs(cantidadAMoverseY)) && (cantDeMovs<entrenador->movsDisponibles || !esRRo); cantDeMovs++){
		if(xDestino != entrenador->x) {
			int direccionEnX = cantidadAMoverseX/abs(cantidadAMoverseX);
			entrenador->x = entrenador->x + direccionEnX;
		}else if(yDestino != entrenador->y) {
			int direccionEnY = cantidadAMoverseY/abs(cantidadAMoverseY);
			entrenador->y = entrenador->y + direccionEnY;
		}else {
			//Llego, no deberia entrar ak, hay algo mal
			exit(6);
		}

		printf("Se mueve entrenador %d a X:%d Y:%d\n",entrenador->id,entrenador->x, entrenador->y);
		sleep(configData->retardoCicloCPU);


	}
}
void analizarCaptura(t_entrenador* entrenador) {
	if(entrenador->x == entrenador->pokemonCapturando->x && entrenador->y == entrenador->pokemonCapturando->y){
		enviar_mensaje_catch_pokemon(entrenador, entrenador->pokemonCapturando->especie,entrenador->x ,entrenador->y);
		moverColas(cola_EXEC,cola_BLOQUED,entrenador);
		//wait()
		//lo capturo
		//mando catch de ak?, y paso a block
		//capturoPokemon(entrenador);
	}else {
		printf("termino RR, vuelve a ready\n");
		moverColas(cola_EXEC,cola_READY,entrenador);
		sem_post(&sem_colas_no_vacias);
	}

}


