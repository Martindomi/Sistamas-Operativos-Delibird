#include <commons/config.h>
#include <commons/log.h>

int main(int argc, char *argv[]){
	char* ip_broker;
	char* puerto_broker;
	char* ip_team;
	char* puerto_team;
	char* ip_gamecard;
	char* puerto_gamecard;
	t_log* logger;
	t_config* config;

	logger = log_create("gameboy.log", "GAMEBOY", false, LOG_LEVEL_INFO);
	config = config_create("gameboy.config");

	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	ip_team = config_get_string_value(config, "IP_TEAM");
	puerto_team = config_get_string_value(config, "PUERTO_TEAM");
	ip_gamecard = config_get_string_value(config, "IP_GAMECARD");
	puerto_gamecard = config_get_string_value(config, "PUERTO_GAMECARD");

	log_info(logger, "ESTOY LOGEANDO");

	log_destroy(logger);
}
