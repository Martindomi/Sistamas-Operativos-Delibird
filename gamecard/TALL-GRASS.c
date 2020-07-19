#include "TALL-GRASS.h"

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
char* generar_path_directorio_pokemon(char* pokemon){
	char* pathArchivo = string_new();
	string_append(&pathArchivo,ptoMontaje);
	string_append(&pathArchivo,"/Files");
	if (!string_starts_with(pokemon,"/")) string_append(&pathArchivo,"/");
	string_append(&pathArchivo,pokemon);
	return pathArchivo;
}
char* generar_path_archivo_pokemon_metadata(char*pokemon){
	char* path = string_new();
	string_append(&path,generar_path_directorio_pokemon(pokemon));
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
int buscar_block_disponible(){
	int bloqueLibre;
	log_info(logger, "buscando espacio disponible");
	for(bloqueLibre=0;bitarray_test_bit(bitmap,bloqueLibre)&& (bloqueLibre<blocks); bloqueLibre++){
		log_info(logger,"el bloque disponible es %i",bloqueLibre);
	}
	if (bloqueLibre>=blocks) return NO_MORE_BLOCKS;
	return bloqueLibre;
}
int crear_block(){
	log_debug(logger,"crear nuevo block");

		int i;
		for (i = 0; i< blocks; i++){
			int block_fd = open(string_from_format("%s/Blocks/%d.bin", ptoMontaje ,i),O_CREAT|O_RDWR,0664);
			if(block_fd<0){
				log_error(logger,"No se pudo abrir el archivo");
				exit(1);
			}
			close(block_fd);
		}
		log_debug(logger, "se crearon %i bloques",i-1);
		return 1;
}
int cantidad_digitos(int numero) {
	int count = 0;
	while(numero != 0) {
		numero /= 10;
		++count;
	}
	return count;
}
int validar_existencia_archivo(char*path){
	log_debug(logger,"Verificando si existe el archivo %d en el sistema de archivos", path);
	int archivo_fd= open(path,O_RDWR,0664);
	if(archivo_fd<0){
		log_debug(logger,string_from_format("No existe el archivo %s",path));
		return false;
	}else{
		log_debug(logger,string_from_format("Existe el archivo %s",path));
		return true;
	} close(archivo_fd);
}
void actualizar_tamanio_archivo(char*path){
	t_config* configuracion = config_create(path);
	int tamanio = config_get_int_value(configuracion,"SIZE");

	tamanio = calcular_tamanio_archivo(path);

	config_set_value(configuracion,"SIZE",string_itoa(tamanio));
	config_save(configuracion);
	config_destroy(configuracion);
}
void agregar_block_al_metadata(int block,char* pathPokemon){
	t_config* configuracion = config_create(pathPokemon);
	char* bloques = string_new();
	string_append(&bloques,config_get_string_value(configuracion,"BLOCKS"));

	bloques[strlen(bloques)-1]='\0';
	if(strcmp(bloques,"[")){
	string_append(&bloques,",");
	}
	string_append(&bloques,string_itoa(block));
	string_append(&bloques,"]");

	config_set_value(configuracion,"BLOCKS",bloques);
	config_save(configuracion);
	config_destroy(configuracion);

	free(bloques);
	return;
}
void crear_directorio(char*path){
	mkdir(path,0777);
	log_debug(logger,"se ha creado el directorio %d",path);
}
void crear_metadata_directorio(char* dir){
	FILE* directorio = fopen((string_from_format("%s/Metadata.bin",dir)),"a");
	fputs((string_from_format("DIRECTORY=%s\n","Y")),directorio);
	fclose(directorio);
}
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

	}else{
	crear_directorio(string_from_format("%s/Metadata", ptoMontaje)); /*modif1*/
	FILE* archivo = fopen(metadata,"a");
	fprintf(archivo,"BLOCK_SIZE=%d\n",block_size);
	fprintf(archivo,"BLOCKS=%d\n",blocks);
	fprintf(archivo,"MAGIC_NUMBER=TALL_GRASS\n");


	fclose(archivo);

	log_debug(logger, "Se ha iniciado Metadata correctamente");
	}free(metadata);
}
void actualizar_bitmap(bool valor, int pos){
	char*pathBitmap = string_from_format("%s/Metadata/Bitmap.bin",ptoMontaje);

	int bitmap_fd = open(pathBitmap,O_CREAT | O_RDWR,0664);

	if(bitmap_fd<0){
		log_error(logger, "no se pudo actualizar el archivo bitmap");
		exit (1);
	}
	int length = sizeof(bitmap_fd);
	ftruncate(bitmap_fd,blocks);
	char* bitmap_a = mmap (NULL,blocks,PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED,bitmap_fd,0);

	if(valor)
		bitarray_set_bit(bitmap,pos);
	else
		bitarray_clean_bit(bitmap,pos);
	memcpy(bitmap_a,bitmap->bitarray,blocks);
	msync(bitmap_a,bitmap_fd,MS_SYNC);
	munmap(bitmap_a,blocks);
	close(bitmap_fd);

	return;
}
void iniciar_bitmap(){
	log_debug(logger, "Iniciando Bitmap");

	int sizeBitarray = blocks;
	while((sizeBitarray%8)!=0) sizeBitarray++;

	char*pathBitmap = string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje);
	int bitmap_fd = open(pathBitmap, O_RDWR,0664);

	if(bitmap_fd<0){
		log_error(logger, "no se pudo abrir el archivo");
		bitmap_fd = open(pathBitmap,O_CREAT|O_RDWR,0664);
		bitmap = mmap(NULL,sizeBitarray,PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED,bitmap_fd,0);
		bitmap = bitarray_create_with_mode(string_repeat('\0',sizeBitarray),sizeBitarray,LSB_FIRST);

	}else{
	ftruncate(bitmap_fd,sizeBitarray);
	bitmap = mmap (NULL,sizeBitarray,PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED,bitmap_fd,0);
	struct stat stats;
	stat(pathBitmap,&stats);


	char* data =malloc(stats.st_size);
	read(bitmap_fd,data,stats.st_size);

	bitmap = bitarray_create_with_mode(data,stats.st_size,LSB_FIRST);
	}
	msync(bitmap->bitarray,bitmap_fd,MS_SYNC);

	close(bitmap_fd);
	free(pathBitmap);
}
int bitmap_vacio(char*path){
	int i;
	int bitmap_fd = open(path,O_CREAT|O_RDWR,0664);
	int sizeBitarray = blocks;
	while((sizeBitarray%8)!=0) sizeBitarray++;
	ftruncate(bitmap_fd,sizeBitarray);
	if(bitmap_fd <0){
		log_error(logger, "no se pudo abrir el archivo");
		exit(1);
	}
		for(i=0;i<sizeBitarray;i++){
		if(bitarray_test_bit(bitmap, i)){
			munmap(bitmap,sizeBitarray); //Revisar si ejecuta y libera bitmap
			close(bitmap_fd);
			return false;
		 }
	}

	close(bitmap_fd);
	return true;
}
void iniciar_files_dir(){
	if(!validar_existencia_archivo(string_from_format("%s/Files/Metadata.bin", ptoMontaje))){
	crear_directorio(string_from_format("%s/Files", ptoMontaje));
	crear_metadata_directorio(string_from_format("%s/Files", ptoMontaje));
}
}
void iniciar_blocks_dir(){
	if(bitmap_vacio(string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje))){
	crear_directorio(string_from_format("%s/Blocks",ptoMontaje));
	crear_block();
	log_info(logger,"existen valores previos");
}
}
void iniciar_metadata_dir(){
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
/*void liberar_blocks(char** blockArr){
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
}*/
/*Corregit y quitar Fseek, buscar otra forma
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
/*Arrastra correccion de tam ocupado*/
/*Arrastra correccion de tam ocupado*/
/*Arrastra correccion de tam ocupado*/
/*¿Deberia cambiar Fopen por open?int crear_block (){
	log_debug(logger,"crear nuevo block");

	int i;
	for (i = 0; i< blocks; i++){

		FILE* f = fopen(string_from_format("%s/Blocks/%d.bin", ptoMontaje ,i),"w");
		fclose (f);
			log_debug(logger, "se creo el nuevo block: int %d",i);
	}
	log_debug(logger, "No se pudo crear el bloque");
	return 1;
}*/
/*Modificar
void escribir_bloque(char* path, char*mensaje){
	FILE*archivo = fopen(path, "a");
	fseek(archivo,0,SEEK_END);
	fprintf(archivo, mensaje);
	fclose(archivo);
	return;
}*/
/*actualizar concepto
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
actualizar concepto
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
}*/
/*actualizar concepto
void mensaje_new_pokemon(int posX,int posY,int cant,char* pokemon){

	char*pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	if(validar_existencia_archivo(pathPokemon)){
		//if(!archivo_abierto(pathPokemon)){
		abrir_archivo(pathPokemon);
		if(verificar_existencia_posiciones(posX,posY,pathPokemon)){

			char* bloque = bloque_contenedor_linea(posX,posY,pathPokemon);
			char*pathBloque= generar_path_bloque(bloque);
			modificar_archivo_NEW_POKEMON(posX,posY,cant,pathBloque);
			actualizar_tamanio_archivo(pathPokemon);

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
actualizar concepto
void agregar_mensaje_NEW_POKEMON(int posX, int posY, int cant,char*pokemonPath){
	char* entradaMensaje = generar_linea_de_entrada_mensaje(posX,posY,cant);
	int tamEntrada = strlen (entradaMensaje);
	char*pathBloque = generar_path_bloque(string_from_format("%i", bloque_espacio_en_blocks_libre(tamEntrada,pokemonPath)));
	escribir_bloque(pathBloque,entradaMensaje);
	actualizar_tamanio_archivo(pokemonPath);
	return;
}*/
/*actualizar concepto
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
}*/
/*actualizar concepto
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
	}*/
/*actualizar concepto
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
	int posCantidad = j;

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
		char buffer[posicionBusqueda-1];
		fseek(bloque,-posicionBusqueda,SEEK_END);
		//fseek(bloque, 0, SEEK_END);

		fread(buffer, (posicionBusqueda), 1, bloque);
		printf("buffer %s", buffer);
		fseek(bloque,(pos+longitudMensaje),SEEK_SET);
		/*if(cantidadDigitosActualizada < cantidadDigitosActual) {
			char* mensajeRepetir = string_repeat('\0', cantidadDigitosActual-cantidadDigitosActualizada);
			printf("%s", mensajeRepetir);
			fputs(string_from_format("%i%s%s",cantidadActualizada, buffer, mensajeRepetir),bloque);
		} else {
			fputs(string_from_format("%i%s",cantidadActualizada, buffer),bloque);
		}

		printf("%s\n", pathBloque);

	}

	fclose(bloque);

	return;

}*/
/*int validar_existencia_archivo(char* path){
	log_debug(logger,"Verificando si existe el archivo %d en el sistema de archivos", path);
	FILE* archivo = fopen(path,"r");
	if(archivo!=NULL){
	fclose (archivo);
		return true;
	} else return false;
}*/
/*actualizar
void crear_files_metadata(char* pokemon, char* mensaje){

	//
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	//char*pathBloque =generar_path_bloque(string_itoa(bloquePokemon));

	//escribir_bloque(pathBloque, mensaje);
	//actualizar_bitmap(1,bloquePokemon);
	crear_directorio(generar_path_directorio_pokemon(pokemon));

	FILE* archivo = fopen(pathPokemon,"a");
		fprintf(archivo, string_from_format("DIRECTORY=%s\n","N"));
	char* blocks = string_from_format("BLOCKS=[]\n");
	//char* blocks = string_from_format("BLOCKS=[%d]\n", bloquePokemon);
		fprintf(archivo, blocks);
	//char* pathPokemonMetadata = string_from_format("%s/Metadata.bin", pathPokemon);
	//	fprintf(archivo, string_from_format("SIZE=%i\n", (tam_ocupado_en_el_block(pathBloque))));
		fprintf(archivo, string_from_format("SIZE=0\n"));
		fprintf(archivo, string_from_format("OPEN=%s","N"));
	fclose(archivo);

	log_info(logger,string_from_format("El archivo se ha creado exitosamente"));
	return;
	}


/*void iniciar_bitmap(){
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
	 }else {
	bitmap = bitarray_create_with_mode(string_repeat('\0',sizeBitarray),sizeBitarray,LSB_FIRST);

	FILE* bitmap_f = fopen(string_from_format("%s/Metadata/Bitmap.bin", ptoMontaje),"w");
	fwrite(bitmap->bitarray,sizeBitarray,1,bitmap_f);
	fclose(bitmap_f);
	}
}
char leer_ultima_pos_archivo (char*path){
	FILE*archivo = fopen(path,"rb");
	fseek(archivo,-1,SEEK_END);
	char ultimo = fgetc(archivo);
	fclose(archivo);
	return ultimo;
}*/

//Nuevos desarrollos

void crear_archivo_pokemon_metadata(char* pokemon,char* mensaje){
	int tamanioMensaje =strlen(mensaje);
	char*pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	int bloquePokemon = buscar_block_disponible(tamanioMensaje);
	char* pathBloque = generar_path_bloque(string_itoa(bloquePokemon));

	escribir_mensaje_en_block(bloquePokemon, mensaje,AGREGAR);
	actualizar_bitmap(1,bloquePokemon);
	crear_directorio(generar_path_directorio_pokemon(pokemon));

	FILE* archivo = fopen(pathPokemon,"a");
	fprintf(archivo, string_from_format("DIRECTORY=%s\n","N"));
	char* blocks = string_from_format("BLOCKS=[%d]\n", bloquePokemon);
	fprintf(archivo, blocks);
	char* pathPokemonMetadata = string_from_format("%s/Metadata.bin", pathPokemon);
	fprintf(archivo, string_from_format("SIZE=%i\n", (tamanio_ocupado_bloque(pathBloque))));
	fprintf(archivo, string_from_format("OPEN=%s","N"));
	fclose(archivo);

	log_info(logger,string_from_format("El archivo se ha creado exitosamente"));
	return;
}
void quitar_bloque_de_metadata(char*path,char* bloque){
	t_config* cfg = config_create(path);
	char* bloques = string_new();
	char*bloques1=string_new();
	string_append(&bloques1,config_get_string_value(cfg,"BLOCKS"));
	char**listaDeBloques = string_n_split(bloques1,2,bloque);
	string_append(&bloques,listaDeBloques[0]);
	bloques[strlen(bloques)-1]='\0';
	string_append(&bloques,listaDeBloques[1]);

	config_set_value(cfg,"BLOCKS",bloques);
	config_save(cfg);
	config_destroy(cfg);
	free(bloques1);
	free(bloques);
	return;
}
void vaciar_bloque(char* bloque){

	char* pathBloque = generar_path_bloque(bloque);
	int bloque_fd = open(pathBloque,O_RDWR,0664);
	if(bloque_fd <0){
			log_error(logger, string_from_format("No se pudo abrir el bloque %i", bloque));
			exit(1);
		}
	ftruncate(bloque_fd,block_size);
	char* bloqueVacio = mmap(NULL,block_size,PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED,bloque_fd,0);
	char* cadenaVacia = string_repeat('\0',block_size);
	ftruncate(bloque_fd,0);
	memcpy(bloqueVacio,cadenaVacia,0);
	msync(bloqueVacio,bloque_fd,0);
	munmap(bloqueVacio,0);
	close(bloque_fd);
	free(cadenaVacia);
}
void liberar_bloque(char* bloque,char*pathPokemon){
	vaciar_bloque(bloque);
	actualizar_bitmap(0,atoi(bloque));
	quitar_bloque_de_metadata(pathPokemon,bloque);

}
int tamanio_real_archivo(char*pathPokemon){
	t_config* configArchivo = config_create(pathPokemon);
	int tam = config_get_int_value(configArchivo,"SIZE");
	config_destroy(configArchivo);
	return tam;
}
int tamanio_ocupado_bloque(char*path){
	int bloque_fd = open(path,O_RDWR,0664);
	 struct stat stats;
	 stat(path,&stats);
	 close(bloque_fd);
	 return stats.st_size;

}
int tam_disponible_en_bloque(char*path){
	int tamDisponible;
	int size = block_size;
	int tamOcupado = tamanio_ocupado_bloque(path);
	int tamDisponiblePrevio = size-tamOcupado;

	if (tamDisponiblePrevio < 0){
		log_debug(logger,"no tiene espacio disponible");
	}else{
		tamDisponible = tamDisponiblePrevio;
		log_debug(logger,string_from_format("el bloque tiene %i bytes disponibles",tamDisponible));
	}
return  tamDisponible;
}
int block_completo(char*path){
	 if(tamanio_ocupado_bloque(path)== block_size) return true;
	 else return false;
}
int calcular_tamanio_archivo(char*path){
	int i;
	int tamanio=0;
	char** blocks = obtener_array_de_bloques(path);

	for(i=0; i<(tam_array_bloques(blocks));i++)
	{
		char* pathBlock = generar_path_bloque(blocks[i]);
		tamanio = tamanio + tamanio_ocupado_bloque(pathBlock);
		}

	return tamanio;
}
void escribir_mensaje_en_block(int bloque, char* mensaje,int accion){
	char* pathBloque = generar_path_bloque(string_itoa(bloque));
	int tamMensaje = string_length(mensaje);
	int tamOcupado = tamanio_ocupado_bloque(pathBloque);

	int bloque_fd = open (pathBloque,O_RDWR,0664);
	if(bloque_fd <0){
		log_error(logger, string_from_format("No se pudo abrir el bloque %i", bloque));
		exit(1);
	}

		switch(accion){
	case AGREGAR: ftruncate(bloque_fd,tamMensaje+tamOcupado);
	char*mensajeArchivo1 = mmap (NULL,tamMensaje+tamOcupado,PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED,bloque_fd,0);

	if(tamOcupado == 0){
			memcpy(mensajeArchivo1,mensaje,tamMensaje+tamOcupado);
			printf("%s",mensajeArchivo1);
		}else{
			struct stat stats;
			stat(pathBloque,&stats);
			char*data = malloc(stats.st_size);
			read(bloque_fd,data,stats.st_size);
			char* nuevoData = string_substring_until(data,tamOcupado);
			string_append(&nuevoData,mensaje);
			memcpy(mensajeArchivo1,nuevoData,tamOcupado+tamMensaje);
			free(data);
			free(nuevoData);
		}
		msync(mensajeArchivo1,bloque_fd,tamMensaje+tamOcupado); // revisar si modifica algo lo de msyc con tamOcupado+tamMensaje
		munmap(mensajeArchivo1,tamMensaje+tamOcupado);
		break;
	case MODIFICAR:ftruncate(bloque_fd,tamMensaje);
	char*mensajeArchivo2 = mmap (NULL,tamMensaje,PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED,bloque_fd,0);
	memcpy(mensajeArchivo2,mensaje,tamMensaje);
	msync(mensajeArchivo2,bloque_fd,tamMensaje); // revisar si modifica algo lo de msyc con tamOcupado+tamMensaje
 	munmap(mensajeArchivo2,tamMensaje);
		break;
		}


	close(bloque_fd);
}
void agregar_nuevo_mensaje(char* mensaje, char*pathPokemon){
	int tamMensaje = string_length(mensaje);
	char** bloques = obtener_array_de_bloques (pathPokemon);
	int tamArray = tam_array_bloques(bloques);

	if(tamArray == 0){
		int bloqueNuevo = buscar_block_disponible();
		actualizar_bitmap(1,bloqueNuevo);
		escribir_mensaje_en_block (bloqueNuevo, mensaje,AGREGAR);
		agregar_block_al_metadata(bloqueNuevo,pathPokemon);
		actualizar_tamanio_archivo(pathPokemon);

	}else{
	char*path = generar_path_bloque(bloques[tamArray-1]);
	int tamDisponible = tam_disponible_en_bloque(path);
	if(tamMensaje <= tamDisponible){
		escribir_mensaje_en_block(atoi(bloques[tamArray-1]),mensaje,AGREGAR);
		actualizar_tamanio_archivo(pathPokemon);

	}else{
		char* mensajeBloqueFinal = string_substring_until(mensaje,(tamDisponible));
		escribir_mensaje_en_block(atoi(bloques[tamArray-1]),mensajeBloqueFinal,AGREGAR);
		int bloqueNuevo = buscar_block_disponible();
		actualizar_bitmap(1,bloqueNuevo);
		char* mensajeBloqueNuevo = string_substring_from(mensaje,tamDisponible);
		escribir_mensaje_en_block (bloqueNuevo, mensajeBloqueNuevo,AGREGAR);

		agregar_block_al_metadata(bloqueNuevo,pathPokemon);
		actualizar_tamanio_archivo(pathPokemon);

		free(mensajeBloqueFinal);
		free(mensajeBloqueNuevo);
	}
	}
}
void tratar_contenido_en_bloques(char*contenido,char* pathPokemon){
	int tamContenido = string_length(contenido);
	char** bloques = obtener_array_de_bloques(pathPokemon);
	int tamArray = tam_array_bloques(bloques);
	int i;
	int j;
	int cantBloques =tamContenido / block_size;
	if((tamContenido%block_size)!=0) cantBloques++;

	if(cantBloques == tamArray){
		 j= block_size;
		for(i=0;i< tamArray;i++){
			char*mensajePorBloque = string_new();
			if ((tamContenido-j)< 0){
			char* contenido2 =string_substring_from(contenido,j-block_size);
			 string_append(&mensajePorBloque,string_substring_until(contenido2,(block_size-(j-tamContenido))));
			 free(contenido2);
			}else{
				char* contenidoAux = string_substring_from(contenido,j-block_size);
				string_append(&mensajePorBloque,string_substring_until(contenidoAux,j));
				free(contenidoAux);
			}
			escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,MODIFICAR);
			j = j+block_size;
		}

	}else{
		if(cantBloques > tamArray){
			 j= block_size;
				for(i=0;i< tamArray;i++){
					char*mensajePorBloque = string_new();
					char*contenidoAux = string_substring_from(contenido,j-block_size);
					 string_append(&mensajePorBloque,string_substring_until(contenidoAux,j));
					escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,MODIFICAR);
					j = j+block_size;
					free(contenidoAux);
				}
			int h;

			for(h=0;h<=(cantBloques-tamArray);h++){
				int nuevoBloque = buscar_block_disponible();
				actualizar_bitmap(1,nuevoBloque);
				char*mensajePorBloque=string_new();
				if((tamContenido-j)<block_size){
					char* contenidoMuletita = string_substring_from(contenido,j);
					string_append(&mensajePorBloque,string_substring_until(contenidoMuletita,tamContenido-j));
					free(contenidoMuletita);
				}else{
				char*contenidoM = string_substring_from(contenido,j);
				string_append(&mensajePorBloque,string_substring_until(contenidoM,j+block_size));
				}
				j=j+block_size;
				escribir_mensaje_en_block(atoi(bloques[i]),mensajePorBloque,MODIFICAR);
				agregar_block_al_metadata(nuevoBloque,pathPokemon);
				actualizar_tamanio_archivo(pathPokemon);
			}
		}else{
			if(cantBloques<tamArray){
				int tamDiferencia = tamArray-cantBloques;
				int d;
				for(d=1;d<=tamDiferencia;d++){
					liberar_bloque(bloques[tamArray-d],pathPokemon);
				}

				j= 0;
				for(i=0;i< cantBloques;i++){
					char*mensajePorBloque = string_new();
					if((tamContenido-j)<block_size){
						char* contenidoAux = string_substring_from(contenido,j);
						string_append(&mensajePorBloque,string_substring_until(contenidoAux,tamContenido-j));
					}else {
						char* contenidoAux = string_substring_from(contenido,j);
						string_append(&mensajePorBloque, string_substring_until(contenidoAux,j+block_size));
					}
					escribir_mensaje_en_block(atoi(bloques[i]), mensajePorBloque,MODIFICAR);
					j = j+block_size;
						}
					}
				}
			}
		}
void tratar_mensaje_NEW_POKEMON(int posX, int posY, int cant, char* pokemon){
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	if(validar_existencia_archivo(pathPokemon)){
		//if(!archivo_abierto(pathPokemon)){
		abrir_archivo(pathPokemon);
		char* mensaje = generar_linea_de_entrada_mensaje(posX, posY,cant);
		int i=0;
		char* contenidoBloques = string_new();
		char** bloques = obtener_array_de_bloques(pathPokemon);

		for(i=0; i<tam_array_bloques(bloques);i++){
			char*pathBloque = generar_path_bloque(bloques[i]);
			int bloque_fd = open (pathBloque,O_RDWR,0664);

			if(bloque_fd<0){
				log_error(logger,"No se pudo abrir el archivo");
				error(1);
				}
			struct stat stats;
			stat(pathBloque,&stats);

			char*data = malloc(stats.st_size);
			read(bloque_fd,data,stats.st_size);

			string_append(&contenidoBloques,string_substring_until(data,stats.st_size));
			free(data);
			free(pathBloque);
			close(bloque_fd);
		}
		char*mensajeCorto = string_from_format("%i-%i=",posX,posY);
		if(string_contains(contenidoBloques,mensajeCorto)){
			int i;
			int siguientePos=0;

			for(i=0;i<string_length(contenidoBloques);i=siguientePos){
			char*stringComparar = string_substring_from(contenidoBloques,i);
				if(!(string_starts_with(stringComparar,mensajeCorto))){
						int j;
						for(j=0;stringComparar[j]!='\n'; j++);
									siguientePos = i+j+1;
						}else{
							break;
						}
			}
			char* stringCant = string_substring_from(contenidoBloques,(i+string_length(mensajeCorto)));
			char** listaDeContenidosTotal= string_n_split(stringCant,2,"\n");
			int cantActual = atoi(listaDeContenidosTotal[0]);
			int nuevaCantidad = cant + cantActual;
			char*contenidoFinal = string_new();
			string_append(&contenidoFinal,string_substring_until(contenidoBloques,(i+(string_length(mensajeCorto)))));
			string_append(&contenidoFinal,string_itoa(nuevaCantidad));
			string_append(&contenidoFinal,string_substring_from(stringCant,cantidad_digitos(cantActual)));

			tratar_contenido_en_bloques(contenidoFinal,pathPokemon);
		}else{
			agregar_nuevo_mensaje(mensaje,pathPokemon);
		}
	}else{
	char* mensaje = generar_linea_de_entrada_mensaje(posX, posY,cant);
	crear_archivo_pokemon_metadata(pokemon,mensaje);

}
}
char* tratar_mensaje_CATCH_POKEMON(int posX,int posY, char*pokemon){
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
		if(validar_existencia_archivo(pathPokemon)){
			//if(!archivo_abierto(pathPokemon)){
			abrir_archivo(pathPokemon);
			char* mensaje = string_from_format("%i-%i=",posX,posY);;
			int i=0;
			char* contenidoBloques = string_new();
			char** bloques = obtener_array_de_bloques(pathPokemon);

			for(i=0; i<tam_array_bloques(bloques);i++){
				char*pathBloque = generar_path_bloque(bloques[i]);
				int bloque_fd = open (pathBloque,O_RDWR,0664);

				if(bloque_fd<0){
					log_error(logger,"No se pudo abrir el archivo");
					error(1);
					}
				struct stat stats;
				stat(pathBloque,&stats);

				char*data = malloc(stats.st_size);
				read(bloque_fd,data,stats.st_size);

				string_append(&contenidoBloques,string_substring_until(data,stats.st_size));
				free(data);
				free(pathBloque);
				close(bloque_fd);
			}
			if(string_contains(contenidoBloques,mensaje)){
				int i;
				int siguientePos=0;

				for(i=0;i<string_length(contenidoBloques);i=siguientePos){
					char*stringComparar = string_substring_from(contenidoBloques,i);
					if(!(string_starts_with(stringComparar,mensaje))){
						int j;
						for(j=0;stringComparar[j]!='\n'; j++);
							siguientePos = i+j+1;
					}else break;
				}char* stringCant = string_substring_from(contenidoBloques,(i+string_length(mensaje)));
				char** listaDeContenidosTotal= string_n_split(stringCant,2,"\n");
				int cantActual = atoi(listaDeContenidosTotal[0]);
				int nuevaCantidad = cantActual-1;
				char*contenidoFinal = string_new();
				if(nuevaCantidad == 0){
					string_append(&contenidoFinal,string_substring_until(contenidoBloques,(i-1)));
					string_append(&contenidoFinal,string_substring_from(stringCant,(cantidad_digitos(cantActual))));
				}else{
					if(nuevaCantidad<0){
						log_error(logger,"Ocurrio un error con las posiciones mencionadas");
						return "fail";
					}else{
				string_append(&contenidoFinal,string_substring_until(contenidoBloques,(i+(string_length(mensaje)))));
				string_append(&contenidoFinal,string_itoa(nuevaCantidad));
				string_append(&contenidoFinal,string_substring_from(stringCant,cantidad_digitos(cantActual)));
					}

			}tratar_contenido_en_bloques(contenidoFinal,pathPokemon);
			actualizar_tamanio_archivo(pathPokemon);
			cerrar_archivo(pathPokemon);
			return "OK";

}log_error(logger,"No hay ningun pokemon %s en la posicion %i-%i solicitada",pokemon,posX,posY);
return "FAIL";
} log_error(logger,"No existe el pokemon %s dentro del sistema de archivos",pokemon);
return "FAIL";
}
t_list* tratar_mensaje_GET_POKEMON(char*pokemon){
	char* pathPokemon = generar_path_archivo_pokemon_metadata(pokemon);
	t_list* listadoPos = list_create();
		if(validar_existencia_archivo(pathPokemon)){
			//if(!archivo_abierto(pathPokemon)){
					abrir_archivo(pathPokemon);
					int i=0;
					char* contenidoBloques = string_new();
					char** bloques = obtener_array_de_bloques(pathPokemon);

					for(i=0; i<tam_array_bloques(bloques);i++){
						char*pathBloque = generar_path_bloque(bloques[i]);
						int bloque_fd = open (pathBloque,O_RDWR,0664);

						if(bloque_fd<0){
							log_error(logger,"No se pudo abrir el archivo");
							error(1);
							}
						struct stat stats;
						stat(pathBloque,&stats);

						char*data = malloc(stats.st_size);
						read(bloque_fd,data,stats.st_size);

						string_append(&contenidoBloques,string_substring_until(data,stats.st_size));
						free(data);
						free(pathBloque);
						close(bloque_fd);
					}
					int h,j;
					int siguientePos;

					for(h=0;h<string_length(contenidoBloques);h=siguientePos){
						for(j=h;contenidoBloques[j]!= '\n';j++);

						char*mensajeAux = string_new();
						char*mensajeAux2 =string_substring_from(contenidoBloques,h);
						string_append(&mensajeAux,string_substring_until(mensajeAux2,j-h));

						char** listaMensajeAux = string_n_split(mensajeAux,2,"=");
						char**listaMensaje = string_n_split(listaMensajeAux[0],2,"-");

						list_add(listadoPos,atoi (listaMensaje[0]));
						list_add(listadoPos,atoi (listaMensaje[1]));

						siguientePos = j+1;
					}
					return listadoPos;
}else{
	log_error(logger,"No existe el pokemon %s dentro del file system",pokemon);
	list_clean(listadoPos);
	return listadoPos ;
}
}
