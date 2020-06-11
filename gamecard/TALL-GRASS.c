#include <stdio.h>
#include <stdlib.h>
#include "TALL-GRASS.h"

char* tomar_datos_de_block (int nroBlock){
	log_debug(logger,"tomando los datos del block %d",nroBlock);
	FILE* block_f = fopen(string_from_format("%s/Blocks/%d.bin",ptoMontaje,nroBlock), "r");

	//calcular_tam_bloque(block_f) (podria tomarlo como una funcion aparte)

	int tamBlock;
	char* buffer;

	fseek(block_f,0L,SEEK_END);
	tamBlock = ftell (block_f);
	fseek(block_f,0L,SEEK_SET);

	buffer = malloc(tamBlock);
	fread (buffer, tamBlock, 1, block_f);
	buffer = string_substring_untill(buffer,tamBlock);
	return buffer;
}
void iniciar_filesystem(){
	log_debug(logger,"inicializando filesystem TALLGRASS");

	iniciar_metadata_dir ();
	iniciar_blocks_dir();
	iniciar_files_dir();

}
t_configFS* crear_config(int argc, char* argv[]){
	if(argc>1){
		if(validar_existencia_archivo(argv[1])){
			configTG =levantar_configuracion_filesystem(argv[1]);
			log_info(logger,"se levanto correctamente la configuracion del filesystem");
		}else{
			log_error(logger, "El path recibido no corresponde a un filesystem en servicio");
		}
	}
	else if(validar_existencia_archivo(config)){
		configTG=levantar_configuracion_filesystem(config);
		log_info(logger, "La configuracion fue levantada correctamente");
	}else if (validar_existencia_archivo(string_substring_from(config,3))){
		configTG= levantar_configuracion_filesystem(config);
		log_info(logger, "configuracion levantada correctamente");
	}else{
		log_error(logger, "No se pudo levatar el archivo de configuración");
		exit(EXIT_FAILURE);
	}
	return configTG;
}

t_configFS* levantar_configuracion_filesystem(char*archivo){
	t_configFS* configTG = malloc(sizeof(t_configFS));
	t_config configuracion =config_create(archivo);

	configTG->ptoEscucha = malloc(strlen(config_get_string_value(configuracion,"PUERTO_BROKER"))+1)
	strcpy(configTG->ptoEscucha,config_get_string_value(configuracion,"PUERTO_BROKER"));

	configTG->ptoMontaje =malloc(strlen(config_get_string_value(configuracion,"PUNTO_MONTAJE_TALLGRASS"))+1);
	strcpy(configTG->ptoMontaje,config_get_string(configuracion,"PUNTO_MONTAJE_TALLGRASS"));
	if(!string_ends_with(configTG->ptoMontaje,"/")) string_append(&configTG->ptoMontaje,"/");

	configTG->block_size = config_get_int_value(configuracion,"BLOCK_SIZE");
	configTG->blocks = config_get_int_value(configuracion,"BLOCKS");

	config_destroy(configuracion);
	return configTG;
}
void iniciar_metadata_dir(){
	crear_directorio(string_from_format("%s/Metadata", ptoMontaje));
	iniciar_metadata();
	iniciar_bitmap();
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
		}config_destroy(metadata);
		free(metadataFS);
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
		fread(data,stats.st_stize);
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
	crear_directorio(string_from_format("%/Blocks"),ptoMontaje);
	crear_metadata_directorio(string_from_format("%s/Blocks",ptoMontaje));
}
void actualizar_bitmap(bool valor, int pos){
	if(valor)
		bitarray_set_bit(bitmap,pos);
	else
		bitarray_clean_bit(bitmap,pos);
	FILE*bitmap_f = fopen(string_from_format("%s/Metadata/Bitmap.bin",ptoMontaje),"w");
	fwrite(bitmap->bitarray,bitmap->size,1,bitmap_f);
	fclose(bitmap_f);
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
bool validar_existencia_archivo(char* path){

	log_info(logger,"Verificando si existe el archivo %s en el sistema de archivos", path);

	FILE* archivo = fopen(path,"r");
	if(archivo!=NULL){
		fclose (archivo);
		return true;
	}
	return false;
}
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
	FILE* archivo = fopen(string_from_format("%s/Metadata.bin",crear_path_archivos(pokemon)));
		fputs("DIRECTORY=%s\n",'N',archivo);
		fputs("SIZE=%s\n'", 0 , archivo);
		char* blocks = string_from_format("BLOCKS=[%d]", posPrimerBloque);
		fputs (blocks,archivo);
		fputs("OPEN=%s\n",'N', archivo);
	fclose(archivo);
		log_debug(logger,"El archivo se ha creado exitosamente");
		return ;
}
void crear_directorio(char*path){
	mkdir(path,0777);
	log_debug(logger,"se ha creado el directorio %d",path);
}
void crear_files_metadata_directorio(char* dir){
	FILE* directorio = fopen(("%s/metadata.bin",dir),"a");
	fputs ("DIRECTORY\n = %s",'Y', directorio);
	fclose(directorio);
}
char* crear_path_archivos(char*pokemon){
	char* pathArchivo = string_new();
	string_append(&pathArchivo,ptoMontaje);
	string_append(&pathArchivo,"Files");
	if (!string_starts_with(pokemon,'/')) strig_append(&pathArchivo,'/');
	string_append(&pathArchivo,pokemon);
	return pathArchivo;
}
bool archivo_abierto(char* pokemon){
	FILE* archivo = buscar_archivo(pokemon);
	FILE* puntero = &(fseek(archivo,0L,SEEK_END)-1);
	if (*puntero == 'y') return true;
	else return false;
}

FILE* buscar_archivo(char* pokemon){

}

int main() {

}
