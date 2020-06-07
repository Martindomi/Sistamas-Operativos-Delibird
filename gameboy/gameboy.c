#include "gameboy.h"

int main(int argc, char *argv[]){
	t_config* config;
	char* proceso;
	uint32_t cola_destino;

	inicializar_datos(config);

	if(argc >= 2) {
		proceso = argv[1];
	} else {
		log_info(logger, "Faltan argumentos\n");
		return -1;
	}

	if(argc >= 3) {
		cola_destino = obtener_cola_mensaje(argv[2]);
		if(cola_destino == 0) {
			log_info(logger, "Ingrese una cola valida.");
			return -1;
		}
	} else {
		log_info(logger, "Faltan argumentos\n");
		return -1;
	}

	if((strcmp(proceso,"SUSCRIPTOR") == 0) && (argc == 4)) {
		// TODO SUSCRIPCION
		realizar_suscripcion(cola_destino);
	} else if ((strcmp(proceso,"SUSCRIPTOR") == 0) && (argc != 4)) {
		manejar_error_mensaje();
		return -1;
	} else if(clasificar_mensaje(argc, argv) == -1) {
		return -1;
	}

	log_destroy(logger);
	config_destroy(config);

	return 0;
}

int clasificar_mensaje(int argc, char* argv[], uint32_t cola_destino, char* proceso) {

	int conexion;
	char* mensaje;

	switch(cola_destino) {
		case NEW_POKEMON: {
			if((strcmp(proceso,"BROKER") == 0) && (argc == 7)){
				conexion = crear_conexion(ip_broker, puerto_broker);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4], (uint32_t) argv[5], (uint32_t) argv[6], conexion);
			} else if((strcmp(proceso,"GAMECARD") == 0) && (argc == 8)){
				conexion = crear_conexion(ip_gamecard, puerto_gamecard);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4], (uint32_t) argv[5], (uint32_t) argv[6],(uint32_t) argv[7], conexion);
			} else {
				manejar_error_mensaje();
				return -1;
			}
			break;
		}
		case GET_POKEMON: {
			if((strcmp(proceso,"BROKER") == 0) && (argc == 4)){
				conexion = crear_conexion(ip_broker, puerto_broker);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4], conexion);
			} else if((strcmp(proceso,"GAMECARD") == 0) && (argc == 6)){
				conexion = crear_conexion(ip_gamecard, puerto_gamecard);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], conexion);
			} else {
				manejar_error_mensaje();
				return -1;
			}
			break;
		}
		case CATCH_POKEMON: {
			if((strcmp(proceso,"BROKER") == 0) && (argc == 6)){
				conexion = crear_conexion(ip_broker, puerto_broker);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], conexion);
			} else if((strcmp(proceso,"GAMECARD") == 0) && (argc == 7)){
				conexion = crear_conexion(ip_gamecard, puerto_gamecard);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5],(uint32_t) argv[6], conexion);
			} else {
				manejar_error_mensaje();
				return -1;
			}
			break;
		}
		case APPEARED_POKEMON: {
			if((strcmp(proceso,"BROKER") == 0) && (argc == 7)){
				conexion = crear_conexion(ip_broker, puerto_broker);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], (uint32_t) argv[6], conexion);
			} else if((strcmp(proceso,"TEAM") == 0) && (argc == 6)){
				conexion = crear_conexion(ip_team, puerto_team);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], conexion);
			} else {
				manejar_error_mensaje();
				return -1;
			}
			break;
		}
		case CAUGHT_POKEMON: {
			if((strcmp(proceso,"BROKER") == 0) && (argc == 5)){
				conexion = crear_conexion(ip_broker, puerto_broker);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4],conexion);
			} else {
				manejar_error_mensaje();
				return -1;
			}
			break;
		}
	}

	mensaje = client_recibir_mensaje(conexion);
	log_info(logger, mensaje);
	liberar_conexion(conexion);
	return 0;
}

uint32_t obtener_cola_mensaje(char* cola_string) {
	if(strcmp(cola_string,"NEW_POKEMON") == 0) {
		return NEW_POKEMON;
	} else if(strcmp(cola_string,"GET_POKEMON") == 0) {
		return GET_POKEMON;
	} else if(strcmp(cola_string,"APPEARED_POKEMON") == 0) {
		return APPEARED_POKEMON;
	} else if(strcmp(cola_string,"LOCALIZED_POKEMON") == 0) {
		return LOCALIZED_POKEMON;
	} else if(strcmp(cola_string,"CATCH_POKEMON") == 0) {
		return CATCH_POKEMON;
	} else if(strcmp(cola_string,"CAUGHT_POKEMON") == 0) {
		return CAUGHT_POKEMON;
	} else {
		return 0;
	}
}

void manejar_error_mensaje() {
	log_info(logger, "Mensaje invalido");
}

void inicializar_datos(t_config* config) {
	logger = log_create("/home/utnso/tp-2020-1c-Elite-Four/gameboy/gameboy.log", "GAMEBOY", 1, LOG_LEVEL_INFO);
	config = config_create("/home/utnso/tp-2020-1c-Elite-Four/gameboy/gameboy.config");
	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	ip_team = config_get_string_value(config, "IP_TEAM");
	puerto_team = config_get_string_value(config, "PUERTO_TEAM");
	ip_gamecard = config_get_string_value(config, "IP_GAMECARD");
	puerto_gamecard = config_get_string_value(config, "PUERTO_GAMECARD");
	ip_gameboy = config_get_string_value(config, "IP_GAMEBOY");
	puerto_gameboy = config_get_string_value(config, "PUERTO_GAMEBOY");
}

void realizar_suscripcion(uint32_t cola_destino) {
	char* ip_puerto_gameboy = malloc(strlen(ip_gameboy) + strlen(puerto_gameboy) + strlen(":") + 2);
	strcpy(ip_puerto_gameboy, ip_gameboy);
	strcat(ip_puerto_gameboy, ":");
	strcat(ip_puerto_gameboy, puerto_gameboy);
	int conexion;
	char* mensaje;

	conexion = crear_conexion(ip_broker, puerto_broker);
	enviar_mensaje_suscribir(cola_destino, ip_puerto_gameboy, conexion);

	mensaje = client_recibir_mensaje(conexion);

	log_info(logger, mensaje);

	liberar_conexion(conexion);
	free(ip_puerto_gameboy);
}
