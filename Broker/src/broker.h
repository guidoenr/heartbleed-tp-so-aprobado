
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<utils.h>

typedef struct {
    int size_memoria;
	int size_min_memoria;
	char* algoritmo_memoria;
	char* algoritmo_reemplazo;
	char* algoritmo_particion_libre;
	char* ip_broker;
	char* puerto;
	int frecuencia_compactacion;
} t_config_broker;

t_log* logger;
t_log* iniciar_logger(void);
t_config_broker* leer_config(void);
void terminar_programa(t_log*, t_config_broker*);
void liberar_config(t_config_broker*);


