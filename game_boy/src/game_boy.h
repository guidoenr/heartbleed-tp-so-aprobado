#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.h"
#include "/home/utnso/workspace/tp-2020-1c-heartbleed/Utils/src/Utils.c"

typedef struct {
	char* ip_broker;
	char* puerto_broker;
	char* ip_team;
	char* puerto_team;
	char* ip_gameCard;
	char* puerto_gameCard;
	char* log_file;
} t_config_game_boy;

t_log* logger;
t_config_game_boy* config_game_boy;
void iniciar_programa(int cantidad_parametros);
void iniciar_logger(char* file, char* program_name);
void leer_config(void);
int seleccionar_proceso(char *parametros[]);
op_code obtener_enum_de_string (char *s);
char* armar_mensaje(char *parametros[]);
void terminar_programa(int, t_log*, t_config_game_boy*);
void liberar_conexion(int);
void mostrar_menu(void);
void liberar_logger(t_log* logger);
void liberar_config(t_config_game_boy*);
