#include "broker.h"

int main(void) {
	sem_init(&semaforo, 0, 1);
	iniciar_programa();
	terminar_programa(logger, config_broker);
	return 0;
}

void iniciar_programa(){
	id_mensaje_univoco = 0;
	leer_config();
	iniciar_logger(config_broker->log_file, "broker");
	reservar_memoria();

	crear_colas_de_mensajes();
    crear_listas_de_suscriptores();
	log_info(logger, "IP: %s", config_broker -> ip_broker);
	iniciar_servidor(config_broker -> ip_broker, config_broker -> puerto);

}

void reservar_memoria(){
	uint32_t tamanio_memoria = config_broker ->  size_memoria;
	memoria_cache = malloc(tamanio_memoria);
	//SE RESERVA LA MEMORIA DINÁMICAMENTE-REVISAR 0_0
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
		    	exit(-2);
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
	liberar_memoria_cache();
	sem_destroy(&semaforo);
}

void liberar_memoria_cache(){
	free(memoria_cache -> mensaje);
	free(memoria_cache);
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
		case ACK:
			msg = malloc(sizeof(t_ack));
			uint32_t id_mensaje;
			op_code codigo = recibir_confirmacion_de_recepcion(cliente_fd, &id_mensaje);
			desencolar_mensaje(id_mensaje,codigo);
			free(msg);
		case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
	sem_destroy(&semaforo);///mmm dudoso
}

void* recibir_mensaje(uint32_t socket_cliente, uint32_t* size) {
	//t_paquete* paquete = malloc(sizeof(t_paquete));
	void* buffer;
	log_info(logger, "Recibiendo mensaje.");
	sem_wait(&semaforo);
	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
	log_info(logger, "Tamano de paquete recibido: %d", *size);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	sem_post(&semaforo);
	log_info(logger, "Mensaje recibido: %s", buffer);
    return buffer;
}

void agregar_mensaje(uint32_t cod_op, uint32_t size, void* payload, uint32_t socket_cliente){
	log_info(logger, "Agregando mensaje");
	log_info(logger, "Size: %d", size);
	log_info(logger, "Socket_cliente: %d", socket_cliente);
	log_info(logger, "Payload: %s", (char*) payload);
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete -> id_mensaje = generar_id_univoco();
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
	encolar_mensaje(paquete, paquete -> codigo_operacion);
	sem_post(&semaforo);


	free(a_agregar);
	free(paquete -> buffer -> stream);
	free(paquete -> buffer);
	free(paquete);

}

uint32_t generar_id_univoco(){
	pthread_mutex_t mutex_id_univoco = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex_id_univoco);
	id_mensaje_univoco++;
	pthread_mutex_unlock(&mutex_id_univoco);

	pthread_mutex_destroy(&mutex_id_univoco);

	return id_mensaje_univoco;
}

void encolar_mensaje(t_paquete* paquete, op_code codigo_operacion){

	t_mensaje *mensaje = malloc(sizeof(t_mensaje));
	mensaje->estado_mensaje = EN_ESPERA;
	mensaje->id = paquete->id_mensaje;
	mensaje->mensaje = paquete->buffer;

	switch (codigo_operacion) {
			case GET_POKEMON:
				list_add(colas_de_mensajes -> cola_get, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes get.");
				break;
			case CATCH_POKEMON:
				list_add(colas_de_mensajes -> cola_catch, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes catch.");
				break;
			case LOCALIZED_POKEMON:
				list_add(colas_de_mensajes -> cola_localized, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes localized.");
				break;
			case CAUGHT_POKEMON:
				list_add(colas_de_mensajes -> cola_caught, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes caught.");
				break;
			case APPEARED_POKEMON:
				list_add(colas_de_mensajes -> cola_appeared, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes appeared.");
				break;
			case NEW_POKEMON:
				list_add(colas_de_mensajes -> cola_new, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes new.");
				break;
			case SUBSCRIPTION:
				recibir_suscripcion(mensaje);
				break;
			default:
				log_info(logger, "El codigo de operacion es invalido");
				exit(-6);
	}
}

void recibir_suscripcion(t_paquete* paquete){

	t_suscripcion* mensaje_suscripcion = malloc(sizeof(t_suscripcion));
	mensaje_suscripcion = deserealizar_suscripcion(paquete -> buffer -> stream);

	log_info(logger, "Se recibe una suscripción.");
		switch (mensaje_suscripcion->cola_a_suscribir) {
			 case GET_POKEMON:
				suscribir_a_cola(listas_de_suscriptos -> lista_suscriptores_get, mensaje_suscripcion);
				break;
			 case CATCH_POKEMON:
				suscribir_a_cola(listas_de_suscriptos -> lista_suscriptores_catch, mensaje_suscripcion);
				break;
			 case LOCALIZED_POKEMON:
				suscribir_a_cola(listas_de_suscriptos -> lista_suscriptores_localized, mensaje_suscripcion);
				break;
			 case CAUGHT_POKEMON:
				suscribir_a_cola(listas_de_suscriptos -> lista_suscriptores_caught, mensaje_suscripcion);
				break;
			 case APPEARED_POKEMON:
				suscribir_a_cola(listas_de_suscriptos -> lista_suscriptores_appeared, mensaje_suscripcion);
				break;
			 case NEW_POKEMON:
				suscribir_a_cola(listas_de_suscriptos -> lista_suscriptores_new, mensaje_suscripcion);
				break;
			 default:
				log_info(logger, "Ingrese un codigo de operacion valido");
				break;
		 }

	 free(mensaje_suscripcion);

}

//REVISAR EL LIST REMOVE BY CONDITION (debería ser remove & destroy?).
//Ver de agregar threads.
void suscribir_a_cola(t_list* lista_suscriptores, t_suscripcion* suscripcion){

	list_add(lista_suscriptores, &suscripcion -> socket);
	log_info(logger, "EL cliente fue suscripto a la cola de mensajes:%s.", (char*)(suscripcion -> cola_a_suscribir));
	uint32_t socket_suscripto = suscripcion -> socket;

	bool es_la_misma_suscripcion(void* socket_suscripcion){
		return socket_suscripto == (uint32_t)socket_suscripcion;
	}

	//Nombre malisimo, hay que revisar.
	informar_mensajes_previos(suscripcion);

	if(suscripcion -> tiempo_suscripcion != 0){
		sleep(suscripcion -> tiempo_suscripcion);
		list_remove_by_condition(lista_suscriptores, es_la_misma_suscripcion);
		log_info(logger, "La suscripcion fue anulada correctamente.");
	}

}

//REVISAR SI LOS HISTORIALES ESTÁN BIEN RELACIONADOS
void informar_mensajes_previos(t_suscripcion* una_suscripcion){

	switch(una_suscripcion -> cola_a_suscribir){
		case GET_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(listas_de_suscriptos -> lista_suscriptores_get, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes get del historial");
			break;
		case CATCH_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(listas_de_suscriptos -> lista_suscriptores_catch, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes catch del historial");
			break;
		case LOCALIZED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(listas_de_suscriptos -> lista_suscriptores_localized, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes localized del historial");
			break;
		case CAUGHT_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(listas_de_suscriptos -> lista_suscriptores_caught, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes caught del historial");
			break;
		case NEW_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(listas_de_suscriptos -> lista_suscriptores_new, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes new del historial");
			break;
		case APPEARED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(listas_de_suscriptos -> lista_suscriptores_appeared, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes appeared del historial");
			break;
		default:
			log_info(logger, "No se pudo descargar el historial de mensajes satisfactoriamente.");
			break;
	}
}

void descargar_historial_mensajes(t_list* lista_suscripta, uint32_t socket_cliente){
//LA LISTA EN REALIDAD TIENE QUE SER EL CACHE DE ESA LISTA. Más adelante hay que cambiarlo cuando ya sepamos
// cómo es la estructura de la cache.
}

t_suscripcion* deserealizar_suscripcion(void* stream){
	t_suscripcion* suscripcion = malloc(sizeof(t_suscripcion));
	memcpy(&(suscripcion->socket), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(suscripcion->tiempo_suscripcion), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(suscripcion->cola_a_suscribir), stream, sizeof(op_code));
	stream += sizeof(op_code);
	return suscripcion;
}

void enviar_mensajes_get(){
	void enviar_mensaje_get(void* mensaje){
		t_mensaje* mensaje_a_enviar = malloc(sizeof(t_mensaje));
	    mensaje_a_enviar = mensaje;
	    if(mensaje_a_enviar == EN_ESPERA) {
		enviar_mensaje(GET_POKEMON, mensaje_a_enviar-> mensaje, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		free(mensaje_a_enviar);
	    }
	}
	list_iterate(listas_de_suscriptos -> lista_suscriptores_get, enviar_mensaje_get); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
}

/*REVISAR 0_0
void enviar_mensajes_catch(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes catch y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_catch, 0);

	void enviar_mensaje_catch(void* socket){
		enviar_mensaje(CATCH_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_catch, enviar_mensaje_catch); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);
}

void enviar_mensajes_localized(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes localized y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_localized, 0);

	void enviar_mensaje_localized(void* socket){
		enviar_mensaje(LOCALIZED_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_localized, enviar_mensaje_localized); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);

}

void enviar_mensajes_caught(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes caught y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_caught, 0);

	void enviar_mensaje_caught(void* socket){
		enviar_mensaje(CAUGHT_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_caught, enviar_mensaje_caught); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);
}

void enviar_mensajes_appeared(){
	t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
	//Se toma la cola de mensajes appeared y se envía a todos los procesos suscriptos
	// a la cola.
	paquete_a_enviar = list_get(colas_de_mensajes -> cola_appeared, 0);

	void enviar_mensaje_appeared(void* socket){
		enviar_mensaje(APPEARED_POKEMON, paquete_a_enviar, socket, sizeof(t_paquete)); //REVISAR SIZEOF
		recibir_confirmacion_de_recepcion(socket);
	}

	list_iterate(listas_de_suscriptos -> lista_suscriptores_appeared, enviar_mensaje_appeared); //El segundo parámetro es una operación que hace enviar a los sockets un paquete?
	free(paquete_a_enviar);
}*/

op_code recibir_confirmacion_de_recepcion(uint32_t socket_cliente, uint32_t id_mensaje){
	op_code acknowledgment;
	recv(socket_cliente, & acknowledgment, sizeof(op_code), MSG_WAITALL);
	recv(socket_cliente, id_mensaje, sizeof(uint32_t), MSG_WAITALL);
	return acknowledgment;
}



void desencolar_mensaje(uint32_t id_mensaje, op_code codigo_operacion){

 switch (codigo_operacion) {
			case GET_POKEMON:
				list_remove_and_destroy_element(colas_de_mensajes -> cola_get, id_mensaje, sizeof(t_mensaje));
				log_info(logger, "Mensaje eliminado a cola de mensajes get.");
				break;
			case CATCH_POKEMON:
				list_remove_and_destroy_element(colas_de_mensajes -> cola_catch, id_mensaje,sizeof(t_mensaje));
				log_info(logger, "Mensaje eliminado a cola de mensajes catch.");
				break;
			case LOCALIZED_POKEMON:
				list_remove_and_destroy_element(colas_de_mensajes -> cola_localized, id_mensaje,sizeof(t_mensaje));
				log_info(logger, "Mensaje eliminado a cola de mensajes localized.");
				break;
			case CAUGHT_POKEMON:
				list_remove_and_destroy_element(colas_de_mensajes -> cola_caught, id_mensaje,sizeof(t_mensaje));
				log_info(logger, "Mensaje eliminado a cola de mensajes caught.");
				break;
			case APPEARED_POKEMON:
				list_remove_and_destroy_element(colas_de_mensajes -> cola_appeared, id_mensaje,sizeof(t_mensaje));
				log_info(logger, "Mensaje eliminado a cola de mensajes appeared.");
				break;
			case NEW_POKEMON:
				list_remove_and_destroy_element(colas_de_mensajes -> cola_new, id_mensaje,sizeof(t_mensaje));
				log_info(logger, "Mensaje eliminado a cola de mensajes new.");
				break;
			/*case SUBSCRIPTION:
				recibir_suscripcion(mensaje);
				break;*/
			default:
				log_info(logger, "El codigo de operacion es invalido");
				exit(-6);
	}
}













