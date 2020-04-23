#include "broker.h"

int main(void) {

	logger = iniciar_logger();
	t_config_broker* config = leer_config();
	log_info(logger, "IP: %s", config -> ip_broker);

	iniciar_servidor(config -> ip_broker, config -> puerto);

//	log_info(logger, "config leida");
//
//	log_info(logger, "terminar programa");
	terminar_programa(logger, config);
//	log_info(logger, "programa terminado");

	 return 0;
}

t_log* iniciar_logger(void) {
	t_log* logger = log_create("broker.log", "broker", 1, LOG_LEVEL_INFO);

	if(logger == NULL) {
		printf("fallo la creacion del logger\n");
		exit(1);
	}
	return logger;
}

t_config_broker* leer_config() {
	t_config* config;

	t_config_broker* config_broker = malloc(sizeof(t_config_broker));

	config = config_create("broker.config");

	config_broker -> size_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> size_min_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> algoritmo_memoria = strdup(config_get_string_value(config, "ALGORITMO_MEMORIA"));
	config_broker -> algoritmo_reemplazo = strdup(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
	config_broker -> algoritmo_particion_libre = strdup(config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE"));
	config_broker -> ip_broker = strdup(config_get_string_value(config, "IP_BROKER"));
	config_broker -> puerto = strdup(config_get_string_value(config, "PUERTO_BROKER"));
	config_broker -> frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");

	config_destroy(config);

	return config_broker;
}

void liberar_config(t_config_broker* config) {
	free(config -> algoritmo_memoria);
	free(config -> algoritmo_reemplazo);
	free(config -> algoritmo_particion_libre);
	free(config -> ip_broker);
	free(config -> puerto);
	free(config);
}

void terminar_programa(t_log* logger, t_config_broker* config) {
	liberar_config(config);
	liberar_logger(logger);
}
