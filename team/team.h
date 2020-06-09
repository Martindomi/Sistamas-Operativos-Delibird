#include "libraries/entrenadores.h"
#include "libraries/planificacion.h"
#include "libraries/utils.h"
#include <conexiones.h>

sem_t sem_cpu;


void aplica_funcion_escucha(int * socket);
void crear_hilo_entrenadores(t_list* listaEntrenadores);
t_mensajeTeam esperoMensaje();



