#include "game_card.h"

int main(void) {
	t_config_game_card* config = leer_config();
	iniciar_logger("gameCard.log","gamercard");

	//int socket_br = crear_conexion(config -> ip_broker, config -> puerto_broker);
	//int socket_gb = crear_conexion(config -> ip_gameBoy, config -> puerto_gameBoy);
	//enviar_mensaje(GC_LOCALIZED_POKEMON_BR, "Localized Pokemon", socket_br);
	//iniciar_servidor(config -> ip_gameCard,config -> puerto_gameCard);


	terminar_programa(socket,config);
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

void escribirMetadata(char* path){


}
int tamanio_de_metadata(t_metadata metadata){
	int stringLong = strlen(metadata.magic)+1 ;
	return stringLong + (sizeof(int) *2) ;
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

void process_request(int cod_op, int cliente_fd) { // Cada case depende del que toque ese modulo.
	int size;
	void* msg;

	log_info(logger,"Codigo de operacion %d",cod_op);

	switch (cod_op) {
		case GET_POKEMON:
			msg = malloc(sizeof(t_get_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			informarAlBroker(msg,cliente_fd,GET_POKEMON);
			agregar_mensaje(GET_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CATCH_POKEMON:
			msg = malloc(sizeof(t_catch_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			informarAlBroker(msg,cliente_fd,CATCH_POKEMON);
			agregar_mensaje(CATCH_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case NEW_POKEMON:
			msg = malloc(sizeof(t_new_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			informarAlBroker(msg,cliente_fd,NEW_POKEMON);

			// hilo
			agregar_mensaje(NEW_POKEMON, size, msg, cliente_fd);
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

int funcionHiloNewPokemon(){

//	existePokemon();
//	if (!sePuedeAbrirElArchivo()){
//		//finalizarHilo y reintentar operacion
//	}
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
	    new_str[0] = '\0';   // ensures the memory is an empty string
	    strcat(new_str,str1);
	    strcat(new_str,str2);
	} else {
	    log_error(logger,"error al concatenar");
	}
	return new_str;
}


