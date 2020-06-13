#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include<conexiones.h>

#include "TALL-GRASS.h"

char* ip_broker;
char* puerto_broker;
char* ip_gamecard;
char* puerto_gamecard;
char* ptoMontaje;
int block_size;
int blocks;
t_log *logger;
