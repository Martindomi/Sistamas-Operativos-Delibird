#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <semaphore.h>

#define  THREAD_POOL 5
pthread_t  thread_pool[THREAD_POOL];

sem_t mutex_sem;
sem_t mutex_envio;

t_log *logger_broker;
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
void distribuir_mensaje_sin_enviar_a(char* suscriptor, int cola, puntero_mensaje puntero_mensaje);
void* distribuir_mensajes(void* puntero_cola);

int cantidad_mensajes;

#endif /* SERVIDOR_H_ */
