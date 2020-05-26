
#include "entrenadores.h"


void inicializar_entrenadores (t_config *config, t_list* entrenadores_list){

	int i=0;
	int cant_objetivo, cant_capturado;
	char ** posiciones;
	t_entrenador *unEntrenador;

	char** read_posiciones= config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** read_pokemones= config_get_array_value(config,"POKEMON_ENTRENADORES");
	char** read_objetivos= config_get_array_value(config,"OBJETIVOS_ENTRENADORES");

	while(read_posiciones[i]!= NULL){

		unEntrenador = malloc(sizeof(t_entrenador));
		unEntrenador->estado= NEW;

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


		//puts(*(unEntrenador.pokemonesObjetivo));
		//puts(*(unEntrenador.pokemonesObjetivo +1));
		//puts(*(unEntrenador.pokemonesObjetivo +2));
		//puts(*(unEntrenador.pokemonesObjetivo +3));


		cant_objetivo = calcularCantidadLista(unEntrenador->pokemonesObjetivo);
		cant_capturado = calcularCantidadLista(unEntrenador->pokemonesCapturados);

		unEntrenador->espacioLibre = cant_objetivo - cant_capturado;

		sem_init(&(unEntrenador->sem_entrenador),0,0);
		//printf("%d\n",unEntrenador.espacioLibre);

		list_add(entrenadores_list,unEntrenador);


		i++;

		//t_entrenador *entrenador = list_get(entrenadores_list,0);
		//printf("%d\n", entrenador->x);


		liberarArrayDeStrings(posiciones);

	}

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
	printf("Espacio libre %d\n", entrenador->espacioLibre);
	printf("ESTADO: %d\n", entrenador->estado);
	printf("X: %d\n", entrenador->x);
	printf("Y: %d\n", entrenador->y);
	printf("EL entrenador capturo el pokemon %s \n",entrenador->pokemonesCapturados[0]);
	printf("El entrenador necesita un %s \n", entrenador->pokemonesObjetivo[1]);
	puts("");

	}
}

void *main_entrenador(t_entrenador* entrenador){

	printf("Posicion entrenador: %d %d\n", entrenador->x, entrenador->y);
	sem_wait(&(entrenador->sem_entrenador));
	printf("Se desbloque esta cosa como dijo el sino no va andar\n");
	list_add(cola_EXIT,entrenador);
	list_remove(cola_READY,0);

}

