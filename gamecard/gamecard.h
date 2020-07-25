#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include<conexiones.h>
#include "utils.h"
#include "TALL-GRASS.h"
#include <commons/collections/list.h>

void funcion_NEW_POKEMON();
void funcion_CATCH_POKEMON();
void funcion_GET_POKEMON();


int aplica_funcion_escucha(int * socket);


t_dictionary* dicSemaforos;
sem_t semDict;
sem_t semNEW;
sem_t semGET;
sem_t semCATCH;
sem_t mutexNEW;
sem_t mutexCATCH;
sem_t mutexGET;

t_list* lista_NEW;
t_list* lista_GET;
t_list* lista_CATCH;


