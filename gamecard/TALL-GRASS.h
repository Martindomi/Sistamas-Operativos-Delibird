
#ifndef TALL_GRASS_H_
#define TALL_GRASS_H_
#include <stdio.h>
#include <stdlib.h>

#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <sys/stat.h>

#define PUERTO "PUERTO"
#define IP "IP"
#define PTO_MONTAJE "PTO_MONTAJE"
#define CONFIG_FIELDS_N_ 3
#define configuracionFS "../gamecard.config"

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


char* ptoMontaje;
int block_size;
int blocks;
char* magic_number;


t_log *logger;
t_bitarray* bitmap;
t_configFS* configTG;
t_config*config;

#endif /* TALL_GRASS_H_ */
