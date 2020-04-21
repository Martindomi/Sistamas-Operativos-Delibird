#include <commons/config.h>
#include <commons/log.h>

int main(int argc, char *argv[]){
	char* ip;
	char* puerto;
	t_log* logger;
	t_config* config;

	logger = log_create("gamecard.log", "GAMECARD", false, LOG_LEVEL_INFO);
	config = config_create("gamecard.config");
	ip = config_get_string_value(config, "IP_GAMECARD");
	puerto = config_get_string_value(config, "PUERTO_GAMECARD");

	log_info(logger, "ESTOY LOGEANDO");
	log_info(logger, ip);
	log_info(logger, puerto);

	log_destroy(logger);
}
