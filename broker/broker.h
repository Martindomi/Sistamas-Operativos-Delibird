#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include <commons/config.h>
#include <commons/log.h>

t_log *logger_broker;

typedef struct {
	t_list* suscriptores;
	t_list* mensajes;
} t_cola_mensaje;


t_cola_mensaje* new_pokemon;
t_cola_mensaje* appeared_pokemon;

int cantidad_mensajes;

#endif /* SERVIDOR_H_ */
