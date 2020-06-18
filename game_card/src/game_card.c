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
	config = leer_config();
	iniciar_logger("gameCard.log","gamercard");

	punto_montaje = config->punto_montaje_tallgrass;
	int socket_br;

	//conectarse(socket_br);
	iniciarTallGrass();

//	t_new_pokemon* simple = malloc(sizeof(t_new_pokemon));
//	simple->cantidad= 1;
//	simple->id_mensaje= 124;
//	simple->pokemon= "Simple";
//	simple->posicion[0]= 16;
//	simple->posicion[1] = 51;
//
//	t_new_pokemon* kennyS = malloc(sizeof(t_new_pokemon));
//	kennyS->cantidad= 1;
//	kennyS->id_mensaje= 124;
//	kennyS->pokemon= "KennyS";
//	kennyS->posicion[0]= 16;
//	kennyS->posicion[1] = 51;
//
//	free(kennyS)

	//funcionHiloNewPokemon(kennyS, socket_br);
	laboratorio_de_pruebas();


	terminar_programa(socket,config);
}

//punto_montaje = /home/utnso/workspace/tp-2020-1c-heartbleed/game_card/Montaje


void laboratorio_de_pruebas(){

	char* path = concatenar(config->punto_montaje_tallgrass,"/PRUEBA.bin");

	FILE* f = fopen(path,"wb");
	fclose(f);

	t_config* metadata_config = config_create(path);

	config_set_value(metadata_config,"holaasdas","chau");

	int result = config_save_in_file(metadata_config,path);

}


int file_current_size(FILE* f){

	char* path = concatenar(config->punto_montaje_tallgrass,"/Metadata/Metadata.bin");
	t_metadata metadataFS = leer_fs_metadata(path);

	int pos = ftell(f);

	return metadataFS.blocksize - pos;

}

void conectarse(int socket){
	socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	int time = config->tiempo_reintento_conexion;
	if (socket == -1 ){
		log_info(logger,"imposible conectar con broker, reintento en: %d",time);
		sleep(time);
		socket=0;
		conectarse(socket); //terrible negrada, pero anda o no nada?
	} else {
		log_info(logger,"conexion exitosa con broker ");
	}
}

void iniciarTallGrass(){

	punto_montaje = config->punto_montaje_tallgrass;
	log_info(logger,"Iniciando tallgrass en: %s",punto_montaje);

	if (!isDir(punto_montaje)){

		crear_directorios(punto_montaje);
		crearMetadata(punto_montaje);
		crear_bitmap(punto_montaje);
		crear_blocks(punto_montaje);

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
	uint32_t socket 			      = crear_conexion(config -> ip_broker, config -> puerto_broker);
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

	t_config_game_card* config = config_create("Debug/game_card.config");

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

void liberar_config(t_config_game_card* config) {
	free(config -> punto_montaje_tallgrass);
	free(config -> ip_broker);
	free(config -> puerto_broker);
	free(config);
}


void terminar_programa(int conexion,t_config_game_card* config) {
	liberar_config(config);
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
		createFileWithSize(bloque,sizeBlock);
		i++;
	}
	log_info(logger,"Se crearon %d blocks de tamaño: %d bytes",cantidadBloques -1 ,sizeBlock);

}

void createFileWithSize(char* path,int size) {

        int X = size - 1 ;

        FILE *fp = fopen(path, "wb");
        fseek(fp, X , SEEK_SET);
        fputc('\0', fp);

        fclose(fp);

}

void crearMetadata(char* path){

	char* realPath = concatenar(path,"/Metadata/Metadata.bin");
//
//	FILE* f = fopen(realPath,"w-b");
//	fclose(f);

	t_config* metadata_config = malloc(sizeof(t_config));

	metadata_config -> path = realPath;
	metadata_config -> properties = dictionary_create();

	dictionary_put(metadata_config -> properties,"BLOCKS",5192);
	dictionary_put(metadata_config -> properties,"BLOCKS_SIZE",64);
	dictionary_put(metadata_config -> properties,"MAGIC_NUMBER","TALL_GRASS");

	int a = config_save(metadata_config);

	config_destroy(metadata_config);

	log_info(logger,"Se creo Metadata.bin en: %s",realPath);
	free(realPath);

}

void escribir_campo_blocks_metadata(FILE* f,char* blocks){
	char* a_escribir = concatenar("BLOCKS=",blocks);
	int tam = strlen(a_escribir) + 1;
	fwrite(a_escribir,tam,1,f);
	free(a_escribir);
}

void escribir_campo_blocksize_metadata(FILE* f,char* blocksize){
	char* a_escribir = concatenar("BLOCKS_SIZE=",blocksize);
	int tam = strlen(a_escribir) + 1;
	fwrite(a_escribir,tam,1,f);
	free(a_escribir);
}

void escribir_campo_magic_metadata(FILE* f,char* magic){
	char* a_escribir = concatenar("MAGIC_NUMBER=",magic);
	int tam = strlen(a_escribir) + 1;
	fwrite(a_escribir,tam,1,f);
	free(a_escribir);

}

void crearPokemon(t_new_pokemon* newPoke){

	char* metaPath = obtenerPathMetaFile(newPoke);
	char* dirPokemon = obtenerPathDirPokemon(newPoke);

	t_file_metadata metadata = generar_file_metadata(newPoke);

	escribir_block_inicial(metadata,newPoke);

	FILE* file = fopen(metaPath,"wb");

//	typedef struct{
//		char directory;
//		int size;
//		t_list* blocks;
//		char open;
//	}t_file_metadata;

	escribir_campo_open(file,metadata.open);
	escribir_campo_directory(file,metadata.directory);
	escribir_campo_size(file,metadata.size);
	escribir_campo_blocks(file,metadata.blocks);

	fclose(file);

	unlock_file(metaPath);

}

void append(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

void escribir_campo_open(FILE* f,char* metadata_open){

	char* a_escribir = concatenar("OPEN=",metadata_open);
	fwrite(a_escribir,strlen(a_escribir)+1,1,f);
	free(a_escribir);
}

void escribir_campo_directory(FILE* f,char* metadata_directory){

	char* a_escribir = concatenar("DIRECTORY=",metadata_directory);
	int tam = strlen("DIRECTORY=") + strlen(metadata_directory) + 1;
	fwrite(a_escribir,tam,1,f);
	free(a_escribir);
}

void escribir_campo_size(FILE* f,char* metadata_size){
	char* a_escribir = concatenar("SIZE=",metadata_size);
	int tam = strlen("SIZE=") + strlen(metadata_size)+1;
	fwrite(a_escribir,tam,1,f);
	free(a_escribir);
}

void escribir_campo_blocks(FILE* f ,t_list* blocks){
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

	int size = strlen(format) + 1 ;

	fwrite(format,size,1,f);
	free(format);
}


t_file_metadata generar_file_metadata(t_new_pokemon* newPoke){

	t_file_metadata metadata;
	metadata.directory = "N";
	metadata.open = "N";
	metadata.blocks = asignar_block_inicial();
	metadata.size = string_itoa(calcular_size_inicial(newPoke)); //TODO que es este size?
	return metadata;
}

int calcular_size_inicial(t_new_pokemon* newPoke){
	return strlen(posicion_into_string(newPoke)) + 1;
}

void escribir_block_inicial(t_file_metadata metadata,t_new_pokemon* newPoke){

	char* path = concatenar(config->punto_montaje_tallgrass,"/Blocks/");
	char* blockPath = concatenar(path,string_itoa(metadata.blocks->head->data));
	char* finalPath = concatenar(blockPath,".bin");

	FILE* block = fopen(finalPath,"wb");

	char* a_escribir = posicion_into_string(newPoke);
	int size = strlen(a_escribir) + 1;
	int i = 1;

	while (file_current_size(block) >=0 && i<= size){
			fwrite(&a_escribir[i],1,1,block);
			i++;
		}
	free(path);
	free(blockPath);
	free(finalPath);
}


char* posicion_into_string(t_new_pokemon* newpoke){

	char* a = concatenar(string_itoa(newpoke->posicion[0]),"-");
	char* b = concatenar(a,string_itoa(newpoke->posicion[1]));
	char* c = concatenar(b,"=");
	char* d = concatenar(c,string_itoa(newpoke->cantidad));
	return d;
}
//tengo 64 bytes para escribir en un block
//un char pesa 1 byte

t_list* asignar_block_inicial(){

	t_list* lista = list_create();

	int block = buscarBlockLibre();

	list_add(lista,block);

	return lista;
}

t_file_metadata leer_file_metadata(char* path){

	t_file_metadata fmetadata_leido;
	FILE* file = fopen(path,"rb");


	fread(&fmetadata_leido.open,sizeof(char),1,file);
	fread(&fmetadata_leido.directory,sizeof(char),1,file);
	fread(&fmetadata_leido.size,sizeof(int),1,file);
	fread(&fmetadata_leido.blocks,4,1,file);

	// TODO

	fclose(file);
	return fmetadata_leido;
}

char* buscarBlockLibre(){ //TODO, EL BITARRAY EN UN FILE? PARA QUE

	//TODO ACA ESTA HARDCODEADO EL BITARRAY JUSTO POR ESO, NO LO PUEDO LEER DEL ARCHIVO
	//TODO leer desde el Bitmap.bin.

	int blocks = 5192;
	int size_in_bytes = 8 / blocks;
	char* bytes = calloc(size_in_bytes, sizeof(char));
	t_bitarray* bitarray = bitarray_create_with_mode(bytes, size_in_bytes, LSB_FIRST);

	int blockLibre;


	for(int i=0; i<= 5192 ; i++){

		if (bitarray_test_bit(bitarray,i) != true){

			blockLibre = i + 1;				//porque el array arranca en 0
			log_info(logger,"El block %d anda masa, usalo tranka nomas",i+1);
			break;
		} else {
				log_info(logger,"El block %d esta ocupado kpo, sigo buscando",i+1);
		}
	}

	//TODO guardar y grabar el bitarray en el Bitmap.bin
	return blockLibre;

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

	t_metadata metadata;
	FILE* file = fopen(path,"rb");

	if (!isFile(path)){

		log_warning(logger,"No existe el archivo a leer");

	} else {




		// TODO




	}

	fclose(file);

	return metadata;
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

	char* path = concatenar(config->punto_montaje_tallgrass,"/Metadata/Metadata.bin");
	t_metadata metadata = leer_fs_metadata(path);
	int blocks = atoi(metadata.blocks);
	return blocks/ 8;
}

int crear_bitmap(char* path){

	char* bitmapPath = concatenar(path,"/Metadata/Bitmap.bin");
	char* metadataPath = concatenar(path,"/Metadata/Metadata.bin");
	t_metadata fs_metadata = leer_fs_metadata(metadataPath);

	int blocks = atoi(fs_metadata.blocks);

	int size_in_bytes = (blocks/8);
	int size_in_bits = blocks;
	int tam = blocks;

	char* bytes = calloc(size_in_bytes, sizeof(char)); // esto me da [0,0,0,0,0,0...n]

	t_bitarray* bitarray = bitarray_create_with_mode(bytes, size_in_bytes, LSB_FIRST);

	log_info(logger,"Se creo el bitarray de %d posiciones, con un tamaño de %d bytes",blocks,size_in_bytes);


	FILE* file = fopen(bitmapPath,"wb");

	char* a_escribir = bitarray->bitarray;

	fwrite(a_escribir,size_in_bytes,1,file);


	bitarray_destroy(bitarray);
	fclose(file);
	free(bytes);
	return tam;

}

t_bitarray* obtener_bitmap(int size){

	char* path = concatenar(config->punto_montaje_tallgrass,"/Metadata/Bitmap.bin");
	char* a_leer;

	int size_in_bytes = default_bitarray_sizebytes();
	FILE* file = fopen(path,"wb");

	fread(&a_leer,size_in_bytes,1,file);



	t_bitarray* bitarray = bitarray_create_with_mode(a_leer, (8/5192), LSB_FIRST);


	fclose(file);
	return bitarray;

}

void mostrarBitarray(int tam){
	t_bitarray* bitarray = obtener_bitmap(tam);

	for(int i=1;i<bitarray->size;i++){
		bool x = bitarray_test_bit(bitarray,i);
		printf("bit: %b",x);
	}
	bitarray_destroy(bitarray);
}

int bitarray_default_size(){
	t_metadata metadata = leer_fs_metadata(concatenar(config->punto_montaje_tallgrass,"/Metadata/Metadata.bin"));
	int blockks = atoi(metadata.blocks);
	return (8/blockks);
}

bool isDirectory(char* path){
	t_file_metadata fileMeta;
	FILE* file = fopen(path,"rb");
	fread(&fileMeta.directory,sizeof(char),1,file);
	fclose(file);

	return fileMeta.directory == 'Y';
}

bool isOpen(char* path){

	char a;
	char* total;
	FILE* file = fopen(path,"rb");

	fread(&a,1,1,file);
	while (a != "/0"){
		total = concatenar(total,a);
	}


	fclose(file);
	return true;
}

void lock_file(char* path){
	FILE* update_file = fopen(path,"r+b");

	fseek(update_file,0,SEEK_SET);
	fputs("OPEN=Y",update_file);
	fclose(update_file);
}

void unlock_file(char* path){
	FILE* update_file = fopen(path,"r+b");

	fseek(update_file,0,SEEK_SET);
	fputs("OPEN=N",update_file);
	fclose(update_file);
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
			funcionHiloNewPokemon(msg,cliente_fd);

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

void funcionHiloNewPokemon(t_new_pokemon* new_pokemon,int socket){

	verificarExistenciaPokemon(new_pokemon);
	verificarAperturaPokemon(new_pokemon,socket);

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

	char* path = obtenerPathMetaFile(new_pokemon);
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


void verificarExistenciaPokemon(t_new_pokemon* newpoke){
	punto_montaje = config->punto_montaje_tallgrass;

	char* montaje = concatenar(punto_montaje,"/Files/");
	char* path = concatenar(montaje,newpoke->pokemon);
	if (existeDirectorio(path)){

		log_info(logger,"Existe el pokemon %s en : %s y se le van a agregar posiciones",newpoke->pokemon,path);


	} else{

		mkdir(path, 0777);
		log_info(logger,"Se creo el directorio %s en %s",newpoke->pokemon,path);

		crearPokemon(newpoke);
		log_info(logger,"Se creo el file_metadata de %s",newpoke->pokemon);
	}

}


int existeDirectorio(char* path){
	DIR* dir = opendir(path);
	int x = dir;
	closedir(dir);
	return x;
}

void verificarAperturaPokemon(t_new_pokemon* newpoke,int socket){

	char* path = obtenerPathMetaFile(newpoke);
	bool x = isOpen(path);
	if (isOpen(path)){
		int secs = config->tiempo_retardo_operacion;
		log_info(logger,"No se puede acceder a este pokemon porque esta en uso (OPEN=Y), reintentando en: %d",secs);
		sleep(secs);
		funcionHiloNewPokemon(newpoke,socket);

	} else {

		log_info(logger,"Se puede acceder al pokemon, lockeo el archivo %s",newpoke->pokemon);
		lock_file(path);

		}
	}

char* obtenerPathMetaFile(t_new_pokemon* pokemon){
	punto_montaje = config->punto_montaje_tallgrass;
	char* path  = concatenar (punto_montaje,"/Files/");
	char* path2 = concatenar (path,pokemon->pokemon); //terrible negrada esto, pero anda o no anda?
	char* path3 = concatenar(path2,"/Metadata.bin");
	return path3;
}

char* obtenerPathDirPokemon(t_new_pokemon* pokemon){
	punto_montaje = config->punto_montaje_tallgrass;
	char* path = concatenar(punto_montaje,"/Files/");
	char* path2 = concatenar(path,pokemon->pokemon);
	return path2;
}

bool existePokemonEnPosicion(t_new_pokemon* pokemon){
	char* meta_file_path = obtenerPathMetaFile(pokemon);

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



