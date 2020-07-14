#include "game_boy.h"

int main(int argc, char * argv[]) {

  uint32_t id_mensaje = 0;
  iniciar_programa(argc);
  uint32_t socket = seleccionar_proceso(argv);
  recibir_id_de_mensaje_enviado(socket, id_mensaje);
  terminar_programa(socket, logger, config_game_boy);

}

void iniciar_programa(uint32_t argc) {


  leer_config();
  iniciar_logger(config_game_boy -> log_file, "gameboy");
  if (argc == 1) {
    log_error(logger, "\n No se han ingresado parametros. \n");
    exit(-1);
  }

  log_info(logger, "El size del comando es: %i", argc); // delete
}
///char*  parametros[] = BROKER GET_POKEMON PIKACHU 2 5 // vector de punteros de char === vector de strings
uint32_t seleccionar_proceso(char * parametros[]) {

  uint32_t conexion;
  char * proceso = parametros[1];
  uint32_t tamanio_mensaje;
  void* mensaje;

  if (string_equals_ignore_case(proceso, "")) {
    log_error(logger, "No ha ingresado un proceso correcto");
    exit(-4);
  }
  op_code cod_op = obtener_enum_de_string(parametros[2]);

  if (string_equals_ignore_case(parametros[2], "")) {
    log_error(logger, "No ha ingresado un codigo de operacion");
    exit(-4);
  }

  if (string_equals_ignore_case(proceso, "BROKER")) {
    conexion = crear_conexion(config_game_boy -> ip_broker, config_game_boy -> puerto_broker);
    if(conexion != -1) {
		log_info(logger, "Conexion creada con el proceso Broker");
	}
    if (config_game_boy == NULL) {
      log_error(logger, "No se pudo leer el archivo config.");
      exit(-9);
    }


    switch (cod_op) {
		case GET_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_get_pokemon(parametros), GET_POKEMON);
		  mensaje = armar_mensaje_get_pokemon(parametros);
		  enviar_mensaje(GET_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
		case CATCH_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_catch_pokemon(parametros), CATCH_POKEMON);
		  mensaje = armar_mensaje_catch_pokemon(parametros);
		  enviar_mensaje(CATCH_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
		case CAUGHT_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_caught_pokemon(parametros), CAUGHT_POKEMON);
		  mensaje = armar_mensaje_caught_pokemon(parametros);
		  enviar_mensaje(CAUGHT_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
		case APPEARED_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_appeared_pokemon(parametros), APPEARED_POKEMON);
		  mensaje = armar_mensaje_appeared_pokemon(parametros);
		  enviar_mensaje(APPEARED_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
		case NEW_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_new_pokemon(parametros), NEW_POKEMON);
		  mensaje = armar_mensaje_new_pokemon(parametros);
		  enviar_mensaje(NEW_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
		default:
		  log_error(logger, "Ingrese un codigo de operacion valido");
		  exit(-6);
		  break;
    }
  }

  if (string_equals_ignore_case(proceso, "GAMECARD")) {
    conexion = crear_conexion(config_game_boy -> ip_gameCard, config_game_boy -> puerto_gameCard);
    if(conexion != -1) {
    	log_info(logger, "Conexion creada con el proceso Game Card");
    }

    switch (cod_op) {
       case GET_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_get_pokemon(parametros), GET_POKEMON);
		  mensaje = armar_mensaje_get_pokemon(parametros);
		  enviar_mensaje(GET_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
       case CATCH_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_catch_pokemon(parametros), CATCH_POKEMON);
		  mensaje = armar_mensaje_catch_pokemon(parametros);
		  enviar_mensaje(CATCH_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
       case NEW_POKEMON:
		  tamanio_mensaje = size_mensaje(armar_mensaje_new_pokemon(parametros), NEW_POKEMON);
		  mensaje = armar_mensaje_new_pokemon(parametros);
		  enviar_mensaje(NEW_POKEMON, mensaje, conexion, tamanio_mensaje);
		  break;
		default:
		  log_error(logger, "Ingrese un codigo de operacion valido");
		  exit(-6);
		  break;
    }
  }

  if (string_equals_ignore_case(proceso, "TEAM")) {
    conexion = crear_conexion(config_game_boy -> ip_team, config_game_boy -> puerto_team);
    if(conexion != -1) {
    	log_info(logger, "Conexion creada con el proceso Team");
    }
    tamanio_mensaje = size_mensaje(armar_mensaje_appeared_pokemon(parametros), APPEARED_POKEMON);
	mensaje = armar_mensaje_appeared_pokemon(parametros);
	enviar_mensaje(APPEARED_POKEMON, mensaje, conexion, tamanio_mensaje);
  }

  if (string_equals_ignore_case(proceso, "SUSCRIPTOR")) {
    conexion = crear_conexion(config_game_boy -> ip_broker, config_game_boy -> puerto_broker);
    if(conexion != -1) {
		log_info(logger, "Conexion creada con el proceso Broker");
    }
    tamanio_mensaje = size_mensaje(armar_mensaje_suscripcion(parametros), SUBSCRIPTION);
    mensaje = armar_mensaje_suscripcion(parametros);
    enviar_mensaje(SUBSCRIPTION, mensaje, conexion, tamanio_mensaje);
  }

  if (conexion < 0) {
    log_error(logger, "No se puedo realizar la conexion");
    return conexion;
  }

  return conexion;
}

op_code obtener_enum_de_string(char * string) {
  static struct {
    char * string;
    uint32_t numero_enum;
  }
  map[] = {
    {
      "GET_POKEMON",
      GET_POKEMON
    }, //1
    {
      "CATCH_POKEMON",
      CATCH_POKEMON
    }, //2
    {
      "LOCALIZED_POKEMON",
      LOCALIZED_POKEMON
    }, //3
    {
      "CAUGHT_POKEMON",
      CAUGHT_POKEMON
    }, //4
    {
      "APPEARED_POKEMON",
      APPEARED_POKEMON
    }, //5
    {
      "NEW_POKEMON",
      NEW_POKEMON
    }, //6
    {
      "SUBSCRIPTION",
      SUBSCRIPTION
    } //7

  };
  uint32_t i;
  for (i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
    if (string_equals_ignore_case(string, map[i].string)) {
      return map[i].numero_enum;
    }
  }
  return 0;
}

t_suscripcion* armar_mensaje_suscripcion(char * parametros[]) {
   t_suscripcion* a_enviar = malloc(sizeof(t_suscripcion));

  char * cola_a_suscribir = parametros[2];
  a_enviar -> socket = 10000;

  if (string_equals_ignore_case(cola_a_suscribir, "COLA_GET")) {
    a_enviar -> cola_a_suscribir = GET_POKEMON;
  }

  if (string_equals_ignore_case(cola_a_suscribir, "COLA_CATCH")) {
    a_enviar -> cola_a_suscribir = CATCH_POKEMON;
  }

  if (string_equals_ignore_case(cola_a_suscribir, "COLA_LOCALIZED")) {
    a_enviar -> cola_a_suscribir = LOCALIZED_POKEMON;
  }

  if (string_equals_ignore_case(cola_a_suscribir, "COLA_CAUGHT")) {
    a_enviar -> cola_a_suscribir = CAUGHT_POKEMON;
  }

  if (string_equals_ignore_case(cola_a_suscribir, "COLA_APPEARED")) {
    a_enviar -> cola_a_suscribir = APPEARED_POKEMON;
  }

  if (string_equals_ignore_case(cola_a_suscribir, "COLA_NEW")) {
    a_enviar -> cola_a_suscribir = NEW_POKEMON;
  }

   //Esto esta setteado, hay que adaptarlo.
  a_enviar -> tiempo_suscripcion = atof(parametros[3]);
  a_enviar -> id_proceso = 14000;

  return a_enviar;

}


t_get_pokemon* armar_mensaje_get_pokemon(char * parametros[]) {

  uint32_t tamanio = strlen(parametros[3]);
  t_get_pokemon* a_enviar = malloc(sizeof(t_get_pokemon));
  a_enviar -> pokemon = malloc(tamanio);

  a_enviar -> pokemon = parametros[3];
  if(parametros[4] != NULL ){
	  a_enviar -> id_mensaje = atof(parametros[4]);
  } else {
	  a_enviar -> id_mensaje = 0;
  }

  return a_enviar;
}

t_catch_pokemon* armar_mensaje_catch_pokemon(char * parametros[]) {
  uint32_t tamanio = strlen(parametros[3]);
  t_catch_pokemon* a_enviar = malloc(sizeof(t_catch_pokemon));
  a_enviar -> pokemon = malloc(tamanio);

  a_enviar -> pokemon = parametros[3];
  a_enviar -> posicion[0] = atof(parametros[4]);
  a_enviar -> posicion[1] = atof(parametros[5]);
  if(parametros[6] != NULL){
	  a_enviar -> id_mensaje = atof(parametros[6]);
  } else {
	  a_enviar -> id_mensaje = 0; ///A LABURAR A FUTURO
  }

  return a_enviar;
}

t_caught_pokemon* armar_mensaje_caught_pokemon(char * parametros[]) {

  t_caught_pokemon* a_enviar = malloc(sizeof(t_caught_pokemon));

  a_enviar -> id_mensaje = 0;
  a_enviar -> id_mensaje_correlativo = atof(parametros[3]);
  a_enviar -> resultado = atof(parametros[4]);

  return a_enviar;
}

t_appeared_pokemon* armar_mensaje_appeared_pokemon(char * parametros[]) {
  uint32_t tamanio = strlen(parametros[3]);
  t_appeared_pokemon* a_enviar = malloc(sizeof(t_appeared_pokemon));
  a_enviar -> pokemon = malloc(tamanio);

  a_enviar -> pokemon = parametros[3];
  a_enviar -> posicion[0] = atof(parametros[4]);
  a_enviar -> posicion[1] = atof(parametros[5]);
  a_enviar -> id_mensaje = 0;
  if(parametros[6] != NULL){
	  a_enviar -> id_mensaje_correlativo = atof(parametros[6]);
  }

  return a_enviar;
}

t_new_pokemon* armar_mensaje_new_pokemon(char * parametros[]) {
  uint32_t tamanio = strlen(parametros[3]);
  t_new_pokemon* a_enviar = malloc(sizeof(t_new_pokemon));
  a_enviar -> pokemon = malloc(tamanio);

  a_enviar -> pokemon = parametros[3];
  a_enviar -> posicion[0] = atof(parametros[4]);
  a_enviar -> posicion[1] = atof(parametros[5]);
  a_enviar -> cantidad = atof(parametros[6]);
  if(parametros[7] != NULL){
	  a_enviar -> id_mensaje = atof(parametros[7]);
  } else {
	  a_enviar -> id_mensaje = 0; /// A futuro
  }

  return a_enviar;
}


void leer_config() {

  config_game_boy = malloc(sizeof(t_config_game_boy));

  config = config_create("game_boy.config"); ///Si queres debuguear agrega el path seria Debug/game_boy.config

  if (config == NULL) {
    log_error(logger, "no se pudo encontrar el path del config");
    exit(-2);
  }
  config_game_boy -> ip_broker = config_get_string_value(config, "IP_BROKER");
  config_game_boy -> puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
  config_game_boy -> ip_team = config_get_string_value(config, "IP_TEAM");
  config_game_boy -> puerto_team = config_get_string_value(config, "PUERTO_TEAM");
  config_game_boy -> ip_gameCard = config_get_string_value(config, "IP_GAMECARD");
  config_game_boy -> puerto_gameCard = config_get_string_value(config, "PUERTO_GAMECARD");
  config_game_boy -> log_file = config_get_string_value(config, "LOG_FILE");

}

void liberar_config(t_config_game_boy * config_game_boy) {
  free(config_game_boy -> ip_broker);
  free(config_game_boy -> puerto_broker);
  free(config_game_boy -> ip_team);
  free(config_game_boy -> puerto_team);
  free(config_game_boy -> ip_gameCard);
  free(config_game_boy -> puerto_gameCard);
  free(config_game_boy -> log_file);
  free(config_game_boy);
}

void terminar_programa(uint32_t conexion, t_log * logger, t_config_game_boy * config_game_boy) {
  //liberar_config(config_game_boy);
  liberar_logger(logger);
  liberar_conexion(conexion);
  config_destroy(config);
}

void process_request(uint32_t cod_op, uint32_t cliente_fd) {}

void recibir_id_de_mensaje_enviado(uint32_t socket_cliente, uint32_t id_mensaje_enviado) {
  uint32_t id = 0;
  recv(socket_cliente, & id, sizeof(uint32_t), MSG_WAITALL);
  id_mensaje_enviado = id;
  log_info(logger, "El ID de mensaje enviado es: %d", id_mensaje_enviado); // delete?
}
