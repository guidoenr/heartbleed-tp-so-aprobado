#include "game_boy.h"

int main(int argc,char* argv[]){

	leer_config();
	iniciar_programa(argc);

    int socket = seleccionar_proceso(argv);

	terminar_programa(socket, logger, config_game_boy);
}

void iniciar_programa(int argc){

	if(argc == 0){
			printf("No se han ingresado los parametros.");
			exit(-1);
	}
	iniciar_logger(config_game_boy->log_file, "gameboy");
	log_info(logger, "El size del comando es: %i", argc);

}

int seleccionar_proceso(char *parametros[]){
	  int conexion;
   	  log_info(logger, "el proceso recibido es:%s", parametros[0]);
      char* proceso =  strdup(parametros[1]);
      log_info(logger, "el proceso recibido es:%s", proceso);
      if (strcmp(proceso, "BROKER") == 0)
    	  conexion = crear_conexion(config_game_boy -> ip_broker,config_game_boy-> puerto_broker );

      if (strcmp(proceso, "GAMECARD") == 0)
      		  conexion = crear_conexion(config_game_boy -> ip_gameCard,config_game_boy-> puerto_gameCard );

      if (strcmp(proceso, "TEAM") == 0)
      		  conexion = crear_conexion(config_game_boy -> ip_team,config_game_boy-> puerto_team );

      /*if (strcmp(proceso, "SUBSCRIPTOR") == 0){
    	   tiene instrucciones espcailes
      }*/

      if(conexion<0){
    	  log_info(logger,"No se puedo realizar la conexion");
    	  return conexion;
      }
     log_info(logger,"Se puedo realizar la conexion");
     enviar_mensaje(GB_NEW_POKEMON_BR, "Get Pokemon", conexion);
     return conexion;
}

void leer_config() {

    config_game_boy = malloc(sizeof(t_config_game_boy));

	t_config* config = config_create("game_boy.config"); ///Si queres debaguear agrega el path seria Debug/game_boy.config

	if(config == NULL){
    	printf("no se pudo encontrar el path del config");
    	return exit(-2);
    }
	config_game_boy -> ip_broker = strdup(config_get_string_value(config,"IP_BROKER"));
	config_game_boy-> puerto_broker = strdup(config_get_string_value(config,"PUERTO_BROKER"));
	config_game_boy -> ip_team = strdup(config_get_string_value(config,"IP_TEAM"));
	config_game_boy-> puerto_team = strdup(config_get_string_value(config,"PUERTO_TEAM"));
	config_game_boy -> ip_gameCard = strdup(config_get_string_value(config,"IP_GAMECARD"));
	config_game_boy-> puerto_gameCard = strdup(config_get_string_value(config,"PUERTO_GAMECARD"));
	config_game_boy-> log_file = strdup(config_get_string_value(config,"LOG_FILE"));
	config_destroy(config);
}

void liberar_config(t_config_game_boy* config) {
    free(config -> ip_broker);
	free(config -> puerto_broker);
	free(config ->  ip_team);
	free(config -> puerto_team);
	free(config -> ip_gameCard);
	free(config -> puerto_gameCard);
	free(config -> log_file);
	free(config);
}

void terminar_programa(int conexion,t_log* logger, t_config_game_boy* config) {
	liberar_config(config);
	liberar_logger(logger);
	liberar_conexion(conexion);
}
