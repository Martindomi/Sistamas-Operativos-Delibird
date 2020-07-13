#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include<math.h>
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <semaphore.h>
#include <time.h>

#define  THREAD_POOL 6
#define  SEM_POOL 8

pthread_t  thread_pool[THREAD_POOL];

sem_t mutexLista[SEM_POOL];
sem_t mutexDistribucion;
sem_t mutexIds;

t_log* loggerBroker;
t_config* configBroker;
char* ipBroker;
char* puertoBroker;
int tamanoMemoria;
int tamanoMinimoParticion;
char* algoritmoMemoria;
char* algoritmoReemplazo;
char* algoritmoParticionLibre;
char* algoritmoBuddySystem;
char* bsAlgoritmoReemplazo;
int frecuenciaCompactacion;
char* logFile;

int cantidadBusquedasFallidas;

int* punteroMemoriaPrincipal;

typedef struct {
	uint32_t id;
	uint32_t idCorrelativo;
	int colaMensaje;
	int* punteroMemoria;
	bool ocupada;
	bool izq;
	bool der;
	t_list* historicoBuddy;
	uint32_t tamanoMensaje;
	t_list* suscriptores_enviados;
	t_list* suscriptores_ack;
	time_t lruHora;
} t_particion;
typedef t_particion* punteroParticion;

t_list* particiones;

pthread_t thread;

typedef struct {
	t_list* suscriptores;
	t_list* mensajes;
} t_cola_mensaje;

t_cola_mensaje* new_pokemon;
t_cola_mensaje* appeared_pokemon;
t_cola_mensaje* get_pokemon;
t_cola_mensaje* localized_pokemon;
t_cola_mensaje* catch_pokemon;
t_cola_mensaje* caught_pokemon;

int mensajes_nuevos();
void distribuir_mensajes_cola(int cola);
t_cola_mensaje* selecciono_cola(int cola);
void distribuir_mensaje_sin_enviar_a(char* suscriptor, int cola, puntero_mensaje puntero_mensaje_completo);
void* distribuir_mensajes(void* puntero_cola);
bool fue_respondido(t_mensaje* mensaje_completo, t_cola_mensaje* cola_mensaje);
void* buscar_memoria_libre(t_mensaje* mensajeCompleto, uint32_t colaMensaje);
void* buscar_memoria_libre_first_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje);
void* buscar_memoria_libre_best_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje);

void* bs_segun_algoritmo(t_mensaje* mensajeCompleto, uint32_t colaMensaje);
void* bs_first_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje);
void* bs_best_fit(t_mensaje* mensajeCompleto, uint32_t colaMensaje);
int potencia_de_dos_cercana(uint32_t tamanioMensaje);
void dividir_particiones(punteroParticion particionInicial, int index ,uint32_t tamanioNecesario);
void bs_consolidar();
void bs_eliminar_particion();
void bs_eliminar_particion_fifo();
void bs_eliminar_particion_lru();
//int primer_puntero_desocupado();


void compactar_memoria();
void eliminar_particion();
int eliminar_particion_fifo();
int eliminar_particion_lru();
void intercambio_particiones(punteroParticion punteroParticionDesocupada,
		punteroParticion punteroParticionOcupada);
punteroParticion buscar_particion_mensaje(uint32_t idMensaje);

int cantidad_mensajes;

#endif /* SERVIDOR_H_ */
