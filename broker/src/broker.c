#include "broker.h"


int main(void) {


	iniciar_programa();
	//recibir_suscripcion(socket_cliente, lista_a_suscribir);
//	log_info(logger, "config leida");



//	log_info(logger, "terminar programa");
	terminar_programa(logger, config_broker);
//	log_info(logger, "programa terminado");

	 return 0;
}

void iniciar_programa(){

	iniciar_logger("broker.log", "broker");
	/*if(config == NULL){
		    	log_info(logger,"No se pudo encontrar el path del config.");
		    	return exit(-2);
	}*/

	leer_config();
	crear_colas_de_mensajes();

	log_info(logger, "IP: %s", config_broker -> ip_broker);

	iniciar_servidor(config_broker -> ip_broker, config_broker -> puerto);

}

void crear_colas_de_mensajes(){

	colas_de_mensajes = malloc(sizeof(t_colas_mensajes));

	colas_de_mensajes -> cola_new = list_create();
	colas_de_mensajes -> cola_appeared = list_create();
	colas_de_mensajes -> cola_get = list_create();
	colas_de_mensajes -> cola_localized = list_create();
	colas_de_mensajes -> cola_catch = list_create();
	colas_de_mensajes -> cola_caught = list_create();

	crear_listas_de_suscriptores();

}

void crear_listas_de_suscriptores(){
	listas_de_suscriptos = malloc(sizeof(t_listas_suscriptores));

	listas_de_suscriptos -> lista_suscriptores_new = list_create();
	listas_de_suscriptos -> lista_suscriptores_appeared = list_create();
	listas_de_suscriptos -> lista_suscriptores_get = list_create();
	listas_de_suscriptos -> lista_suscriptores_localized = list_create();
	listas_de_suscriptos -> lista_suscriptores_catch = list_create();
	listas_de_suscriptos -> lista_suscriptores_caught = list_create();
}

void leer_config() {

	t_config* config;

	config_broker = malloc(sizeof(t_config_broker));

	config = config_create("Debug/broker.config");


	config_broker -> size_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> size_min_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> algoritmo_memoria = config_get_string_value(config, "ALGORITMO_MEMORIA");
	config_broker -> algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	config_broker -> algoritmo_particion_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	config_broker -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_broker -> puerto = config_get_string_value(config, "PUERTO_BROKER");
	config_broker -> frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
	config_broker -> log_file = config_get_string_value(config, "LOG_FILE");

	config_destroy(config);
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
	liberar_listas();
	liberar_config(config);
	liberar_logger(logger);
}

void liberar_listas(){
	list_destroy(colas_de_mensajes -> cola_new);
	list_destroy(colas_de_mensajes -> cola_appeared);
	list_destroy(colas_de_mensajes -> cola_get);
	list_destroy(colas_de_mensajes -> cola_localized);
	list_destroy(colas_de_mensajes -> cola_catch);
	list_destroy(colas_de_mensajes -> cola_caught);
	list_destroy(listas_de_suscriptos -> lista_suscriptores_new);
	list_destroy(listas_de_suscriptos -> lista_suscriptores_appeared);
	list_destroy(listas_de_suscriptos -> lista_suscriptores_get);
	list_destroy(listas_de_suscriptos -> lista_suscriptores_localized);
	list_destroy(listas_de_suscriptos -> lista_suscriptores_catch);
	list_destroy(listas_de_suscriptos -> lista_suscriptores_caught);
}

void recibir_suscripcion(int socket_cliente, char* cola_de_mensajes){

	log_info(logger, "Se recibe una suscripción.");

	if (strcmp(cola_de_mensajes, "GET_POKEMON") == 0)
		list_add(listas_de_suscriptos -> lista_suscriptores_get, socket_cliente);
	//En realidad no debería dentificarse con sockets, hay que buscar una forma de identificar uniocamente al proceso.
	if (strcmp(cola_de_mensajes, "LOCALIZED_POKEMON") == 0)
		list_add(listas_de_suscriptos -> lista_suscriptores_localized, socket_cliente);
	if (strcmp(cola_de_mensajes, "NEW_POKEMON") == 0)
		list_add(listas_de_suscriptos -> lista_suscriptores_new, socket_cliente);
	if (strcmp(cola_de_mensajes, "APPEARED_POKEMON") == 0)
		list_add(listas_de_suscriptos -> lista_suscriptores_appeared, socket_cliente);
	if (strcmp(cola_de_mensajes, "CATCH_POKEMON") == 0)
		list_add(listas_de_suscriptos -> lista_suscriptores_catch, socket_cliente);
	if (strcmp(cola_de_mensajes, "CAUGHT_POKEMON") == 0)
			list_add(listas_de_suscriptos -> lista_suscriptores_caught, socket_cliente);

}

/*void encolar_mensaje(t_paquete* paquete, op_code codigo_operacion){

	switch (codigo_operacion) {
		case TE_GET_POKEMON_BR:
			list_add(colas_de_mensajes -> cola_get, paquete);
			break;
		case TE_CATCH_POKEMON_BR:
			list_add(colas_de_mensajes -> cola_catch, paquete);
			break;
		case GC_LOCALIZED_POKEMON_BR:
			list_add(colas_de_mensajes -> cola_localized, paquete);
			break;
		case GC_CAUGHT_POKEMON_BR:
			list_add(colas_de_mensajes -> cola_caught, paquete);
			break;
		case GB_NEW_POKEMON_BR:
			list_add(colas_de_mensajes -> cola_new, paquete);
			break;
		case GB_CAUGHT_POKEMON_BR:
			list_add(colas_de_mensajes -> cola_caught, paquete);
			break;
	}

}
*/
