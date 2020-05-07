#include "game_boy.h"

int main(int argc,char* argv[]){

	int id_mensaje = 0;
	iniciar_programa(argc);
    int socket = seleccionar_proceso(argv);
    recibir_id_de_mensaje_enviado(socket, id_mensaje);
	terminar_programa(socket, logger, config_game_boy);
}

void iniciar_programa(int argc){
    leer_config();
	iniciar_logger(config_game_boy->log_file, "gameboy");

	if(argc == 1){
			printf("\n No se han ingresado parametros. \n");
			exit(-1);
	}

	log_info(logger, "El size del comando es: %i", argc);
}
///char*  parametros[] = BROKER GET_POKEMON PIKACHU 2 5 // vector de puntertos de char === vector de strings
int seleccionar_proceso(char *parametros[]){

  int conexion;
  char* proceso = parametros[1];
  if(strcmp(proceso, "") == 0){
	  log_info(logger, "No ha ingresado un proceso correcto");
	  exit (-4);
  }
 op_code cod_op = obtener_enum_de_string(parametros[2]);

  if(strcmp(parametros[2], "") == 0){
	  log_info(logger, "No ha ingresado un codigo de operacion");
	  exit (-4);
  }

  if (strcmp(proceso, "BROKER") == 0){
	  conexion = crear_conexion(config_game_boy -> ip_broker,config_game_boy-> puerto_broker );

	  if(config_game_boy == NULL){
	      	printf("No se pudo leer el archivo config.");
	      	exit(-9);
	  }

	  switch (cod_op) {
	  	  	  case GET_POKEMON:
	  	  		mensaje = malloc(sizeof(t_get_pokemon));
	  	  		armar_mensaje_get_pokemon(parametros, mensaje);
	  	  		enviar_mensaje(1, mensaje, conexion);
	  	        break;
	  	  	  case CATCH_POKEMON:
	  	  		mensaje = malloc(sizeof(t_catch_pokemon));
	  	  		armar_mensaje_catch_pokemon(parametros, mensaje);
	  	  		enviar_mensaje(2,mensaje, conexion);
	  	  		break;
	  	 	  case CAUGHT_POKEMON:
	  	  		mensaje = malloc(sizeof(t_caught_pokemon));
	  	  		armar_mensaje_caught_pokemon(parametros, mensaje);
	  	  		enviar_mensaje(4, mensaje, conexion);
	  	  		break;
	  	  	  case APPEARED_POKEMON:
	  	  		mensaje = malloc(sizeof(t_appeared_pokemon));
	  	  		armar_mensaje_appeared_pokemon(parametros, mensaje);
	  	  		enviar_mensaje(5, mensaje , conexion);
	  	  		break;
	  	  	  case NEW_POKEMON:
	  	  		mensaje = malloc(sizeof(t_new_pokemon));
	  	  		armar_mensaje_new_pokemon(parametros, mensaje);
	  	  		enviar_mensaje(6, mensaje, conexion);
	  	  	    break;
	  	 	  default:
	  		  	log_info(logger, "Ingrese un codigo de operacion valido");
	  		  	exit (-6);
	  		  	break;
	  	  }
  }
  if (strcmp(proceso, "GAMECARD") == 0){
	  conexion = crear_conexion(config_game_boy -> ip_gameCard,config_game_boy-> puerto_gameCard );
	  switch (cod_op) {
	  	  	  case GET_POKEMON:
	  	  		mensaje = malloc(sizeof(t_get_pokemon));
				armar_mensaje_get_pokemon(parametros, mensaje);
				enviar_mensaje(1, mensaje, conexion);
				break;
	  	  	  case CATCH_POKEMON:
				mensaje = malloc(sizeof(t_catch_pokemon));
				armar_mensaje_catch_pokemon(parametros, mensaje);
				enviar_mensaje(2, mensaje, conexion);
				break;
	  	  	  case NEW_POKEMON:
	  	  		mensaje = malloc(sizeof(t_new_pokemon));
	  	  		armar_mensaje_new_pokemon(parametros, mensaje);
	  	  		enviar_mensaje(5, mensaje, conexion);
	  	  		break;
	  	  	  default:
	  	  		log_info(logger, "Ingrese un codigo de operacion valido");
	  	  		exit (-6);
	  	  		break;
	  }
  }

  if (strcmp(proceso, "TEAM") == 0){
	  conexion = crear_conexion(config_game_boy -> ip_team,config_game_boy-> puerto_team );
	  mensaje = malloc(sizeof(t_appeared_pokemon));
	  armar_mensaje_appeared_pokemon(parametros, mensaje);
	  enviar_mensaje(4, mensaje , conexion);
  }

  if (strcmp(proceso, "SUSCRIPTOR") == 0){
	   conexion = crear_conexion(config_game_boy -> ip_broker, config_game_boy -> puerto_broker);
	   mensaje = malloc(sizeof(t_suscripcion));
	   armar_mensaje_suscripcion(parametros, mensaje);
	   enviar_mensaje(7, mensaje, conexion);
   }


   if (conexion < 0){
	  log_info(logger,"No se puedo realizar la conexion");
	  return conexion;
   }

     log_info(logger,"Se pudo realizar la conexion");
     return conexion;
}

op_code obtener_enum_de_string (char* string ) {
    static struct {
        char* string;
        int numero_enum;
    } map[] = {
        {"GET_POKEMON", GET_POKEMON },//1
        {"CATCH_POKEMON", CATCH_POKEMON },//2
        {"LOCALIZED_POKEMON", LOCALIZED_POKEMON },//3
		{"CAUGHT_POKEMON", CAUGHT_POKEMON},//4
		{"APPEARED_POKEMON", APPEARED_POKEMON},//5
		{"NEW_POKEMON",NEW_POKEMON},//6
		{"SUSCRIPTION",SUSCRIPTION}//7

    };
    int i;
    for ( i = 0 ; i < sizeof(map)/sizeof(map[0]); i++ ) {
        if ( strcmp(string,map[i].string) == 0 ) {
            return map[i].numero_enum;
        }
    }
    return 0;
}

void armar_mensaje_suscripcion(char* parametros[], t_suscripcion* a_enviar){

	char* cola_a_suscribir = parametros[2];

	if(strcmp(cola_a_suscribir,"COLA_GET") == 0){
		a_enviar -> cola_a_suscribir = GET_POKEMON;
	}

	if(strcmp(cola_a_suscribir,"COLA_CATCH") == 0){
		a_enviar -> cola_a_suscribir = CATCH_POKEMON;
	}

	if(strcmp(cola_a_suscribir,"COLA_LOCALIZED") == 0){
		a_enviar -> cola_a_suscribir = LOCALIZED_POKEMON;
	}

	if(strcmp(cola_a_suscribir,"COLA_CAUGHT") == 0){
		a_enviar -> cola_a_suscribir = CAUGHT_POKEMON;
	}

	if(strcmp(cola_a_suscribir,"COLA_APPEARED") == 0){
		a_enviar -> cola_a_suscribir = APPEARED_POKEMON;
	}

	if(strcmp(cola_a_suscribir,"COLA_NEW") == 0){
		a_enviar -> cola_a_suscribir = NEW_POKEMON;
	}

	a_enviar -> socket = 10000;//Esto esta setteado, hay que adaptarlo.

	//hay que pensar como agregar el tiempo de suscripcion para el caso de game_boy
}

void armar_mensaje_get_pokemon(char *parametros[], t_get_pokemon* a_enviar){
	a_enviar -> pokemon = parametros [3];
	a_enviar -> id_mensaje = 1;
}

void armar_mensaje_catch_pokemon(char *parametros[], t_catch_pokemon* a_enviar){
	a_enviar->pokemon = parametros[3];
	a_enviar->posicion[0] = (int)parametros[4];
	a_enviar->posicion[1] = (int)parametros[5];
	a_enviar -> id_mensaje = 1; ///A LABURAR A FUTURO
}

void armar_mensaje_caught_pokemon(char *parametros[], t_caught_pokemon* a_enviar){
	a_enviar->id_mensaje = (int)parametros[3];
	a_enviar->resultado = (int)parametros[4];
}

void armar_mensaje_appeared_pokemon(char *parametros[], t_appeared_pokemon* a_enviar){
	a_enviar->pokemon = parametros[3];
	a_enviar->posicion[0] = (int)parametros[4];
	a_enviar->posicion[1] = (int)parametros[5];
	a_enviar->id_mensaje = (int)parametros[6];
}

void armar_mensaje_new_pokemon(char *parametros[], t_new_pokemon* a_enviar){
	a_enviar->pokemon = parametros [3];
	a_enviar->posicion[0] = atoi(parametros[4]);
	a_enviar-> posicion[1] = atoi(parametros[5]);
	a_enviar -> cantidad = atoi(parametros[6]);
	a_enviar->id_mensaje = 1;/// A futuro
}

void leer_config() {

    config_game_boy = malloc(sizeof(t_config_game_boy));

	t_config* config = config_create("game_boy.config"); ///Si queres debaguear agrega el path seria Debug/game_boy.config

	if(config == NULL){
    	printf("no se pudo encontrar el path del config");
    	exit(-2);
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

void process_request(int cod_op, int cliente_fd ){}

void recibir_id_de_mensaje_enviado(int socket_cliente, int id_mensaje_enviado){
	int id = 0;
	recv(socket_cliente, &id, sizeof(int), MSG_WAITALL);
	id_mensaje_enviado = id;
	log_info(logger, "El ID de mensaje enviado es: %d", id_mensaje_enviado);
}
