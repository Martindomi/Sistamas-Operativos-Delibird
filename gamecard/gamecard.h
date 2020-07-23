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

void funcion_NEW_POKEMON(puntero_mensaje mensajeRecibido);
void funcion_CATCH_POKEMON(puntero_mensaje mensajeRecibido);
void funcion_GET_POKEMON(puntero_mensaje mensajeRecibido);

int aplica_funcion_escucha(int * socket);


t_dictionary* dicSemaforos;
sem_t semDict;
