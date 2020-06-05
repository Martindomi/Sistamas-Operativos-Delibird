#include "libraries/entrenadores.h"
#include <conexiones.h>

typedef struct {
	int x,y;
	char* especie;
} t_pokemon;

typedef struct {
	t_pokemon* pokemon;
	t_entrenador* entrenador;
	double distancia;
} t_distancia;


void suscribirse_cola(op_code codigo_cola, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger);
void suscribir(op_code codigo_operacion, char* ip_broker, char* puerto_broker, char* puerto_thread_team, t_log* logger);
void crear_thread_suscripcion(char* ip, char* port);
void esperar_mensajes_broker(int socket_servidor);
void recibe_mensaje_broker(int* socket);
void enviar_menssaje_new_pokemon(t_log* logger, char* ip, char* puerto);
void* hilo_escucha(int);
t_distancia* entrenadorMasCerca(t_pokemon* pokemonNuevo,t_list* lista);
double calcularDistancia(t_entrenador* entrenador, t_pokemon* pokemon);
int planificacionFifo(t_list* colaReady);
