#include "libraries/entrenadores.h"
#include <conexiones.h>

void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger);
void suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger);
void crear_thread_suscripcion(char* ip, char* port);
void esperar_mensajes_broker(int socket_servidor);
void recibe_mensaje_broker(int* socket);
void enviar_menssaje_new_pokemon(t_log* logger, char* ip, char* puerto);
void* hilo_escucha(int);
