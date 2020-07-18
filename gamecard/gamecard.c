#include "gamecard.h"
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
//#include "TALL-GRASS.c"


int main (int argc, char *argv[]) {

	logger = log_create("gamecard.log", "GAMECARD", false, LOG_LEVEL_INFO);

	/*
	t_config* config;


	config = config_create("gamecard.config");
	ip_gamecard = config_get_string_value(config, "IP_GAMECARD");
	puerto_gamecard = config_get_string_value(config, "PUERTO_GAMECARD");

	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");

	int conexion = crear_conexion(ip_broker, puerto_broker);

	enviar_mensaje("hola", conexion);
	//recibir mensaje
	log_info(logger, "mensaje enviado");
	char*mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "mensaje recibido");
	log_info(logger, mensaje);

	log_info(logger, "ESTOY LOGEAdsasNDO");
	log_info(logger, ip_gamecard);
	log_info(logger, puerto_gamecard);*/

	t_configFS* configTG= crear_config(argc,argv);
	ptoMontaje = configTG->ptoMontaje;
	block_size = configTG->block_size;
	blocks = configTG->blocks;


	iniciar_filesystem();

	char*pokemon3 ="charmander";
	char* pokemon = "pikachu";
	tratar_mensaje_NEW_POKEMON(2,3,4,pokemon);
	char*pokemon2 ="delibird";
	tratar_mensaje_NEW_POKEMON(1,2,4,pokemon2);
	tratar_mensaje_CATCH_POKEMON(1,2,pokemon2);
	//tratar_mensaje_NEW_POKEMON(1,3,1,pokemon);
	tratar_mensaje_NEW_POKEMON(4,5,12345,pokemon);
	tratar_mensaje_NEW_POKEMON(2,3,10,pokemon);
	tratar_mensaje_NEW_POKEMON(4,2,12345,pokemon);
	tratar_mensaje_NEW_POKEMON(1,5,12345,pokemon);
	tratar_mensaje_NEW_POKEMON(4,7,12345,pokemon);
	tratar_mensaje_NEW_POKEMON(4,7,1,pokemon);
	tratar_mensaje_NEW_POKEMON(14,51,12345,pokemon);
	tratar_mensaje_NEW_POKEMON(9,5,12345,pokemon);
	tratar_mensaje_CATCH_POKEMON(1,3, pokemon);
	tratar_mensaje_NEW_POKEMON(1,2,4,pokemon3);
	/*char*mensaje= generar_linea_de_entrada_mensaje(4,2,5);;
	//crear_files_metadata(pokemon,mensaje);
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);

	char*pathBloque =generar_path_bloque("2");
	escribir_bloque(pathBloque, mensaje);
	actualizar_bitmap(1,2);
	agregar_block_al_metadata(2,pathPokemon);
	actualizar_tamanio_archivo(pathPokemon);

	/*
	char*pokemon2 ="delibird";
	char*mensaje2=generar_linea_de_entrada_mensaje(1,2,5);
	crear_files_metadata(pokemon2,mensaje2);

	char*pokemon3 ="charmander";
	char*mensaje3="charmander va en el block 3 hjkfjshdjajdjskfhncbahsjektmsmtnemak\n";
	crear_files_metadata(pokemon3,mensaje3);

	char*pokemon4 ="pichu";
		mensaje_new_pokemon(1, 1, 5 ,pokemon4);

		abrir_archivo(generar_path_archivo_pokemon_metadata(pokemon));

	//char archivoEstado = leer_ultima_pos_archivo("/home/utnso/Escritorio/tall-grass/Files/delibird/Metadata.bin");
	//printf ("estado archivo = %c\n", archivoEstado);
	//abrir_archivo("/home/utnso/Escritorio/tall-grass/Files/delibird/Metadata.bin");

	//char* ultimaPos = valor_ultima_posicion(obtener_array_de_bloques("/home/utnso/Escritorio/tall-grass/Files/pikachu/Metadata.bin"),sizeof(obtener_array_de_bloques("/home/utnso/Escritorio/tall-grass/Files/pikachu/Metadata.bin")));
	//printf("ultima pos = %s \n",ultimaPos);

	char*pokemon2 ="delibird";
	char*pokemon3 ="charmander";
	char*pokemon4 ="pichu";
	char*pokemon5="lucia";

	mensaje_new_pokemon(1, 2, 1 ,pokemon);
	mensaje_new_pokemon(2,1,5,pokemon3);
	mensaje_new_pokemon(2,5,5,pokemon2);
	mensaje_new_pokemon(4, 4, 10 ,pokemon);
	mensaje_new_pokemon(1, 3, 10 ,pokemon);
	mensaje_new_pokemon(2,6,8,pokemon3);
	mensaje_new_pokemon(2,7,4,pokemon5);
	mensaje_new_pokemon(5,1,6,pokemon4);


	int value2 = verificar_existencia_posiciones(2,3,"/home/utnso/Escritorio/tall-grass/Files/charmander/Metadata.bin");
	printf(string_from_format("%i\n",value2));*/

	//int value = buscar_posicion_linea_en_bloque(1,1,"/home/utnso/Escritorio/tall-grass/Files/Blocks/0.bin");
	//printf(string_from_format("%i\n",value));




	//log_destroy(logger);
	//liberar_conexion(conexion);

}



