#include<stdio.h>

#include<stdlib.h>

#include<commons/log.h>

#include<commons/string.h>

#include<commons/config.h>

#include<readline/readline.h>

#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

typedef struct {
  char * ip_broker;
  char * puerto_broker;
  char * ip_team;
  char * puerto_team;
  char * ip_gameCard;
  char * puerto_gameCard;
  char * log_file;
}
t_config_game_boy;

t_log * logger;
t_list * parametros;
t_config_game_boy * config_game_boy;
void * mensaje;

void iniciar_programa(uint32_t);
void iniciar_logger(char * , char * );
void leer_config(void);
uint32_t seleccionar_proceso(char * parametros[]);
op_code obtener_enum_de_string(char * );
void armar_mensaje_get_pokemon(char * parametros[], t_get_pokemon * );
void armar_mensaje_catch_pokemon(char * parametros[], t_catch_pokemon * );
void armar_mensaje_localized_pokemon(char * parametros[], t_localized_pokemon * );
void armar_mensaje_caught_pokemon(char * parametros[], t_caught_pokemon * );
void armar_mensaje_appeared_pokemon(char * parametros[], t_appeared_pokemon * );
void armar_mensaje_new_pokemon(char * parametros[], t_new_pokemon * );
void armar_mensaje_suscripcion(char * parametros[], t_suscripcion * );
void terminar_programa(uint32_t, t_log * , t_config_game_boy * );
void liberar_conexion(uint32_t);
void mostrar_menu(void);
void liberar_logger(t_log * );
void liberar_config(t_config_game_boy * );
void recibir_id_de_mensaje_enviado(uint32_t, uint32_t);
