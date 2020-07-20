
#ifndef TALL_GRASS_H_
#define TALL_GRASS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "gamecard.h"

#define PUERTO "PUERTO"
#define IP "IP"
#define PTO_MONTAJE "PTO_MONTAJE"
#define NO_MORE_BLOCKS -1
#define CONFIG_FIELDS_N_ 3
#define configuracionFS "/home/utnso/tp-2020-1c-Elite-Four/gamecard/gamecard.config"
#define AGREGAR 0
#define MODIFICAR 1

/*
#define BLOCK_SIZE "BLOCK_SIZE"
#define BLOCKS "BLOCKS"
#define MAGIC_NUMBER "MAGIC_NUMBER"

#define DIRECTORY
#define SIZE
#define L_BLOCKS
#define OPEN

#define TAM_BLOCKS "TAM_BLOCKS"
#define CANT_BLOCKS "CANT_BLOCKS"
*/
typedef struct{
	char* ptoEscucha;
	char* ptoMontaje;
	int block_size;
	int blocks;
}t_configFS;


t_bitarray* bitmap;
t_configFS* configTG;
t_config*config;

#endif /* TALL_GRASS_H_ */
