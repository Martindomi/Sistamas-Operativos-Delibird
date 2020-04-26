#include "gameboy.h"

int main(int argc, char *argv[]){

	t_log* logger;
	t_config* config;
	int conexion;
	char* mensaje;

	logger = log_create("/home/utnso/tp-2020-1c-Elite-Four/gameboy/gameboy.log", "GAMEBOY", false, LOG_LEVEL_INFO);
	config = config_create("/home/utnso/tp-2020-1c-Elite-Four/gameboy/gameboy.config");

	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	/*ip_team = config_get_string_value(config, "IP_TEAM");
	puerto_team = config_get_string_value(config, "PUERTO_TEAM");
	ip_gamecard = config_get_string_value(config, "IP_GAMECARD");
	puerto_gamecard = config_get_string_value(config, "PUERTO_GAMECARD");*/

	log_info(logger, "ESTOY LOGEANDO");

	conexion = crear_conexion(ip_broker, puerto_broker);
	//enviar mensaje
	log_info(logger, "conexion creada");
	enviar_mensaje("hola", conexion);
	//recibir mensaje
	log_info(logger, "mensaje enviado");
	mensaje = recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "mensaje recibido");
	log_info(logger, mensaje);

	free(mensaje);
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);

}
