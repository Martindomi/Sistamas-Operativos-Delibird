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
char* generar_linea_de_entrada_mensaje(int posX, int posY, int cant){
	char* entradaMensaje = string_new();
			string_append(&entradaMensaje,string_from_format("%i",posX));
			string_append(&entradaMensaje,"-");
			string_append(&entradaMensaje,string_from_format("%i",posY));
			string_append(&entradaMensaje,"=");
			string_append(&entradaMensaje,string_from_format("%i",cant));
			string_append(&entradaMensaje,"\n");
			return entradaMensaje;
}
char* crear_path_archivos(char* pokemon){
	char* pathArchivo = string_new();
	string_append(&pathArchivo,ptoMontaje);
	string_append(&pathArchivo,"/Files");
	if (!string_starts_with(pokemon,"/")) string_append(&pathArchivo,"/");
	string_append(&pathArchivo,pokemon);
	return pathArchivo;
}
char* generar_path_pokemon(char*pokemon){
	char* path = string_new();
	string_append(&path,crear_path_archivos(pokemon));
	string_append(&path,string_from_format("/Metadata.bin"));
	return path;

}
char* generar_path_bloque(char* bloque){

	char* pathBloque = string_new();
		string_append(&pathBloque,ptoMontaje);
		string_append(&pathBloque,"/Blocks");
		if (!string_starts_with(bloque,"/")) string_append(&pathBloque,"/");
		string_append(&pathBloque,bloque);
		string_append(&pathBloque,".bin");
		return pathBloque;
}
char** obtener_array_de_bloques(char*path){
	 t_config* c = config_create(path);
	 char**bloques = config_get_array_value(c,"BLOCKS");

	 config_destroy(c);

	 return bloques;
}
int tam_array_bloques(char** bloques){
	int j;
	for(j=0; bloques[j]!= NULL; j++){}
	return j;
	}
char* bloque_contenedor_linea(int posX,int posY,char* pathPokemon){
	char** bloques = obtener_array_de_bloques(pathPokemon);
	int i;

	for(i=0;i<tam_array_bloques(bloques);i++){

	char* pathBloque= generar_path_bloque(bloques[i]);
	int tamBloque = tam_ocupado_en_el_block(pathBloque);

	FILE*bloque = fopen((pathBloque),"rb");
		fseek(bloque,0,SEEK_SET);
		char stringArchivoEntero[tamBloque] ;
		fread(stringArchivoEntero,tamBloque,1,bloque);
		char*mensaje = string_new();
		string_append(&mensaje, string_from_format("%i-%i=",posX,posY));


		if(string_contains(stringArchivoEntero,mensaje)){
			log_info(logger,string_from_format("El mensaje se encuentra en el bloque %s",bloques[i]));
				return bloques[i];
		}else{
			log_info(logger,string_from_format("la linea no se encuentra en el bloque %i",i));
		}
		fclose(bloque);
	}
		return NULL;
}
void mensaje_new_pokemon(int posX,int posY,int cant,char* pokemon){

	char*pathPokemon = generar_path_pokemon(pokemon);
	if(validar_existencia_archivo(pathPokemon)){
		//if(!archivo_abierto(pathPokemon)){
		abrir_archivo(pathPokemon);
		if(verificar_existencia_posiciones(posX,posY,pathPokemon)){

			char* bloque = bloque_contenedor_linea(posX,posY,pathPokemon);
			char*pathBloque= generar_path_bloque(bloque);
			modificar_archivo_NEW_POKEMON(posX,posY,cant,pathBloque);
			//actualizar_tamanio_archivo(string_length(mensaje),pathPokemon);

		}else{
			agregar_mensaje_NEW_POKEMON(posX,posY,cant,pathPokemon);
		}

	}else{
		char* mensaje = generar_linea_de_entrada_mensaje(posX,posY,cant);
		crear_files_metadata(pokemon,mensaje);
	}

	cerrar_archivo(pathPokemon);
	return;
}





void actualizar_bitmap(bool valor, uint pos){
	if(valor)
		bitarray_set_bit(bitmap,pos);
	else
		bitarray_clean_bit(bitmap,pos);
	FILE*bitmap_f = fopen(string_from_format("%s/Metadata/Bitmap.bin",ptoMontaje),"w");
	fwrite(bitmap->bitarray,bitmap->size,1,bitmap_f);
	fclose(bitmap_f);
}



int tam_ocupado_en_el_block(char*path){
	 int tam;
	 FILE* block = fopen(path,"r");
	 fseek(block,0,SEEK_END);
	 if(ftell(block)== NULL){
		 tam =0;
	 } else {
		 tam = ftell(block);
	 };
	 fclose(block);
	 return tam;
}

int block_completo(char*path){
	 if(tam_ocupado_en_el_block(path)< block_size) return false;
	 else return true;
}
int tam_disponible_en_el_block(char*path){
	int tamDisponible;
	int size = block_size;
	int tamOcupado = tam_ocupado_en_el_block(path);
	int tamDisponiblePrevio = size-tamOcupado;

	if (tamDisponiblePrevio <= 0){

		 tamDisponible = 0;
		log_debug(logger,"no tiene espacio disponible");
	}else{
		tamDisponible = tamDisponiblePrevio;
		log_debug(logger,string_from_format("el bloque tiene %i bytes disponibles",tamDisponible));
	}
return  tamDisponible;
}
int calcular_tamanio_archivo(char*path){
	int i;
	int tamanio=0;
	char** blocks = obtener_array_de_bloques(path);

	for(i=0; i<(sizeof(blocks));i++)
	{
		int numBloque = (i/block_size);
		char* pathBlock = generar_path_bloque(blocks[numBloque]);
		if (block_completo(pathBlock)){
			tamanio=tamanio+block_size;
		}else {
			tamanio = tamanio + tam_ocupado_en_el_block(pathBlock);
		}
	}
	return tamanio;
}

int buscar_block_disponible(int tam){
	int bloqueLibre;
	log_info(logger, "buscando espacio disponible");
	for(bloqueLibre=0;bitarray_test_bit(bitmap,bloqueLibre)&& (bloqueLibre<blocks); bloqueLibre++){
		log_info(logger,"el bloque disponible es %i",bloqueLibre);
	}
	if (bloqueLibre>=blocks) return NO_MORE_BLOCKS;
	return bloqueLibre;
}
void actualizar_tamanio_archivo(int tam,char*path){
	t_config* configuracion = config_create(path);
	int tamanio = config_get_int_value(configuracion,"SIZE");

	tamanio += tam;

	config_set_value(configuracion,"SIZE",string_itoa(tamanio));
	config_save(configuracion);
	config_destroy(configuracion);
}
void agregar_block_al_metadata(int block,char* pathPokemon){
	t_config* configuracion = config_create(pathPokemon);
	char* bloques = string_new();
	string_append(&bloques,config_get_string_value(configuracion,"BLOCKS"));

	bloques[strlen(bloques)-1]='\0';
	string_append(&bloques,",");
	string_append(&bloques,string_itoa(block));
	string_append(&bloques,"]");

	config_set_value(configuracion,"BLOCKS",bloques);
	config_save(configuracion);
	config_destroy(configuracion);

	free(bloques);
	return;
}

/*
char** agregar_valor_a_la_lista(char* bloque, char** bloques){
	int numBloque = (sizeof(bloques)/block_size);
	numBloque ++;
	bloques[numBloque]=bloque;
	return bloques;
}
void agregar_block_al_metadata(int block,char* pathPokemon){
	t_config* configuracion = config_create(pathPokemon);
	char** bloques = config_get_array_value(configuracion,"BLOCKS");
	char* bloque = string_itoa(block);

	bloques =agregar_valor_a_la_lista(bloque, bloques);
	config_set_value(configuracion,"BLOCKS",bloques);
	config_destroy(configuracion);
	free(bloques);
	return;
}
*/


int bloque_espacio_en_blocks_libre(int tamEntrada, char* pathPokemon){
	uint i;
	int block;
	char** bloquesPokemon = obtener_array_de_bloques(pathPokemon);
	int j = tam_array_bloques(bloquesPokemon);

	log_info(logger, "buscando espacio disponible");

	for(i=0;i<=j;i++){
		char*pathBloque = generar_path_bloque(bloquesPokemon[i]);
		int tamBloque =tam_disponible_en_el_block(pathBloque) ;

		if(tamEntrada > tamBloque){
			log_info(logger,string_from_format("el bloque %s no tiene espacio disponible",bloquesPokemon[i]));
			char* info1 = bloquesPokemon[i];
			char* info2 = bloquesPokemon[j-1];

			if((strcmp(info1,info2)) == 0){
				block =buscar_block_disponible(tamEntrada);
				actualizar_bitmap(1,block);
				agregar_block_al_metadata(block,pathPokemon);
				 log_info(logger,string_from_format("se genero el nuevo bloque",block));
				 return block;
				}
		}else{
			return block = atoi(bloquesPokemon[i]);
			log_info(logger,string_from_format("El bloque %s tiene espacio disponible para la nueva entrada",bloquesPokemon[i]));
			}
	}
	return block;
}

int crear_block (){
	log_debug(logger,"crear nuevo block");

	int i;
	for (i = 0; i< blocks; i++){

		FILE* f = fopen(string_from_format("%s/Blocks/%d.bin", ptoMontaje ,i),"w");
		fclose (f);
			log_debug(logger, "se creo el nuevo block: int %d",i);
	}
	log_debug(logger, "No se pudo crear el bloque");
	return 1;
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
	fputs((string_from_format("DIRECTORY=%s\n","Y")),directorio);
	fclose(directorio);
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
	fprintf(archivo,"BLOCK_SIZE=%d\n",block_size);
	fprintf(archivo,"BLOCKS=%d\n",blocks);
	fprintf(archivo,"MAGIC_NUMBER=TALL_GRASS\n");


	fclose(archivo);
	log_debug(logger, "Se ha iniciado Metadata correctamente");
	}
}
void iniciar_bitmap(){
	log_debug(logger, "Iniciando Bitmap");

	int sizeBitarray = blocks;
	while((sizeBitarray%8)!=0) sizeBitarray++;

	char*pathBitmap = string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje);

	if(validar_existencia_archivo(pathBitmap)){
		FILE* bitmap_f = fopen(pathBitmap, "rb");

		struct stat stats;
		fstat(fileno(bitmap_f),&stats);

		char* data =malloc(stats.st_size);
		fread(data,stats.st_size,1,bitmap_f);
		fclose(bitmap_f);

		bitmap = bitarray_create_with_mode(data,stats.st_size,LSB_FIRST);
	} else {
	bitmap = bitarray_create_with_mode(string_repeat('\0',sizeBitarray),sizeBitarray,LSB_FIRST);

	FILE* bitmap_f = fopen(string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje),"w");
	fwrite(bitmap->bitarray,sizeBitarray,1,bitmap_f);
	fclose(bitmap_f);
	}
}

/*char leer_ultima_pos_archivo (char*path){
	FILE*archivo = fopen(path,"rb");
	fseek(archivo,-1,SEEK_END);
	char ultimo = fgetc(archivo);
	fclose(archivo);
	return ultimo;
}*/
int archivo_abierto(char* path){
	t_config* configuracion = config_create(path);
	char* estado = config_get_string_value(configuracion,"OPEN");

	if(strcmp(estado,"Y")){
		return true;
		}else return false;

}

void abrir_archivo(char*path){
	t_config* configuracion = config_create(path);
	char* estado = string_new();
	string_append(&estado,"Y");
	config_set_value(configuracion,"OPEN",estado);
	config_save(configuracion);
	config_destroy(configuracion);
	free(estado);


}
void cerrar_archivo(char*path){
	t_config* configuracion = config_create(path);
		char* estado = string_new();
		string_append(&estado,"N");
		config_set_value(configuracion,"OPEN",estado);
		config_save(configuracion);
		config_destroy(configuracion);
		free(estado);

}

void iniciar_files_dir(){
	crear_directorio(string_from_format("%s/Files", ptoMontaje));
	crear_metadata_directorio(string_from_format("%s/Files", ptoMontaje));
}

void iniciar_blocks_dir(){
	crear_directorio(string_from_format("%s/Blocks",ptoMontaje));
	//crear_metadata_directorio(string_from_format("%s/Blocks",ptoMontaje));
	crear_block();
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

	//if(!string_ends_with(configTG->ptoMontaje,"/")) string_append(&configTG->ptoMontaje,"/");

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
	}
	return configTG;
}

void escribir_bloque(char* path, char*mensaje){
	FILE*archivo = fopen(path, "a");
	fseek(archivo,0,SEEK_END);
	fprintf(archivo, mensaje);
	fclose(archivo);
	return;
}

void crear_files_metadata(char* pokemon, char* mensaje){
	int tamanioMensaje =strlen(mensaje);
	int bloquePokemon = buscar_block_disponible(tamanioMensaje);
	char* pathPokemon = crear_path_archivos(pokemon);
	char*pathBloque =generar_path_bloque(string_itoa(bloquePokemon));

	escribir_bloque(pathBloque, mensaje);
	actualizar_bitmap(1,bloquePokemon);
	crear_directorio(pathPokemon);

	FILE* archivo = fopen(string_from_format("%s/Metadata.bin", pathPokemon),"a");
		fprintf(archivo, string_from_format("DIRECTORY=%s\n","N"));
	char* blocks = string_from_format("BLOCKS=[%d]\n", bloquePokemon);
		fprintf(archivo, blocks);
	//char* pathPokemonMetadata = string_from_format("%s/Metadata.bin", pathPokemon);
		fprintf(archivo, string_from_format("SIZE=%i\n", (tam_ocupado_en_el_block(pathBloque))));
		fprintf(archivo, string_from_format("OPEN=%s","N"));
	fclose(archivo);

	log_info(logger,string_from_format("El archivo se ha creado exitosamente"));
	return;
	}



void agregar_mensaje_NEW_POKEMON(int posX, int posY, int cant,char*pokemonPath){
	char* entradaMensaje = generar_linea_de_entrada_mensaje(posX,posY,cant);
	int tamEntrada = strlen (entradaMensaje);
	char*pathBloque = generar_path_bloque(string_from_format("%i", bloque_espacio_en_blocks_libre(tamEntrada,pokemonPath)));
	actualizar_tamanio_archivo(tamEntrada,pokemonPath);
	escribir_bloque(pathBloque,entradaMensaje);
	return;
}


int verificar_existencia_posiciones(int posX,int posY,char*pathPokemon){
	int i;
	char** bloquesArchivos = obtener_array_de_bloques(pathPokemon);

	for(i=0;i<(tam_array_bloques(bloquesArchivos));i++){

	char* pathBloque = generar_path_bloque(bloquesArchivos[i]);
	int tamBloque = tam_ocupado_en_el_block(pathBloque);

	FILE*bloque = fopen((pathBloque),"rb");


	fseek(bloque,0,SEEK_SET);

	char stringArchivoEntero[tamBloque] ;
	fread(stringArchivoEntero,tamBloque,1,bloque);
	char*mensaje = string_new();
	string_append(&mensaje, string_from_format("%i-%i=",posX,posY));


	if(string_contains(stringArchivoEntero,mensaje)){
		log_info(logger,string_from_format("El mensaje se encuentra en el bloque %s",bloquesArchivos[i]));
			return true;
		}else{
			log_info(logger,string_from_format("El mensaje no se encuentro en el bloque %s",bloquesArchivos[i]));
		}

	log_info(logger,string_from_format("El mensaje no se encuentro en el bloque %s",bloquesArchivos[i]));
	fclose(bloque);
	}

	return false;
}
int buscar_posicion_linea_en_bloque(int posX, int posY, char* pathBloque){
	int siguientePos = 0;
	int i;
	FILE* bloque_f = fopen(pathBloque, "rb");
	int tamBloque = tam_ocupado_en_el_block(pathBloque);

	for(i=0; i<tamBloque; i=siguientePos){
	char buffer [tamBloque];

	fseek(bloque_f,i,SEEK_SET);
	fread(buffer, (tamBloque-i),1,bloque_f);
	char* mensaje = string_new();
	string_append(&mensaje, string_from_format("%i-%i=",posX,posY));

	if(!(string_starts_with(buffer,mensaje))){
		int j;
		for(j=i;buffer[j]!='\n'; j++);
				siguientePos = j+1;
	}else{
		fclose(bloque_f);
		return i;
		}
	}
	fclose(bloque_f);
	return -1;
	}
void modificar_archivo_NEW_POKEMON(int posX, int posY, int cant,char* pathBloque){
	int pos;
	int j;
	char* linea = string_new();
	char* mensaje = string_new();
	string_append(&mensaje,string_from_format("%i-%i=",posX,posY));

	pos = buscar_posicion_linea_en_bloque(posX,posY,pathBloque);

	FILE*bloque = fopen(pathBloque,"r+b");
	fseek(bloque,pos,SEEK_SET);
	/*char buffer[tam_ocupado_en_el_block(pathBloque)-pos];
	//fread(buffer,(tam_ocupado_en_el_block(pathBloque)-pos),1,bloque);
	for(j=pos;buffer[j]!='\n'; j++);
	int posCantidad = j;*/

	fgets(linea, block_size, bloque);

	int longitudMensaje = string_length(mensaje);
	char* stringCantidad = string_substring_from(linea,longitudMensaje);
	int cantidadActual = atoi (stringCantidad);

	int cantidadActualizada = cantidadActual + cant;

	int cantidadDigitosActualizada = cantidad_digitos(cantidadActualizada);
	int cantidadDigitosActual = cantidad_digitos(cantidadActual);

	if(cantidadDigitosActualizada == cantidadDigitosActual) {
		fseek(bloque,(pos+longitudMensaje),SEEK_SET);
		fputs(string_from_format("%i",cantidadActualizada),bloque);
	} else { // 1-1=1\n => 1-1=10\n
		int posicionBusqueda = (tam_ocupado_en_el_block(pathBloque)-(pos + longitudMensaje + cantidadDigitosActual));
		char buffer[posicionBusqueda];
		fseek(bloque,-posicionBusqueda,SEEK_END);
		//fseek(bloque, 0, SEEK_END);

		fread(buffer, posicionBusqueda, 1, bloque);
		printf("buffer %s", buffer);
		fseek(bloque,(pos+longitudMensaje),SEEK_SET);
		/*if(cantidadDigitosActualizada < cantidadDigitosActual) {
			char* mensajeRepetir = string_repeat('\0', cantidadDigitosActual-cantidadDigitosActualizada);
			printf("%s", mensajeRepetir);
			fputs(string_from_format("%i%s%s",cantidadActualizada, buffer, mensajeRepetir),bloque);
		} else {*/
			fputs(string_from_format("%i%s",cantidadActualizada, buffer),bloque);
		//}

		printf("%s\n", pathBloque);

	}

	fclose(bloque);

	return;

}

int cantidad_digitos(int numero) {
	int count = 0;
	while(numero != 0) {
		numero /= 10;
		++count;
	}
	return count;
}




