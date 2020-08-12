#include "TALL-GRASS.h"

/*---------------Generadores de strings (paths y mensajes)--------------------*/
char* generar_linea_de_entrada_mensaje(int posX, int posY, int cant) {
	char* entradaMensaje = string_new();
	char* aux = string_from_format("%i-%i=%i\n", posX, posY, cant);
	string_append(&entradaMensaje, aux);
	/*string_append(&entradaMensaje, "-");
	 string_append(&entradaMensaje, string_from_format("%i", posY));
	 string_append(&entradaMensaje, "=");
	 string_append(&entradaMensaje, string_from_format("%i", cant));
	 string_append(&entradaMensaje, "\n");*/
	free(aux);
	return entradaMensaje;
}
char* generar_path_directorio_pokemon(char* pokemon) {
	char* pathArchivo = string_new();
	char* aux = string_from_format("%s/Files/%s", ptoMontaje, pokemon);
	string_append(&pathArchivo, aux);
	/*string_append(&pathArchivo, "/Files");
	 if (!string_starts_with(pokemon, "/"))
	 string_append(&pathArchivo, "/");
	 string_append(&pathArchivo, pokemon);*/
	free(aux);
	return pathArchivo;
}
char* generar_path_archivo_pokemon_metadata(char*pokemon) {
	char* path = string_new();
	char* pathDirectorioPokemon = generar_path_directorio_pokemon(pokemon);
	char* aux = string_from_format("%s/Metadata.bin", pathDirectorioPokemon);
	string_append(&path, aux);
	/*string_append(&path, generar_path_directorio_pokemon(pokemon));
	 string_append(&path, string_from_format("/Metadata.bin"));*/
	free(aux);
	free(pathDirectorioPokemon);
	return path;
}
char* generar_path_bloque(char* bloque) {
	char* pathBloque = string_new();
	char* aux = string_from_format("%s/Blocks/%s.bin", ptoMontaje, bloque);
	string_append(&pathBloque, aux);
	/*string_append(&pathBloque, ptoMontaje);
	 string_append(&pathBloque, "/Blocks");
	 if (!string_starts_with(bloque, "/"))
	 string_append(&pathBloque, "/");
	 string_append(&pathBloque, bloque);
	 string_append(&pathBloque, ".bin");*/
	free(aux);
	return pathBloque;
}

/*--------------------------- Inicio File system-------------------------------*/
t_configFS* levantar_configuracion_filesystem(char* archivo) {
	t_configFS* configTG = malloc(sizeof(t_configFS));
	t_config* configuracion = config_create(archivo);

	configTG->ptoEscucha = malloc(
			strlen(config_get_string_value(configuracion, "PUERTO_BROKER"))
					+ 1);
	strcpy(configTG->ptoEscucha,
			config_get_string_value(configuracion, "PUERTO_BROKER"));

	configTG->ptoMontaje = malloc(
			strlen(
					config_get_string_value(configuracion,
							"PUNTO_MONTAJE_TALLGRASS")) + 1);
	strcpy(configTG->ptoMontaje,
			config_get_string_value(configuracion, "PUNTO_MONTAJE_TALLGRASS"));

	//if(!string_ends_with(configTG->ptoMontaje,"/")) string_append(&configTG->ptoMontaje,"/");

	configTG->block_size = config_get_int_value(configuracion, "BLOCK_SIZE");
	configTG->blocks = config_get_int_value(configuracion, "BLOCKS");

	config_destroy(configuracion);
	return configTG;
}
void crear_config(int argc, char* argv[]) {
	if (argc > 1) {
		if (validar_existencia_archivo(argv[1])) {
			configTG = levantar_configuracion_filesystem(argv[1]);
			log_info(logger,
					"INICIO FILE SYSTEM: se levanto correctamente la configuracion del filesystem");
		} else {
			//	log_info(logger,
			//	"INICIO FILE SYSTEM: El path recibido no corresponde a un filesystem en servicio");
		}
	} else if (validar_existencia_archivo(configuracionFS)) {
		configTG = levantar_configuracion_filesystem(configuracionFS);
		log_info(logger,
				"INICIO FILE SYSTEM: La configuracion fue levantada correctamente");
	} else if (validar_existencia_archivo(
			string_substring_from(configuracionFS, 3))) {
		configTG = levantar_configuracion_filesystem(configuracionFS);
		log_info(logger,
				"INICIO FILE SYSTEM: La configuracion levantada correctamente");
	} else {
		//log_info(logger, "INICIO FILE SYSTEM: No se pudo levatar el archivo de configuración");
		exit(EXIT_FAILURE);
	}
	//return configTG;
}
void iniciar_filesystem() {
	log_debug(logger, "inicializando filesystem TALLGRASS");
	montar_punto_montaje();
	iniciar_metadata_dir();
	iniciar_blocks_dir();
	iniciar_files_dir();
}
void montar_punto_montaje() {
	if (!validar_existencia_archivo(ptoMontaje)) {
		crear_directorio(ptoMontaje);
	}
}
void iniciar_metadata_dir() {
	iniciar_metadata();
	iniciar_bitmap();
}
void iniciar_metadata() {
	//log_debug(logger, "INICIO FILE SYSTEM :: METADATA: Inicializando archivo Metadata");
	char*metadata = string_from_format("%s/Metadata/Metadata.bin", ptoMontaje);
	if (validar_existencia_archivo(metadata)) {
		t_config* metadataFS = config_create(metadata);
		int cantBloques = config_get_int_value(metadataFS, "BLOCKS");
		int sizeBloque = config_get_int_value(metadataFS, "BLOCK_SIZE");
		if (cantBloques != blocks || sizeBloque != block_size) {
			//log_debug(logger, "INICIO FILE SYSTEM :: METADATA: Existe un filesystem con valores previos en su carpeta Metadata");

		}
		config_destroy(metadataFS);

	} else {
		char* pathDirMetadata = string_from_format("%s/Metadata", ptoMontaje);
		crear_directorio(pathDirMetadata); /*modif1*/
		FILE* archivo = fopen(metadata, "a");
		fprintf(archivo, "BLOCK_SIZE=%d\n", block_size);
		fprintf(archivo, "BLOCKS=%d\n", blocks);
		fprintf(archivo, "MAGIC_NUMBER=TALL_GRASS\n");

		fclose(archivo);
		free(pathDirMetadata);
		//log_debug(logger, "INICIO FILE SYSTEM :: METADATA: Se ha iniciado  correctamente");
	}
	free(metadata);
}
void iniciar_bitmap() {
	log_debug(logger, "Iniciando Bitmap");

	int sizeBitarray = blocks;
	while ((sizeBitarray % 8) != 0)
		sizeBitarray++;

	char*pathBitmap = string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje);
	sem_wait(&mutexBitmap);
	int bitmap_fd = open(pathBitmap, O_RDWR, 0664);

	if (bitmap_fd < 0) {
		//log_info(logger, "BITMAP: No se pudo abrir el archivo");
		bitmap_fd = open(pathBitmap, O_CREAT | O_RDWR, 0664);
		bitmap = mmap(NULL, sizeBitarray, PROT_WRITE | PROT_READ | PROT_EXEC,
		MAP_SHARED, bitmap_fd, 0);
		bitmap = bitarray_create_with_mode(string_repeat('\0', sizeBitarray),
				sizeBitarray, LSB_FIRST);

	} else {
		ftruncate(bitmap_fd, sizeBitarray);
		bitmap = mmap(NULL, sizeBitarray, PROT_WRITE | PROT_READ | PROT_EXEC,
		MAP_SHARED, bitmap_fd, 0);
		struct stat stats;
		stat(pathBitmap, &stats);

		char* data = malloc(stats.st_size);
		read(bitmap_fd, data, stats.st_size);

		bitmap = bitarray_create_with_mode(data, stats.st_size, LSB_FIRST);
	}
	msync(bitmap->bitarray, bitmap_fd, MS_SYNC);

	close(bitmap_fd);
	sem_post(&mutexBitmap);
	free(pathBitmap);
	log_debug(logger,
			"INICIO FILE SYSTEM :: BITMAP: Se ha inicializado correctamente el bitmap");
}
void iniciar_files_dir() {
	char* pathDirectorio = string_from_format("%s/Files/Metadata.bin",
			ptoMontaje);
	if (!validar_existencia_archivo(pathDirectorio)) {
		char* pathAux = string_from_format("%s/Files", ptoMontaje);
		crear_directorio(pathAux);
		crear_metadata_directorio(pathAux);
		free(pathAux);
	}
	free(pathDirectorio);
}
void iniciar_blocks_dir() {
	char* pathAux = string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje);
	if (bitmap_vacio(pathAux)) {
		char* pathBloques = string_from_format("%s/Blocks", ptoMontaje);
		crear_directorio(pathBloques);
		crear_block();
		//log_info(logger, "existen valores previos");
		free(pathBloques);
	}
	free(pathAux);
}

/*------------------------Creación de directorios ------------------------------*/
void crear_directorio(char*path) {
	mkdir(path, 0777);
	//log_debug(logger, "INICIO FILE SYSTEM :: DIRECTORIO: Se ha creado el directorio %d", path);
}
void crear_metadata_directorio(char* dir) {
	char* path = (string_from_format("%s/Metadata.bin", dir));
	FILE* directorio = fopen(path, "a");
	char* directory = string_from_format("DIRECTORY=%s\n", "Y");
	fputs(directory, directorio);
	//fputs((string_from_format("DIRECTORY=%s\n", "Y")), directorio);
	fclose(directorio);
	free(directory);
	free(path);
}

/*-------------------------Gestión de bloques y bitmap ------------------------*/
int crear_block() {
	log_debug(logger, "crear nuevo block");

	int i;
	for (i = 0; i < blocks; i++) {
		char* pathBloque = string_from_format("%s/Blocks/%d.bin", ptoMontaje,
				i);
		int block_fd = open(pathBloque, O_CREAT | O_RDWR, 0664);
		if (block_fd < 0) {
			//	log_info(logger, "INICIO FILE SYSTEM :: BLOQUES : No se creo el bloque correctamente");
			exit(1);
		}
		close(block_fd);
		free(pathBloque);
	}
	//log_debug(logger, "INICIO FILE SYSTEM :: BLOQUES: se crearon %i bloques", i - 1);
	return 1;
}
int buscar_block_disponible() {
	int bloqueLibre;
	//log_debug(logger, "BLOQUES: buscando espacio disponible");
	//sem_wait(&mutexBitmap);
	for (bloqueLibre = 0;
			bitarray_test_bit(bitmap, bloqueLibre) && (bloqueLibre < blocks);
			bloqueLibre++) {
		//log_info(logger, "BLOQUES: el bloque disponible es %i", bloqueLibre);
	}
	//sem_post(&mutexBitmap);
	if (bloqueLibre >= blocks)
		return NO_MORE_BLOCKS;
	return bloqueLibre;
}
int bitmap_vacio(char*path) {
	int i;
	sem_wait(&mutexBitmap);
	int bitmap_fd = open(path, O_CREAT | O_RDWR, 0664);
	int sizeBitarray = blocks;
	while ((sizeBitarray % 8) != 0)
		sizeBitarray++;
	ftruncate(bitmap_fd, sizeBitarray);
	if (bitmap_fd < 0) {
		//	log_info(logger, "BITMAP: No se puede abrir el archivo");
		exit(1);
	}
	for (i = 0; i < sizeBitarray; i++) {
		if (bitarray_test_bit(bitmap, i)) {
			munmap(bitmap, sizeBitarray); //Revisar si ejecuta y libera bitmap
			close(bitmap_fd);
			sem_post(&mutexBitmap);
			return false;
		}
	}

	close(bitmap_fd);
	sem_post(&mutexBitmap);
	return true;
}
void vaciar_bloque(char* bloque) {

	char* pathBloque = generar_path_bloque(bloque);
	int bloque_fd = open(pathBloque, O_RDWR, 0664);
	if (bloque_fd < 0) {
		//log_info(logger,
		//	string_from_format("No se pudo abrir el bloque %i", bloque));
		exit(1);
	}
	ftruncate(bloque_fd, block_size);
	char* bloqueVacio = mmap(NULL, block_size,
	PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, bloque_fd, 0);
	char* cadenaVacia = string_repeat('\0', block_size);
	ftruncate(bloque_fd, 0);
	memcpy(bloqueVacio, cadenaVacia, 0);
	msync(bloqueVacio, bloque_fd, 0);
	munmap(bloqueVacio, 0);
	close(bloque_fd);
	free(cadenaVacia);
	free(pathBloque);
}
void actualizar_bitmap(bool valor, int pos) {
	char*pathBitmap = string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje);
	//sem_wait(&mutexBitmap);
	int bitmap_fd = open(pathBitmap, O_CREAT | O_RDWR, 0664);

	if (bitmap_fd < 0) {
		//log_info(logger, "BITMAP: no se pudo actualizar el archivo bitmap");
		exit(1);
	}
	int length = sizeof(bitmap_fd);
	ftruncate(bitmap_fd, blocks);
	char* bitmap_a = mmap(NULL, blocks, PROT_WRITE | PROT_READ | PROT_EXEC,
	MAP_SHARED, bitmap_fd, 0);

	if (valor)
		bitarray_set_bit(bitmap, pos);
	else
		bitarray_clean_bit(bitmap, pos);
	memcpy(bitmap_a, bitmap->bitarray, blocks);
	msync(bitmap_a, bitmap_fd, MS_SYNC);
	munmap(bitmap_a, blocks);
	close(bitmap_fd);
	free(pathBitmap);
	//sem_post(&mutexBitmap);
	//log_debug(logger,"BITMAP: Se actualizó correctamente el valor de la posicion %i",pos);
	return;
}
int block_completo(char*path) {
	if (tamanio_ocupado_bloque(path) == block_size)
		return true;
	else
		return false;
}
void liberar_bloque(char* bloque, char*pathPokemon) {
	sem_wait(&mutexBitmap);
	//printf("bloque: %s\n",bloque);
	vaciar_bloque(bloque);
	actualizar_bitmap(0, atoi(bloque));
	quitar_bloque_de_metadata(pathPokemon, bloque);
	sem_post(&mutexBitmap);

}

/*-------------------------------Gestion de archivos--------------------------*/
int validar_existencia_archivo(char*path) {
	//log_debug(logger,
	//"ARCHIVO: Verificando si existe el archivo %d en el sistema de archivos",
	//path);
	int archivo_fd = open(path, O_RDWR, 0664);
	if (archivo_fd < 0) {
		//log_info(logger, string_from_format("ARCHIVO:No existe el archivo %s", path));
		return false;
	} else {
		//log_info(logger, string_from_format("ARCHIVO:Existe el archivo %s", path));
		return true;
	}
	close(archivo_fd);
}
int archivo_abierto(char* path) {
	t_config* configuracion = config_create(path);
	char* estado = config_get_string_value(configuracion, "OPEN");

	if (strcmp(estado, "Y") == 0) {
		config_destroy(configuracion);
		return true;
	} else {
		config_destroy(configuracion);
		return false;
	}

}
void abrir_archivo(char*path) {
	t_config* configuracion = config_create(path);
	char* estado = string_new();
	string_append(&estado, "Y");
	config_set_value(configuracion, "OPEN", estado);
	config_save(configuracion);
	config_destroy(configuracion);
	free(estado);

}
void cerrar_archivo(char*path) {
	t_config* configuracion = config_create(path);
	char* estado = string_new();
	string_append(&estado, "N");
	config_set_value(configuracion, "OPEN", estado);
	config_save(configuracion);
	config_destroy(configuracion);
	free(estado);

}
char** obtener_array_de_bloques(char*path) {
	t_config* c = config_create(path);
	char**bloques = config_get_array_value(c, "BLOCKS");

	config_destroy(c);

	return bloques;
}
void actualizar_tamanio_archivo(char*path) {
	t_config* configuracion = config_create(path);
	int tamanio = config_get_int_value(configuracion, "SIZE");

	tamanio = calcular_tamanio_archivo(path);
	char* stringTamanio = string_itoa(tamanio);
	config_set_value(configuracion, "SIZE", stringTamanio);

	config_save(configuracion);
	config_destroy(configuracion);
	free(stringTamanio);

}
void agregar_block_al_metadata(int block, char* pathPokemon) {
	t_config* configuracion = config_create(pathPokemon);
	char* bloques = string_new();
	string_append(&bloques, config_get_string_value(configuracion, "BLOCKS"));

	bloques[strlen(bloques) - 1] = '\0';
	if (strcmp(bloques, "[")) {
		string_append(&bloques, ",");
	}
	char* numeroBloque = string_itoa(block);
	string_append(&bloques, numeroBloque);
	string_append(&bloques, "]");

	config_set_value(configuracion, "BLOCKS", bloques);
	config_save(configuracion);
	config_destroy(configuracion);

	free(bloques);
	free(numeroBloque);
	return;
}
void quitar_bloque_de_metadata(char*path, char* bloque) {
	t_config* cfg = config_create(path);
	char* bloques = string_new();
	char*bloques1 = string_new();

	string_append(&bloques1, config_get_string_value(cfg, "BLOCKS"));

	char**listaDeBloques;

	if (string_length(bloques1) == 3) {
		string_append(&bloques, "[]");
	} else {
		listaDeBloques = string_n_split(bloques1, 2, bloque);
		string_append(&bloques, listaDeBloques[0]);
		bloques[strlen(bloques) - 1] = '\0';
		string_append(&bloques, listaDeBloques[1]);
		free(listaDeBloques[0]);
		free(listaDeBloques[1]);
	}

	config_set_value(cfg, "BLOCKS", bloques);
	config_save(cfg);
	config_destroy(cfg);

	free(listaDeBloques);
	free(bloques1);
	free(bloques);
	return;
}

/*------------------------Calculos para obtener tamaños-----------------------*/
int tamanio_real_archivo(char*pathPokemon) {
	t_config* configArchivo = config_create(pathPokemon);
	int tam = config_get_int_value(configArchivo, "SIZE");
	config_destroy(configArchivo);
	return tam;
}
int tamanio_ocupado_bloque(char*path) {
	int bloque_fd = open(path, O_RDWR, 0664);
	struct stat stats;
	stat(path, &stats);
	close(bloque_fd);
	return stats.st_size;

}
int tam_disponible_en_bloque(char*path) {
	int tamDisponible;
	int size = block_size;
	int tamOcupado = tamanio_ocupado_bloque(path);
	int tamDisponiblePrevio = size - tamOcupado;

	if (tamDisponiblePrevio < 0) {
		log_debug(logger, "BLOCKS: no tiene espacio disponible");
	} else {
		tamDisponible = tamDisponiblePrevio;
		log_debug(logger, "BLOCKS: el bloque tiene %i bytes disponibles",
				tamDisponible);
	}
	return tamDisponible;
}
int calcular_tamanio_archivo(char*path) {
	int i;
	int tamanio = 0;
	char** blocks = obtener_array_de_bloques(path);
	if (tam_array_bloques(blocks) == 0) {
		tamanio = 0;
	} else {
		for (i = 0; i < (tam_array_bloques(blocks)); i++) {
			char* pathBlock = generar_path_bloque(blocks[i]);
			tamanio = tamanio + tamanio_ocupado_bloque(pathBlock);
			free(pathBlock);
			free(blocks[i]);
		}
	}
	free(blocks);
	return tamanio;
}
int cantidad_digitos(int numero) {
	int count = 0;
	while (numero != 0) {
		numero /= 10;
		++count;
	}
	return count;
}
int tam_array_bloques(char** bloques) {
	int j;
	for (j = 0; bloques[j] != NULL; j++) {
	}
	return j;
}

/*----------------------------Tratamiento de mensajes-----------------------*/
void tratar_mensaje_NEW_POKEMON(int posX, int posY, int cant, char* pokemon) {
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	char* mensaje = generar_linea_de_entrada_mensaje(posX, posY, cant);
	sem_t* semaforoNewPokemon = (sem_t*)dictionary_get(dicSemaforos, pokemon);

	sem_wait(semaforoNewPokemon);

	if (validar_existencia_archivo(pathPokemon)) {

		if (!archivo_abierto(pathPokemon)) {
			//printf("Entra a abrir arhcivo\n");
			abrir_archivo(pathPokemon);
			//printf("sale de abrir el archivo\n");
			sem_post(semaforoNewPokemon);

			//char* mensaje = generar_linea_de_entrada_mensaje(posX, posY, cant);
			int i = 0;
			char* contenidoBloques = string_new();
			char** bloques = obtener_array_de_bloques(pathPokemon);

			for (i = 0; i < tam_array_bloques(bloques); i++) {
				char*pathBloque = generar_path_bloque(bloques[i]);
				int bloque_fd = open(pathBloque, O_RDWR, 0664);

				if (bloque_fd < 0) {
					log_info(logger, "No se pudo abrir el archivo");
					error(1);
				}
				struct stat stats;
				stat(pathBloque, &stats);

				char*data = malloc(stats.st_size);
				read(bloque_fd, data, stats.st_size);

				char* untilString = string_substring_until(data, stats.st_size);
				string_append(&contenidoBloques, untilString);
				free(data);
				free(pathBloque);
				close(bloque_fd);
				free(untilString);
				free(bloques[i]);
			}

			free(bloques);
			char*mensajeCorto = string_from_format("%i-%i=", posX, posY);
			if (string_contains(contenidoBloques, mensajeCorto)) {
				int i;
				int siguientePos = 0;

				for (i = 0; i < string_length(contenidoBloques); i =
						siguientePos) {
					char*stringComparar = string_substring_from(
							contenidoBloques, i);
					if (!(string_starts_with(stringComparar, mensajeCorto))) {
						int j;
						for (j = 0; stringComparar[j] != '\n'; j++)
							;
						siguientePos = i + j + 1;
						free(stringComparar);
					} else {
						free(stringComparar);
						break;
					}
				}
				char* stringCant = string_substring_from(contenidoBloques,
						(i + string_length(mensajeCorto)));
				char** listaDeContenidosTotal = string_n_split(stringCant, 2,
						"\n");
				int cantActual = atoi(listaDeContenidosTotal[0]);
				int nuevaCantidad = cant + cantActual;
				char*contenidoFinal = string_new();
				char* stringNiIdea = string_substring_until(contenidoBloques,
						(i + (string_length(mensajeCorto))));
				string_append(&contenidoFinal, stringNiIdea);
				free(stringNiIdea);
				char* cantidadNuevaString = string_itoa(nuevaCantidad);
				string_append(&contenidoFinal, cantidadNuevaString);
				free(cantidadNuevaString);
				char* stringPirulito = string_substring_from(stringCant,
						cantidad_digitos(cantActual));
				string_append(&contenidoFinal, stringPirulito);
				free(stringPirulito);

				sem_wait(semaforoNewPokemon);
				tratar_contenido_en_bloques(contenidoFinal, pathPokemon);
				actualizar_tamanio_archivo(pathPokemon);
				sem_post(semaforoNewPokemon);

				free(stringCant);
				free(contenidoFinal);
				free(listaDeContenidosTotal[0]);
				free(listaDeContenidosTotal[1]);
				free(listaDeContenidosTotal);
			} else {
				sem_wait(semaforoNewPokemon);
				agregar_nuevo_mensaje(mensaje, pathPokemon);
				sem_post(semaforoNewPokemon);

			}
			sleep(tiempo_retardo_operacion);
			log_info(logger,
					"NEW_POKEMON: Se ha modificado el contenido del archivo %s",
					pokemon);
			sem_wait(semaforoNewPokemon);
			//printf("Comienzo cerrar archivo\n");
			cerrar_archivo(pathPokemon);
			//printf("Cierro archivo\n");
			sem_post(semaforoNewPokemon);
			free(mensajeCorto);
			free(contenidoBloques);
			free(pathPokemon);
			free(mensaje);

		} else {
			free(pathPokemon);
			free(mensaje);
			sem_post(semaforoNewPokemon);
			sleep(tiempo_de_reintento_operacion);
			tratar_mensaje_NEW_POKEMON(posX, posY, cant, pokemon);

		}
	} else {
		crear_archivo_pokemon_metadata(pokemon, mensaje);
		sleep(tiempo_retardo_operacion);
		free(pathPokemon);
		free(mensaje);
		sem_post(semaforoNewPokemon);
		log_info(logger,
				"NEW_POKEMON: Se ha modificado el contenido del archivo %s",
				pokemon);
	}

}
char* tratar_mensaje_CATCH_POKEMON(int posX, int posY, char*pokemon) {
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	sem_t* semaforoCatchPokemon =(sem_t*) dictionary_get(dicSemaforos,pokemon);

	//printf("antes del wait de catch\n");
	sem_wait(semaforoCatchPokemon);
	if (validar_existencia_archivo(pathPokemon)) {

		//printf("despues del wait de catch\n");
		if (!archivo_abierto(pathPokemon)) {
			abrir_archivo(pathPokemon);
			sem_post(semaforoCatchPokemon);

			char* mensaje = string_from_format("%i-%i=", posX, posY);

			int i = 0;
			char* contenidoBloques = string_new();
			char** bloques = obtener_array_de_bloques(pathPokemon);

			for (i = 0; i < tam_array_bloques(bloques); i++) {
				char*pathBloque = generar_path_bloque(bloques[i]);
				int bloque_fd = open(pathBloque, O_RDWR, 0664);

				if (bloque_fd < 0) {
					log_info(logger, "No se pudo abrir el archivo");
					error(1);
				}
				struct stat stats;
				stat(pathBloque, &stats);

				char*data = malloc(stats.st_size);
				read(bloque_fd, data, stats.st_size);
				char*dataSubrtring = string_substring_until(data,
						stats.st_size);
				string_append(&contenidoBloques, dataSubrtring);

				free(bloques[i]);
				free(data);
				free(pathBloque);
				close(bloque_fd);
				free(dataSubrtring);
			}
			free(bloques);
			if (string_contains(contenidoBloques, mensaje)) {
				int i;
				int siguientePos = 0;

				for (i = 0; i < string_length(contenidoBloques); i =
						siguientePos) {
					char*stringComparar = string_substring_from(
							contenidoBloques, i);
					if (!(string_starts_with(stringComparar, mensaje))) {
						int j;
						for (j = 0; stringComparar[j] != '\n'; j++)
							;
						siguientePos = i + j + 1;
						free(stringComparar);
					} else {
						free(stringComparar);
						break;
					}
				}
				char* stringCant = string_substring_from(contenidoBloques,
						(i + string_length(mensaje)));
				char** listaDeContenidosTotal = string_n_split(stringCant, 2,
						"\n");
				int cantActual = atoi(listaDeContenidosTotal[0]);

				free(listaDeContenidosTotal[0]);
				free(listaDeContenidosTotal[1]);


				int nuevaCantidad = cantActual - 1;
				char*contenidoFinal = string_new();
				if (nuevaCantidad == 0) {
					if (i == 0) {
						if ((string_length(mensaje)
								+ cantidad_digitos(cantActual) + 1)
								== string_length(contenidoBloques)) {
							char * charRepetido = string_repeat('\0', string_length(contenidoBloques));
							string_append(&contenidoFinal, charRepetido);
							free(charRepetido);
						} else {
							string_append(&contenidoFinal,
									string_substring_from(contenidoBloques,
											(i + string_length(mensaje)
													+ cantidad_digitos(
															cantActual) + 1)));
						}
					} else {
						char* contenidoBloqMenosUno = string_substring_until(
								contenidoBloques, (i - 1));
						string_append(&contenidoFinal, contenidoBloqMenosUno);
						free(contenidoBloqMenosUno);
						char* subStringCantidadDigitos = string_substring_from(
								stringCant, (cantidad_digitos(cantActual)));
						string_append(&contenidoFinal,
								subStringCantidadDigitos);
						free(subStringCantidadDigitos);
						/*string_append(&contenidoFinal,
						 string_substring_until(contenidoBloques,
						 (i - 1)));
						 string_append(&contenidoFinal,
						 string_substring_from(stringCant,
						 (cantidad_digitos(cantActual))));*/
					}
				} else {
					if (nuevaCantidad < 0) {
						log_info(logger,
								"Ocurrio un error con las posiciones mencionadas");
						free(listaDeContenidosTotal);
						free(mensaje);
						return "fail";
					} else {
						char* stringHasta = string_substring_until(
								contenidoBloques,
								(i + (string_length(mensaje))));
						string_append(&contenidoFinal, stringHasta);
						free(stringHasta);
						char* cantNuev = string_itoa(nuevaCantidad);
						string_append(&contenidoFinal, cantNuev);
						free(cantNuev);
						char* stringAlgo = string_substring_from(stringCant,
								cantidad_digitos(cantActual));
						string_append(&contenidoFinal, stringAlgo);
						free(stringAlgo);
						/*string_append(&contenidoFinal,
						 string_substring_until(contenidoBloques,
						 (i + (string_length(mensaje)))));
						 string_append(&contenidoFinal,
						 string_itoa(nuevaCantidad));
						 string_append(&contenidoFinal,
						 string_substring_from(stringCant,
						 cantidad_digitos(cantActual)));*/
					}

				}
				tratar_contenido_en_bloques(contenidoFinal, pathPokemon);
				actualizar_tamanio_archivo(pathPokemon);
				sleep(tiempo_retardo_operacion);
				log_info(logger,
						"CATCH_POKEMON: Se ha modificado el contenido del archivo %s",
						pokemon);
				cerrar_archivo(pathPokemon);
				free(contenidoBloques);
				free(listaDeContenidosTotal);
				free(contenidoFinal);
				free(pathPokemon);
				free(stringCant);
				free(mensaje);
				return "OK";

			}
			log_info(logger,
					"CATCH_POKEMON: No hay ningun pokemon %s en la posicion %i-%i solicitada",
					pokemon, posX, posY);

			sem_wait(semaforoCatchPokemon);
			cerrar_archivo(pathPokemon);
			sem_post(semaforoCatchPokemon);

			free(contenidoBloques);
			free(pathPokemon);
			free(mensaje);
			return "FAIL";
		} else {
			sem_post(semaforoCatchPokemon);
			sleep(tiempo_de_reintento_operacion);
			tratar_mensaje_CATCH_POKEMON(posX, posY, pokemon);
		}
	} else {
		sem_post(semaforoCatchPokemon);
		log_info(logger,
				"CATCH_POKEMON; No existe el pokemon %s dentro del sistema de archivos",
				pokemon);
		free(pathPokemon);

		return "FAIL";
	}

}
t_list* tratar_mensaje_GET_POKEMON(char*pokemon) {
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	t_list* listadoPos = list_create();
	sem_t* semaforoGetPokemon =(sem_t*) dictionary_get(dicSemaforos,pokemon);

	sem_wait(semaforoGetPokemon);
	if (validar_existencia_archivo(pathPokemon)) {
		if (!archivo_abierto(pathPokemon)) {
			abrir_archivo(pathPokemon);
			sem_post(semaforoGetPokemon);

			int i = 0;
			char* contenidoBloques = string_new();
			char** bloques = obtener_array_de_bloques(pathPokemon);

			for (i = 0; i < tam_array_bloques(bloques); i++) {
				char*pathBloque = generar_path_bloque(bloques[i]);
				int bloque_fd = open(pathBloque, O_RDWR, 0664);

				if (bloque_fd < 0) {
					//log_info(logger, "No se pudo abrir el archivo");
					error(1);
				}
				struct stat stats;
				stat(pathBloque, &stats);

				char*data = malloc(stats.st_size);
				read(bloque_fd, data, stats.st_size);

				char*dataSubtring = string_substring_until(data,
										stats.st_size);
				string_append(&contenidoBloques, dataSubtring);
				free(data);
				free(pathBloque);
				free(dataSubtring);
				free(bloques[i]);
				close(bloque_fd);
			}
			free(bloques);
			int h, j;
			int siguientePos;

			for (h = 0; h < string_length(contenidoBloques); h = siguientePos) {
				for (j = h; contenidoBloques[j] != '\n'; j++)
					;

				char*mensajeAux = string_new();
				char*mensajeAux2 = string_substring_from(contenidoBloques, h);
				char*mensajeAux3 = string_substring_until(mensajeAux2, j - h);
				string_append(&mensajeAux,mensajeAux3);

				char** listaMensajeAux = string_n_split(mensajeAux, 2, "=");
				char**listaMensaje = string_n_split(listaMensajeAux[0], 2, "-");

				int posX=atoi(listaMensaje[0]);
				int posy = atoi(listaMensaje[1]);

				list_add(listadoPos, posX);
				list_add(listadoPos, posy);

				siguientePos = j + 1;

				free(listaMensajeAux[0]);
				free(listaMensajeAux[1]);

				free(listaMensaje[0]);
				free(listaMensaje[1]);

				free(mensajeAux);
				free(mensajeAux2);
				free(mensajeAux3);
				free(listaMensaje);
				free(listaMensajeAux);


			}
			sleep(tiempo_retardo_operacion);

			sem_wait(semaforoGetPokemon);
			cerrar_archivo(pathPokemon);
			sem_post(semaforoGetPokemon);

			log_info(logger,
					"GET_POKEMON:se envío el listado de posiciones del pokemon %s existente dentro del file system",
					pokemon);
			free(pathPokemon);
			free(contenidoBloques);
			return listadoPos;

		} else {
			sem_post(semaforoGetPokemon);
			sleep(tiempo_de_reintento_operacion);
			tratar_mensaje_GET_POKEMON(pokemon);
		}
	} else {
		sem_post(semaforoGetPokemon);
		log_info(logger,
				"GET_POKEMON: No existe el pokemon %s dentro del file system",
				pokemon);
		list_clean(listadoPos);
		free(pathPokemon);
		return listadoPos;
	}
}
void tratar_contenido_en_bloques(char*contenido, char* pathPokemon) {
	int tamContenido = string_length(contenido);
	char** bloques = obtener_array_de_bloques(pathPokemon);
	int tamArray = tam_array_bloques(bloques);
	int i;
	int j;
	int cantBloques = tamContenido / block_size;
	if ((tamContenido % block_size) != 0)
		cantBloques++;

	if (cantBloques == tamArray) {
		j = block_size;
		for (i = 0; i < tamArray; i++) {
			char*mensajePorBloque = string_new();
			if ((tamContenido - j) < 0) {
				char* contenido2 = string_substring_from(contenido,
						j - block_size);

				char* contenido3 = string_substring_until(contenido2,(block_size - (j - tamContenido)));
				string_append(&mensajePorBloque,contenido3);
				free(contenido2);
				free(contenido3);
			} else {
				char* contenidoAux = string_substring_from(contenido,
						j - block_size);
				char* substringUntil = string_substring_until(contenidoAux, j);
				string_append(&mensajePorBloque,
						substringUntil);
				free(contenidoAux);
				free(substringUntil);
			}
			escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,
			MODIFICAR);
			j = j + block_size;
			free(mensajePorBloque);
		}
	} else {
		if (cantBloques > tamArray) {
			j = block_size;
			for (i = 0; i < tamArray; i++) {
				char*mensajePorBloque = string_new();
				char*contenidoAux = string_substring_from(contenido,
						j - block_size);
				char* stringSubstringUntil = string_substring_until(contenidoAux, j);
				string_append(&mensajePorBloque,
						stringSubstringUntil);

				escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,
				MODIFICAR);
				j = j + block_size;
				free(contenidoAux);
				free(stringSubstringUntil);
				free(mensajePorBloque);
			}
			int h;

			for (h = 0; h <= (cantBloques - tamArray); h++) {
				sem_wait(&mutexBitmap);
				int nuevoBloque = buscar_block_disponible();
				actualizar_bitmap(1, nuevoBloque);
				sem_post(&mutexBitmap);
				char*mensajePorBloque = string_new();
				if ((tamContenido - j) < block_size) {
					char* contenidoMuletita = string_substring_from(contenido,
							j);
					char* contenidoAux3 = string_substring_until(contenidoMuletita,
							tamContenido - j);
					string_append(&mensajePorBloque,contenidoAux3);
					free(contenidoMuletita);
					free(contenidoAux3);
				} else {
					char*contenidoM = string_substring_from(contenido, j);
					char* str =string_substring_until(contenidoM, j + block_size);
					string_append(&mensajePorBloque,str);
					free(str);
				}
				j = j + block_size;
				escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,
				MODIFICAR);
				agregar_block_al_metadata(nuevoBloque, pathPokemon);
				actualizar_tamanio_archivo(pathPokemon);
			}
		} else {
			if (cantBloques < tamArray) {
				int tamDiferencia = tamArray - cantBloques;
				int d;
				for (d = 1; d <= tamDiferencia; d++) {
					liberar_bloque(bloques[tamArray - d], pathPokemon);
				}

				j = 0;
				for (i = 0; i < cantBloques; i++) {
					char*mensajePorBloque = string_new();
					if ((tamContenido - j) < block_size) {
						char* contenidoAux = string_substring_from(contenido,
								j);
						char* cadenita = string_substring_until(contenidoAux,
								tamContenido - j);
						string_append(&mensajePorBloque,cadenita
								);
						free(contenidoAux);
						free(cadenita);
					} else {
						char* contenidoAux = string_substring_from(contenido,
								j);
						char* auxDeAux = string_substring_until(contenidoAux,
								j + block_size);
						string_append(&mensajePorBloque,auxDeAux);
						free(contenidoAux);
						free(auxDeAux);
					}

					escribir_mensaje_en_block(atoi(bloques[i]),
							mensajePorBloque, MODIFICAR);
					j = j + block_size;
					free(mensajePorBloque);
				}
			}
		}
	}

	for(int u=0; u<tamArray-1; u++){
		free(bloques[u]);
	}
	free(bloques);
}
void agregar_nuevo_mensaje(char* mensaje, char*pathPokemon) {
	int tamMensaje = string_length(mensaje);
	char** bloques = obtener_array_de_bloques(pathPokemon);
	int tamArray = tam_array_bloques(bloques);
	//printf("ENTRA A AGREGAR");
	if (tamArray == 0) {
		sem_wait(&mutexBitmap);
		int bloqueNuevo = buscar_block_disponible();
		actualizar_bitmap(1, bloqueNuevo);
		sem_post(&mutexBitmap);
		escribir_mensaje_en_block(bloqueNuevo, mensaje, AGREGAR);
		//printf("Modifica METADATA AGREGAR BLOCK\n");
		agregar_block_al_metadata(bloqueNuevo, pathPokemon);
		//printf("Modifica ACTUALIZA TAMANO\n");
		actualizar_tamanio_archivo(pathPokemon);
		//printf("SALE DE ACTUALIZAR\n");
	} else {
		char*path = generar_path_bloque(bloques[tamArray - 1]);
		int tamDisponible = tam_disponible_en_bloque(path);
		if (tamMensaje <= tamDisponible) {
			//printf("tamano mensaje menor disponible\n");
			escribir_mensaje_en_block(atoi(bloques[tamArray - 1]), mensaje,
			AGREGAR);
			//printf("actualiza el tamano archivo\n");
			actualizar_tamanio_archivo(pathPokemon);
			//printf("no tengo mas ideas\n");

		} else {
			char* mensajeBloqueFinal = string_substring_until(mensaje,
					(tamDisponible));
			escribir_mensaje_en_block(atoi(bloques[tamArray - 1]),
					mensajeBloqueFinal, AGREGAR);
			sem_wait(&mutexBitmap);
			int bloqueNuevo = buscar_block_disponible();
			actualizar_bitmap(1, bloqueNuevo);
			sem_post(&mutexBitmap);
			char* mensajeBloqueNuevo = string_substring_from(mensaje,
					tamDisponible);
			escribir_mensaje_en_block(bloqueNuevo, mensajeBloqueNuevo,
			AGREGAR);

			//printf("Entra al block metadata no se que cosa\n");
			agregar_block_al_metadata(bloqueNuevo, pathPokemon);
			//printf("Y sale por aqui\n");
			actualizar_tamanio_archivo(pathPokemon);
			//printf("un gusto caballero\n");
			free(mensajeBloqueFinal);
			free(mensajeBloqueNuevo);

		}
		free(path);

	}

	for(int i = 0; i<tamArray; i++){
		free(bloques[i]);
	}
	free(bloques);

}
void escribir_mensaje_en_block(int bloque, char* mensaje, int accion) {

	char* numeroBloque = string_itoa(bloque);
	char* pathBloque = generar_path_bloque(numeroBloque);
	int tamMensaje = string_length(mensaje);
	int tamOcupado = tamanio_ocupado_bloque(pathBloque);
	free(numeroBloque);
	int bloque_fd = open(pathBloque, O_RDWR, 0664);
	if (bloque_fd < 0) {
		//log_info(logger,
		//	string_from_format("No se pudo abrir el bloque %i", bloque));
		exit(1);
	}

	switch (accion) {
	case AGREGAR:
		ftruncate(bloque_fd, tamMensaje + tamOcupado);
		char*mensajeArchivo1 = mmap(NULL, tamMensaje + tamOcupado,
		PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, bloque_fd, 0);

		if (tamOcupado == 0) {
			memcpy(mensajeArchivo1, mensaje, tamMensaje + tamOcupado);
			//printf("%s", mensajeArchivo1);
		} else {
			struct stat stats;
			stat(pathBloque, &stats);
			char*data = malloc(stats.st_size);
			read(bloque_fd, data, stats.st_size);
			char* nuevoData = string_substring_until(data, tamOcupado);
			string_append(&nuevoData, mensaje);
			memcpy(mensajeArchivo1, nuevoData, tamOcupado + tamMensaje);
			free(data);
			free(nuevoData);
		}
		msync(mensajeArchivo1, bloque_fd, tamMensaje + tamOcupado); // revisar si modifica algo lo de msyc con tamOcupado+tamMensaje
		munmap(mensajeArchivo1, tamMensaje + tamOcupado);
		break;
	case MODIFICAR:
		ftruncate(bloque_fd, tamMensaje);
		char*mensajeArchivo2 = mmap(NULL, tamMensaje,
		PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, bloque_fd, 0);
		memcpy(mensajeArchivo2, mensaje, tamMensaje);
		msync(mensajeArchivo2, bloque_fd, tamMensaje); // revisar si modifica algo lo de msyc con tamOcupado+tamMensaje
		munmap(mensajeArchivo2, tamMensaje);
		break;
	}
	free(pathBloque);
	close(bloque_fd);
}
void crear_archivo_pokemon_metadata(char* pokemon, char* mensaje) {
	int tamanioMensaje = strlen(mensaje);
	char*pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	sem_wait(&mutexBitmap);
	int bloquePokemon = buscar_block_disponible(tamanioMensaje);
	char* itoaBloque = string_itoa(bloquePokemon);
	char* pathBloque = generar_path_bloque(itoaBloque);
	free(itoaBloque);

	actualizar_bitmap(1, bloquePokemon);
	sem_post(&mutexBitmap);
	escribir_mensaje_en_block(bloquePokemon, mensaje, AGREGAR);
	char* pathDirectorio = generar_path_directorio_pokemon(pokemon);
	crear_directorio(pathDirectorio);

	FILE* archivo = fopen(pathPokemon, "a");
	fprintf(archivo, "DIRECTORY=%s\n", "N");
	char* blocks = string_from_format("BLOCKS=[%d]\n", bloquePokemon);
	fprintf(archivo, blocks);
	/*char* pathPokemonMetadata = string_from_format("%s/Metadata.bin",
	 pathPokemon);*/
	char* tamanioOcupado = string_from_format("SIZE=%i\n",(tamanio_ocupado_bloque(pathBloque)));

	fprintf(archivo,tamanioOcupado);
	fprintf(archivo, "OPEN=%s", "N");
	fclose(archivo);
	log_debug(logger, "NEW_POKEMON: Se ha creado el archivo para el pokemon %s",
			pokemon);

	free(blocks);
	free(pathPokemon);
	free(pathDirectorio);
	free(pathBloque);
	free(tamanioOcupado);
	return;
}
