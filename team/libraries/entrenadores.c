
#include "entrenadores.h"


void inicializar_entrenadores (t_config *config, t_entrenador unEntrenador, t_list* entrenadores_list){

	int i=0;
	int cant_objetivo, cant_capturado;
	char ** list_values;

	char** read_posiciones= config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** read_pokemones= config_get_array_value(config,"POKEMON_ENTRENADORES");
	char** read_objetivos= config_get_array_value(config,"OBJETIVOS_ENTRENADORES");


	while(read_posiciones[i]!= NULL){

		unEntrenador.estado= NEW;

		list_values = string_split(read_posiciones[i], "|");
		unEntrenador.x = atoi(*(list_values));
		unEntrenador.y = atoi(*(list_values+1));

		//printf("%d\n",unEntrenador.x);
		//printf("%d\n",unEntrenador.y);

		unEntrenador.pokemonesCapturados= string_split(read_pokemones[i], "|");

		//puts(*unEntrenador.pokemonesCapturados);
		//puts(*(unEntrenador.pokemonesCapturados +1));
		//puts(*(unEntrenador.pokemonesCapturados+2));


		unEntrenador.pokemonesObjetivo= string_split(read_objetivos[i], "|");


		//puts(*(unEntrenador.pokemonesObjetivo));
		//puts(*(unEntrenador.pokemonesObjetivo +1));
		//puts(*(unEntrenador.pokemonesObjetivo +2));
		//puts(*(unEntrenador.pokemonesObjetivo +3));


		cant_objetivo = calcularCantidadLista(unEntrenador.pokemonesObjetivo);
		cant_capturado = calcularCantidadLista(unEntrenador.pokemonesCapturados);

		unEntrenador.espacioLibre = cant_objetivo - cant_capturado;
		//printf("%d\n",unEntrenador.espacioLibre);

		list_add(entrenadores_list,&unEntrenador);
		printf("%d\n",list_size(entrenadores_list));

		i++;



	}

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
