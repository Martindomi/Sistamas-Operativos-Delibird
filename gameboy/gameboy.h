
#ifndef TP0_H_
#define TP0_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<conexiones.h>

char* ip_broker;
char* puerto_broker;
char* ip_team;
char* puerto_team;
char* ip_gamecard;
char* puerto_gamecard;
char* ip_gameboy;
char* puerto_gameboy;
char* ACK;

t_log* logger_gameboy;
uint32_t obtener_cola_mensaje(char* cola_string);
#endif /* TP0_H_ */
