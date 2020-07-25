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

	int socket;

	iniciar_tall_grass();
	iniciar_semaforos();

	iniciar_conexion();
	sleep(1);
	iniciar_hilos_suscripcion();


	//pruebas_new_pokemon(socket);
	//pruebas_catch_pokemon(socket);
	//pruebas_get_pokemon(socket);

	sem_wait(&terminarprograma);
	terminar_programa(socket, config_gc);

}


void pruebas_new_pokemon(int socket){

	t_new_pokemon* luca = malloc(sizeof(t_new_pokemon));
	luca->posicion[0]=1;
	luca->posicion[1] = 2;
	luca->cantidad=4;
	luca->id_mensaje = 124;
	luca->pokemon= "Adriano";

	funcion_hilo_new_pokemon(luca, socket);


}
void pruebas_catch_pokemon(int socket){

	t_catch_pokemon* kenny = malloc(sizeof(t_catch_pokemon));
	kenny->pokemon="Luken";
	kenny->posicion[0] = 129;
	kenny->posicion[1] = 547;
	kenny->id_mensaje = 123;


	funcion_hilo_catch_pokemon(kenny, socket);


}
void pruebas_get_pokemon(int socket){

	t_get_pokemon* get_poke = malloc(sizeof(t_get_pokemon));
	get_poke->id_mensaje = 1;
	get_poke->pokemon = "Adriano";

	unlock_file(obtener_path_metafile("Adriano"));
	funcion_hilo_get_pokemon(get_poke, socket);


}
/*-------------------------------------------------------------------------- CONEXIONES ----------------------------------------------------------------------------- */

void process_request(uint32_t cod_op, uint32_t cliente_fd){

	uint32_t size;
	op_code* codigo_op = malloc(sizeof(op_code));

	sem_wait(&muteadito);
	void* stream = recibir_paquete(cliente_fd, &size, codigo_op);
	//cod_op = (*codigo_op);

	void* msg = deserealizar_paquete(stream, cod_op, size);

	t_new_pokemon* newpoke;
	t_get_pokemon* getpoke;
	t_catch_pokemon* catchpoke;

	switch (cod_op) {
		case NEW_POKEMON:
			newpoke = msg;
			informar_al_broker(newpoke->id_mensaje,NEW_POKEMON);
			funcion_hilo_new_pokemon(msg,cliente_fd);
			break;
		case GET_POKEMON:
			getpoke = msg;
			informar_al_broker(getpoke->id_mensaje,GET_POKEMON);
			funcion_hilo_get_pokemon(msg,cliente_fd);
			break;
		case CATCH_POKEMON:
			catchpoke = msg;
			informar_al_broker(catchpoke->id_mensaje,CATCH_POKEMON);
			funcion_hilo_catch_pokemon(msg,cliente_fd);
			break;
		case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
	sem_post(&muteadito);
}

void iniciar_semaforos(){
	sem_init(&semaforo,0,1);
	sem_init(&muteadito,0,1);
	sem_init(&mx_bitmap,0,1);
	sem_init(&terminarprograma,0,0);

}
void* iniciar_server_gamecard(){

	iniciar_servidor(config_gc->ip_gameCard,config_gc->puerto_gameCard);
	return NULL;
}

void iniciar_conexion() {
	log_warning(logger,"Iniciando servidor gamecard [THREAD]");

	uint32_t err = pthread_create(&hilo_servidor, NULL, iniciar_server_gamecard, NULL);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}
	pthread_detach(hilo_servidor);

}

void iniciar_hilos_suscripcion(){
	iniciar_hilo_new();
	iniciar_hilo_get();
	iniciar_hilo_catch();
}



void informar_al_broker(uint32_t id_mensaje, op_code codigo){

	t_ack* ack = malloc(sizeof(t_ack));

	ack -> id_mensaje = id_mensaje;
	ack -> tipo_mensaje = codigo;
	ack -> id_proceso = config_gc -> id_proceso;

	uint32_t size_mensajee = size_ack(ack);

	uint32_t socket = crear_conexion(config_gc -> ip_broker, config_gc -> puerto_broker);

	if(socket != -1) {
		enviar_mensaje(ACK, ack, socket, size_mensajee);
		close(socket);
	}
}


//void* conexion_con_game_boy() {
//	log_info(logger,"Iniciando conexion con gameboy");
//	iniciar_servidor("127.0.0.2", "5662");
//	log_info(logger,"Conexion con gameboy establecida exitosamente");
//	return NULL;
//}

void iniciar_conexion_game_boy() {
	uint32_t err = pthread_create(&hilo_game_boy, NULL, iniciar_server_gamecard, NULL);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}
}

void iniciar_hilo_new(){

	log_warning(logger,"Iniciando servidor de escucha con broker[THREAD]");
	uint32_t err = pthread_create(&hilo_new, NULL, (void*) suscribirse_a,NEW_POKEMON);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}
	pthread_detach(hilo_new);

}
void iniciar_hilo_get(){

	log_warning(logger,"Iniciando servidor de escucha con broker[THREAD]");
	uint32_t err = pthread_create(&hilo_get, NULL, (void*) suscribirse_a,GET_POKEMON);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}
	pthread_detach(hilo_get);

}
void iniciar_hilo_catch(){

	log_warning(logger,"Iniciando servidor de escucha con broker[THREAD]");
	uint32_t err = pthread_create(&hilo_catch, NULL, (void*) suscribirse_a,CATCH_POKEMON);
		if(err != 0) {
			log_error(logger, "El hilo no pudo ser creado!!");
		}
	pthread_detach(hilo_catch);

}

void suscribirse_a(op_code cola) {

	uint32_t socket = crear_conexion(config_gc -> ip_broker, config_gc -> puerto_broker);
	t_suscripcion* suscripcion = malloc(sizeof(t_suscripcion));
	uint32_t cod_op = 0;
	if (socket == -1 ){
			int time = config_gc->tiempo_reintento_conexion;
			log_info(logger,"imposible conectar con broker, reintento en: %d",time);
			sleep(time);
			suscribirse_a(cola);
	}else {

		suscripcion -> cola_a_suscribir= cola;
		suscripcion -> id_proceso = config_gc->id_proceso;
		suscripcion -> socket = socket;
		suscripcion -> tiempo_suscripcion = 0; //ESTE VALOR SIEMPRE ES 0

		uint32_t tamanio_suscripcion = size_mensaje(suscripcion, SUBSCRIPTION);
		enviar_mensaje(SUBSCRIPTION, suscripcion, suscripcion->socket, tamanio_suscripcion);

		while(1) {
			recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
			process_request(cod_op, socket);
		}
	}
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
	config_game_card -> ip_gameCard= strdup(config_get_string_value(config, "IP_GAMECARD"));
	config_game_card -> puerto_gameCard= strdup(config_get_string_value(config, "PUERTO_GAMECARD"));
	config_game_card -> tiempo_retardo_operacion = config_get_int_value(config,"TIEMPO_RETARDO_OPERACION");
	config_game_card -> id_proceso = config_get_int_value(config,"ID_PROCESO");
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

		log_info(logger,"Filesystem TALLGRASS iniciado correctamente, actualmente vacio");

	} else {
		log_info(logger, "Tallgrass ya existente en %s",punto_montaje);

	}
}


void crear_directorios(char* path){

	char* blocks = concatenar(path,"/Blocks");
	char* metadata = concatenar(path,"/Metadata");
	char* files = concatenar(path,"/Files");

	mkdir(path,0777);
	mkdir(metadata,0777);
	mkdir(files,0777);
	mkdir(blocks,0777);

	free(blocks);
	free(metadata);
	free(files);

	log_info(logger,"Se crearon los directorios: Blocks, Metadata, Files");
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
	config_set_value(metadata_config,"BLOCKS","1024");

	int result = config_save(metadata_config);

	if (result != 1){
		log_error(logger,"El resultado al usar config_save genero un -1");
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
	fwrite(data,size_in_bytes,1,file); // TODO , si rompe ojo aca
	fclose(file);

	destrozar_fs_metadata(fs_metadata);
	free(bitmapPath);
	free(metadataPath);
	free(data);

	log_info(logger,"Se creo el bitarray de %d posiciones, con un tamaño de %d bytes",fs_metadata.blocks,size_in_bytes);

}

/*------------------------------------------------------------------------- BITMAP/BITARRAY ----------------------------------------------------------------------------- */


//TODO maybe un semaforo para el bitarray? entiendo que el filesystem es rapido, y mensajes en simultaneo se los banca porque termina afondo.

t_bitarray* obtener_bitmap(){

	log_error(logger,"Eperando el uso del mutex_bitmap");
	sem_wait(&mx_bitmap);
	log_warning(logger,"mutex_bitmap listo");

	char* bitmapPath = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Bitmap.bin");
	int size_in_bytes = bitarray_default_size_in_bytes();
	char* data = malloc(size_in_bytes);

	FILE* bitmap = fopen(bitmapPath,"rb");

	fread(data,size_in_bytes,1,bitmap);

	fclose(bitmap);

	t_bitarray* bitarray = bitarray_create_with_mode(data, size_in_bytes, LSB_FIRST);

	free(bitmapPath);
	return bitarray;

}

void actualizar_bitmap(t_bitarray* bitarray){
	char* bitmapPath = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Bitmap.bin");

	int size_in_bytes = bitarray_default_size_in_bytes();

	FILE* bitmap = fopen(bitmapPath,"wb");
	fwrite(bitarray->bitarray,size_in_bytes,1,bitmap);
	fclose(bitmap);

	bitarray_destroy(bitarray);
	free(bitmapPath);

	sem_post(&mx_bitmap);
}

int bitarray_default_size_in_bytes(){

	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Metadata.bin");

	t_metadata metadata = leer_fs_metadata(path);
	int tam = (metadata.blocks/8);

	free(path);

	return tam;

}
char* buscar_block_libre(){

	t_bitarray* bitarray = obtener_bitmap();

	int block_libre;
	int blocks = (bitarray_default_size_in_bytes()*8);

	for(int i=0; i< blocks; i++){

		if (bitarray_test_bit(bitarray,i) != true){

			block_libre = i;
			log_info(logger,"El block %d esta libre, usalo tranka nomas",i);
			bitarray_set_bit(bitarray,i);
			actualizar_bitmap(bitarray);
			log_info(logger,"El block %d ahora esta en uso",i);
			break;

		} else {
				log_info(logger,"El block %d esta ocupado, sigo buscando",i);
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

int asignar_nuevo_bloque(char* path){

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
	metadata.open = "Y";
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

	log_warning(logger,"Se creo el Metadata.bin con valores OPEN= %s, DIRECTORY=%s, SIZE=%s, BLOCKS=%s",metadata.open,metadata.directory,metadata.size,blocks_array);
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

	bool x = open[0] == 'Y';
	free(open);
	return x;
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

void funcion_hilo_new_pokemon(t_new_pokemon* new_pokemon,uint32_t socket){
	log_info(logger,"----------------------------------------------------------------");
	log_info(logger,"THREAD NEW POKEMON");
	log_info(logger,"Llego un NEW_POKEMON %s en (%d,%d) con cantidad %d",new_pokemon->pokemon,new_pokemon->posicion[0],new_pokemon->posicion[1],new_pokemon->cantidad);

	char* path_metafile = obtener_path_metafile(new_pokemon->pokemon);
	char* dir_path_newpoke = obtener_path_dir_pokemon(new_pokemon->pokemon);

	if (el_pokemon_esta_creado(dir_path_newpoke)){

		log_info(logger,"El pokemon %s ya existe",new_pokemon->pokemon);

		verificar_apertura_pokemon(path_metafile,new_pokemon->pokemon);

		char* key = get_key_from_position(new_pokemon->posicion);
		char* value = get_value_from_cantidad(new_pokemon->cantidad);
		char* temporary_file = generar_archivo_temporal(path_metafile,new_pokemon->pokemon);

		if (existe_la_posicion(key,temporary_file)){

			log_info(logger,"Existia la posicion %s del pokemon %s, se va a actualizar",key,new_pokemon->pokemon);
			actualizar_pokemon(temporary_file, path_metafile, key, value);
			log_info(logger,"Se actualizo la posicion %s del pokemon %s",key,new_pokemon->pokemon);

			} else{
				log_info(logger,"No existia la posicion %s del pokemon %s , se agrega",key,new_pokemon->pokemon);
				agregar_nueva_posicion(new_pokemon,path_metafile, key, value);

			}

		remove(temporary_file);

		free(key);
		free(value);
		free(temporary_file);

		}else{

		log_info(logger,"No existia el pokemon %s",new_pokemon->pokemon);

		mkdir(dir_path_newpoke, 0777);
		log_info(logger,"Se creo el directorio %s en %s",new_pokemon->pokemon,dir_path_newpoke);

		FILE* f = fopen(path_metafile,"wb");
		fclose(f);
		log_info(logger,"Se creo el archivo metafile del pokemon %s vacio",new_pokemon->pokemon);

		crear_pokemon(new_pokemon, path_metafile);
		log_warning(logger,"Se creo por completo el pokemon %s",new_pokemon->pokemon);

		}

	log_info(logger,"Esperando el tiempo de retardo de operacion");

	sleep(config_gc->tiempo_retardo_operacion);

	log_warning(logger,"UNLOCK al pokemon");

	unlock_file(path_metafile);

	t_appeared_pokemon* appeared = armar_appeared(new_pokemon);
	log_info(logger,"APPEARED Armado");

	uint32_t socket_appeared = crear_conexion(config_gc->ip_broker,config_gc->puerto_broker);

	if (socket_appeared == -1){
		log_error(logger,"El broker esta muerto");
		log_error(logger,"socket: %d",socket_appeared);
	}else {
		enviar_mensaje(APPEARED_POKEMON,appeared,socket_appeared, size_mensaje(appeared, APPEARED_POKEMON));
		log_info(logger,"Mensaje enviado");
		uint32_t id = recibir_id_de_mensaje_enviado(socket_appeared);
		log_warning(logger,"Recibo ID del mensaje enviado: %d",id);
	}


	free(path_metafile);
	free(dir_path_newpoke);

	log_error(logger,"THREAD FINISHED");

}

uint32_t recibir_id_de_mensaje_enviado(uint32_t socket_cliente) {
  uint32_t id = 0;

  recv(socket_cliente, &id, sizeof(uint32_t), MSG_WAITALL);
  log_info(logger, "El ID de mensaje enviado es: %d", id);

  close(socket_cliente);
  return id;
}

void crear_pokemon(t_new_pokemon* newPoke,char* path){

	t_file_metadata metadata = generar_file_metadata(newPoke);

	escribir_block_inicial(metadata,newPoke);
	log_info(logger,"Se escribio la posicion del pokemon en el block");
	grabar_metadata_file(metadata,path);

}

bool el_block_tiene_espacio_justo(char* key,char* value,char* ultimo_block_path){
	int size_key_y_value = strlen(key) + strlen(value) + 1;
	int blocksize_default = block_default_size();

	return (blocksize_default - file_size(ultimo_block_path)) >= size_key_y_value; //file_size = tamaño que estoy ocupando
}

bool el_block_tiene_espacio_pero_no_alcanza(char* blockpath){
	int size_block = file_size(blockpath);
	int blocksize_default = block_default_size();
	return size_block < blocksize_default;
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

char* generar_archivo_temporal(char* metapath_file,char* nombre_pokemon){

	t_config* metapath = config_create(metapath_file);
	char** blocks = config_get_array_value(metapath,"BLOCKS");
	config_destroy(metapath);

	char* blockpath;
	int cantidad_blocks = size_char_doble(blocks);
	char a = malloc(sizeof(char));
	char fin = '\n';
	char* random_path = rand_string(nombre_pokemon);
	FILE* temporary = fopen(random_path,"wb");

	int n = 0;
	for(int i = 0; i < cantidad_blocks; i++){

		blockpath = block_path(blocks[i]);
		FILE* block = fopen(blockpath,"rb");
		int filesize = file_size(blockpath);

		while (n < filesize){
			fread(&a,1,1,block);

			if (a== '\0'){
				fwrite(&fin,1,1,temporary);
			}else {
				fwrite(&a,1,1,temporary);
			}

			n++;
		}
		n=0;
		fclose(block);
	}

	fclose(temporary);

	free(blockpath);

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

	if (i < size_temporary && los_blocks_estan_llenos(blocks,cantidad_blocks)){
		log_info(logger,"No alcanzaron los blocks para grabar todo, asigno un nuevo bloque");

		char* block = asignar_nuevo_bloque(path_metafile);
		char* blockpath = block_path(block);
		FILE* blockf = fopen(blockpath,"wb");

		while (i <size_temporary){
			fread(a,1,1,temporary);
			fwrite(a,1,1,blockf);
		}
		fclose(blockf);
		free(blockpath);

	}


	fclose(temporary);
	free(blockpath);
	free(a);

}

bool los_blocks_estan_llenos(char** blocks,int cantidad_blocks){

	int cantidad_llenos;
	char* blockpath;
	for(int i = 0; i<cantidad_blocks; i++){

		blockpath = block_path(blocks[i]);

		if (file_size(blockpath) >=63){
			cantidad_llenos++;
		}

	}

	free(blockpath);

	return cantidad_llenos == cantidad_blocks;
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

void verificar_apertura_pokemon(char* path_metafile,char* nombre_pokemon){

	bool esta_en_uso = esta_lockeado(path_metafile);

	while (esta_en_uso){
		int secs = config_gc->tiempo_retardo_operacion;
		log_error(logger,"No se puede leer ni escribir este pokemon porque esta lockeado, reintentando en: %d",secs);
		sleep(secs);
		esta_en_uso = esta_lockeado(path_metafile);
	}

	log_info(logger,"Se puede acceder al pokemon %s, lockeo el archivo",nombre_pokemon);
	lock_file(path_metafile);


}

void actualizar_pokemon(char* temporary_path,char* path_metafile,char* key,char* value){

	int nuevo_size;
	int cantidad_nueva;
	int cantidad_vieja;

	t_config* temporary_config = config_create(temporary_path);

	cantidad_vieja = config_get_int_value(temporary_config,key);

	cantidad_nueva = cantidad_vieja + atoi(value);

	config_set_value(temporary_config, key, string_itoa(cantidad_nueva));

	config_save(temporary_config);
	config_destroy(temporary_config);

	re_grabar_temporary_en_blocks(temporary_path, path_metafile);

	log_info(logger,"Se actualizo la cantidad %s en el mapa, ahora hay %d",key,cantidad_nueva);

	int length_viejo = strlen(string_itoa(cantidad_vieja));
	int length_nuevo = strlen(string_itoa(cantidad_nueva));

	if (length_nuevo > length_viejo){
		nuevo_size = length_nuevo - length_viejo;
		actualizar_size_new_pokemon(path_metafile,nuevo_size);
	}


}

void actualizar_size_new_pokemon(char* pathmetafile,int nuevosize){

	int nuevo_size;

	t_config* configmeta = config_create(pathmetafile);

	int size_actual = config_get_int_value(configmeta,"SIZE");
	nuevo_size = nuevosize + size_actual;

	config_set_value(configmeta,"SIZE",string_itoa(nuevo_size));
	config_save(configmeta);
	config_destroy(configmeta);
	log_info(logger,"Se actualizo el size del pokemon, su nuevo size es :%d",nuevo_size);
}


void agregar_nueva_posicion(t_new_pokemon* newpoke,char* pathmeta_poke,char* key,char* value){

		t_config* config_poke = config_create(pathmeta_poke);
		char** blocks = config_get_array_value(config_poke,"BLOCKS");
		config_destroy(config_poke);

		int cantidad_blocks = size_char_doble(blocks);
		char* ultimo_block = blocks[cantidad_blocks-1];

		char* path_last_block = block_path(ultimo_block);

		if (el_block_tiene_espacio_justo(key, value,path_last_block)){

			escribir_data_en_block(path_last_block,key,value);

		}else if (el_block_tiene_espacio_pero_no_alcanza(path_last_block)){ // el ultimo cluster tiene espacio pero no alcanza

			char* nuevo_cluster = string_itoa(asignar_nuevo_bloque(pathmeta_poke));
			char* nuevo_block_path = block_path(nuevo_cluster);

			escribir_data_sin_fragmentacion_interna(nuevo_block_path,path_last_block,key,value);

			free(nuevo_block_path);
			free(nuevo_cluster);
		}


		int longitud = strlen(posicion_into_string(key,value))+1;
		actualizar_size_new_pokemon(pathmeta_poke,longitud);

		free(path_last_block);
		free(ultimo_block);			//TODO CAMBIE ACA ALGO
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

	char* key = get_key_from_position(newPoke->posicion);
	char* value = get_value_from_cantidad(newPoke->cantidad);

	size = strlen(posicion_into_string(key, value)) + 1 ;

	free(key);
	free(value);

	return size;

}

void escribir_block_inicial(t_file_metadata metadata,t_new_pokemon* newPoke){

	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Blocks/");
	char* blockPath = concatenar(path,string_itoa(metadata.blocks->head->data));
	char* finalPath = concatenar(blockPath,".bin");

	char* key = get_key_from_position(newPoke->posicion);
	char* value = get_value_from_cantidad(newPoke->cantidad);

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


char* get_key_from_position(uint32_t posicion[2]){

	char* pos_x = string_itoa(posicion[0]);
	char* pos_y = string_itoa(posicion[1]);
	char* key = concatenar(pos_x,"-");
	char* final_key = concatenar(key,pos_y);
	free(pos_x);
	free(pos_y);
	free(key);
	return final_key;
}

char* get_value_from_cantidad(uint32_t cantidad){
	char* value = string_itoa(cantidad);
	return value;
}

t_appeared_pokemon* armar_appeared(t_new_pokemon* new_pokemon){

	t_appeared_pokemon* appeared = malloc(sizeof(t_appeared_pokemon));
	appeared->id_mensaje = 0;
	appeared->id_mensaje_correlativo = new_pokemon->id_mensaje;
	appeared->pokemon = malloc(strlen(new_pokemon->pokemon));
	appeared->pokemon = new_pokemon->pokemon;
	appeared->posicion[0] = new_pokemon->posicion[0];
	appeared->posicion[1] = new_pokemon->posicion[1];

	return appeared;
}


/*-------------------------------------------------------------------------- CATCH-POKEMON ----------------------------------------------------------------------------- */
void funcion_hilo_catch_pokemon(t_catch_pokemon* catch_pokemon,uint32_t socket_br){
	log_info(logger,"----------------------------------------------------------------");
	log_info(logger,"THREAD CATCH POKEMON");
	log_info(logger,"LLEGO UN CATCH %s en la posicion %d-%d ",catch_pokemon->pokemon,catch_pokemon->posicion[0],catch_pokemon->posicion[1]);
	uint32_t resultado = 0;
	int estaba_creado = 0;
	char* meta_path = obtener_path_metafile(catch_pokemon->pokemon);
	char* dir_path = obtener_path_dir_pokemon(catch_pokemon->pokemon);
	char* key = get_key_from_position(catch_pokemon->posicion);
	int se_elimino= 0;

	if (el_pokemon_esta_creado(dir_path)){

		estaba_creado=1;
		verificar_apertura_pokemon(meta_path, catch_pokemon->pokemon);
		log_info(logger,"Existe el pokemon en el filesystem");

		char* temporaryfile = generar_archivo_temporal(meta_path,catch_pokemon->pokemon);

		if (existe_la_posicion(key, temporaryfile)){

			remover_posicion(temporaryfile,key,meta_path);
			re_grabar_temporary_en_blocks(temporaryfile,meta_path);
			log_info(logger,"Se retiro una cantidad en la posicion %s del pokemon %s",key,catch_pokemon->pokemon);

			verificar_espacio_en_blocks(meta_path);
			resultado = 1;

			} else{
			log_error(logger,"No existe la posicion %d - %d del pokemon %s en el mapa",catch_pokemon->posicion[0],catch_pokemon->posicion[1],catch_pokemon->pokemon);
			}

			remove(temporaryfile);
			se_elimino = verificar_espacio_ocupado_por_pokemon(catch_pokemon,meta_path,estaba_creado);

	}else{
		log_error(logger,"No se encuentra el pokemon %s creado",catch_pokemon->pokemon);
		resultado = 0;
		estaba_creado=0;
	}

	log_info(logger,"Esperando el tiempo de retardo de operacion");
	sleep(config_gc->tiempo_retardo_operacion);

	if (estaba_creado){

		if (se_elimino){
			log_info(logger,"Pokemon eliminado del filesystem");
		}else {
			log_info(logger,"UNLOCK Al pokemon");
			unlock_file(meta_path);
		}

	}

	t_caught_pokemon* caught = armar_caught_pokemon(catch_pokemon, resultado);
	log_info(logger,"CAUGHT Armado");

	uint32_t socket_caught = crear_conexion(config_gc->ip_broker,config_gc->puerto_broker);

	if (socket_caught == -1){
		log_error(logger,"El broker esta muerto");
		log_error(logger,"socket: %d",socket_caught);
	}else {
		enviar_mensaje(CAUGHT_POKEMON,caught,socket_caught, size_mensaje(caught, CAUGHT_POKEMON));
		log_info(logger,"Mensaje enviado");
		uint32_t id = recibir_id_de_mensaje_enviado(socket_caught);
		log_warning(logger,"Recibo ID del mensaje enviado: %d",id);
	}

	log_error(logger,"THREAD FINISHED");
}



void remover_posicion(char* temporarypath,char* key,char* metapath){

	int cantidad_vieja;
	int cantidad_nueva;
	int nuevo_len;
	int length_cantidad_nueva;
	int length_cantidad_vieja;

	t_config* temporaryconfig = config_create(temporarypath);

	cantidad_vieja = config_get_int_value(temporaryconfig,key);

	if (cantidad_vieja == 1){

		config_remove_key(temporaryconfig,key);
		nuevo_len = strlen(posicion_into_string(key, string_itoa(cantidad_vieja)))+1;
		actualizar_size_new_pokemon(metapath,(-nuevo_len));

	}else {

		cantidad_nueva = cantidad_vieja - 1;
		config_set_value(temporaryconfig,key,string_itoa(cantidad_nueva));
		length_cantidad_nueva = strlen(string_itoa(cantidad_nueva));
	}


	length_cantidad_vieja = strlen(string_itoa(cantidad_vieja));

	if (length_cantidad_nueva < length_cantidad_vieja){
		nuevo_len = 1;
		actualizar_size_new_pokemon(metapath,(-nuevo_len));
	}

	config_save(temporaryconfig);
	config_destroy(temporaryconfig);

}

void verificar_espacio_en_blocks(char* metapath){

	t_config* metaconfig = config_create(metapath);

	char** blocks = config_get_array_value(metaconfig,"BLOCKS");

	int block_vacio = hay_algun_block_vacio(blocks);

	if (block_vacio != -1){

		liberar_block_del_bitmap(string_itoa(block_vacio));
		liberar_block_de_la_indextable(metapath,block_vacio,metaconfig);
	}

	config_save(metaconfig);
	config_destroy(metaconfig);
	free(blocks);
}

int hay_algun_block_vacio(char** blocks){

	int cantidad_blocks = size_char_doble(blocks);
	char* blockpath;

	for(int i=0; i<cantidad_blocks; i++){

		blockpath = block_path(blocks[i]);

		int blocksize = file_size(blockpath);

		if (blocksize == 0){
			return atoi(blocks[i]);
			break;
		}


	}

	free(blockpath);
	return -1;

}

void liberar_block_del_bitmap(char* numero_block){
	t_bitarray* bitarray = obtener_bitmap();

	int numero = atoi(numero_block);

	bitarray_clean_bit(bitarray,numero);

	actualizar_bitmap(bitarray);
	log_info(logger,"Este pokemon libero el block %s porque habia uno solo en esa posicion",numero_block);
}

void liberar_block_de_la_indextable(char* metapath,char* block_vacio,t_config* metaconfig){

	char** blocks = config_get_array_value(metaconfig,"BLOCKS");

	int posicion_bloque_vacio = posicion_block_vacio(blocks,block_vacio);

	t_list* list_blocks = chardoble_to_tlist(blocks);

	list_remove(list_blocks,posicion_bloque_vacio);

	char* nueva_tabla_indices = list_to_string_array(list_blocks);

	config_set_value(metaconfig,"BLOCKS",nueva_tabla_indices);

	log_info(logger,"Se retiro el block de la indextable del pokemon");
	free(blocks);
	list_destroy(list_blocks);

}


int posicion_block_vacio(char** blocks, char* block_vacio){

	int cantidad_blocks = size_char_doble(blocks);

	for(int i=0; i<cantidad_blocks; i++){

		if (atoi(blocks[i]) == block_vacio){
			return i;
			break;
		}

	}
	return -1;

}

int verificar_espacio_ocupado_por_pokemon(t_catch_pokemon* catch_pokemon,char* meta_path,int creado){

	char* poke_dir = obtener_path_dir_pokemon(catch_pokemon->pokemon);

	t_config* metaconfig = config_create(meta_path);

	char** blocks = config_get_array_value(metaconfig,"BLOCKS");

	config_destroy(metaconfig);

	if (size_char_doble(blocks) == 0 ){ // no tiene clusters este loquito
		log_warning(logger,"Este catch hizo que el pokemon no use mas clusters, por lo tanto se elimina del filesystem");
		remove(meta_path);
		rmdir(poke_dir);
		return 1;
	}

	free(poke_dir);
	free(blocks);
	return 0;
}

t_caught_pokemon* armar_caught_pokemon(t_catch_pokemon* catch_pokemon,uint32_t resultado){

	t_caught_pokemon* caught_pokemon = malloc(sizeof(t_caught_pokemon));
	caught_pokemon->resultado = resultado;
	caught_pokemon->id_mensaje_correlativo = catch_pokemon->id_mensaje;
	caught_pokemon->id_mensaje = 0;

	return caught_pokemon;
}


/*-------------------------------------------------------------------------- GET-POKEMON ----------------------------------------------------------------------------- */

void funcion_hilo_get_pokemon(t_get_pokemon* get_pokemon,uint32_t socket_br){
	log_info(logger,"----------------------------------------------------------------");
	log_error(logger,"THREAD GET_POKEMON");
	log_info(logger,"Llego un GET_POKEMON de %s",get_pokemon->pokemon);

	char* dir_path = obtener_path_dir_pokemon(get_pokemon->pokemon);
	char* meta_path = obtener_path_metafile(get_pokemon->pokemon);
	char* temporary_file;

	bool estaba = 0;

	t_localized_pokemon* localized_pokemon = malloc(sizeof(t_localized_pokemon));

	uint32_t socket_localized = crear_conexion(config_gc->ip_broker,config_gc->puerto_broker);

	if (socket_localized != -1 ){
		localized_pokemon->id_mensaje = 0;
	    localized_pokemon->id_mensaje_correlativo = get_pokemon->id_mensaje;
		//localized_pokemon->pokemon = malloc(strlen(get_pokemon -> pokemon));
		localized_pokemon->pokemon = get_pokemon->pokemon;

		 if (el_pokemon_esta_creado(dir_path)){
			 log_info(logger,"El pokemon %s existe en el filesystem",get_pokemon->pokemon);

		 	verificar_apertura_pokemon(meta_path, get_pokemon->pokemon);

		 	temporary_file = generar_archivo_temporal(meta_path,get_pokemon->pokemon);

		 	localized_pokemon->posiciones = obtener_posiciones_y_cantidades(meta_path,temporary_file);
		 	localized_pokemon->tamanio_lista = localized_pokemon->posiciones->elements_count;

		 	estaba=1;
		 	 }else{
		 		 log_warning(logger,"El pokemon %s no existe en el filesystem, te mando la lista vacia",get_pokemon->pokemon);
		 		 localized_pokemon->tamanio_lista = 0;
		 		 localized_pokemon->posiciones = list_create();
		 	 }
		//log_info(logger,"LOCALIZED Armado"); TODO NUNCA PONER NADA ACA AMIGUITO


		enviar_mensaje_localized_gc(LOCALIZED_POKEMON,localized_pokemon,socket_localized, size_mensaje(localized_pokemon, LOCALIZED_POKEMON));
		uint32_t id = recibir_id_de_mensaje_enviado(socket_localized);

		log_warning(logger,"Mensaje enviado, ID del mensaje enviado: %d",id);
	 }else {
		 log_error(logger,"El broker esta muerto");
		 log_error(logger,"Socket: %d",socket_localized);

	 }

	 log_info(logger,"Esperando el tiempo de reintento de operacion");
	 sleep(config_gc->tiempo_retardo_operacion);

	 if (estaba==1){
		 unlock_file(meta_path);
		 log_info("THREAD finished, unlockeo el pokemon %s",localized_pokemon->pokemon);
		 remove(temporary_file);
	 } else {
		 log_info(logger,"THREAD finished, el pokemon no existia en el filesystem");
	 }




	 free(dir_path);
	 free(meta_path);
}

t_list* obtener_posiciones_y_cantidades(char* meta_path,char* temporary_file){

	t_list* lista_de_todo = list_create();

	llenar_lista(lista_de_todo,temporary_file);

	return lista_de_todo;
}

void llenar_lista(t_list* lista_de_todo, char* temporary_file){

	t_config* temporary_config = config_create(temporary_file);
	int keys_amount = config_keys_amount(temporary_config);
	config_destroy(temporary_config);
	FILE* temporary = fopen(temporary_file,"rb");

	for(int i=0; i<keys_amount; i++){

		char* linea = leer_linea(temporary);
		char* value = get_value_from_linea(linea);
		log_info(logger,"%s",value);
		for (int i=0 ; i<atoi(value); i++){
			char* x = get_x_from_linea(linea);
			char* y = get_y_from_linea(linea);
			uint32_t xx =  atoi(x);
			uint32_t yy = atoi(y);
			list_add(lista_de_todo,&xx);
			list_add(lista_de_todo,&yy);

		}
		//free(linea);
	}



	/*uint32_t* resultadodecuuenta = malloc(sizeof(uint32_t));
	resultadodecuuenta = (lista_de_todo->elements_count )/2;
	list_add_in_index(lista_de_todo,0,resultadodecuuenta);*/

	fclose(temporary);
}

char* leer_linea(FILE* archivo){

	int i = 0;

	char* linea = string_new();
	char a;

	fread(&a,1,1,archivo);

	while (a!= '\n'){
		appendy(linea,a);
		fread(&a,1,1,archivo);
		i++;
	}
	//free(a);

	return linea;
}

void appendy(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

char* get_value_from_linea(char* linea){
	char** lista = string_split(linea,"=");
	char* variable = malloc(strlen(lista[1])+1);
	variable = lista[1];
	return variable;
}


char* get_x_from_linea(char* linea){

	char** lista = string_split(linea,"-");
	char* variable = malloc(strlen(lista[0])+1);
	variable = lista[0];
	return variable;
}


char* get_y_from_linea(char* linea){

	char** lista_entera = string_split(linea,"=");
	char** lista_key_value = string_split(lista_entera[0],"-");
	char* variable = malloc(strlen(lista_key_value[1])+1);
	variable = lista_key_value[1];
	return variable;

}
/*-------------------------------------------------------------------------- TOOLS ----------------------------------------------------------------------------- */

int block_default_size(){

	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Metadata.bin");

	t_config* configmeta = config_create(path);

	int blocks = config_get_int_value(configmeta,"BLOCKS_SIZE");

	config_destroy(configmeta);

	free(path);
	return blocks;
}
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

char* list_to_string_array(t_list* lista){

	char* format = "[";

	if (lista->elements_count == 1){

		format = concatenar(format,string_itoa(lista->head->data));

	} else if (lista->elements_count == 0){

		log_info(logger,"La tabla de indices del pokemon solamente tenia un cluster, ahora queda vacia");

	} else {

			while (lista->head->next != NULL){
				format = concatenar(format,string_itoa(lista->head->data));
				format = concatenar(format,",");
				lista->head = lista->head->next;

					if (lista->head->next == NULL){
						format=concatenar(format,string_itoa(lista->head->data));
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

char* posicion_into_string(char* key,char*value){
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

char* obtener_path_metafile(char* nombre_pokemon){
	punto_montaje = config_gc->punto_montaje_tallgrass;
	char* path  = concatenar (punto_montaje,"/Files/");
	char* path2 = concatenar (path,nombre_pokemon); //terrible negrada esto, pero anda o no anda?
	char* path3 = concatenar(path2,"/Metadata.bin");
	free(path);
	free(path2);
	return path3;
}

char* obtener_path_dir_pokemon(char* nombre_pokemon){
	punto_montaje = config_gc->punto_montaje_tallgrass;
	char* path = concatenar(punto_montaje,"/Files/");
	char* path2 = concatenar(path,nombre_pokemon);
	free(path);
	return path2;
}

uint32_t sizeNewPokemon(t_new_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon->pokemon);
}


uint32_t sizeAppearedPokemon(t_appeared_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon->pokemon);
}

int size_char_doble(char** array){

	int i = 0;
	int size = 0;
	int a;
	while(array[i] != NULL){
		a=array[i];
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

char* rand_string(char* nombre_pokemon) {

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
    char* rand = concatenar(randomString,nombre_pokemon);
    char* random_string = concatenar(rand,".bin");

    free(randomString);
    free(rand);

    return random_string;
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



