#include "gameboy.h"

int main(int argc, char *argv[]){
	t_log* logger;
	t_config* config;
	int conexion;
	char* mensaje ;

	logger = log_create("/home/utnso/tp-2020-1c-Elite-Four/gameboy/gameboy.log", "GAMEBOY", 1, LOG_LEVEL_INFO);
	config = config_create("/home/utnso/tp-2020-1c-Elite-Four/gameboy/gameboy.config");

	ip_broker = config_get_string_value(config, "IP_BROKER");
	puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	ip_team = config_get_string_value(config, "IP_TEAM");
	puerto_team = config_get_string_value(config, "PUERTO_TEAM");
	ip_gamecard = config_get_string_value(config, "IP_GAMECARD");
	puerto_gamecard = config_get_string_value(config, "PUERTO_GAMECARD");

	log_info(logger, "ESTOY LOGEANDO");

	/*if( (conexion = crear_conexion(ip_broker, puerto_broker)) == -1){
		printf("NO SE CONECTO");
	}*/
	//enviar mensaje
	//printf("%d",conexion);
	log_info(logger, "conexion creada");
	//enviar_mensaje("hola", conexion);
	//recibir mensaje
	log_info(logger, "mensaje enviado");
	//mensaje = client_recibir_mensaje(conexion);
	//loguear mensaje recibido
	log_info(logger, "mensaje recibido");

	if(clasificar_mensaje(argc, argv) == 0)
		return 0;

	//free(mensaje);
	log_info(logger, "SOMOS RE GROSO AMEO");
	log_destroy(logger);
	//config_destroy(config);
	//liberar_conexion(conexion);

}

int clasificar_mensaje(int argc, char* argv[]) {
	char* proceso;
	char* cola_destino;
	int conexion;
	char* mensaje;
	if(argc >= 2) {
		proceso = argv[1];
	} else {
		printf("Faltan argumentos\n");
		return 0;
	}

	if(argc >= 3) {
		cola_destino = argv[2];
	} else {
		printf("Faltan argumentos\n");
		return 0;
	}
	if(strcmp(cola_destino,"NEW_POKEMON") == 0){
		if((strcmp(proceso,"BROKER") == 0) && (argc == 7)){
			conexion = crear_conexion(ip_broker, puerto_broker);
			send_message_new_pokemon(argv[3], (uint32_t) argv[4], (uint32_t) argv[5], (uint32_t) argv[6], conexion);
		}
	}else{
			if((strcmp(proceso,"GAMECARD") == 0) && (argc == 8)){
				conexion = crear_conexion(ip_gamecard, puerto_gamecard);
					send_message_new_pokemon(argv[3], (uint32_t) argv[4], (uint32_t) argv[5], (uint32_t) argv[6],(uint32_t) argv[7], conexion);
			}
	}
	if(strcmp(cola_destino,"GET_POKEMON") == 0){
		if((strcmp(proceso,"BROKER") == 0) && (argc == 4)){
			conexion = crear_conexion(ip_broker, puerto_broker);
			send_message_new_pokemon(argv[3], (uint32_t) argv[4], conexion);
		}
	}else{
			if((strcmp(proceso,"GAMECARD") == 0) && (argc == 6)){
				conexion = crear_conexion(ip_gamecard, puerto_gamecard);
					send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], conexion);
			}
	}
	if(strcmp(cola_destino,"CATCH_POKEMON") == 0){
		if((strcmp(proceso,"BROKER") == 0) && (argc == 6)){
			conexion = crear_conexion(ip_broker, puerto_broker);
			send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], conexion);
		}
	}else{
			if((strcmp(proceso,"GAMECARD") == 0) && (argc == 7)){
				conexion = crear_conexion(ip_gamecard, puerto_gamecard);
					send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5],(uint32_t) argv[6], conexion);
			}
	}
	if(strcmp(cola_destino,"APPEARED_POKEMON") == 0){
			if((strcmp(proceso,"BROKER") == 0) && (argc == 7)){
				conexion = crear_conexion(ip_broker, puerto_broker);
				send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], (uint32_t) argv[6], conexion);
			}
		}else{
				if((strcmp(proceso,"TEAM") == 0) && (argc == 6)){
					conexion = crear_conexion(ip_team, puerto_team);
						send_message_new_pokemon(argv[3], (uint32_t) argv[4],(uint32_t) argv[5], conexion);
				}
		}

	if(strcmp(cola_destino,"CAUGHT_POKEMON") == 0){
				if((strcmp(proceso,"BROKER") == 0) && (argc == 5)){
					conexion = crear_conexion(ip_broker, puerto_broker);
					send_message_new_pokemon(argv[3], (uint32_t) argv[4],conexion);
				}
	}
	}


	//if(strcmp(proceso, "BROKER") == 0) {
	//	conexion = crear_conexion(ip_broker, puerto_broker);
	//}

	//if(strcmp(cola_destino, "NEW_POKEMON") == 0) {
		//if(argc == 7) {
			//send_message_new_pokemon(argv[4], (uint32_t) argv[5], (uint32_t) argv[6], (uint32_t) argv[7], conexion);
		//	mensaje = client_recibir_mensaje(conexion);
		//} else {
			//printf("Cantidad erronea de argumentos\n");
			//return 0;
		//}
	//}
		//if(strcmp(cola_destino, "APPEARED_POKEMON") == 0) {
			//	if(argc == 7) {
				//	send_message_new_pokemon(argv[3], (uint32_t) argv[4], (uint32_t) argv[5], (uint32_t) argv[6], conexion);
					//mensaje = client_recibir_mensaje(conexion);
				//} else {
					//printf("Cantidad erronea de argumentos\n");
					//return 0;
				//}
		//}
		//if(strcmp(cola_destino, "CATCH_POKEMON") == 0) {
			//			if(argc == 6) {
				//			send_message_new_pokemon(argv[3], (uint32_t) argv[4], (uint32_t) argv[5], conexion);
					//		mensaje = client_recibir_mensaje(conexion);
						//} else {
							//printf("Cantidad erronea de argumentos\n");
						//	return 0;
					//	}
		//}
	//	if(strcmp(cola_destino, "CAUGHT_POKEMON") == 0) {
		//				if(argc == 5) {
			//				send_message_new_pokemon(argv[3], (uint32_t) argv[4], conexion);
				//			mensaje = client_recibir_mensaje(conexion);
					//	} else {
						//	printf("Cantidad erronea de argumentos\n");
						//	return 0;
		//				}
		//}
		//if(strcmp(cola_destino, "GET_POKEMON") == 0) {
			//					if(argc == 4) {
				//					send_message_new_pokemon(argv[3],conexion);
					//				mensaje = client_recibir_mensaje(conexion);
						//		} else {
							//		printf("Cantidad erronea de argumentos\n");
								//	return 0;
							//	}
	//	}

		//	return 1;
//}

