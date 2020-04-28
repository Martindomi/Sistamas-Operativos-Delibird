#include <conexiones.h>

int main(int argc, char *argv[]){

	char * configPath = "broker.config";
	char * ipconfig= "IP_BROKER";
	char * puertocofing= "PUERTO_BROKER";

	iniciar_servidor(configPath, ipconfig,puertocofing);

	return EXIT_SUCCESS;

}
