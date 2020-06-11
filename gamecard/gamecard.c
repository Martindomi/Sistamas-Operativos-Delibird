#include <commons/config.h>
#include <commons/log.h>
#include <conexiones.h>
#include "gamecard.h"
#include "TALL-GRASS.c"
#include "TALL-GRASS.h"


int main (int argc, char *argv[]) {
	char* ip;
	char* puerto;
	t_log* logger;
	t_config* config;

	logger = log_create("gamecard.log", "GAMECARD", false, LOG_LEVEL_INFO);
	config = config_create("gamecard.config");
	ip = config_get_string_value(config, "IP_GAMECARD");
	puerto = config_get_string_value(config, "PUERTO_GAMECARD");

	char*ip_broker = config_get_string_value(config, "IP_BROKER");
	char*puerto_broker = config_get_string_value(config, "PUERTO_BROKER");

	int conexion = crear_conexion(ip_broker, puerto_broker);

	enviar_mensaje("hola", conexion);
	//recibir mensaje
	log_info(logger, "mensaje enviado");
	char*mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "mensaje recibido");
	log_info(logger, mensaje);

	log_info(logger, "ESTOY LOGEANDO");
	log_info(logger, ip);
	log_info(logger, puerto);

	t_configFS* configTG= crear_config(argc,argv);
	char*ptoMontje = configTG->ptoMontaje;
	int block_size = configTG->block_size;
	int blocks = configTG->blocks;


	iniciar_filesystem();

	log_destroy(logger);
	liberar_conexion(conexion);

}


