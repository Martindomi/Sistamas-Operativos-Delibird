#include <conexiones.h>
#

int main(int argc, char *argv[]){

	char * configPath = "../broker.config";
	char * ipconfig= "IP_BROKER";
	char * puertocofing= "PUERTO_BROKER";

	t_log *logger;

	logger =log_create("../broker.log", "BROKER", false, LOG_LEVEL_INFO);
	log_info(logger, "ESTOY LOGEANDO");

	iniciar_servidor(configPath, ipconfig,puertocofing);

	return EXIT_SUCCESS;

}
