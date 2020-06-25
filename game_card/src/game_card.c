#include "game_card.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(void) {
	config_gc = leer_config();
	iniciar_logger("gameCard.log","gamercard");

	punto_montaje = config_gc->punto_montaje_tallgrass;
	int socket_br;

	//conectarse(socket_br);
	iniciar_tall_grass();

	t_new_pokemon* simple = malloc(sizeof(t_new_pokemon));
	simple->posicion[0]= 118;
	simple->posicion[1] = 21;
	simple->cantidad=15;
	simple->id_mensaje = 124;
	simple->pokemon= "Simple";

	t_new_pokemon* luken = malloc(sizeof(t_new_pokemon));
	luken->posicion[0]= 10;
	luken->posicion[1] = 10;
	luken->cantidad=125;
	luken->id_mensaje = 1242;
	luken->pokemon= "Luken";

	funcion_hilo_new_pokemon(luken, socket_br);

	terminar_programa(socket,config_gc);
}

void laboratorio_de_pruebas(){





}

/*-------------------------------------------------------------------------- CONEXIONES ----------------------------------------------------------------------------- */

void process_request(uint32_t cod_op, uint32_t cliente_fd){ // Cada case depende del que toque ese modulo.
	uint32_t* size;
	void* msg;

	log_info(logger,"Codigo de operacion %d",cod_op);

	switch (cod_op) {
		case GET_POKEMON:
//			msg = malloc(sizeof(t_get_pokemon));
//			msg = recibir_mensaje(cliente_fd, &size);
//			informarAlBroker(msg,cliente_fd,GET_POKEMON);
//			agregar_mensaje(GET_POKEMON, size, msg, cliente_fd);
//			free(msg);
			break;
		case CATCH_POKEMON:
//			msg = malloc(sizeof(t_catch_pokemon));
//			msg = recibir_mensaje(cliente_fd, &size);
//			informarAlBroker(msg,cliente_fd,CATCH_POKEMON);
//			agregar_mensaje(CATCH_POKEMON, size, msg, cliente_fd);
//			free(msg);
			break;
		case NEW_POKEMON:
			//void* recibir_mensaje(uint32_t socket_cliente, uint32_t* size);
			msg = malloc(sizeof(t_new_pokemon));
			//msg = recibir_new_pokemon(cliente_fd,size); // retorna un t_new_pokemon
			informarAlBroker(cliente_fd, NEW_POKEMON);
			funcion_hilo_new_pokemon(msg,cliente_fd);

			//agregar_mensaje(NEW_POKEMON, size, msg, cliente_fd); ??
			free(msg);
			break;

		case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
}

void informarAlBroker(int socket,op_code codigo){
	char* aviso = concatenar("ACK[gamecard]: ",(char*) codigo);
	int tamanioAviso = sizeof(aviso) + 1;
	enviar_mensaje(codigo,aviso,socket,tamanioAviso);
}

void conectarse_a_br(int socket){
	socket = crear_conexion(config_gc -> ip_broker, config_gc -> puerto_broker);
	int time = config_gc->tiempo_reintento_conexion;
	if (socket == -1 ){
		log_info(logger,"imposible conectar con broker, reintento en: %d",time);
		sleep(time);
		socket=0;
		conectarse_a_br(socket); //terrible negrada, pero anda o no nada?
	} else {
		log_info(logger,"conexion exitosa con broker ");
	}
}

void conectarse_a_gb(int socket){
	socket = crear_conexion(config_gc->ip_gameBoy, config_gc->puerto_gameBoy);
	int time = config_gc->tiempo_reintento_conexion;
	if (socket == -1 ){
		log_info(logger,"imposible conectar con gameboy, reintento en: %d",time);
		sleep(time);
		socket=0;
		conectarse_a_gb(socket); //terrible negrada, pero anda o no nada?
	} else {
		log_info(logger,"conexion exitosa con gameboy ");
	}
}

void suscribirme_a_colas() {
	suscribirse_a(NEW_POKEMON);
	suscribirse_a(CATCH_POKEMON);
	suscribirse_a(GET_POKEMON);
}

void suscribirse_a(op_code cola) {
	uint32_t socket 			      = crear_conexion(config_gc -> ip_broker, config_gc -> puerto_broker);
	t_suscripcion* suscripcion 		  = malloc(sizeof(t_suscripcion));
	suscripcion -> id_proceso 		  = malloc(sizeof(char*));
	uint32_t tamanio_suscripcion	  = sizeof(t_suscripcion);

	suscripcion -> cola_a_suscribir   = cola;
	suscripcion -> id_proceso		  = "1"; //ESTE VALOR SE SACA DE CONFIG
	suscripcion -> socket 		      = socket;
	suscripcion -> tiempo_suscripcion = 0; //ESTE VALOR SIEMPRE ES 0

	enviar_mensaje(SUBSCRIPTION, suscripcion, socket, tamanio_suscripcion);

	free(suscripcion -> id_proceso);
	free(suscripcion);

}

t_config_game_card* leer_config() {

	t_config_game_card* config_game_card = malloc(sizeof(t_config_game_card));

	t_config* config = config_create("Debug/game_card.config");

	config_game_card -> tiempo_reintento_conexion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_CONEXION");
	config_game_card -> tiempo_reintento_operacion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_OPERACION");
	config_game_card -> punto_montaje_tallgrass = strdup(config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS"));
	config_game_card -> ip_broker = strdup(config_get_string_value(config, "IP_BROKER"));
	config_game_card -> puerto_broker = strdup(config_get_string_value(config, "PUERTO_BROKER"));
	config_game_card -> ip_gameBoy= strdup(config_get_string_value(config, "IP_GAMEBOY"));
	config_game_card -> puerto_gameBoy= strdup(config_get_string_value(config, "PUERTO_GAMECARD"));
	config_game_card -> ip_gameCard= strdup(config_get_string_value(config, "IP_GAMEBOY"));
	config_game_card -> puerto_gameCard= strdup(config_get_string_value(config, "PUERTO_GAMECARD"));
	config_game_card -> tiempo_retardo_operacion = config_get_int_value(config,"TIEMPO_RETARDO_OPERACION");

	config_destroy(config);
	return config_game_card;
}

void liberar_config_gc(t_config_game_card* config) {
	free(config->ip_broker);
	free(config->ip_gameBoy);
	free(config->ip_gameCard);
	free(config->puerto_broker);
	free(config->puerto_gameBoy);
	free(config->puerto_gameCard);
	free(config->punto_montaje_tallgrass);
	free(config);
}


void terminar_programa(int conexion,t_config_game_card* config_gc) {
	liberar_config_gc(config_gc);
	liberar_logger(logger);
	liberar_conexion(conexion);
}

/*-------------------------------------------------------------------------- TALL-GRASS ----------------------------------------------------------------------------- */

void iniciar_tall_grass(){

	punto_montaje = config_gc->punto_montaje_tallgrass;
	log_info(logger,"Iniciando tallgrass en: %s",punto_montaje);

	if (!isDir(punto_montaje)){

		crear_directorios(punto_montaje);
		crear_metadata_fs(punto_montaje);
		crear_bitmap(punto_montaje);
		crear_blocks(punto_montaje);

		log_info(logger,"TALL_GRASS levantado exitosamente");

	} else {
		log_info(logger, "Tallgrass ya existente en %s",punto_montaje);
	}
}


void crear_directorios(char* path){

	char* blocks = concatenar(path,"/Blocks");
	char* metadata = concatenar(path,"/Metadata");
	char* files = concatenar(path,"/Files");

	//podria verificar si existen los directorios pero ni idea TODO
	//aclaro que esta terrible negrada es porque el fopen(w+b) no te crea directorios, python si obvio porque es mejor

	mkdir(path,0777);
	mkdir(metadata,0777);
	mkdir(files,0777);
	mkdir(blocks,0777);

	free(blocks);
	free(metadata);
	free(files);
}

void crear_blocks(char* path){

	char* realPath = concatenar(path,"/Metadata/Metadata.bin");
	t_metadata metadata = leer_fs_metadata(realPath);

	int cantidadBloques = metadata.blocks;
	int sizeBlock = metadata.blocksize;
	int i = 0;

	char* blocksPath = concatenar(path,"/Blocks/");


	while(i < cantidadBloques){
		char* c = string_itoa(i);
		char* block = concatenar(blocksPath,c);
		char* bloque = concatenar(block,".bin");
		create_file_with_size(bloque,sizeBlock);

		free(block);
		free(bloque);
		free(c);
		i++;
	}
	log_info(logger,"Se crearon %d blocks de tamaño: %d bytes",cantidadBloques -1 ,sizeBlock);

	free(blocksPath);
	free(realPath);


}

void crear_metadata_fs(char* path){

	char* realPath = concatenar(path,"/Metadata/Metadata.bin");

	FILE* f = fopen(realPath,"wb");
	fclose(f);

	t_config* metadata_config = config_create(realPath);

	config_set_value(metadata_config,"MAGIC_NUMBER","TALL_GRASS");
	config_set_value(metadata_config,"BLOCKS_SIZE","64");
	config_set_value(metadata_config,"BLOCKS","5192");

	int result = config_save(metadata_config);

	if (result != 1){
		log_info(logger,"Ojo capo, algo hiciste mal y EFICIENTE SEGURO QUE NO ES....");
		log_info(logger,"NA posta, hay algo mal en el config_create");
	}

	config_destroy(metadata_config);
	log_info(logger,"Se creo Metadata.bin en: %s",realPath);
	free(realPath);

}

void crear_bitmap(char* path){

	char* bitmapPath = concatenar(path,"/Metadata/Bitmap.bin");
	char* metadataPath = concatenar(path,"/Metadata/Metadata.bin");
	t_metadata fs_metadata = leer_fs_metadata(metadataPath);

	int size_in_bytes = (fs_metadata.blocks/8);
	char* data = string_repeat('0',(size_in_bytes*8));

	FILE* file = fopen(bitmapPath,"wb");
	fwrite(data,649,1,file);
	fclose(file);

	destrozar_fs_metadata(fs_metadata);
	free(bitmapPath);
	free(metadataPath);
	free(data);

	log_info(logger,"Se creo el bitarray de %d posiciones, con un tamaño de %d bytes",fs_metadata.blocks,size_in_bytes);

}

void crear_pokemon(t_new_pokemon* newPoke,char* path){

	t_file_metadata metadata = generar_file_metadata(newPoke);

	escribir_block_inicial(metadata,newPoke);

	grabar_metadata_file(metadata,path);

}
/*------------------------------------------------------------------------- BITMAP/BITARRAY ----------------------------------------------------------------------------- */

t_bitarray* obtener_bitmap(){

	char* bitmapPath = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Bitmap.bin");
	int size_in_bytes = bitarray_default_size();
	char* data = malloc(649);


	FILE* bitmap = fopen(bitmapPath,"rb");

	fread(data,649,1,bitmap);

	fclose(bitmap);

	t_bitarray* bitarray = bitarray_create_with_mode(data, size_in_bytes, LSB_FIRST);

	free(bitmapPath);
	return bitarray;

}

void actualizar_bitmap(t_bitarray* bitarray){
	char* bitmapPath = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Bitmap.bin");

	FILE* bitmap = fopen(bitmapPath,"wb");
	fwrite(bitarray->bitarray,649,1,bitmap);
	fclose(bitmap);

	bitarray_destroy(bitarray);
	free(bitmapPath);

}

int bitarray_default_size(){

	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Metadata.bin");

	t_metadata metadata = leer_fs_metadata(path);
	int tam = (metadata.blocks/8);

	free(path);

	return tam;

}
char* buscar_block_libre(){

	t_bitarray* bitarray = obtener_bitmap();

	int block_libre;
	int blocks = (bitarray_default_size()*8);

	for(int i=0; i< blocks; i++){

		if (bitarray_test_bit(bitarray,i) != true){

			block_libre = i;
			log_info(logger,"El block %d esta libre, usalo tranka nomas",i);
			bitarray_set_bit(bitarray,i);
			actualizar_bitmap(bitarray);
			log_info(logger,"El block %d ahora esta en uso",i);
			break;

		} else {
				log_info(logger,"El block %d esta ocupado kpo, sigo buscando",i);
		}
	}

	return block_libre;

}

t_list* asignar_block_inicial(){

	t_list* lista = list_create();

	int block = buscar_block_libre();

	list_add(lista,block);

	return lista;
}

int asignar_nuevo_bloque(t_new_pokemon* newpoke,char* path,char* key, char* value){

	int block = buscar_block_libre();

	t_file_metadata newpoke_metadata_file = leer_file_metadata(path);

	list_add(newpoke_metadata_file.blocks,block);

	grabar_metadata_file(newpoke_metadata_file,path);

	return block;
}

/*-------------------------------------------------------------------------- METADATA-FILES----------------------------------------------------------------------------- */

t_file_metadata generar_file_metadata(t_new_pokemon* newPoke){

	t_file_metadata metadata;

	metadata.directory = "N";
	metadata.open = "N";
	metadata.blocks = asignar_block_inicial();
	metadata.size = string_itoa(calcular_size_inicial(newPoke));

	return metadata;

}

t_file_metadata leer_file_metadata(char* path){

	t_file_metadata file_metadata_leido;

	t_config* meta_config = config_create(path);

	file_metadata_leido.directory = strdup(config_get_string_value(meta_config,"DIRECTORY"));
	file_metadata_leido.open = strdup(config_get_string_value(meta_config,"OPEN"));
	file_metadata_leido.size = string_itoa(config_get_int_value(meta_config,"SIZE"));

	file_metadata_leido.blocks = chardoble_to_tlist(config_get_array_value(meta_config,"BLOCKS"));

	config_destroy(meta_config);

	return file_metadata_leido;
}

int tamanio_de_metadata(t_metadata metadata){
	int stringLong = sizeof("TALL_GRASS") + 1 ;
	return stringLong + (sizeof(int) *2) ;
}

int tamanio_file_metadata(t_file_metadata fileMeta){
	int listSize = list_size(fileMeta.blocks);
	return sizeof(char) + listSize + sizeof(int) + sizeof(char);
}

t_metadata leer_fs_metadata(char* path){

	t_metadata metadata_fs;
	t_config* config_metadata = config_create(path);

	metadata_fs.blocks = config_get_int_value(config_metadata,"BLOCKS");
	metadata_fs.blocksize= config_get_int_value(config_metadata,"BLOCKS_SIZE");
	metadata_fs.magic = strdup(config_get_string_value(config_metadata,"MAGIC_NUMBER"));

	config_destroy(config_metadata);

	return metadata_fs;

}

void grabar_metadata_file(t_file_metadata metadata,char* path){

	char* blocks_array = list_to_string_array(metadata.blocks);

	t_config* meta_file_pokemon = config_create(path);

	config_set_value(meta_file_pokemon,"OPEN",metadata.open);
	config_set_value(meta_file_pokemon,"DIRECTORY",metadata.directory);
	config_set_value(meta_file_pokemon,"SIZE",metadata.size);
	config_set_value(meta_file_pokemon,"BLOCKS",blocks_array);

	config_save(meta_file_pokemon);

	log_info(logger,"Se creo el Metadata.bin con valores OPEN= %s, DIRECTORY=%s, SIZE=%s, BLOCKS=%s",metadata.open,metadata.directory,metadata.size,blocks_array);
	config_destroy(meta_file_pokemon);
	free(blocks_array);

}

bool is_directory(char* path){
	t_config* dirconfig = config_create(path);
	char* directory = strdup(config_get_string_value(dirconfig,"DIRECTORY"));
	config_destroy(dirconfig);
	return directory == "Y";

}

bool esta_lockeado(char* path){

	t_config* file_meta_config = config_create(path);

	char* open = strdup(config_get_string_value(file_meta_config,"OPEN"));
	config_destroy(file_meta_config);

	return open[0] == 'Y';
}

void lock_file(char* path){

	t_config* file_config = config_create(path);

	config_set_value(file_config,"OPEN","Y");

	config_save(file_config);

	config_destroy(file_config);

}

void unlock_file(char* path){

	t_config* file_config = config_create(path);

	config_set_value(file_config,"OPEN","N");

	config_save(file_config);

	config_destroy(file_config);
}





/*-------------------------------------------------------------------------- NEW-POKEMON ----------------------------------------------------------------------------- */

void funcion_hilo_new_pokemon(t_new_pokemon* new_pokemon,int socket){
	log_info(logger,"THREAD NEW POKEMON");
	log_info(logger,"Llego un NEW_POKEMON %s en (%d,%d) con cantidad %d",new_pokemon->pokemon,new_pokemon->posicion[0],new_pokemon->posicion[1],new_pokemon->cantidad);

	char* path_metafile = obtener_path_metafile(new_pokemon);
	char* dir_path_newpoke = obtener_path_dir_pokemon(new_pokemon);

	if (el_pokemon_esta_creado(dir_path_newpoke)){

		log_info(logger,"El pokemon %s ya existe",new_pokemon->pokemon);

		verificar_apertura_new_pokemon(path_metafile,new_pokemon->pokemon);

		char* key = get_key_from_position(new_pokemon);
		char* value = get_value_from_position(new_pokemon);
		char* temporary_file = generar_archivo_temporal(path_metafile);

		if (existe_la_posicion(key,temporary_file)){

			log_info(logger,"Existia la posicion %s del pokemon %s, se va a actualizar",key,new_pokemon->pokemon);
			actualizar_pokemon(temporary_file, path_metafile, key, value);
			log_info(logger,"Se actualizo la posicion %s del pokemon %s",key,new_pokemon->pokemon);

			} else{
				log_info(logger,"No existia la posicion %s del pokemon %s , se agrega",key,new_pokemon->pokemon);
				agregar_nueva_posicion(new_pokemon,path_metafile, key, value);

			}

		}else{ // NO EXISTIA EL POKEMON

		log_info(logger,"No existia el pokemon %s",new_pokemon->pokemon);

		mkdir(dir_path_newpoke, 0777);
		log_info(logger,"Se creo el directorio %s en %s",new_pokemon->pokemon,dir_path_newpoke);

		FILE* f = fopen(path_metafile,"wb");
		fclose(f);
		log_info(logger,"Se creo el archivo metafile del pokemon %s vacio",new_pokemon->pokemon);

		crear_pokemon(new_pokemon, path_metafile);
		log_info(logger,"Se creo por completo el pokemon %s",new_pokemon->pokemon);


		}

	log_info(logger,"THREAD FINISHED , unlockeo el pokemon");
	unlock_file(path_metafile);

	//t_appeared_pokemon* appeared = armar_appeared(new_pokemon);
	//enviar_appeared POKKEMON TODO serializacion

	free(path_metafile);
	free(dir_path_newpoke);
	//destrozar_new_pokemon(new_pokemon);

}

bool el_block_tiene_espacio_justo(char* key,char* value,char* ultimo_block_path){
	int size_key_y_value = strlen(key) + strlen(value) + 1 ;
	return (64 - file_size(ultimo_block_path)) >= size_key_y_value; //file_size = tamaño que estoy ocupando
}

bool el_block_tiene_espacio_pero_no_alcanza(char* blockpath){
	return file_size(blockpath) < 64;
}

int la_posicion_ya_existe(t_new_pokemon* newpoke,char* meta_path, char* key_posicion){

	t_config* metadata_file = config_create(meta_path);

	char** blocks = config_get_array_value(metadata_file,"BLOCKS");

	config_destroy(metadata_file);

	return block_que_tiene_esa_posicion(blocks, key_posicion);
}

int block_que_tiene_esa_posicion(char** blocks,char* key){

	int cantidad_blocks = size_char_doble(blocks);
	int i = 0;


	while(i < cantidad_blocks){
		char* pathblock = block_path(blocks[i]);
		t_config* config = config_create(pathblock);

		if (config_has_property(config,key)){
			return i;
			break;
		}

		config_destroy(config);
		i++;
		free(pathblock);
	}


	free(key);
	free(blocks);
	return -1;
}

bool esta_la_posicion_mal_grabada(char* key,char* temporary_file){
	return existe_la_key_mal_grabada(key, temporary_file);
}

char* generar_archivo_temporal(char* metapath_file){

	t_config* metapath = config_create(metapath_file);
	char** blocks = config_get_array_value(metapath,"BLOCKS");
	config_destroy(metapath);

	char* blockpath;
	int cantidad_blocks = size_char_doble(blocks);
	char* a = malloc(1);
	char* random_path = rand_string();
	FILE* temporary = fopen(random_path,"wb");

	int n = 0;
	for(int i = 0; i < cantidad_blocks; i++){

		blockpath = block_path(blocks[i]);
		FILE* block = fopen(blockpath,"rb");
		int filesize = file_size(blockpath);

		while (n < filesize){
			fread(a,1,1,block);
			fwrite(a,1,1,temporary);
			n++;
		}
		n=0;
		fclose(block);
	}

	fclose(temporary);

	free(blockpath);
	free(a);

	return random_path;
}

bool existe_la_key_mal_grabada(char* key,char* temporary_file){

	t_config* config = config_create(temporary_file);

	bool x = config_has_property(config,key);

	config_destroy(config);

	free(temporary_file);
	return x;

}

void re_grabar_temporary_en_blocks(char* temporary_file,char* path_metafile){

	t_config* config_meta = config_create(path_metafile);
	char** blocks = config_get_array_value(config_meta,"BLOCKS");
	config_destroy(config_meta);


	int cantidad_blocks = size_char_doble(blocks);
	int size_temporary = file_size(temporary_file);
	limpiar_blocks(blocks);

	char* blockpath;
	char* a = malloc(1);

	int i =0;
	int b =0;

	FILE* temporary = fopen(temporary_file,"rb");

	while(b < cantidad_blocks){

		blockpath = block_path(blocks[b]);
		FILE* block = fopen(blockpath,"wb");
		int current_size = file_current_size(block);

		while(current_size!=0 && i<size_temporary){

			fread(a,1,1,temporary);
			fwrite(a,1,1,block);
			i++;
			current_size = file_current_size(block);

		}

		fclose(block);
		b++;
	}

	fclose(temporary);
	remove(temporary_file);
	free(blockpath);
	free(a);

}

void limpiar_blocks(char** blocks){

	char* pathblock;

	for (int i =0; i<size_char_doble(blocks); i++){

		pathblock = block_path(blocks[i]);
		FILE* block = fopen(pathblock,"wb");
		fclose(block);
	}

	free(pathblock);

}

bool existe_la_posicion(char* key,char* temporary_path){

	t_config* temporary_config = config_create(temporary_path);
	bool x = config_has_property(temporary_config,key);
	config_destroy(temporary_config);
	return x;
}


bool el_pokemon_esta_creado(char* path){
	return es_directorio(path);
}

void verificar_apertura_new_pokemon(char* path_metafile,char* nombre_pokemon){

	bool esta_en_uso = esta_lockeado(path_metafile);

	while (esta_en_uso){
		int secs = config_gc->tiempo_retardo_operacion;
		log_info(logger,"No se puede leer ni escribir este pokemon porque esta en uso OPEN=Y, reintentando en: %d",secs);
		sleep(secs);
		esta_en_uso = esta_lockeado(path_metafile);
	}


	log_info(logger,"Se puede acceder al pokemon %s, lockeo el archivo",nombre_pokemon);
	lock_file(path_metafile);


}

void actualizar_pokemon(char* temporary_path,char* path_metafile,char* key,char* value){

	int nueva_cantidad;

	t_config* temporary_config = config_create(temporary_path);

	nueva_cantidad = config_get_int_value(temporary_config,key);
	nueva_cantidad += atoi(value);

	config_set_value(temporary_config, key, string_itoa(nueva_cantidad));

	config_save(temporary_config);
	config_destroy(temporary_config);

	re_grabar_temporary_en_blocks(temporary_path, path_metafile);

}


void agregar_nueva_posicion(t_new_pokemon* newpoke,char* pathmeta_poke,char* key,char* value){

		t_config* config_poke = config_create(pathmeta_poke);
		char** blocks = config_get_array_value(config_poke,"BLOCKS");
		config_destroy(config_poke);

		int cantidad_blocks = size_char_doble(blocks);
		int ultimo_block = blocks[cantidad_blocks];

		char* path_last_block = block_path(string_itoa(ultimo_block));

		if (el_block_tiene_espacio_justo(key, value,path_last_block)){

			escribir_data_en_block(path_last_block,key,value);

		}else if (el_block_tiene_espacio_pero_no_alcanza(path_last_block)){ // el ultimo cluster tiene espacio pero no alcanza

			int nuevo_cluster = asignar_nuevo_bloque(newpoke,pathmeta_poke,key,value);
			char* nuevo_block_path = block_path(string_itoa(nuevo_cluster));

			escribir_data_sin_fragmentacion_interna(nuevo_block_path,path_last_block,key,value);

			free(nuevo_block_path);
		}

		free(path_last_block);
}


void escribir_data_sin_fragmentacion_interna(char* block_nuevo,char* block_viejo, char*key, char*value){


	char* a_escribir = posicion_into_string(key, value);
	int leng =strlen(a_escribir)+1;
	int i = 0;

	FILE* file_block_viejo = fopen(block_viejo,"ab");

	int size_block_viejo = file_current_size(file_block_viejo);

	while (size_block_viejo > 0){
		fwrite(&a_escribir[i],1,1,file_block_viejo);
		size_block_viejo = file_current_size(file_block_viejo);
		i++;
	}

	fclose(file_block_viejo);

	FILE* file_block_nuevo = fopen(block_nuevo,"wb");

	while(i<leng){
		fwrite(&a_escribir[i],1,1,file_block_nuevo);
		i++;
	}

	fclose(file_block_nuevo);

	free(a_escribir);

}

void escribir_data_en_block(char* blockpath,char* key,char* value){

	char* a_escribir = posicion_into_string(key, value);
	int length = strlen(a_escribir) + 1;
	int blocksize = file_size(blockpath);

	FILE* block = fopen(blockpath,"ab");
	fwrite(a_escribir,length,1,block);
	fclose(block);

	free(a_escribir);
}
int calcular_size_inicial(t_new_pokemon* newPoke){
	int size;

	char* key = get_key_from_position(newPoke);
	char* value = get_value_from_position(newPoke);


	size = strlen(key) + strlen(value) + 1;

	free(key);
	free(value);

	return size;

}

void escribir_block_inicial(t_file_metadata metadata,t_new_pokemon* newPoke){

	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Blocks/");
	char* blockPath = concatenar(path,string_itoa(metadata.blocks->head->data));
	char* finalPath = concatenar(blockPath,".bin");

	char* key = get_key_from_position(newPoke);
	char* value = get_value_from_position(newPoke);

	t_config* block = config_create(finalPath);

	config_set_value(block,key,value);

	int status = config_save_in_file(block,finalPath);

	if(status != 1){
		log_info(logger, "Error al escribir en el block %s el pokemon %s",finalPath,newPoke->pokemon);
	}

	config_destroy(block);
	free(path);
	free(blockPath);
	free(finalPath);
}


char* get_key_from_position(t_new_pokemon* newpoke){
	char* pos_x = string_itoa(newpoke->posicion[0]);
	char* pos_y = string_itoa(newpoke->posicion[1]);
	char* key = concatenar(pos_x,"-");
	char* final_key = concatenar(key,pos_y);
	return final_key;
}

char* get_value_from_position(t_new_pokemon* newpoke){
	char* value = string_itoa(newpoke->cantidad);
	return value;
}


/*-------------------------------------------------------------------------- TOOLS ----------------------------------------------------------------------------- */


int file_size(char* path){
	FILE* f = fopen(path,"r");
	fseek(f, 0L, SEEK_END);
	int i = ftell(f);
	fclose(f);
	return i;
}

bool archivoVacio(char* path){
	return file_size(path) == 0;
}

bool isFile(char* path){
	FILE* f = fopen(path,"rb");
	fclose(f);
	return f != NULL;
}

bool isDir(const char* name){
    DIR* directory = opendir(name);

    if(directory != NULL){
     closedir(directory);
     return true;
    } else {
    	return false;
    }
}

char* list_to_string_array(t_list* blocks){

	char* format = "[";

	if (blocks->elements_count == 1){

		format = concatenar(format,string_itoa(blocks->head->data));

	} else {

			while (blocks->head->next != NULL){
				format = concatenar(format,string_itoa(blocks->head->data));
				format = concatenar(format,",");
				blocks->head = blocks->head->next;

					if (blocks->head->next == NULL){
						format=concatenar(format,string_itoa(blocks->head->data));
						}
				}
	}

	format = concatenar(format,"]");

	return format;
}

void create_file_with_size(char* path,int size) {
        int X = size - 1 ;
        FILE *fp = fopen(path, "wb");
        fseek(fp, X , SEEK_SET);
        fputc('\0', fp);
        fclose(fp);
}

int file_current_size(FILE* f){

	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Metadata.bin");
	t_metadata metadataFS = leer_fs_metadata(path);

	int pos = ftell(f);
	free(path);
	return metadataFS.blocksize - pos;

}

char* posicion_into_string(char*key,char*value){
	char* posicion = concatenar(key,"=");
	char* posicion2 = concatenar(posicion,value);
	free(posicion);
	return posicion2;
}

char* block_path(char* block){
	char* path1 = concatenar(config_gc->punto_montaje_tallgrass,"/Blocks/");
	char* path2 = concatenar(path1,block);
	char* path3 = concatenar(path2,".bin");
	free(path1);
	free(path2);
	return path3;
}

bool es_directorio(char* path){
	DIR* dir = opendir(path);
	int x = dir;
	closedir(dir);
	return x;
}

char* obtener_path_metafile(t_new_pokemon* pokemon){
	punto_montaje = config_gc->punto_montaje_tallgrass;
	char* path  = concatenar (punto_montaje,"/Files/");
	char* path2 = concatenar (path,pokemon->pokemon); //terrible negrada esto, pero anda o no anda?
	char* path3 = concatenar(path2,"/Metadata.bin");
	return path3;
}

char* obtener_path_dir_pokemon(t_new_pokemon* pokemon){
	punto_montaje = config_gc->punto_montaje_tallgrass;
	char* path = concatenar(punto_montaje,"/Files/");
	char* path2 = concatenar(path,pokemon->pokemon);
	return path2;
}

uint32_t sizeNewPokemon(t_new_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon->pokemon) + 1;
}


uint32_t sizeAppearedPokemon(t_appeared_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon->pokemon) + 1;
}

t_appeared_pokemon* armar_appeared(t_new_pokemon* new_pokemon){

	//TODO
		t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));

		appeared_pokemon->posicion[0] = new_pokemon->posicion[0];
		appeared_pokemon->posicion[1] = new_pokemon->posicion[1];
		appeared_pokemon->pokemon = new_pokemon->pokemon;
		appeared_pokemon->id_mensaje_correlativo = 0;
		appeared_pokemon->id_mensaje = new_pokemon->id_mensaje;
		return appeared_pokemon;
}

int size_char_doble(char** array){

	int i = 0;
	int size = 0;

	while(array[i] != NULL){
		i++;
		size++;
	}

	return size;

}

void destrozar_new_pokemon(t_new_pokemon* new_pokemon){
	free(new_pokemon->pokemon);
}

void destrozar_metadata_file(t_file_metadata metadata){
	free(metadata.directory);
	free(metadata.open);
	free(metadata.size);
	list_destroy(metadata.blocks);
}

void destrozar_fs_metadata(t_metadata metadata){
	free(metadata.magic);
}

char* rand_string() {

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *randomString = NULL;

    int length = rand () % (35-3+1) + 3;   //numero = rand () % (N-M+1) + M;   // Este está entre M y N
    if (length) {
        randomString = malloc(sizeof(char) * (length +1));

        if (randomString) {
            for (int n = 0;n < length;n++) {
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }
    char* rand = concatenar(randomString,".bin");
    free(randomString);
    return rand;
}

char* generar_string_desde_blocks(char** blocks){

	char* data;

	for (int i = 0; i<= size_char_doble(blocks); i++){ //RECORRO TODOS LOS BLOQUES

		char* pathblock = block_path(*blocks[i]);
		FILE* block  = fopen(pathblock,"rb");

		int n = 0;

		while (!feof(block)){
			fread(data[i],1,1,block);
			n++;
		}

		free(pathblock);
		fclose(block);

	}

	return data;

}

t_list* chardoble_to_tlist(char** chardoble){

	int size_chardoble = size_char_doble(chardoble);
	t_list* lista = list_create();

	for(int i=0; i<size_chardoble; i++){

		list_add(lista,atoi(chardoble[i]));
	}
	return lista;
}


