#include "libraries/entrenadores.h"
#include "libraries/planificacion.h"
#include "libraries/utils.h"
#include <conexiones.h>
#include <fcntl.h>
#include <errno.h>

t_list* ids_mensajes_enviados;
char* ACK;



void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger);
void crear_thread_suscripcion(op_code op_code, char* ip_broker, char* port_broker, char* port_team, t_log* logger);
void esperar_mensajes_broker(int socket_servidor);
void enviar_mensaje_new_pokemon(t_log* logger, char* ip, char* puerto);
void enviar_mensaje_new_pokemon2(t_log* logger, char* ip, char* puerto);
sem_t sem_cpu;

void mover_entrenador_new_sin_espacio(t_entrenador* enternador);
int aplica_funcion_escucha(int * socket);
void crear_hilo_entrenadores(t_list* listaEntrenadores);
t_mensajeTeam esperoMensaje();
