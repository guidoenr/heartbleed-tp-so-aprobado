#include "game_boy.h"

int main(int argc, char * argv[]) {

  uint32_t id_mensaje = 0;
  parsear(argv);
  iniciar_programa(argc);
  uint32_t socket = seleccionar_proceso(argv);
  recibir_id_de_mensaje_enviado(socket, id_mensaje);
  terminar_programa(socket, logger, config_game_boy);
}

void iniciar_programa(uint32_t argc) {

  leer_config();
  iniciar_logger(config_game_boy -> log_file, "gameboy");

  if (argc == 1) {
    printf("\n No se han ingresado parametros. \n");
    exit(-1);
  }

  log_info(logger, "El size del comando es: %i", argc);
}
void* parsear(char ** datos_de_consola) {
  lista_parametros = list_create();
  uint32_t posiciones[2];
  uint32_t posicion = 0;
  int tienePosiciones = 1;
  op_code cod_op = obtener_enum_de_string(datos_de_consola[2]);
  if(CAUGHT_POKEMON == cod_op && GET_POKEMON ==cod_op){
	  tienePosiciones = 0;
  }
  while ( * datos_de_consola != NULL) {
	if(posicion == 4 && tienePosiciones){
     posiciones[0] = atoi(datos_de_consola[posicion-1]);
	}
	if(posicion == 5 && tienePosiciones){
     posiciones[1] = atoi(datos_de_consola[posicion-1]);
     list_add(lista_parametros, posiciones);
	}
	if(posicion > 2 && posicion != 4 && posicion != 5){
	 list_add(lista_parametros, datos_de_consola );
	 }
	datos_de_consola++;
    posicion++;
	}
  return lista_parametros;
  }

uint32_t seleccionar_proceso(char* parametros[]) {
  uint32_t conexion;
  char * proceso = parametros[1];

  if (string_equals_ignore_case(proceso, "")) {
    log_info(logger, "No ha ingresado un proceso correcto");
    exit(-4);
  }
  op_code cod_op = obtener_enum_de_string(parametros[2]);

  if (string_equals_ignore_case(proceso, "BROKER")) {
    conexion = crear_conexion(config_game_boy -> ip_broker, config_game_boy -> puerto_broker);

    if (config_game_boy == NULL) {
      printf("No se pudo leer el archivo config.");
      exit(-9);
    }

    switch (cod_op) {
    case GET_POKEMON:
      mensaje = malloc(sizeof(t_get_pokemon));
      armar_mensaje_get_pokemon(mensaje);
      enviar_mensaje(GET_POKEMON, mensaje, conexion);
      break;
    case CATCH_POKEMON:
      mensaje = malloc(sizeof(t_catch_pokemon));
      armar_mensaje_catch_pokemon(mensaje);
      enviar_mensaje(CATCH_POKEMON, mensaje, conexion);
      break;
    case CAUGHT_POKEMON:
      mensaje = malloc(sizeof(t_caught_pokemon));
      armar_mensaje_caught_pokemon(mensaje);
      enviar_mensaje(CAUGHT_POKEMON, mensaje, conexion);
      break;
    case APPEARED_POKEMON:
      mensaje = malloc(sizeof(t_appeared_pokemon));
      armar_mensaje_appeared_pokemon(mensaje);
      enviar_mensaje(APPEARED_POKEMON, mensaje, conexion);
      break;
    case NEW_POKEMON:
      mensaje = malloc(sizeof(t_new_pokemon));
      armar_mensaje_new_pokemon(mensaje);
      enviar_mensaje(NEW_POKEMON, mensaje, conexion);
      break;
    default:
      log_info(logger, "Ingrese un codigo de operacion valido");
      exit(-6);
      break;
    }
  }
  if (string_equals_ignore_case(proceso, "GAMECARD")) {
    conexion = crear_conexion(config_game_boy -> ip_gameCard, config_game_boy -> puerto_gameCard);
    switch (cod_op) {
    case GET_POKEMON:
      mensaje = malloc(sizeof(t_get_pokemon));
      armar_mensaje_get_pokemon(mensaje);
      enviar_mensaje(GET_POKEMON, mensaje, conexion);
      break;
    case CATCH_POKEMON:
      mensaje = malloc(sizeof(t_catch_pokemon));
      armar_mensaje_catch_pokemon(mensaje);
      enviar_mensaje(CATCH_POKEMON, mensaje, conexion);
      break;
    case NEW_POKEMON:
      mensaje = malloc(sizeof(t_new_pokemon));
      armar_mensaje_new_pokemon(mensaje);
      enviar_mensaje(NEW_POKEMON, mensaje, conexion);
      break;
    default:
      log_info(logger, "Ingrese un codigo de operacion valido");
      exit(-6);
      break;
    }
  }

  if (string_equals_ignore_case(proceso, "TEAM")) {
    conexion = crear_conexion(config_game_boy -> ip_team, config_game_boy -> puerto_team);
    mensaje = malloc(sizeof(t_appeared_pokemon));
    armar_mensaje_appeared_pokemon(mensaje);
    enviar_mensaje(APPEARED_POKEMON, mensaje, conexion);
  }

  if (string_equals_ignore_case(proceso, "SUSCRIPTOR")) {
    conexion = crear_conexion(config_game_boy -> ip_broker, config_game_boy -> puerto_broker);
    mensaje = malloc(sizeof(t_suscripcion));
    armar_mensaje_suscripcion(mensaje);
    enviar_mensaje(SUBSCRIPTION, mensaje, conexion);
  }

  if (conexion < 0) {
    log_info(logger, "No se puedo realizar la conexion");
    return conexion;
  }

  log_info(logger, "Se pudo realizar la conexion");
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

void armar_mensaje_suscripcion( t_suscripcion * a_enviar) {
/*
  char * cola_a_suscribir = parametros[2];

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

  a_enviar -> socket = 10000; //Esto esta setteado, hay que adaptarlo.

  //hay que pensar como agregar el tiempo de suscripcion para el caso de game_boy
   *
   */
}

void armar_mensaje_get_pokemon(t_get_pokemon * a_enviar) {
}

void armar_mensaje_catch_pokemon(t_catch_pokemon * a_enviar) {
}

void armar_mensaje_caught_pokemon(t_caught_pokemon * a_enviar) {
}

void armar_mensaje_appeared_pokemon(t_appeared_pokemon * a_enviar) {
}

void armar_mensaje_new_pokemon(t_new_pokemon * a_enviar) {
}

void leer_config() {

  config_game_boy = malloc(sizeof(t_config_game_boy));

  t_config * config = config_create("Debug/game_boy.config"); ///Si queres debaguear agrega el path seria Debug/game_boy.config

  if (config == NULL) {
    printf("no se pudo encontrar el path del config");
    exit(-2);
  }
  config_game_boy -> ip_broker = strdup(config_get_string_value(config, "IP_BROKER"));
  config_game_boy -> puerto_broker = strdup(config_get_string_value(config, "PUERTO_BROKER"));
  config_game_boy -> ip_team = strdup(config_get_string_value(config, "IP_TEAM"));
  config_game_boy -> puerto_team = strdup(config_get_string_value(config, "PUERTO_TEAM"));
  config_game_boy -> ip_gameCard = strdup(config_get_string_value(config, "IP_GAMECARD"));
  config_game_boy -> puerto_gameCard = strdup(config_get_string_value(config, "PUERTO_GAMECARD"));
  config_game_boy -> log_file = strdup(config_get_string_value(config, "LOG_FILE"));
  config_destroy(config);
}

void liberar_config(t_config_game_boy * config) {
  free(config -> ip_broker);
  free(config -> puerto_broker);
  free(config -> ip_team);
  free(config -> puerto_team);
  free(config -> ip_gameCard);
  free(config -> puerto_gameCard);
  free(config -> log_file);
  free(config);
}

void terminar_programa(uint32_t conexion, t_log * logger, t_config_game_boy * config) {
  liberar_config(config);
  liberar_logger(logger);
  liberar_conexion(conexion);
}

void process_request(uint32_t cod_op, uint32_t cliente_fd) {}

void recibir_id_de_mensaje_enviado(uint32_t socket_cliente, uint32_t id_mensaje_enviado) {
  uint32_t id = 0;
  recv(socket_cliente, & id, sizeof(uint32_t), MSG_WAITALL);
  id_mensaje_enviado = id;
  log_info(logger, "El ID de mensaje enviado es: %d", id_mensaje_enviado);
}
