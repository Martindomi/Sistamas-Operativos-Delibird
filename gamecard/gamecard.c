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
	crear_files_metadata(pokemon);

	//log_destroy(logger);
	//liberar_conexion(conexion);

}

void aplica_funcion_escucha() {

}



