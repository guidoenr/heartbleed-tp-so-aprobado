#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<utils.h>

typedef struct {
	char* ip_broker;
	char* puerto_broker;
	char* ip_team;
	char* puerto_team;
	char* ip_gameCard;
	char* puerto_gameCard;
} t_config_game_boy;

t_log* logger;
t_config_game_boy* config_game_boy;
void iniciar_logger(void);
void leer_config(void);
void preparar_mensaje(int cantidad_parametros,char *parametros[]);
void terminar_programa(int, t_log*, t_config_game_boy*);
void liberar_conexion(int);
void mostrar_menu(void);
void liberar_logger(t_log* logger);
void liberar_config(t_config_game_boy*);
