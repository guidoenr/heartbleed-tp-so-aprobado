#include "broker.h"


int main(void) {


	iniciar_programa();
	terminar_programa(logger, config_broker);

	 return 0;
}

void iniciar_programa(){
    leer_config();
	iniciar_logger(config_broker->log_file, "broker");

	crear_colas_de_mensajes();
    crear_listas_de_suscriptores();
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

	config = config_create("broker.config");

	if(config == NULL){
		    	log_info(logger,"No se pudo encontrar el path del config.");
		    	return exit(-2);
	}
	config_broker -> size_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> size_min_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> algoritmo_memoria = strdup(config_get_string_value(config, "ALGORITMO_MEMORIA"));
	config_broker -> algoritmo_reemplazo = strdup(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
	config_broker -> algoritmo_particion_libre = strdup(config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE"));
	config_broker -> ip_broker = strdup(config_get_string_value(config, "IP_BROKER"));
	config_broker -> puerto = strdup(config_get_string_value(config, "PUERTO_BROKER"));
	config_broker -> frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
	config_broker -> log_file = strdup(config_get_string_value(config, "LOG_FILE"));

	config_destroy(config);
}

void liberar_config(t_config_broker* config) {
	free(config -> algoritmo_memoria);
	free(config -> algoritmo_reemplazo);
	free(config -> algoritmo_particion_libre);
	free(config -> ip_broker);
	free(config -> puerto);
	free(config->log_file);
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
//---------------------------------------------------------------------------------------------------------------------------

/*EL SERVICE DEL BROKER*/
void process_request(int cod_op, int cliente_fd) {
	int size;
	void* msg;

	log_info(logger,"Codigo de operacion %d",cod_op);

	switch (cod_op) {
		case GET_POKEMON:
			msg = malloc(sizeof(t_get_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			//agregar_mensaje(GET_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CATCH_POKEMON:
			msg = malloc(sizeof(t_catch_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			 (CATCH_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case LOCALIZED_POKEMON:
			msg = malloc(sizeof(t_localized_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			//agregar_mensaje(LOCALIZED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CAUGHT_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			//agregar_mensaje(CAUGHT_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case APPEARED_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			//agregar_mensaje(APPEARED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case NEW_POKEMON:
			msg = malloc(sizeof(t_new_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			//agregar_mensaje(NEW_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case SUBSCRIPTION:
			msg = malloc(sizeof(t_new_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			//agregar_mensaje(SUBSCRIPTION, size, msg, cliente_fd);
			free(msg);
			break;

		case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
}

/*void* recibir_mensaje(int socket_cliente, int* size) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	void* buffer;
	int op_code;
	log_info(logger, "Recibiendo mensaje.");

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	log_info(logger, "Tamano de paquete recibido: %d", *size);

	recv(socket_cliente, op_code, sizeof(int), MSG_WAITALL);
	log_info(logger, "Codigo de operacion del mensaje recibido: %d", op_code);

	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	log_info(logger, "Mensaje recibido: %s", buffer);
	//El broker suma el mensaje recibido a la cola (creo que broker va a tener que tener su propio recibir mensaje).

	//paquete -> id_mensaje = id_mensje_univoco++;
	//paquete -> buffer -> size = *size;
	//paquete -> buffer -> stream = buffer;
	//Se puede usar el cod_op para saber a que lista se tiene que agregar.
	//encolar_mensaje(paquete, paquete -> codigo_operacion);

	return buffer;
}*/
















