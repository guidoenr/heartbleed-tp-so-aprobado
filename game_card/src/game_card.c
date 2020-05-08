#include "game_card.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main(void) {
	t_config_game_card* config = leer_config();
	iniciar_logger("gameCard.log","gamercard");

	//int socket_br = crear_conexion(config -> ip_broker, config -> puerto_broker);
	//int socket_gb = crear_conexion(config -> ip_gameBoy, config -> puerto_gameBoy);
	//enviar_mensaje(GC_LOCALIZED_POKEMON_BR, "Localized Pokemon", socket_br);
	//iniciar_servidor(config -> ip_gameCard,config -> puerto_gameCard);

	t_new_pokemon pikachu;
	pikachu.cantidad = 1;
	pikachu.id_mensaje = 2;
	pikachu.posicion[0] = 1;
	pikachu.posicion[1] = 6;
	pikachu.pokemon = "pikachu";

	verificarPokemon(pikachu);
	verificarAperturaPokemon(pikachu);

	terminar_programa(socket,config);
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


void crearMetadata(char* path){

	FILE* file = fopen(path,"wb"); //write-binary
	if (file==NULL){
		FILE* file = fopen(path,"wb");
		fclose(file);
		log_info(logger,"se creo el archivo metadata.bin VACIO");

	}else{

		t_metadata metadata;
		metadata.blocksize = 64;
		metadata.blocks = 5192;
		metadata.magic = "TALL_GRASS";
		int tam = tamanio_de_metadata(metadata);
		fwrite(&metadata,tam,1,file);

		log_info(logger,"se creo el archivo metadata.bin de tamaño: %d",tam);

		fclose(file);

	}

}

void crearMetadataFile(char* path,t_new_pokemon newPoke){
	FILE* file = fopen(path,"wb");

	t_file_metadata meta;
	meta.directory = 'Y';
	meta.size = 62; //TODO preguntar que es esto?
	meta.blocks[0] = newPoke.posicion[0];
	meta.blocks[1] = newPoke.posicion[1];// recontra TODO ni idea
	meta.open = 'N';

	fwrite(&meta,sizeof(tamanio_de_file_metadata(meta)),1,file);

	fclose(file);

}
int tamanio_de_metadata(t_metadata metadata){
	int stringLong = strlen(metadata.magic)+1 ;
	return stringLong + (sizeof(int) *2) ;
}

int tamanio_file_metadata(t_file_metadata fileMeta){
	return sizeof(char) + sizeof(fileMeta.blocks) + sizeof(int) + sizeof(char); // TODO BLOCKS?
}

void leerMetadata(char* path){
	FILE* file = fopen(path,"rb"); //read-binary

	if (!isFile(path)){
		log_warning(logger,"no existe el archivo a leer");
	}

	t_metadata metadataLeido; //despues vemos como se usa esta verga

	fread(&(metadataLeido.blocksize),sizeof(int),1,file);
	fread(&(metadataLeido.blocks),sizeof(int),1,file);
	fread(&(metadataLeido.magic),13,1,file);

	log_info(logger, "se leyo el metadata con tamaño %d",tamanio_de_metadata(metadataLeido));

	fclose(file);
}

int fileSize(char* path){
	FILE* f = fopen(path,"r");
	fseek(f, 0L, SEEK_END);
	int i = ftell(f);
	fclose(f);
	return i;
}

int isFile(char* path){
	FILE* f = fopen(path,"rb");
	fclose(f);
	return f != NULL;
}

void crearBitmap(char* path){
	FILE* file = fopen(path,"wb");
	int status;
	status=0; //arranca en 0
	fwrite(&status,sizeof(int),1,file);
	log_info(logger,"se creo el bitmap.bin con status: %d",status);
	fclose(file);
}

int estadoBitmap(char* path){
	FILE* file = fopen(path,"rb");
	int status;
	fread(&status,sizeof(int),1,file);
	fclose(file);
	return status;
}

int isDirectory(char* path){
	t_file_metadata fileMeta;
	FILE* file = fopen(path,"rb");
	fread(&fileMeta.directory,sizeof(char),1,file);
	fclose(file);

	return fileMeta.directory == 'Y';
}

int isOpen(char* path){
	t_file_metadata fileMeta;
	FILE* file = fopen(path,"rb");
	fread(&fileMeta.open,sizeof(char),1,file);
		fclose(file);
		return fileMeta.open == 'Y';
}

void process_request(uint32_t cod_op, uint32_t cliente_fd){ // Cada case depende del que toque ese modulo.
	uint32_t size;
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
			msg = malloc(sizeof(t_new_pokemon));
			//msg = recibir_mensaje(cliente_fd,size);
			informarAlBroker(msg,cliente_fd,NEW_POKEMON);

			// hilo
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

void informarAlBroker(void* msg,int socket,op_code codigo){
	//8 = ACK
	enviar_mensaje("ACK", "recibi el mensaje[ACK]", socket);
	log_info(logger,"recibi el msg %s",codigo);
}

int funcionHiloNewPokemon(void* buffer){

								//TODO como castear el buffer al t_new_pokemon
	t_new_pokemon msg;
	verificarPokemon(msg);
	verificarAperturaPokemon(msg);

//	if (existePokemonEnEsaPosicion()){
//		agregarAPoisiconExistente();
//	} else {
//		agregarAlFinalDelArchivo();
//	}
//
//	cerrarArchivo();

	return 1;
}

char* concatenar(char* str1,char* str2){
	char* new_str ;
	if((new_str = malloc(strlen(str1)+strlen(str2)+1)) != NULL){
	    new_str[0] = '\0';
	    strcat(new_str,str1);
	    strcat(new_str,str2);
	} else {
	    log_error(logger,"error al concatenar");
	}
	return new_str;
}

void verificarPokemon(t_new_pokemon newpoke){
	char* montaje = "montaje/Pokemon/";		  // esto va a cambiar con el tallgras, pero es un TODO para la entrega 21 maso
	char* path = concatenar(montaje,newpoke.pokemon); // DE TODAS FORMAS DEJO UNA ALGORITMIA FANTASTATICA
	if (existeDirectorio(path)){

		log_info(logger,"existe el dir: %s",path);
	} else{
		mkdir(path, 0777);
		log_info(logger,"se creo el directorio: %s",path);
	}

}

int existeDirectorio(char* path){

	DIR* dir = opendir(path);
	int x = dir;
	closedir(dir);
	return x;
}

void verificarAperturaPokemon(t_new_pokemon msg){
	char* path = concatenar ("montaje/Pokemon/",msg.pokemon);
	char* path2 = concatenar (path,"/Metadata.bin"); //terrible negrada esto, pero anda o no anda?
	log_info(logger,path2);
	if (isOpen(path2)){
		//finalizar hilo y reintentar operacion TODO
	} else {

	}
}


