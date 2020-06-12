#include "TALL-GRASS.h"

/*char* tomar_datos_de_block (int nroBlock){
	log_debug(logger,("tomando los datos del block %d",nroBlock));
		FILE* block_f = fopen(string_from_format("%s/Blocks/%d.bin",ptoMontaje,nroBlock), "r");

		//calcular_tam_bloque(block_f) (podria tomarlo como una funcion aparte)

		int tamBlock;
		char* buffer;

		fseek(block_f,0L,SEEK_END);
		tamBlock = ftell (block_f);
		fseek(block_f,0L,SEEK_SET);

		buffer = malloc(tamBlock);
		fread(buffer, tamBlock, 1, block_f);
		buffer = string_substring_untill(buffer,tamBlock);
	return buffer;
}*/
void actualizar_bitmap(bool valor, int pos){
	if(valor)
		bitarray_set_bit(bitmap,pos);
	else
		bitarray_clean_bit(bitmap,pos);
	FILE*bitmap_f = fopen(string_from_format("%s/Metadata/Bitmap.bin",ptoMontaje),"w");
	fwrite(bitmap->bitarray,bitmap->size,1,bitmap_f);
	fclose(bitmap_f);
}
char* generar_path_bloque(char*bloque){
	char* pathBloque = string_new();
		string_append(&pathBloque,ptoMontaje);
		string_append(&pathBloque,"Blocks");
		if (!string_starts_with(bloque,"/")) string_append(&pathBloque,"/");
		string_append(&pathBloque,bloque);
		return pathBloque;
}
char** obtener_array_de_bloques(char*path){
	 t_config* c = config_create(path);
	 char** bloques = config_get_array_value(c,"BLOCKS");
	 config_destroy(c);
	 return bloques;
}

int tam_ocupado_en_el_block(char*path){
	 int tam;
	 FILE* block = fopen(path,"r");
	 fseek(block,0,SEEK_END);
	 tam = ftell(block);
	 fclose(block);
	 return tam;
}

int block_completo(char*path, int size){
	 if(tam_ocupado_en_el_block(path)<size) return false;
	 else return true;
}
int tam_disponible_en_el_block(char*path,int size){
	int tamDisponible;
	int tamOcupado = tam_ocupado_en_el_block(path);
	tamDisponible = size-tamOcupado;
	return tamDisponible;
}

int calcular_tamanio_archivo(char*path){
	int i;
	int tamanio=0;
	t_config* configArchivo = config_create(path);
	int size = config_get_int_value(configArchivo,"SIZE");
	char** blocks = obtener_array_de_bloques(path);

	for(i=0; i<(sizeof(blocks));i++)
	{
		char* pathBlock = generar_path_bloque(blocks[i]);
		if (block_completo(pathBlock,size)){
			tamanio=tamanio+size;
		}else {
			tamanio = tamanio + tam_ocupado_en_el_block(pathBlock);
		}
	}
	config_destroy(configArchivo);
	return tamanio;
}
int crear_block (){
	log_debug(logger,"crear nuevo block");

	int i;
	for (i = 0; i< bitmap->size /block_size; i++){
		if(bitarray_test_bit(bitmap,i)==0){
			FILE* f = fopen(string_from_format("%s/Blocks/%d.bin", ptoMontaje ,i),"w");
			if(f == NULL)
				return 1;
			fclose (f);
			actualizar_bitmap(true, i);
			log_debug(logger, "se creo el nuevo block: int %d",i);
			return i;
		}
	}
	log_debug(logger, "No se pudo crear el bloque");
	return -1;
}

void liberar_blocks(char** blockArr){
	log_debug(logger, "Liberando blocks");
	int i = 0;
	while (blockArr[i] != NULL){
		int bitPos = atoi(blockArr[i]);
		bitarray_clean_bit(bitmap,bitPos);
		i++;
	}
	FILE*bitmap_f = fopen(string_from_format("%s/Metadata/Bitmap.bin",ptoMontaje),"w");
	fwrite(bitmap->bitarray,bitmap->size,1,bitmap_f);
	fclose(bitmap_f);
	log_info(logger,"Se han liberado los blocks del Bitmap");
}

int validar_existencia_archivo(char* path){
	log_debug(logger,"Verificando si existe el archivo %d en el sistema de archivos", path);
	FILE* archivo = fopen(path,"r");
	if(archivo!=NULL){
	fclose (archivo);
		return true;
	} else return false;
}
void crear_directorio(char*path){
	mkdir(path,0777);
	log_debug(logger,"se ha creado el directorio %d",path);
}
void crear_metadata_directorio(char* dir){
	FILE* directorio = fopen((string_from_format("%s/metadata.bin",dir)),"a");
	fputs((string_from_format("DIRECTORY\n = %s","Y")),directorio);
	fclose(directorio);
}
char* crear_path_archivos(char* pokemon){
	char* pathArchivo = string_new();
	string_append(&pathArchivo,ptoMontaje);
	string_append(&pathArchivo,"Files");
	if (!string_starts_with(pokemon,"/")) string_append(&pathArchivo,"/");
	string_append(&pathArchivo,pokemon);
	return pathArchivo;
}

void iniciar_metadata(){
	log_debug(logger, "inicializando archivo Metadata");
	char*metadata = string_from_format("%s/Metadata/Metadata.bin", ptoMontaje);
	if(validar_existencia_archivo(metadata)){
		t_config* metadataFS = config_create(metadata);
		int cantBloques = config_get_int_value(metadataFS, "BLOCKS");
		int sizeBloque = config_get_int_value(metadataFS, "BLOCK_SIZE");
		if (cantBloques != blocks ||  sizeBloque != block_size){
				log_debug(logger,"Existe un filesystem con valores previos");
		}
		config_destroy(metadataFS);
		free(metadata);
	}else{

	FILE* archivo = fopen(string_from_format("%s/Metadata/Metadata.bin", ptoMontaje),"a");
	fprintf(archivo,"BLOCK_SIZE=%d",block_size);
	fprintf(archivo,"BLOCKS=%d",blocks);
	fprintf(archivo,"MAGIC_NUMBER=TALL_GRASS\n");


	fclose(archivo);
	log_debug(logger, "Se ha iniciado Metadata correctamente");
	}
}
void iniciar_bitmap(){
log_debug(logger, "Iniciando Bitmap");

	int sizeBitarray = blocks;
	if((sizeBitarray%8)!=0) sizeBitarray++;

	char*pathBitmap = string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje);

	if(validar_existencia_archivo(pathBitmap)){
		FILE* bitmap_f = fopen(pathBitmap, "rb");

		struct stat stats;
		fstat(fileno(bitmap_f),&stats);

		char* data =malloc(stats.st_size);
		fread(data,stats.st_size,1,bitmap_f);
		fclose(bitmap_f);

		bitmap = bitarray_create_with_mode(data,stats.st_size,LSB_FIRST);
	} else{
	bitmap = bitarray_create_with_mode(string_repeat('\0',sizeBitarray),sizeBitarray,LSB_FIRST);

	FILE* bitmap_f = fopen(string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje),"w");
	fwrite(bitmap->bitarray,sizeBitarray,1,bitmap_f);
	fclose(bitmap_f);
}
}

void iniciar_files_dir(){
	crear_directorio(string_from_format("%s/Files", ptoMontaje));
	crear_metadata_directorio(string_from_format("%s/Files", ptoMontaje));
}

void iniciar_blocks_dir(){
	crear_directorio(string_from_format("%/Blocks",ptoMontaje));
	crear_metadata_directorio(string_from_format("%s/Blocks",ptoMontaje));
}

void iniciar_metadata_dir(){
	crear_directorio(string_from_format("%s/Metadata", ptoMontaje));
	iniciar_metadata();
	iniciar_bitmap();
}

void iniciar_filesystem(){

	log_debug(logger,"inicializando filesystem TALLGRASS");
	iniciar_metadata_dir();
	iniciar_blocks_dir();
	iniciar_files_dir();
}

t_configFS* levantar_configuracion_filesystem(char* archivo){
	t_configFS* configTG = malloc(sizeof(t_configFS));
	t_config* configuracion = config_create(archivo);

		configTG->ptoEscucha = malloc(strlen(config_get_string_value(configuracion,"PUERTO_BROKER"))+1);
		strcpy(configTG->ptoEscucha,config_get_string_value(configuracion,"PUERTO_BROKER"));

		configTG->ptoMontaje =malloc(strlen(config_get_string_value(configuracion,"PUNTO_MONTAJE_TALLGRASS"))+1);
		strcpy(configTG->ptoMontaje,config_get_string_value(configuracion,"PUNTO_MONTAJE_TALLGRASS"));

		if(!string_ends_with(configTG->ptoMontaje,"/")) string_append(&configTG->ptoMontaje,"/");

		configTG->block_size = config_get_int_value(configuracion,"BLOCK_SIZE");
		configTG->blocks = config_get_int_value(configuracion,"BLOCKS");

		config_destroy(configuracion);
		return configTG;
}

t_configFS* crear_config(int argc, char* argv[]){
	if(argc>1){
			if(validar_existencia_archivo(argv[1])){
				configTG = levantar_configuracion_filesystem(argv[1]);
				log_info(logger,"se levanto correctamente la configuracion del filesystem");
			}else{
				log_error(logger,"El path recibido no corresponde a un filesystem en servicio");
			}
		}
		else if(validar_existencia_archivo(configuracionFS)){
			configTG = levantar_configuracion_filesystem(configuracionFS);
			log_info(logger, "La configuracion fue levantada correctamente");
		}else if (validar_existencia_archivo(string_substring_from(configuracionFS,3))){
			configTG = levantar_configuracion_filesystem(configuracionFS);
			log_info(logger, "configuracion levantada correctamente");
		}else{
			log_error(logger, "No se pudo levatar el archivo de configuración");
			exit(EXIT_FAILURE);
		}return configTG;
}
/*bool archivo_abierto(char* pokemon){
	FILE* archivo = buscar_archivo(pokemon);
	FILE* puntero = &(fseek(archivo,0L,SEEK_END)-1);
	if (*puntero == 'y') return true;
	else return false;
}*/

/*FILE* buscar_archivo(char* pokemon){}*/

/*bool es_directorio(char*path){
	char* subCarpeta = string_substring_untill(path,string_pos_char(path, '/'));
	if (subCarpeta == NULL) {
		return true;
		log_debug(logger, "%s es un directorio",path);
	} else return false;
	log_info(logger, "%s es un archivo", path);
}*/

void crear_files_metadata(char*pokemon){
	int posPrimerBloque = crear_block();
	FILE* archivo = fopen(string_from_format("%s/Metadata.bin",crear_path_archivos(pokemon)),"a");
		fputs(string_from_format("DIRECTORY=%s\n",'N'),archivo);
		fputs(string_from_format("SIZE=%s\n'",calcular_tamanio_archivo(pokemon)), archivo);
		char* blocks = string_from_format("BLOCKS=[%d]", posPrimerBloque);
		fputs (blocks,archivo);
		fputs(string_from_format("OPEN=%s\n",'N'), archivo);
	fclose(archivo);
		log_debug(logger,"El archivo se ha creado exitosamente");
		return ;
}


