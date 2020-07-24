
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

sem_t mutexBitmap;

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

char* generar_linea_de_entrada_mensaje(int posX, int posY, int cant);
char* generar_path_directorio_pokemon(char* pokemon);
char* generar_path_archivo_pokemon_metadata(char*pokemon);
char* generar_path_bloque(char* bloque);
char** obtener_array_de_bloques(char*path);
int tam_array_bloques(char** bloques);
int buscar_block_disponible();
int crear_block();
int cantidad_digitos(int numero);
int validar_existencia_archivo(char*path);
void actualizar_tamanio_archivo(char*path);
void agregar_block_al_metadata(int block, char* pathPokemon);
void crear_directorio(char*path);
void crear_metadata_directorio(char* dir);
int archivo_abierto(char* path);
void abrir_archivo(char*path);
void cerrar_archivo(char*path);
void iniciar_metadata();
void actualizar_bitmap(bool valor, int pos);
void iniciar_bitmap();
int bitmap_vacio(char*path);
void iniciar_files_dir();
void iniciar_blocks_dir();
void iniciar_metadata_dir();
void montar_punto_montaje();
void iniciar_filesystem();
t_configFS* levantar_configuracion_filesystem(char* archivo);
t_configFS* crear_config(int argc, char* argv[]);
void crear_archivo_pokemon_metadata(char* pokemon, char* mensaje);
void quitar_bloque_de_metadata(char*path, char* bloque);
void vaciar_bloque(char* bloque);
void liberar_bloque(char* bloque, char*pathPokemon);
int tamanio_real_archivo(char*pathPokemon);
int tamanio_ocupado_bloque(char*path);
int tam_disponible_en_bloque(char*path);
int block_completo(char*path);
int calcular_tamanio_archivo(char*path) ;
void escribir_mensaje_en_block(int bloque, char* mensaje, int accion);
void agregar_nuevo_mensaje(char* mensaje, char*pathPokemon);
void tratar_contenido_en_bloques(char*contenido, char* pathPokemon);
void tratar_mensaje_NEW_POKEMON(int posX, int posY, int cant, char* pokemon);
char* tratar_mensaje_CATCH_POKEMON(int posX, int posY, char*pokemon);
t_list* tratar_mensaje_GET_POKEMON(char*pokemon);

#endif /* TALL_GRASS_H_ */
