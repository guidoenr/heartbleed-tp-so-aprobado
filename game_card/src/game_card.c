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
	simple->cantidad= 1;
	simple->id_mensaje= 124;
	simple->pokemon= "Simple";
	simple->posicion[0]= 16;
	simple->posicion[1] = 51;

	funcion_hilo_new_pokemon(simple, socket_br);

	laboratorio_de_pruebas();


	terminar_programa(socket,config_gc);
}

//punto_montaje = /home/utnso/workspace/tp-2020-1c-heartbleed/game_card/Montaje


void laboratorio_de_pruebas(){

}


void conectarse(int socket){
	socket = crear_conexion(config_gc -> ip_broker, config_gc -> puerto_broker);
	int time = config_gc->tiempo_reintento_conexion;
	if (socket == -1 ){
		log_info(logger,"imposible conectar con broker, reintento en: %d",time);
		sleep(time);
		socket=0;
		conectarse(socket); //terrible negrada, pero anda o no nada?
	} else {
		log_info(logger,"conexion exitosa con broker ");
	}
}

int file_current_size(FILE* f){

	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Metadata.bin");
	t_metadata metadataFS = leer_fs_metadata(path);

	int pos = ftell(f);

	return metadataFS.blocksize - pos;

}

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
	char* a = string_repeat('0',(size_in_bytes*8));

	FILE* file = fopen(bitmapPath,"wb");
	fclose(file);

	t_config* config_bitmap = config_create(bitmapPath);

	config_set_value(config_bitmap,"BITARRAY",a);

	int result = config_save(config_bitmap);
	if (result != 1 ){
		log_info(logger,"Error en la creacion de bitarray al estilo tall_gd3_grass");
	}

	config_destroy(config_bitmap);

	free(bitmapPath);
	free(metadataPath);

	log_info(logger,"Se creo el bitarray de %d posiciones, con un tamaño de %d bytes",fs_metadata.blocks,size_in_bytes);

}

void create_file_with_size(char* path,int size) {

        int X = size - 1 ;
        FILE *fp = fopen(path, "wb");
        fseek(fp, X , SEEK_SET);
        fputc('\0', fp);

        fclose(fp);

}



void crear_pokemon(t_new_pokemon* newPoke){

	char* meta_file_path = obtener_path_metafile(newPoke);

	t_file_metadata metadata = generar_file_metadata(newPoke);

	escribir_block_inicial(metadata,newPoke);

	char* blocks_array = list_to_string_array(metadata.blocks);

	t_config* meta_file_pokemon = config_create(meta_file_path);

	config_set_value(meta_file_pokemon,"OPEN",metadata.open);
	config_set_value(meta_file_pokemon,"DIRECTORY",metadata.directory);
	config_set_value(meta_file_pokemon,"SIZE",metadata.size);
	config_set_value(meta_file_pokemon,"BLOCKS",blocks_array);

	config_save_in_file(meta_file_pokemon,meta_file_path);

	free(blocks_array);
	free(meta_file_path);
	config_destroy(meta_file_pokemon);

}

void append(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
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


t_file_metadata generar_file_metadata(t_new_pokemon* newPoke){

	t_file_metadata metadata;

	metadata.directory = "N";
	metadata.open = "N";
	metadata.blocks = asignar_block_inicial();
	metadata.size = string_itoa(calcular_size_inicial(newPoke));

	return metadata;

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

	char* new_poke_key = get_key_from_position(newPoke);
	char* new_poke_value = get_value_from_position(newPoke);

	t_config* block = config_create(finalPath);

	config_set_value(block,new_poke_key,new_poke_value);

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


t_list* asignar_block_inicial(){

	t_list* lista = list_create();

	int block = buscar_block_libre();

	list_add(lista,block);

	return lista;
}

t_file_metadata leer_file_metadata(char* path){

	t_file_metadata fmetadata_leido;



	// TODO


	return fmetadata_leido;
}

char* buscar_block_libre(){ //TODO, EL BITARRAY EN UN FILE? PARA QUE

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



int fileSize(char* path){
	FILE* f = fopen(path,"r");
	fseek(f, 0L, SEEK_END);
	int i = ftell(f);
	fclose(f);
	return i;
}

bool archivoVacio(char* path){
	return fileSize(path) == 0;
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

// 1 - 2 = 16;

int default_bitarray_sizebytes(){
	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Metadata.bin");
	t_metadata metadata = leer_fs_metadata(path);
	return (metadata.blocks/ 8);
}



t_bitarray* obtener_bitmap(){

	char* bitmapPath = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Bitmap.bin");
	int size_in_bytes = bitarray_default_size();

	t_config* bitarray_config = config_create(bitmapPath);

	char* data = config_get_string_value(bitarray_config,"BITARRAY");

	config_destroy(bitarray_config);

	t_bitarray* bitarray = bitarray_create_with_mode(data, size_in_bytes, LSB_FIRST);

	return bitarray;

}

void actualizar_bitmap(t_bitarray* bitarray){
	char* bitmapPath = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Bitmap.bin");

	t_config* bitmap_config = config_create(bitmapPath);

	bitarray_set_bit(bitarray,53);
	bitarray_set_bit(bitarray,521);

	bool x = bitarray_test_bit(bitarray,53);
	bool y =bitarray_test_bit(bitarray,521);
	bool z = bitarray_test_bit(bitarray,524);
	bool zed = bitarray_test_bit(bitarray,24);
	bool zdqw = bitarray_test_bit(bitarray,4);


	config_set_value(bitmap_config,"BITARRAY",bitarray->bitarray);

	int status = config_save(bitmap_config);

	config_destroy(bitmap_config);
	bitarray_destroy(bitarray);
	free(bitmapPath);

}

int bitarray_default_size(){
	char* path = concatenar(config_gc->punto_montaje_tallgrass,"/Metadata/Metadata.bin");
	t_metadata metadata = leer_fs_metadata(path);
	return (metadata.blocks/8);

}


bool isDirectory(char* path){
	t_file_metadata fileMeta;
	FILE* file = fopen(path,"rb");
	fread(&fileMeta.directory,sizeof(char),1,file);
	fclose(file);

	return fileMeta.directory == 'Y';
}

bool is_open(char* path){

	t_config* file_meta_config = config_create(path);

	char* open = strdup(config_get_string_value(file_meta_config,"OPEN"));

	config_destroy(file_meta_config);

	return open == "Y";
}

void lock_file(char* path){

	t_config* file_config = config_create(path);

	config_set_value(file_config,"OPEN","Y");

	config_save(path);

	config_destroy(file_config);

}

void unlock_file(char* path){

	t_config* file_config = config_create(path);

	config_set_value(file_config,"OPEN","N");

	config_save(path);

	config_destroy(file_config);
}

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

void funcion_hilo_new_pokemon(t_new_pokemon* new_pokemon,int socket){

	log_info(logger,"Llego un NEW_POKEMON %s en (%d,%d) con cantidad %d",new_pokemon->pokemon,new_pokemon->posicion[0],new_pokemon->posicion[1],new_pokemon->cantidad);

	verificar_existencia_pokemon(new_pokemon);
	verificar_apertura_pokemon(new_pokemon,socket);

	if (existePokemonEnPosicion(new_pokemon)){
		//agregarAPosicionExistente(new_pokemon);

		log_info(logger,"Existe pokemon ");

		//agregarAPoisiconExistente(); TODO PARA BLOCKS Y FILESYTEM
	} else {
		log_info(logger,"No existe pokemon \n");

		//agregarAlFinalDelArchivo(); TODO PARA BLOCKS Y FILESYTEM
	}

	t_appeared_pokemon* appeared_pokemon = armar_appeared(new_pokemon);

//
//	enviar_appeared_pokemon(appeared_pokemon,socket);
//	free(appeared_pokemon->id_mensaje);
//	free(appeared_pokemon->id_mensaje_correlativo);
//	free(appeared_pokemon->pokemon);
//	free(appeared_pokemon->posicion);

	char* path = obtener_path_metafile(new_pokemon);
	unlock_file(path);

}

t_appeared_pokemon* armar_appeared(t_new_pokemon* new_pokemon){

		t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));

		appeared_pokemon->posicion[0] = new_pokemon->posicion[0];
		appeared_pokemon->posicion[1] = new_pokemon->posicion[1];
		appeared_pokemon->pokemon = new_pokemon->pokemon;
		appeared_pokemon->id_mensaje_correlativo = 0;
		appeared_pokemon->id_mensaje = new_pokemon->id_mensaje;
		return appeared_pokemon;
}


void verificar_existencia_pokemon(t_new_pokemon* newpoke){
	punto_montaje = config_gc->punto_montaje_tallgrass;

	char* montaje = concatenar(punto_montaje,"/Files/");
	char* path = concatenar(montaje,newpoke->pokemon);
	if (existe_directorio(path)){
		log_info(logger,"Existe el pokemon %s en : %s y se le van a agregar posiciones",newpoke->pokemon,path);

	} else{

		mkdir(path, 0777);
		log_info(logger,"Se creo el directorio %s en %s",newpoke->pokemon,path);

		FILE* f = fopen(obtener_path_metafile(newpoke),"wb");
		fclose(f);

		log_info(logger,"Se creo el file_metadata de %s vacio",newpoke->pokemon);


		crear_pokemon(newpoke);
		log_info(logger,"Se creo por completo el pokemon %s ",newpoke->pokemon);

	}

}


int existe_directorio(char* path){
	DIR* dir = opendir(path);
	int x = dir;
	closedir(dir);
	return x;
}

void verificar_apertura_pokemon(t_new_pokemon* newpoke,int socket){

	char* path = obtener_path_metafile(newpoke);
	bool x = is_open(path);

	if (is_open(path)){
		int secs = config_gc->tiempo_retardo_operacion;
		log_info(logger,"No se puede acceder a este pokemon porque esta en uso (OPEN=Y), reintentando en: %d",secs);
		sleep(secs);
		funcion_hilo_new_pokemon(newpoke,socket);

	} else {

		log_info(logger,"Se puede acceder al pokemon, lockeo el archivo %s",newpoke->pokemon);
		lock_file(path);

		}
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

bool existePokemonEnPosicion(t_new_pokemon* pokemon){
	char* meta_file_path = obtener_path_metafile(pokemon);

}

//void enviar_new_pokemon(t_new_pokemon* pokemon, uint32_t socket_cliente) {
//	t_buffer* buffer = malloc(sizeof(t_buffer));
//
//	buffer -> size = sizeNewPokemon(pokemon);
//	buffer -> stream = malloc(buffer -> size);
//	buffer -> stream = pokemon;
//	log_info(logger,"Armando paquete New_Pokemon");
//	//TAMBIEN HAY QUE LLENAR EL CAMPO DE ID DEL PAQUETE
//	t_paquete* paquete = malloc(sizeof(t_paquete));
//	paquete -> codigo_operacion = NEW_POKEMON;
//	paquete -> buffer = buffer;
//	//HABRIA QUE DARLE UN VALOR AL SIZE SERIALIZADO
//	uint32_t size_serializado;
//	void* stream = serializar_paquete(paquete, &size_serializado);
//	log_info(logger,"Paquete serializado con tamaño :%d",size_serializado);
//	//EN VEZ DE SERIALIZARLO ACA PODES USAR LA FUNCION ENVIAR_MENSAJE
//	//DEL UTILS QUE YA TE LO SERIALIZA ADENTRO.
//	send(socket_cliente, stream, size_serializado, 0);
//	log_info(logger,"Paquete enviado");
//	//free(buffer -> stream);
//	free(buffer);
//	free(paquete);
//	free(stream);
//}
//
//void enviar_appeared_pokemon(t_appeared_pokemon* appeared_pokemon,int socket){
//		t_buffer* buffer = malloc(sizeof(t_buffer));
//
//		buffer -> size = sizeAppearedPokemon(appeared_pokemon);
//		buffer -> stream = malloc(buffer -> size);
//		buffer -> stream = appeared_pokemon;
//		log_info(logger,"Armando paquete APPEARED_POKEMON");
//
//		t_paquete* paquete = malloc(sizeof(t_paquete));
//		paquete -> codigo_operacion = APPEARED_POKEMON;
//		paquete -> buffer = buffer;
//
//		uint32_t size_serializado;
//		void* stream = serializar_paquete(paquete, &size_serializado);
//		log_info(logger,"Paquete serializado con tamaño :%d",size_serializado);
//		send(socket, stream, size_serializado, 0);
//		log_info(logger,"Paquete enviado");
//		//free(buffer -> stream);
//		free(buffer);
//		free(paquete);
//		free(stream);
//}
//
//t_new_pokemon* recibir_new_pokemon(uint32_t socket_cliente, uint32_t* size){
//		log_info(logger, "recibiendo new_pokemon");
//		recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
//		log_info(logger, "tamaño new_pokemon: %d", *size);
//		void* buffer = malloc(*size);
//		recv(socket_cliente, buffer, *size, MSG_WAITALL);
//		log_info(logger, "mensaje recibido: %s", buffer);
//
//		t_new_pokemon* pokemon = deserealizar_new_pokemon(buffer);
//		return pokemon;
//	}
//
//t_appeared_pokemon* recibir_appeared_pokemon(uint32_t socket_cliente, uint32_t* size){
//		log_info(logger, "recibiendo appeared_pokemon");
//		recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
//		log_info(logger, "tamaño appeared_pokemon: %d", *size);
//		void* buffer = malloc(*size);
//		recv(socket_cliente, buffer, *size, MSG_WAITALL);
//		log_info(logger, "mensaje recibido: %s", buffer);
//
//		t_new_pokemon* pokemon = deserealizar_appeared_pokemon(buffer);
//		return pokemon;
//	}

uint32_t sizeNewPokemon(t_new_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon->pokemon) + 1;
}


uint32_t sizeAppearedPokemon(t_appeared_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon->pokemon) + 1;
}



