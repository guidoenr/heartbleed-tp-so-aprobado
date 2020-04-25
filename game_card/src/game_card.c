#include "game_card.h"

int main(void) {
	t_config_game_card* config = leer_config();
	logger = iniciar_logger();

//	int socket_br = crear_conexion(config -> ip_broker, config -> puerto_broker);
//	int socket_gb = crear_conexion(config -> ip_gameBoy, config -> puerto_gameBoy);
//	enviar_mensaje(GC_LOCALIZED_POKEMON_BR, "Localized Pokemon", socket);

	//iniciar_servidor(config -> ip_gameCard,config -> puerto_gameCard);
	escribirArchivoBin();
	leerArchivoBin();
	terminar_programa(socket, logger, config);
}


t_log* iniciar_logger(void) {

	t_log* logger = log_create("gameCard.log", "gameCard", 1, LOG_LEVEL_INFO);

	if (logger == NULL){
		printf("error en la creacion del logger/n");
		exit(1);
	}
	return logger;
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

void terminar_programa(int conexion,t_log* logger, t_config_game_card* config) {
	liberar_config(config);
	liberar_logger(logger);
	liberar_conexion(conexion);
}


void escribirArchivoBin(){

	FILE* file = fopen("metadata.bin","wb"); //write-binary

		t_metadata metadata;
		metadata.blocksize = 64;
		metadata.blocks = 5192;
		metadata.magic = "TALL_GRASS";

		fwrite(&metadata,tamanio_de_metadata(metadata),1,file);
		log_info(logger,"se creo el archivo metadata.bin");

		fclose(file);


}
int tamanio_de_metadata(t_metadata metadata){
	int stringLong = strlen(metadata.magic)+1 ;
	return stringLong + (sizeof(int) *2) ;
}

void leerArchivoBin(char* pathArchivo){

	FILE* file = fopen("metadata.bin","rb"); //read-binary

	if (file == NULL){
		log_warning(logger,"no existe el archivo a leer");
	}

	t_metadata metadataLeido;


	fread(&(metadataLeido.blocksize),sizeof(int),1,file);
	fread(&(metadataLeido.blocks),sizeof(int),1,file);
	fread(&(metadataLeido.magic),13,1,file);

	log_info(logger, "se leyo el metadata ");

	fclose(file);
}

int fileSize(char* filename){
	FILE* f = fopen(filename,"r");
	fseek(f, 0L, SEEK_END);
	int i = ftell(f);
	fclose(f);
	return i;
}

