#include <commons/config.h>
#include <commons/log.h>
#include <conexiones.h>

int main(int argc, char *argv[]){
	char* ip;
	char* puerto;
	t_log* logger;
	t_config* config;

	logger = log_create("team.log", "TEAM", false, LOG_LEVEL_INFO);
	config = config_create("team.config");
	ip = config_get_string_value(config, "IP_BROKER");
	puerto = config_get_string_value(config, "PUERTO_BROKER");

	log_info(logger, "ESTOY LOGEANDO");
	log_info(logger, ip);
	log_info(logger, puerto);

	int conexion = crear_conexion(ip, puerto);

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

	log_destroy(logger);
	liberar_conexion(conexion);
}
