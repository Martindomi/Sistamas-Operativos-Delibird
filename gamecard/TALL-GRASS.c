#include "TALL-GRASS.h"

char* generar_linea_de_entrada_mensaje(int posX, int posY, int cant) {
	char* entradaMensaje = string_new();
	char* aux = string_from_format("%i-%i=%i\n",posX,posY,cant);
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
	char* aux = string_from_format("%s/Files/%s",ptoMontaje,pokemon);
	string_append(&pathArchivo,aux);
	/*string_append(&pathArchivo, "/Files");
	if (!string_starts_with(pokemon, "/"))
		string_append(&pathArchivo, "/");
	string_append(&pathArchivo, pokemon);*/
	free(aux);
	return pathArchivo;
}
char* generar_path_archivo_pokemon_metadata(char*pokemon) {
	char* path = string_new();
	char* aux = string_from_format("%s/Metadata.bin",(generar_path_directorio_pokemon(pokemon)));
	string_append(&path,aux);
	/*string_append(&path, generar_path_directorio_pokemon(pokemon));
	string_append(&path, string_from_format("/Metadata.bin"));*/
	free(aux);
	return path;
}
char* generar_path_bloque(char* bloque) {
	char* pathBloque = string_new();
	char* aux = string_from_format("%s/Blocks/%s.bin",ptoMontaje,bloque);
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
char** obtener_array_de_bloques(char*path) {
	t_config* c = config_create(path);
	char**bloques = config_get_array_value(c, "BLOCKS");

	config_destroy(c);

	return bloques;
}
int tam_array_bloques(char** bloques) {
	int j;
	for (j = 0; bloques[j] != NULL; j++) {
	}
	return j;
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
int crear_block() {
	log_debug(logger, "crear nuevo block");

	int i;
	for (i = 0; i < blocks; i++) {
		int block_fd = open(
				string_from_format("%s/Blocks/%d.bin", ptoMontaje, i),
				O_CREAT | O_RDWR, 0664);
		if (block_fd < 0) {
		//	log_error(logger, "INICIO FILE SYSTEM :: BLOQUES : No se creo el bloque correctamente");
			exit(1);
		}
		close(block_fd);
	}
	//log_debug(logger, "INICIO FILE SYSTEM :: BLOQUES: se crearon %i bloques", i - 1);
	return 1;
}
int cantidad_digitos(int numero) {
	int count = 0;
	while (numero != 0) {
		numero /= 10;
		++count;
	}
	return count;
}
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
void actualizar_tamanio_archivo(char*path) {
	t_config* configuracion = config_create(path);
	int tamanio = config_get_int_value(configuracion, "SIZE");

	tamanio = calcular_tamanio_archivo(path);

	config_set_value(configuracion, "SIZE", string_itoa(tamanio));
	config_save(configuracion);
	config_destroy(configuracion);
}
void agregar_block_al_metadata(int block, char* pathPokemon) {
	t_config* configuracion = config_create(pathPokemon);
	char* bloques = string_new();
	string_append(&bloques, config_get_string_value(configuracion, "BLOCKS"));

	bloques[strlen(bloques) - 1] = '\0';
	if (strcmp(bloques, "[")) {
		string_append(&bloques, ",");
	}
	string_append(&bloques, string_itoa(block));
	string_append(&bloques, "]");

	config_set_value(configuracion, "BLOCKS", bloques);
	config_save(configuracion);
	config_destroy(configuracion);

	free(bloques);
	return;
}
void crear_directorio(char*path) {
	mkdir(path, 0777);
	//log_debug(logger, "INICIO FILE SYSTEM :: DIRECTORIO: Se ha creado el directorio %d", path);
}
void crear_metadata_directorio(char* dir) {
	FILE* directorio = fopen((string_from_format("%s/Metadata.bin", dir)), "a");
	fputs((string_from_format("DIRECTORY=%s\n", "Y")), directorio);
	fclose(directorio);
}
int archivo_abierto(char* path) {
	t_config* configuracion = config_create(path);
	char* estado = config_get_string_value(configuracion, "OPEN");

	if (strcmp(estado, "Y") == 0) {
		return true;
	} else
		return false;

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
		crear_directorio(string_from_format("%s/Metadata", ptoMontaje)); /*modif1*/
		FILE* archivo = fopen(metadata, "a");
		fprintf(archivo, "BLOCK_SIZE=%d\n", block_size);
		fprintf(archivo, "BLOCKS=%d\n", blocks);
		fprintf(archivo, "MAGIC_NUMBER=TALL_GRASS\n");

		fclose(archivo);

		//log_debug(logger, "INICIO FILE SYSTEM :: METADATA: Se ha iniciado  correctamente");
	}
	free(metadata);
}
void actualizar_bitmap(bool valor, int pos) {
	char*pathBitmap = string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje);
	//sem_wait(&mutexBitmap);
	int bitmap_fd = open(pathBitmap, O_CREAT | O_RDWR, 0664);

	if (bitmap_fd < 0) {
		//log_error(logger, "BITMAP: no se pudo actualizar el archivo bitmap");
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
	//sem_post(&mutexBitmap);
	//log_debug(logger,"BITMAP: Se actualizó correctamente el valor de la posicion %i",pos);
	return;
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
		//log_error(logger, "BITMAP: No se pudo abrir el archivo");
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
	log_debug(logger,"INICIO FILE SYSTEM :: BITMAP: Se ha inicializado correctamente el bitmap");
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
	//	log_error(logger, "BITMAP: No se puede abrir el archivo");
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
void iniciar_files_dir() {
	if (!validar_existencia_archivo(
			string_from_format("%s/Files/Metadata.bin", ptoMontaje))) {
		crear_directorio(string_from_format("%s/Files", ptoMontaje));
		crear_metadata_directorio(string_from_format("%s/Files", ptoMontaje));
	}
}
void iniciar_blocks_dir() {
	if (bitmap_vacio(
			string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje))) {
		crear_directorio(string_from_format("%s/Blocks", ptoMontaje));
		crear_block();
		//log_info(logger, "existen valores previos");
	}
}
void iniciar_metadata_dir() {
	iniciar_metadata();
	iniciar_bitmap();
}
void montar_punto_montaje(){
	if(!validar_existencia_archivo(ptoMontaje)){
		crear_directorio(ptoMontaje);
	}
}
void iniciar_filesystem() {
	log_debug(logger, "inicializando filesystem TALLGRASS");
	montar_punto_montaje();
	iniciar_metadata_dir();
	iniciar_blocks_dir();
	iniciar_files_dir();
}
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
t_configFS* crear_config(int argc, char* argv[]) {
	if (argc > 1) {
		if (validar_existencia_archivo(argv[1])) {
			configTG = levantar_configuracion_filesystem(argv[1]);
			log_info(logger,
					"INICIO FILE SYSTEM: se levanto correctamente la configuracion del filesystem");
		} else {
		//	log_error(logger,
				//	"INICIO FILE SYSTEM: El path recibido no corresponde a un filesystem en servicio");
		}
	} else if (validar_existencia_archivo(configuracionFS)) {
		configTG = levantar_configuracion_filesystem(configuracionFS);
		log_info(logger, "INICIO FILE SYSTEM: La configuracion fue levantada correctamente");
	} else if (validar_existencia_archivo(
			string_substring_from(configuracionFS, 3))) {
		configTG = levantar_configuracion_filesystem(configuracionFS);
		log_info(logger, "INICIO FILE SYSTEM: La configuracion levantada correctamente");
	} else {
		//log_error(logger, "INICIO FILE SYSTEM: No se pudo levatar el archivo de configuración");
		exit(EXIT_FAILURE);
	}
	return configTG;
}
void crear_archivo_pokemon_metadata(char* pokemon, char* mensaje) {
	int tamanioMensaje = strlen(mensaje);
	char*pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	sem_wait(&mutexBitmap);
	int bloquePokemon = buscar_block_disponible(tamanioMensaje);
	char* pathBloque = generar_path_bloque(string_itoa(bloquePokemon));

	actualizar_bitmap(1, bloquePokemon);
	sem_post(&mutexBitmap);
	escribir_mensaje_en_block(bloquePokemon, mensaje, AGREGAR);
	crear_directorio(generar_path_directorio_pokemon(pokemon));

	FILE* archivo = fopen(pathPokemon, "a");
	fprintf(archivo, string_from_format("DIRECTORY=%s\n", "N"));
	char* blocks = string_from_format("BLOCKS=[%d]\n", bloquePokemon);
	fprintf(archivo, blocks);
	char* pathPokemonMetadata = string_from_format("%s/Metadata.bin",
			pathPokemon);
	fprintf(archivo,
			string_from_format("SIZE=%i\n",
					(tamanio_ocupado_bloque(pathBloque))));
	fprintf(archivo, string_from_format("OPEN=%s", "N"));
	fclose(archivo);
	log_debug(logger,"NEW_POKEMON: Se ha creado el archivo para el pokemon %s",pokemon);
	return;
}
void quitar_bloque_de_metadata(char*path, char* bloque) {
	t_config* cfg = config_create(path);
	char* bloques = string_new();
	char*bloques1 = string_new();
	string_append(&bloques1, config_get_string_value(cfg, "BLOCKS"));
	if(string_length(bloques1) == 3){
	string_append(&bloques, "[]");
	}else{
	char**listaDeBloques = string_n_split(bloques1, 2, bloque);
	string_append(&bloques, listaDeBloques[0]);
	bloques[strlen(bloques) - 1] = '\0';
	string_append(&bloques, listaDeBloques[1]);
	}
	config_set_value(cfg, "BLOCKS", bloques);
	config_save(cfg);
	config_destroy(cfg);
	free(bloques1);
	free(bloques);
	return;
}
void vaciar_bloque(char* bloque) {

	char* pathBloque = generar_path_bloque(bloque);
	int bloque_fd = open(pathBloque, O_RDWR, 0664);
	if (bloque_fd < 0) {
		//log_error(logger,
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
}
void liberar_bloque(char* bloque, char*pathPokemon) {
	sem_wait(&mutexBitmap);
	vaciar_bloque(bloque);
	actualizar_bitmap(0, atoi(bloque));
	quitar_bloque_de_metadata(pathPokemon, bloque);
	sem_post(&mutexBitmap);

}

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
		log_debug(logger,
				string_from_format("BLOCKS: el bloque tiene %i bytes disponibles",
						tamDisponible));
	}
	return tamDisponible;
}
int block_completo(char*path) {
	if (tamanio_ocupado_bloque(path) == block_size)
		return true;
	else
		return false;
}
int calcular_tamanio_archivo(char*path) {
	int i;
	int tamanio = 0;
	char** blocks = obtener_array_de_bloques(path);
	if(tam_array_bloques(blocks) == 0){
		tamanio = 0;
	}else{
	for (i = 0; i < (tam_array_bloques(blocks)); i++) {
		char* pathBlock = generar_path_bloque(blocks[i]);
		tamanio = tamanio + tamanio_ocupado_bloque(pathBlock);
	}
	}
	return tamanio;
}
void escribir_mensaje_en_block(int bloque, char* mensaje, int accion) {
	char* pathBloque = generar_path_bloque(string_itoa(bloque));
	int tamMensaje = string_length(mensaje);
	int tamOcupado = tamanio_ocupado_bloque(pathBloque);

	int bloque_fd = open(pathBloque, O_RDWR, 0664);
	if (bloque_fd < 0) {
		//log_error(logger,
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

	close(bloque_fd);
}
void agregar_nuevo_mensaje(char* mensaje, char*pathPokemon) {
	int tamMensaje = string_length(mensaje);
	char** bloques = obtener_array_de_bloques(pathPokemon);
	int tamArray = tam_array_bloques(bloques);

	if (tamArray == 0) {
		sem_wait(&mutexBitmap);
		int bloqueNuevo = buscar_block_disponible();
		actualizar_bitmap(1, bloqueNuevo);
		sem_post(&mutexBitmap);
		escribir_mensaje_en_block(bloqueNuevo, mensaje, AGREGAR);
		agregar_block_al_metadata(bloqueNuevo, pathPokemon);
		actualizar_tamanio_archivo(pathPokemon);

	} else {
		char*path = generar_path_bloque(bloques[tamArray - 1]);
		int tamDisponible = tam_disponible_en_bloque(path);
		if (tamMensaje <= tamDisponible) {
			escribir_mensaje_en_block(atoi(bloques[tamArray - 1]), mensaje,
			AGREGAR);
			actualizar_tamanio_archivo(pathPokemon);

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
			escribir_mensaje_en_block(bloqueNuevo, mensajeBloqueNuevo, AGREGAR);

			agregar_block_al_metadata(bloqueNuevo, pathPokemon);
			actualizar_tamanio_archivo(pathPokemon);

			free(mensajeBloqueFinal);
			free(mensajeBloqueNuevo);
		}
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
				string_append(&mensajePorBloque,
						string_substring_until(contenido2,
								(block_size - (j - tamContenido))));
				free(contenido2);
			} else {
				char* contenidoAux = string_substring_from(contenido,
						j - block_size);
				string_append(&mensajePorBloque,
						string_substring_until(contenidoAux, j));
				free(contenidoAux);
			}
			escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,
			MODIFICAR);
			j = j + block_size;
		}
	} else {
		if (cantBloques > tamArray) {
			j = block_size;
			for (i = 0; i < tamArray; i++) {
				char*mensajePorBloque = string_new();
				char*contenidoAux = string_substring_from(contenido,
						j - block_size);
				string_append(&mensajePorBloque,
						string_substring_until(contenidoAux, j));
				escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,
				MODIFICAR);
				j = j + block_size;
				free(contenidoAux);
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
					string_append(&mensajePorBloque,
							string_substring_until(contenidoMuletita,
									tamContenido - j));
					free(contenidoMuletita);
				} else {
					char*contenidoM = string_substring_from(contenido, j);
					string_append(&mensajePorBloque,
							string_substring_until(contenidoM, j + block_size));
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
						string_append(&mensajePorBloque,
								string_substring_until(contenidoAux,
										tamContenido - j));
					} else {
						char* contenidoAux = string_substring_from(contenido,
								j);
						string_append(&mensajePorBloque,
								string_substring_until(contenidoAux,
										j + block_size));
					}
					escribir_mensaje_en_block(atoi(bloques[i]),
							mensajePorBloque, MODIFICAR);
					j = j + block_size;
				}
			}
		}
	}
}
void tratar_mensaje_NEW_POKEMON(int posX, int posY, int cant, char* pokemon) {
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	char* mensaje = generar_linea_de_entrada_mensaje(posX, posY, cant);
	//sem_t* semaforoNewPokemon = (sem_t*)dictionary_get(dicSemaforos, pokemon);

	//sem_wait(semaforoNewPokemon);

	if (validar_existencia_archivo(pathPokemon)) {
		//sem_post(semaforoNewPokemon);


		if (!archivo_abierto(pathPokemon)) {
			//sem_wait(semaforoNewPokemon);

			abrir_archivo(pathPokemon);
			//sem_post(semaforoNewPokemon);

			//char* mensaje = generar_linea_de_entrada_mensaje(posX, posY, cant);
			int i = 0;
			char* contenidoBloques = string_new();
			char** bloques = obtener_array_de_bloques(pathPokemon);

			for (i = 0; i < tam_array_bloques(bloques); i++) {
				char*pathBloque = generar_path_bloque(bloques[i]);
				int bloque_fd = open(pathBloque, O_RDWR, 0664);

				if (bloque_fd < 0) {
					log_error(logger, "No se pudo abrir el archivo");
					error(1);
				}
				struct stat stats;
				stat(pathBloque, &stats);

				char*data = malloc(stats.st_size);
				read(bloque_fd, data, stats.st_size);

				string_append(&contenidoBloques,
						string_substring_until(data, stats.st_size));
				free(data);
				free(pathBloque);
				close(bloque_fd);
			}
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
					} else {
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
				string_append(&contenidoFinal,
						string_substring_until(contenidoBloques,
								(i + (string_length(mensajeCorto)))));
				string_append(&contenidoFinal, string_itoa(nuevaCantidad));
				string_append(&contenidoFinal,
						string_substring_from(stringCant,
								cantidad_digitos(cantActual)));

				tratar_contenido_en_bloques(contenidoFinal, pathPokemon);
				actualizar_tamanio_archivo(pathPokemon);
			} else {
				agregar_nuevo_mensaje(mensaje, pathPokemon);
			}
			//printf("Cerrar archivo %d\n", tiempo_retardo_operacion);
			sleep(tiempo_retardo_operacion);
			log_info(logger,"NEW_POKEMON: Se ha modificado el contenido del archivo %s",pokemon);
			//sem_wait(semaforoNewPokemon);

			cerrar_archivo(pathPokemon);
			//sem_post(semaforoNewPokemon);

		} else {
			//printf("Reintenta operacion\n");
			//sem_post(semaforoNewPokemon);

			sleep(tiempo_de_reintento_operacion);
			tratar_mensaje_NEW_POKEMON(posX,posY,cant,pokemon);

		}
	} else {
		//printf("No existe archivo\n");
		//char* mensaje = generar_linea_de_entrada_mensaje(posX, posY, cant);

		crear_archivo_pokemon_metadata(pokemon, mensaje);
		sleep(tiempo_retardo_operacion);
		//sem_post(semaforoNewPokemon);

		log_info(logger,"NEW_POKEMON: Se ha modificado el contenido del archivo %s",pokemon);

	}
	free(mensaje);
}
char* tratar_mensaje_CATCH_POKEMON(int posX, int posY, char*pokemon) {
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	if (validar_existencia_archivo(pathPokemon)) {
		if (!archivo_abierto(pathPokemon)) {
			abrir_archivo(pathPokemon);
			char* mensaje = string_from_format("%i-%i=", posX, posY);

			int i = 0;
			char* contenidoBloques = string_new();
			char** bloques = obtener_array_de_bloques(pathPokemon);

			for (i = 0; i < tam_array_bloques(bloques); i++) {
				char*pathBloque = generar_path_bloque(bloques[i]);
				int bloque_fd = open(pathBloque, O_RDWR, 0664);

				if (bloque_fd < 0) {
					log_error(logger, "No se pudo abrir el archivo");
					error(1);
				}
				struct stat stats;
				stat(pathBloque, &stats);

				char*data = malloc(stats.st_size);
				read(bloque_fd, data, stats.st_size);

				string_append(&contenidoBloques,
						string_substring_until(data, stats.st_size));
				free(data);
				free(pathBloque);
				close(bloque_fd);
			}
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
					} else
						break;
				}
				char* stringCant = string_substring_from(contenidoBloques,
						(i + string_length(mensaje)));
				char** listaDeContenidosTotal = string_n_split(stringCant, 2,
						"\n");
				int cantActual = atoi(listaDeContenidosTotal[0]);
				int nuevaCantidad = cantActual - 1;
				char*contenidoFinal = string_new();
				if (nuevaCantidad == 0) {
					if(i == 0) {
						if((string_length(mensaje)+cantidad_digitos(cantActual)+1) == string_length(contenidoBloques)){
							string_append(&contenidoFinal,string_repeat('\0',string_length(contenidoBloques)));
						}else{
							string_append(&contenidoFinal,
							string_substring_from(contenidoBloques, (i + string_length(mensaje) + cantidad_digitos(cantActual) + 1)));
					}
					} else {
						string_append(&contenidoFinal,
								string_substring_until(contenidoBloques, (i - 1)));
						string_append(&contenidoFinal,
								string_substring_from(stringCant,
										(cantidad_digitos(cantActual))));
					}
				} else {
					if (nuevaCantidad < 0) {
						log_error(logger,
								"Ocurrio un error con las posiciones mencionadas");
						return "fail";
					} else {
						string_append(&contenidoFinal,
								string_substring_until(contenidoBloques,
										(i + (string_length(mensaje)))));
						string_append(&contenidoFinal,
								string_itoa(nuevaCantidad));
						string_append(&contenidoFinal,
								string_substring_from(stringCant,
										cantidad_digitos(cantActual)));
					}

				}
				tratar_contenido_en_bloques(contenidoFinal, pathPokemon);
				actualizar_tamanio_archivo(pathPokemon);
				sleep(tiempo_retardo_operacion);
				log_info(logger,"CATCH_POKEMON: Se ha modificado el contenido del archivo %s",pokemon);
				cerrar_archivo(pathPokemon);
				free(contenidoFinal);
				return "OK";

			}
			log_error(logger,
					"CATCH_POKEMON: No hay ningun pokemon %s en la posicion %i-%i solicitada",
					pokemon, posX, posY);
			return "FAIL";
		} else {
			sleep(tiempo_de_reintento_operacion);
			tratar_mensaje_CATCH_POKEMON(posX,posY,pokemon);
		}
	} else {
		log_error(logger,
				"CATCH_POKEMON; No existe el pokemon %s dentro del sistema de archivos",
				pokemon);
		return "FAIL";
	}
}
t_list* tratar_mensaje_GET_POKEMON(char*pokemon) {
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	t_list* listadoPos = list_create();
	if (validar_existencia_archivo(pathPokemon)) {
		if(!archivo_abierto(pathPokemon)){
		abrir_archivo(pathPokemon);
		int i = 0;
		char* contenidoBloques = string_new();
		char** bloques = obtener_array_de_bloques(pathPokemon);

		for (i = 0; i < tam_array_bloques(bloques); i++) {
			char*pathBloque = generar_path_bloque(bloques[i]);
			int bloque_fd = open(pathBloque, O_RDWR, 0664);

			if (bloque_fd < 0) {
				//log_error(logger, "No se pudo abrir el archivo");
				error(1);
			}
			struct stat stats;
			stat(pathBloque, &stats);

			char*data = malloc(stats.st_size);
			read(bloque_fd, data, stats.st_size);

			string_append(&contenidoBloques,
					string_substring_until(data, stats.st_size));
			free(data);
			free(pathBloque);
			close(bloque_fd);
		}
		int h, j;
		int siguientePos;

		for (h = 0; h < string_length(contenidoBloques); h = siguientePos) {
			for (j = h; contenidoBloques[j] != '\n'; j++)
				;

			char*mensajeAux = string_new();
			char*mensajeAux2 = string_substring_from(contenidoBloques, h);
			string_append(&mensajeAux,
					string_substring_until(mensajeAux2, j - h));

			char** listaMensajeAux = string_n_split(mensajeAux, 2, "=");
			char**listaMensaje = string_n_split(listaMensajeAux[0], 2, "-");

			list_add(listadoPos, atoi(listaMensaje[0]));
			list_add(listadoPos, atoi(listaMensaje[1]));

			siguientePos = j + 1;
		}
		sleep(tiempo_retardo_operacion);
		cerrar_archivo(pathPokemon);
		return listadoPos;

	}else{
		sleep(tiempo_de_reintento_operacion);
		tratar_mensaje_GET_POKEMON(pokemon);
	}
	} else {
		log_error(logger, "GET_POKEMON: No existe el pokemon %s dentro del file system",
				pokemon);
		list_clean(listadoPos);
		return listadoPos;
	}
}
