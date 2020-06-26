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

	char* pokemon = "pikachu";
	char*mensaje= generar_linea_de_entrada_mensaje(4,2,5);;
	crear_files_metadata(pokemon,mensaje);

	char*pokemon2 ="delibird";
	char*mensaje2=generar_linea_de_entrada_mensaje(1,2,5);
	crear_files_metadata(pokemon2,mensaje2);

	char*pokemon3 ="charmander";
	char*mensaje3="charmander va en el block 3 hjkfjshdjajdjskfhncbahsjektmsmtnemak\n";
	crear_files_metadata(pokemon3,mensaje3);

	char archivoEstado = leer_ultima_pos_archivo("/home/utnso/Escritorio/tall-grass/Files/delibird/Metadata.bin");
	printf ("estado archivo = %c\n", archivoEstado);
	abrir_archivo("/home/utnso/Escritorio/tall-grass/Files/delibird/Metadata.bin");

	char* ultimaPos = valor_ultima_posicion(obtener_array_de_bloques("/home/utnso/Escritorio/tall-grass/Files/pikachu/Metadata.bin"),sizeof(obtener_array_de_bloques("/home/utnso/Escritorio/tall-grass/Files/pikachu/Metadata.bin")));
	printf("ultima pos = %s",ultimaPos);

	//agregar_mensaje_NEW_POKEMON(1, 1, 10 ,pokemon);
	agregar_mensaje_NEW_POKEMON(2,3,5,pokemon3);
	//agregar_mensaje_NEW_POKEMON(2,3,5,pokemon2);
	//agregar_mensaje_NEW_POKEMON(4, 3, 10 ,pokemon);
	agregar_mensaje_NEW_POKEMON(1, 1, 10 ,pokemon3);


	//log_destroy(logger);
	//liberar_conexion(conexion);

}

void aplica_funcion_escucha() {

}



