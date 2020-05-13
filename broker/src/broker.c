#include "broker.h"

int main(void) {
	sem_init(&semaforo, 0, 1);
	iniciar_programa();
	enviar_mensajes_get();
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
		    	printf("No se pudo encontrar el path del config.");
		    	return exit(-2);
	}
	config_broker->size_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
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


//---------------------------------------------------------------------------------------------------------------------------

/*EL SERVICE DEL BROKER*/
void process_request(uint32_t cod_op, uint32_t cliente_fd) {
	uint32_t size;
	void* msg;

	log_info(logger,"Codigo de operacion %d",cod_op);

	switch (cod_op) {
		case GET_POKEMON:
			msg = malloc(sizeof(t_get_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(GET_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CATCH_POKEMON:
			msg = malloc(sizeof(t_catch_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(CATCH_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case LOCALIZED_POKEMON:
			msg = malloc(sizeof(t_localized_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(LOCALIZED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CAUGHT_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(CAUGHT_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case APPEARED_POKEMON:
			msg = malloc(sizeof(t_caught_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(APPEARED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case NEW_POKEMON:
			msg = malloc(sizeof(t_new_pokemon));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(NEW_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case SUBSCRIPTION:
			msg = malloc(sizeof(t_suscripcion));
			msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(SUBSCRIPTION, size, msg, cliente_fd);
			free(msg);
			break;
		case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
}

void* recibir_mensaje(uint32_t socket_cliente, uint32_t* size) {
	//t_paquete* paquete = malloc(sizeof(t_paquete));
	void* buffer;
	log_info(logger, "Recibiendo mensaje.");
	sem_wait(&semaforo);
	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
	log_info(logger, "Tamano de paquete recibido: %d", *size);
    sem_post(&semaforo);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	log_info(logger, "Mensaje recibido: %s", buffer);

	return buffer;
}

void agregar_mensaje(uint32_t cod_op, uint32_t size, void* payload, uint32_t socket_cliente){
	log_info(logger, "Agregando mensaje");
	log_info(logger, "Size: %d", size);
	log_info(logger, "Socket_cliente: %d", socket_cliente);
	log_info(logger, "Payload: %s", (char*) payload);
	t_paquete* paquete = malloc(sizeof(t_paquete));

	//Usar semaforos para incrementar
	//Notificar al proceso que lo mando con el id_mensaje generado
	paquete -> id_mensaje = id_mensaje_univoco++;
	paquete -> codigo_operacion = cod_op;
	paquete -> buffer = malloc(sizeof(t_buffer));
	paquete -> buffer -> size = size;
	paquete -> buffer -> stream = malloc(paquete -> buffer -> size);
	memcpy(paquete -> buffer -> stream, payload, paquete -> buffer -> size);

	send(socket_cliente, &(paquete -> id_mensaje) , sizeof(uint32_t), 0);

	uint32_t bytes = paquete -> buffer -> size + 2 * sizeof(uint32_t);
	void* a_agregar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_agregar, bytes, 0); // a donde se envía este paquete?

	sem_wait(&semaforo);
	if(cod_op == SUBSCRIPTION) {
		recibir_suscripcion(paquete, paquete -> buffer -> stream);
	} else {
		encolar_mensaje(paquete, paquete -> codigo_operacion);
	}
	sem_post(&semaforo);

	free(a_agregar);
	free(paquete -> buffer -> stream);
	free(paquete -> buffer);
	free(paquete);

}

void encolar_mensaje(t_paquete* paquete, op_code codigo_operacion){

	switch (codigo_operacion) {
			case GET_POKEMON:
				list_add(colas_de_mensajes -> cola_get, paquete);
				log_info(logger, "Mensaje agregado a cola de mensajes get.");
				break;
			case CATCH_POKEMON:
				list_add(colas_de_mensajes -> cola_catch, paquete);
				log_info(logger, "Mensaje agregado a cola de mensajes catch.");
				break;
			case LOCALIZED_POKEMON:
				list_add(colas_de_mensajes -> cola_localized, paquete);
				log_info(logger, "Mensaje agregado a cola de mensajes localized.");
				break;
			case CAUGHT_POKEMON:
				list_add(colas_de_mensajes -> cola_caught, paquete);
				log_info(logger, "Mensaje agregado a cola de mensajes caught.");
				break;
			case APPEARED_POKEMON:
				list_add(colas_de_mensajes -> cola_appeared, paquete);
				log_info(logger, "Mensaje agregado a cola de mensajes appeared.");
				break;
			case NEW_POKEMON:
				list_add(colas_de_mensajes -> cola_new, paquete);
				log_info(logger, "Mensaje agregado a cola de mensajes new.");
				break;
			/*case SUSCRIPTION:
				recibir_suscripcion(paquete);
				break;*/
				//El stream de una suscripción debería tener el socket del cliente.
			default:
				log_info(logger, "El codigo de operacion es invalido");
				exit (-6);
	}
}

void recibir_suscripcion(t_paquete* paquete, t_suscripcion* mensaje_suscripcion){

	//RECIBE BIEN LA SUSCRIPCION PERO NO SABE A QUE COLA.

	mensaje_suscripcion = malloc(sizeof(t_suscripcion));
	op_code cola_de_mensajes = mensaje_suscripcion -> cola_a_suscribir;
	uint32_t socket_cliente = mensaje_suscripcion -> socket;

	log_info(logger, "Se recibe una suscripción.");

	 switch (cola_de_mensajes) {
	 	 case GET_POKEMON:
	 		list_add(listas_de_suscriptos -> lista_suscriptores_get, &socket_cliente);
	 		log_info(logger, "EL cliente fue suscripto a la cola de mensajes get.");
	 		break;
	 	 case CATCH_POKEMON:
			list_add(listas_de_suscriptos -> lista_suscriptores_catch, &socket_cliente);
			log_info(logger, "EL cliente fue suscripto a la cola de mensajes catch.");
			break;
	 	 case LOCALIZED_POKEMON:
			list_add(listas_de_suscriptos -> lista_suscriptores_localized, &socket_cliente);
			log_info(logger, "EL cliente fue suscripto a la cola de mensajes localized.");
			break;
	 	 case CAUGHT_POKEMON:
			list_add(listas_de_suscriptos -> lista_suscriptores_caught, &socket_cliente);
			log_info(logger, "EL cliente fue suscripto a la cola de mensajes caught.");
			break;
	 	 case APPEARED_POKEMON:
			list_add(listas_de_suscriptos -> lista_suscriptores_appeared, &socket_cliente);
			log_info(logger, "EL cliente fue suscripto a la cola de mensajes appeared.");
			break;
	 	 /*case NEW_POKEMON:
			list_add(listas_de_suscriptos -> lista_suscriptores_new, &socket_cliente);
			log_info(logger, "EL cliente fue suscripto a la cola de mensajes new.");
			break;*/
	 	 default:
	 		log_info(logger, "Ingrese un codigo de operacion valido");
	 		break;
	 }

	 free(mensaje_suscripcion);
}


void enviar_mensajes_get(){
	//t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes get y se envía a todos los procesos suscriptos
	// a la cola get.
	//paquete_a_enviar = list_get(colas_de_mensajes -> cola_get, 0);
	//list_iterate(listas_de_suscriptos -> lista_suscriptores_get, ); El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	//free(paquete_a_enviar);
}
















