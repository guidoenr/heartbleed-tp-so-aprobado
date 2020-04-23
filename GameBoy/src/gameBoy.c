#include "gameBoy.h"

int main(int argc,char* argv[]){
	argv = malloc(sizeof(char*)*argc);
	if(argc ==0){
		printf("No se han ingresado los parametros");
		return -1;
	}
    leer_config();
	iniciar_logger();
    preparar_mensaje(argc,argv);
    int socket;
	log_info(logger, "El ip es : %s", config_game_boy -> ip_broker);
	log_info(logger, "El port es : %s ", config_game_boy -> puerto_broker);
	terminar_programa(socket, logger, config_game_boy);
}
void preparar_mensaje(int cantidad_parametros, char *parametros[]){
      char* proceso = malloc(sizeof(parametros)*cantidad_parametros);
      memcpy(proceso ,&parametros[0],10);///toma el proceso
      log_info(logger, "el proceso recibido es:%s",proceso);
      free(parametros);
      free(proceso);
}

void iniciar_logger(void) {

	logger = log_create("gameBoy.log", "gameBoy", 1, LOG_LEVEL_INFO);

	if (logger == NULL){
		printf("ERROR EN LA CREACION DEL LOGGER/n");
		exit(1);
	}
}

void leer_config() {

    config_game_boy = malloc(sizeof(t_config_game_boy));

	t_config* config = config_create("Debug/gameBoy.config");

	config_game_boy -> ip_broker = strdup(config_get_string_value(config,"IP_BROKER"));
	config_game_boy-> puerto_broker = strdup(config_get_string_value(config,"PUERTO_BROKER"));
	config_game_boy -> ip_team = strdup(config_get_string_value(config,"IP_TEAM"));
	config_game_boy-> puerto_team = strdup(config_get_string_value(config,"PUERTO_TEAM"));
	config_game_boy -> ip_gameCard = strdup(config_get_string_value(config,"IP_GAMECARD"));
	config_game_boy-> puerto_gameCard = strdup(config_get_string_value(config,"PUERTO_GAMECARD"));

	config_destroy(config);
}




void liberar_config(t_config_game_boy* config) {
    free(config -> ip_broker);
	free(config -> puerto_broker);
	free(config ->  ip_team);
	free(config -> puerto_team);
	free(config -> ip_gameCard);
	free(config -> puerto_gameCard);
	free(config);
}

void terminar_programa(int conexion,t_log* logger, t_config_game_boy* config) {
	liberar_config(config);

	liberar_logger(logger);
	liberar_conexion(conexion);
}
