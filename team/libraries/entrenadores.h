/*
 * entrenadores.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef LIBRARIES_ENTRENADORES_H_
#define LIBRARIES_ENTRENADORES_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

typedef enum
{

	NEW=1,
	READY= 2,
	EXEC= 3,
	BLOCK= 4,
	EXIT= 5

}t_estado;

typedef struct {

	int x;
	int y;
	char** pokemonesCapturados;
	char** pokemonesObjetivo;
	int espacioLibre;
	t_estado estado;

}t_entrenador;


void inicializar_entrenadores (t_config*,t_list* );
int calcularCantidadLista(char**);
void imprimirLista(t_list*);


#endif /* LIBRARIES_ENTRENADORES_H_ */
