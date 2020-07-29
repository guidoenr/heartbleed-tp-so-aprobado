#include "broker.h"

int main(void) {

	iniciar_programa();

	terminar_programa(logger);
	return 0;
}

void iniciar_programa() {
	id_mensaje_univoco = 0;
	particiones_liberadas = 0;
	numero_particion = 1;
	nodo_id = 0;

	iniciar_semaforos_broker();

	leer_config();
	iniciar_logger(config_broker -> log_file, "broker");
	reservar_memoria();
	crear_hilo_signal();
	crear_colas_de_mensajes();
	crear_listas_de_suscriptores();
	iniciar_servidor(config_broker -> ip_broker, config_broker -> puerto);
	
}

void crear_hilo_signal() {
	uint32_t err = pthread_create(&hilo_signal, NULL, main_hilo_signal, NULL);
	if(err != 0) {
		log_error(logger, "El hilo no pudo ser creado!!");
	}
	pthread_detach(hilo_signal);
}

void* main_hilo_signal() {

	signal(SIGUSR1, sig_handler);

	return NULL;
}

void sig_handler(uint32_t signo) {

	uint32_t un_signo = signo;
	if(un_signo == SIGUSR1){
		dump_de_memoria();
	}
}

void reservar_memoria() {

	memoria = malloc(config_broker ->  size_memoria);
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")) {
		arrancar_buddy();
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"PARTICIONES")) {
		memoria_con_particiones = list_create();
		iniciar_memoria_particiones(memoria_con_particiones);
	}

}

void crear_colas_de_mensajes() {

	cola_new = list_create();
	cola_appeared = list_create();
	cola_get = list_create();
	cola_localized = list_create();
	cola_catch = list_create();
	cola_caught = list_create();
}

void crear_listas_de_suscriptores() {

	lista_suscriptores_new = list_create();
	lista_suscriptores_appeared = list_create();
	lista_suscriptores_get = list_create();
	lista_suscriptores_localized = list_create();
	lista_suscriptores_catch = list_create();
	lista_suscriptores_caught = list_create();
}

void leer_config() {

	t_config* config;

	config_broker = malloc(sizeof(t_config_broker));

	config = config_create("Debug/broker.config");

	if(config == NULL){
		printf("No se pudo encontrar el path del config.");
		exit(-2);
	}
	config_broker -> size_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> size_min_memoria  = config_get_int_value(config, "TAMANO_MINIMO_PARTICION");
	config_broker -> algoritmo_memoria = strdup(config_get_string_value(config, "ALGORITMO_MEMORIA"));
	config_broker -> algoritmo_reemplazo = strdup(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
	config_broker -> algoritmo_particion_libre = strdup(config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE"));
	config_broker -> ip_broker = strdup(config_get_string_value(config, "IP_BROKER"));
	config_broker -> puerto = strdup(config_get_string_value(config, "PUERTO_BROKER"));
	config_broker -> frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
	config_broker -> log_file = strdup(config_get_string_value(config, "LOG_FILE"));
	config_broker -> memory_log = strdup(config_get_string_value(config, "DUMP_CACHE"));
}

void liberar_config(t_config_broker* config) {
	free(config -> algoritmo_memoria);
	free(config -> algoritmo_reemplazo);
	free(config -> algoritmo_particion_libre);
	free(config -> ip_broker);
	free(config -> puerto);
	free(config -> log_file);
	free(config -> memory_log);
	free(config);
}

void terminar_programa(t_log* logger) {
	liberar_listas();
	liberar_config(config_broker);
	liberar_logger(logger);
	liberar_memoria_cache();
	liberar_semaforos_broker();
	config_destroy(config);
}

void liberar_memoria_cache() {
	free(memoria);
}

void liberar_listas() {
	list_destroy(cola_new);
	list_destroy(cola_appeared);
	list_destroy(cola_get);
	list_destroy(cola_localized);
	list_destroy(cola_catch);
	list_destroy(cola_caught);
	list_destroy(lista_suscriptores_new);
	list_destroy(lista_suscriptores_appeared);
	list_destroy(lista_suscriptores_get);
	list_destroy(lista_suscriptores_localized);
	list_destroy(lista_suscriptores_catch);
	list_destroy(lista_suscriptores_caught);
}


//---------------------------------------------------------------------------------------------------------------------------

/*EL SERVICE DEL BROKER*/
void process_request(uint32_t cod_op, uint32_t cliente_fd) {
	uint32_t size;
	sem_wait(&muteadito);
	void* stream = recibir_paquete(cliente_fd, &size, &cod_op);
	void* mensaje_e_agregar = deserealizar_paquete(stream, cod_op, size);
	
	switch (cod_op) {
		case GET_POKEMON:
		agregar_mensaje(GET_POKEMON, size, mensaje_e_agregar, cliente_fd);
		break;
		case CATCH_POKEMON:
		agregar_mensaje(CATCH_POKEMON, size, mensaje_e_agregar, cliente_fd);
		break;
		case LOCALIZED_POKEMON:
		agregar_mensaje(LOCALIZED_POKEMON, size, mensaje_e_agregar, cliente_fd);
		break;
		case CAUGHT_POKEMON:
		agregar_mensaje(CAUGHT_POKEMON, size, mensaje_e_agregar, cliente_fd);
		break;
		case APPEARED_POKEMON:
		agregar_mensaje(APPEARED_POKEMON, size, mensaje_e_agregar, cliente_fd);
		break;
		case NEW_POKEMON:
		agregar_mensaje(NEW_POKEMON, size, mensaje_e_agregar, cliente_fd);
		break;
		case SUBSCRIPTION:
		recibir_suscripcion(mensaje_e_agregar,cliente_fd);
		break;
		case ACK:
		actualizar_mensajes_confirmados(mensaje_e_agregar);
		break;
		case 0:
		log_error(logger,"...No se encontro el tipo de mensaje en el process request.");
		pthread_exit(NULL);
		case -1:
		pthread_exit(NULL);
	}
	sem_post(&muteadito);
	
	//free(stream);

}

void agregar_mensaje(uint32_t cod_op, uint32_t size, void* mensaje, uint32_t socket_cliente) {
	t_mensaje* mensaje_a_agregar = malloc(sizeof(t_mensaje));
	uint32_t nuevo_id = generar_id_univoco();

	mensaje_a_agregar -> id_mensaje = nuevo_id;

	if(cod_op == APPEARED_POKEMON) {

		t_appeared_pokemon* mensaje_pokemon = mensaje;
		mensaje_a_agregar -> id_correlativo = mensaje_pokemon -> id_mensaje_correlativo;

	} else if(cod_op == LOCALIZED_POKEMON) {

		t_localized_pokemon* mensaje_pokemon = mensaje;
		mensaje_a_agregar -> id_correlativo = mensaje_pokemon -> id_mensaje_correlativo;

		t_localized_pokemon* mensaje2;
		t_list* para_parsear;
		uint32_t array[100];

		para_parsear = mensaje_pokemon->posiciones;
		t_link_element* cabeza = para_parsear -> head;


		for (int i = 0; i < para_parsear -> elements_count; i++) {
			array[i] = *(uint32_t*) cabeza->data;
			cabeza = cabeza->next;
		}

		para_parsear = list_create();

		for (int i=0; i< mensaje_pokemon->tamanio_lista; i++) {
			list_add(para_parsear,&array[i]);
		}

		mensaje_pokemon->posiciones = para_parsear;

	} else if(cod_op == CAUGHT_POKEMON) {

		t_caught_pokemon* mensaje_pokemon = mensaje;
		mensaje_a_agregar -> id_correlativo = mensaje_pokemon -> id_mensaje_correlativo;

	} else {
		mensaje_a_agregar -> id_correlativo = 0;
	}

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {
		mensaje_a_agregar -> payload =  malloc(sizeof(t_memoria_buddy));

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
		mensaje_a_agregar -> payload = malloc(sizeof(t_memoria_dinamica));

	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria en agregar mensaje.");
	}

	mensaje_a_agregar -> codigo_operacion = cod_op;
	mensaje_a_agregar -> suscriptor_enviado = list_create();
	mensaje_a_agregar -> suscriptor_recibido = list_create();
	mensaje_a_agregar -> tamanio_mensaje = obtener_tamanio_contenido_mensaje(mensaje, cod_op);

	if(cod_op == LOCALIZED_POKEMON) {
		t_localized_pokemon* localized = mensaje;
		mensaje_a_agregar -> tamanio_lista_localized = localized -> tamanio_lista;
	}

	send(socket_cliente, &(nuevo_id) , sizeof(uint32_t), 0); //Avisamos,che te asiganmos un id al mensaje

	if(puede_guardarse_mensaje(mensaje_a_agregar)) {
		encolar_mensaje(mensaje_a_agregar, cod_op);
		guardar_en_memoria(mensaje_a_agregar, mensaje);
	}
}

bool puede_guardarse_mensaje(t_mensaje* un_mensaje) {
	uint32_t no_se_repite_correlativo = 1;

	bool tiene_mismo_id_correlativo(void* mensaje) {
		t_mensaje* msg = mensaje;
		return (msg -> id_correlativo) == (un_mensaje -> id_correlativo);
	}

	switch(un_mensaje -> codigo_operacion) {
		case LOCALIZED_POKEMON:
		if(list_find(cola_localized, tiene_mismo_id_correlativo)) {
			no_se_repite_correlativo = 0;
		} else {
			no_se_repite_correlativo = 1;
		}
		break;
		case CAUGHT_POKEMON:
		if(list_find(cola_caught, tiene_mismo_id_correlativo)) {
			no_se_repite_correlativo = 0;
		} else {
			no_se_repite_correlativo = 1;
		}
		break;
		case APPEARED_POKEMON:
		if(list_find(cola_localized, tiene_mismo_id_correlativo)) {
			no_se_repite_correlativo = 0;
		} else {
			no_se_repite_correlativo = 1;
		}
		break;
		default:
		no_se_repite_correlativo = 1;
		break;
	}

	if(config_broker -> size_memoria < un_mensaje -> tamanio_mensaje){
		log_error(logger, "No se puede guardar el mensaje pues su tamanio es mayor al de la memoria.");
		no_se_repite_correlativo = 0;
	}

	return no_se_repite_correlativo;
}

uint32_t obtener_tamanio_contenido_mensaje(void* mensaje, uint32_t codigo) {
	uint32_t tamanio;
	t_get_pokemon* get;
	t_catch_pokemon* catch;
	t_localized_pokemon* localized;
	t_new_pokemon* new;
	t_appeared_pokemon* appeared;

	switch(codigo) {
		case GET_POKEMON:
		get = mensaje;
		tamanio = strlen(get -> pokemon);
		break;
		case CATCH_POKEMON:
		catch = mensaje;
		tamanio = strlen(catch -> pokemon) + (2*sizeof(uint32_t));
		break;
		case LOCALIZED_POKEMON:
		localized = mensaje;
		tamanio = strlen(localized -> pokemon) + sizeof(uint32_t) + (list_size(localized -> posiciones) * sizeof(uint32_t));
		break;
		case CAUGHT_POKEMON:
		tamanio = sizeof(uint32_t);
		break;
		case APPEARED_POKEMON:
		appeared = mensaje;
		tamanio = strlen(appeared -> pokemon) + (2*sizeof(uint32_t));
		break;
		case NEW_POKEMON:
		new = mensaje;
		tamanio = strlen(new -> pokemon) + (3*sizeof(uint32_t));
		break;
		default:
		log_error(logger, "...No se puede obtener el tamaño del contenido del mensaje.");
		tamanio = 0;
		break;
	}
	return tamanio;
}

uint32_t generar_id_univoco() {

	sem_wait(&muteadin);
	id_mensaje_univoco++;
	sem_post(&muteadin);

	return id_mensaje_univoco;
}

void encolar_mensaje(t_mensaje* mensaje, op_code codigo_operacion) {

	establecer_tiempo_de_carga(mensaje);

	switch (codigo_operacion) {
		case GET_POKEMON:
		enviar_mensajes(mensaje, lista_suscriptores_get);
		list_add(cola_get, mensaje);
		log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes get.");
		break;
		case CATCH_POKEMON:
		enviar_mensajes(mensaje, lista_suscriptores_catch);
		list_add(cola_catch, mensaje);
		log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes catch.");
		break;
		case LOCALIZED_POKEMON:
		enviar_mensajes(mensaje, lista_suscriptores_localized);
		list_add(cola_localized, mensaje);
		log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes localized.");
		break;
		case CAUGHT_POKEMON:
		enviar_mensajes(mensaje, lista_suscriptores_caught);
		list_add(cola_caught, mensaje);
		log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes caught.");
		break;
		case APPEARED_POKEMON:
		enviar_mensajes(mensaje, lista_suscriptores_appeared);
		list_add(cola_appeared, mensaje);
		log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes appeared.");
		break;
		case NEW_POKEMON:
		enviar_mensajes(mensaje, lista_suscriptores_new);
		list_add(cola_new, mensaje);
		log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes new.");
		break;
		default:
		log_error(logger, "...El codigo de operacion es invalido en encolar mensaje.");
		exit(-6);
	}
}

//-----------------------SUSCRIPCIONES------------------------//
void recibir_suscripcion(t_suscripcion* mensaje_suscripcion,uint32_t socket) {

	op_code cola_a_suscribir = mensaje_suscripcion -> cola_a_suscribir;
	mensaje_suscripcion->socket= socket;
	switch (cola_a_suscribir) {
		case GET_POKEMON:
		suscribir_a_cola(lista_suscriptores_get, mensaje_suscripcion, cola_a_suscribir);
		break;

		case CATCH_POKEMON:
		suscribir_a_cola(lista_suscriptores_catch, mensaje_suscripcion, cola_a_suscribir);
		break;

		case LOCALIZED_POKEMON:
		suscribir_a_cola(lista_suscriptores_localized, mensaje_suscripcion, cola_a_suscribir);
		break;

		case CAUGHT_POKEMON:
		suscribir_a_cola(lista_suscriptores_caught, mensaje_suscripcion, cola_a_suscribir);
		break;

		case APPEARED_POKEMON:
		suscribir_a_cola(lista_suscriptores_appeared, mensaje_suscripcion, cola_a_suscribir);
		break;

		case NEW_POKEMON:
		suscribir_a_cola(lista_suscriptores_new, mensaje_suscripcion, cola_a_suscribir);
		break;

		default:
		log_info(logger, "...La suscripcion no se puede realizar.");
		break;
	}
}

void suscribir_a_cola(t_list* lista_suscriptores, t_suscripcion* suscripcion, op_code cola_a_suscribir) {

	char* cola;
	switch(cola_a_suscribir) {
		case GET_POKEMON:
		cola = "get";
		break;
		case CATCH_POKEMON:
		cola = "catch";
		break;
		case LOCALIZED_POKEMON:
		cola = "localized";
		break;
		case CAUGHT_POKEMON:
		cola = "caught";
		break;
		case APPEARED_POKEMON:
		cola = "appeared";
		break;
		case NEW_POKEMON:
		cola = "new";
		break;
		default:
		log_error(logger, "...Se desconoce la cola a suscribir.");
		break;
	}

	list_add(lista_suscriptores, suscripcion);
	log_info(logger, "El cliente fue suscripto a la cola de mensajes %s.", cola);
	
	informar_mensajes_previos(suscripcion, cola_a_suscribir);

	bool es_la_misma_suscripcion(void* una_suscripcion) {
		t_suscripcion* otra_suscripcion = una_suscripcion;
		return otra_suscripcion -> id_proceso == suscripcion -> id_proceso;
	}

	if(suscripcion -> tiempo_suscripcion != 0) {
		list_remove_by_condition(lista_suscriptores, es_la_misma_suscripcion);
		log_info(logger, "La suscripcion temporal fue anulada correctamente.");
	}

}

void informar_mensajes_previos(t_suscripcion* una_suscripcion, op_code cola_a_suscribir) {

	char* cola;
	switch(cola_a_suscribir) {
			case GET_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(GET_POKEMON, una_suscripcion -> socket, una_suscripcion -> id_proceso);
			cola = "get";
			break;
			case CATCH_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(CATCH_POKEMON, una_suscripcion -> socket, una_suscripcion -> id_proceso);
			cola = "catch";
			break;
			case LOCALIZED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(LOCALIZED_POKEMON, una_suscripcion -> socket, una_suscripcion -> id_proceso);
			cola = "localized";
			break;
			case CAUGHT_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(CAUGHT_POKEMON, una_suscripcion -> socket, una_suscripcion -> id_proceso);
			cola = "caught";
			break;
			case NEW_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(NEW_POKEMON, una_suscripcion -> socket, una_suscripcion -> id_proceso);
			cola = "new";
			break;
			case APPEARED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(APPEARED_POKEMON, una_suscripcion -> socket, una_suscripcion -> id_proceso);
			cola = "appeared";
			break;
			default:
			log_error(logger, "...No se pudo descargar el historial de mensajes satisfactoriamente.");
			break;
		}

		log_info(logger, "El suscriptor %d recibe los mensajes del historial de la cola %s", una_suscripcion -> id_proceso, cola);
}

void descargar_historial_mensajes(op_code tipo_mensaje, uint32_t socket_cliente, uint32_t id_proceso) {

	void mandar_mensajes_viejos(void* mensaje) {
		t_mensaje* un_mensaje = mensaje;
		uint32_t size = 0;

		if(id_proceso < 14000) {

			void* mensaje_a_enviar = preparar_mensaje(un_mensaje);
			size = size_mensaje(mensaje_a_enviar, tipo_mensaje);
			enviar_mensaje(tipo_mensaje, mensaje_a_enviar, socket_cliente, size);
			agregar_suscriptor_a_enviados_sin_confirmar(un_mensaje, id_proceso);

		}
		actualizar_ultima_referencia(un_mensaje);
	}

	switch(tipo_mensaje) {
		case GET_POKEMON:
			list_iterate(cola_get, mandar_mensajes_viejos);

		break;
		case CATCH_POKEMON:
			list_iterate(cola_catch, mandar_mensajes_viejos);

		break;
		case LOCALIZED_POKEMON:
			list_iterate(cola_localized, mandar_mensajes_viejos);

		break;
		case CAUGHT_POKEMON:
			list_iterate(cola_caught, mandar_mensajes_viejos);
		break;
		case APPEARED_POKEMON:
			list_iterate(cola_appeared, mandar_mensajes_viejos);

		break;
		case NEW_POKEMON:
			list_iterate(cola_new, mandar_mensajes_viejos);
		break;
		default:
		log_info(logger, "...No se puede enviar el mensaje pedido en descargar historial mensajes.");
		break;
	}
}

void* preparar_mensaje(t_mensaje* un_mensaje) {
	void* mensaje_armado;

	switch(un_mensaje -> codigo_operacion) {
		case GET_POKEMON:
		mensaje_armado = preparar_mensaje_get(un_mensaje);
		break;
		case CATCH_POKEMON:
		mensaje_armado = preparar_mensaje_catch(un_mensaje);
		break;
		case LOCALIZED_POKEMON:
		mensaje_armado = preparar_mensaje_localized(un_mensaje);
		break;
		case CAUGHT_POKEMON:
		mensaje_armado = preparar_mensaje_caught(un_mensaje);
		break;
		case APPEARED_POKEMON:
		mensaje_armado = preparar_mensaje_appeared(un_mensaje);
		break;
		case NEW_POKEMON:
		mensaje_armado = preparar_mensaje_new(un_mensaje);
		break;
		default:
		log_error(logger, "... El broker no puede preparar el mensaje para enviarlo a otro modulo.");
		break;
	}

	return mensaje_armado;
}

t_get_pokemon* preparar_mensaje_get(t_mensaje* mensaje) {
	uint32_t tamanio;
	t_get_pokemon* mensaje_get = malloc(sizeof(t_get_pokemon));

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {

		t_memoria_dinamica* particion_del_mensaje = mensaje -> payload;	
		mensaje_get -> id_mensaje = mensaje -> id_mensaje;
		tamanio = particion_del_mensaje -> tamanio;
		mensaje_get -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_get -> pokemon, particion_del_mensaje -> contenido, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_get -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {

		t_memoria_buddy* buddy_del_mensaje = mensaje -> payload;
		mensaje_get -> id_mensaje = mensaje -> id_mensaje;
		tamanio = buddy_del_mensaje -> tamanio_mensaje;
		mensaje_get -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_get -> pokemon, buddy_del_mensaje -> contenido, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_get -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria para preparar el mensaje get.");
	}

	return mensaje_get;
}

t_catch_pokemon* preparar_mensaje_catch(t_mensaje* un_mensaje) {
	uint32_t tamanio;
	t_catch_pokemon* mensaje_catch = malloc(sizeof(t_catch_pokemon));
	void* contenido_a_enviar;

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_catch -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  particion_del_mensaje -> tamanio - sizeof(uint32_t) * 2;
		contenido_a_enviar = particion_del_mensaje -> contenido;
		mensaje_catch -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_catch -> pokemon, contenido_a_enviar, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_catch -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
		memcpy(&(mensaje_catch -> posicion[0]), contenido_a_enviar + tamanio , sizeof(uint32_t));
		memcpy(&(mensaje_catch -> posicion[1]), contenido_a_enviar + tamanio + sizeof(uint32_t), sizeof(uint32_t));
		
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {
		t_memoria_buddy* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_catch -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  buddy_del_mensaje -> tamanio_mensaje - sizeof(uint32_t) * 2;
		contenido_a_enviar = buddy_del_mensaje -> contenido;
		mensaje_catch -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_catch -> pokemon, contenido_a_enviar, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_catch -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
		memcpy(&(mensaje_catch -> posicion[0]), contenido_a_enviar + tamanio , sizeof(uint32_t));
		memcpy(&(mensaje_catch -> posicion[1]), contenido_a_enviar + tamanio + sizeof(uint32_t), sizeof(uint32_t));
	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria para preparar el mensaje catch.");
	}

	return mensaje_catch;
}

t_localized_pokemon* preparar_mensaje_localized(t_mensaje* un_mensaje) {
	uint32_t tamanio;

	t_localized_pokemon* mensaje_localized = malloc(sizeof(t_localized_pokemon));

	uint32_t offset = 0;
	void* contenido_a_enviar;

	uint32_t posicion[un_mensaje->tamanio_lista_localized];

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {

		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_localized -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_localized -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		tamanio = (particion_del_mensaje -> tamanio) - ((un_mensaje -> tamanio_lista_localized)*sizeof(uint32_t)) - sizeof(uint32_t);
		contenido_a_enviar = particion_del_mensaje -> contenido;
		mensaje_localized -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_localized -> pokemon, contenido_a_enviar, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_localized -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3

		mensaje_localized -> tamanio_lista = un_mensaje -> tamanio_lista_localized;
		mensaje_localized -> posiciones = list_create();
		offset+=tamanio + sizeof(uint32_t);

		if (mensaje_localized->posiciones->elements_count > 0) {
			for(int i=0;i<(un_mensaje -> tamanio_lista_localized);i++) {
				memcpy(&(posicion[i]), contenido_a_enviar + offset, sizeof(uint32_t));
				log_warning(logger,"ALGO PARA MOSTRAR NUMERITO : %d",posicion[i]);
				offset += sizeof(uint32_t);
				list_add(mensaje_localized -> posiciones, &posicion[i]);
			}
		}

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {

		t_memoria_buddy* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_localized -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_localized -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		tamanio = (buddy_del_mensaje -> tamanio_mensaje) - ((un_mensaje -> tamanio_lista_localized)*sizeof(uint32_t))- sizeof(uint32_t);
		contenido_a_enviar = buddy_del_mensaje -> contenido;
		mensaje_localized -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_localized -> pokemon, contenido_a_enviar, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_localized -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
		mensaje_localized -> tamanio_lista = un_mensaje -> tamanio_lista_localized;
		mensaje_localized -> posiciones = list_create();
		offset+=tamanio + sizeof(uint32_t);

		for(int i=0;i<(un_mensaje -> tamanio_lista_localized);i++) {
			memcpy(&(posicion[i]), contenido_a_enviar + offset, sizeof(uint32_t));
			log_warning(logger,"ALGO PARA MOSTRAR NUMERITO : %d",posicion[i]);
			offset += sizeof(uint32_t);
			list_add(mensaje_localized -> posiciones, &posicion[i]);
		}
	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria para preparar el mensaje localized.");
	}
	return mensaje_localized;
}

t_caught_pokemon* preparar_mensaje_caught(t_mensaje* un_mensaje) {
	t_caught_pokemon* mensaje_caught = malloc(sizeof(t_caught_pokemon));

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_caught -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_caught -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		memcpy(&(mensaje_caught -> resultado), particion_del_mensaje -> contenido, sizeof(uint32_t));

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {
		t_memoria_buddy* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_caught -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_caught -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		memcpy(&(mensaje_caught -> resultado), buddy_del_mensaje -> contenido, sizeof(uint32_t));

	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria para preparar el mensaje caught.");
	}
	return mensaje_caught;
}

t_new_pokemon* preparar_mensaje_new(t_mensaje* un_mensaje) {
	uint32_t tamanio;
	t_new_pokemon* mensaje_new = malloc(sizeof(t_new_pokemon));

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {

		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_new -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  particion_del_mensaje -> tamanio - sizeof(uint32_t) * 3;
		mensaje_new -> pokemon = malloc(tamanio+1);

		memcpy(mensaje_new -> pokemon, particion_del_mensaje -> contenido, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_new -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
		memcpy(&(mensaje_new -> posicion[0]), (particion_del_mensaje -> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_new -> posicion[1]), (particion_del_mensaje -> contenido) + tamanio + sizeof(uint32_t), sizeof(uint32_t));
		memcpy(&(mensaje_new -> cantidad), ((particion_del_mensaje -> contenido) + tamanio + sizeof(uint32_t)*2), sizeof(uint32_t));

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {

		t_memoria_buddy* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_new -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  buddy_del_mensaje  -> tamanio_mensaje - sizeof(uint32_t) * 3;
		mensaje_new -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_new -> pokemon, buddy_del_mensaje -> contenido, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_new -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
		memcpy(&(mensaje_new -> posicion[0]), (buddy_del_mensaje -> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_new -> posicion[1]), (buddy_del_mensaje -> contenido) + tamanio + sizeof(uint32_t), sizeof(uint32_t));
		memcpy(&(mensaje_new -> cantidad), ((buddy_del_mensaje -> contenido) + tamanio + sizeof(uint32_t)*2), sizeof(uint32_t));

	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria para preparar el mensaje new.");
	}
	return mensaje_new;
}

t_appeared_pokemon* preparar_mensaje_appeared(t_mensaje* un_mensaje) {
	uint32_t tamanio;
	t_appeared_pokemon* mensaje_appeared = malloc(sizeof(t_appeared_pokemon));

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_appeared -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_appeared -> id_mensaje_correlativo = un_mensaje -> id_correlativo;

		tamanio =  particion_del_mensaje -> tamanio - sizeof(uint32_t) * 2;
		mensaje_appeared -> pokemon = malloc(tamanio+1);

		memcpy(mensaje_appeared -> pokemon, particion_del_mensaje -> contenido, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_appeared -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
		memcpy(&(mensaje_appeared -> posicion[0]), (particion_del_mensaje -> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_appeared -> posicion[1]), ((particion_del_mensaje -> contenido) + tamanio + sizeof(uint32_t)), sizeof(uint32_t));

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {
		t_memoria_buddy* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_appeared -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_appeared -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		tamanio =  buddy_del_mensaje -> tamanio_mensaje - sizeof(uint32_t) * 2;
		mensaje_appeared -> pokemon = malloc(tamanio+1);
		memcpy(mensaje_appeared -> pokemon, buddy_del_mensaje -> contenido, tamanio);
		char barra_cero = '\0';
		memcpy(mensaje_appeared -> pokemon + tamanio, &barra_cero, 1); //STRING APPEND FURIO3
		memcpy(&(mensaje_appeared -> posicion[0]), (buddy_del_mensaje -> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_appeared -> posicion[1]), ((buddy_del_mensaje -> contenido) + tamanio + sizeof(uint32_t)), sizeof(uint32_t));

	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria para preparar el mensaje appeared.");
	}
	return mensaje_appeared;
}

void actualizar_ultima_referencia(t_mensaje* un_mensaje) {

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
		((t_memoria_dinamica*) un_mensaje -> payload) -> ultima_referencia = timestamp();

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {
		((t_memoria_buddy*) un_mensaje -> payload) -> ultima_referencia = timestamp();

	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria al actualizar la ultima referencia.");
	}

}

void establecer_tiempo_de_carga(t_mensaje* un_mensaje) {

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
		((t_memoria_dinamica*) un_mensaje -> payload) -> tiempo_de_carga = timestamp();

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {
		((t_memoria_buddy*) un_mensaje -> payload) -> tiempo_de_carga = timestamp();

	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria al establecer el tiempo de carga.");
	}
	actualizar_ultima_referencia(un_mensaje);
}


//---------------------------MENSAJES---------------------------//

void actualizar_mensajes_confirmados(t_ack* mensaje_confirmado) {

	log_info(logger, "Se recibio un ACK del mensaje con id %d", mensaje_confirmado -> id_mensaje);

	void actualizar_suscripto(void* mensaje) {
		t_mensaje* mensaje_ok = mensaje;
		if(mensaje_ok -> id_mensaje == mensaje_confirmado -> id_mensaje) {
			agregar_suscriptor_a_enviados_confirmados(mensaje_ok,  mensaje_confirmado -> id_proceso);
		} 
	}

	bool es_la_misma_suscripcion(void* una_suscripcion) {
		t_suscripcion* otra_suscripcion = una_suscripcion;
		return otra_suscripcion -> id_proceso == mensaje_confirmado -> id_proceso;
	}

	switch(mensaje_confirmado -> tipo_mensaje) {
		case GET_POKEMON:
		if(!list_any_satisfy(lista_suscriptores_get, es_la_misma_suscripcion)) {
			log_info(logger, "El ACK recibido es de un mensaje que no se encuentra en memoria.");
		} else {
			list_iterate(cola_get, actualizar_suscripto);
				borrar_mensajes_confirmados(GET_POKEMON, cola_get, lista_suscriptores_get);
		}
		break;

		case CATCH_POKEMON:

		if(!list_any_satisfy(lista_suscriptores_catch, es_la_misma_suscripcion)) {
			log_info(logger, "El ACK recibido es de un mensaje que no se encuentra en memoria.");
		} else {
			list_iterate(cola_catch, actualizar_suscripto);
				borrar_mensajes_confirmados(CATCH_POKEMON, cola_catch, lista_suscriptores_catch);
		}			
		break;

		case LOCALIZED_POKEMON:
		if(!list_any_satisfy(lista_suscriptores_localized, es_la_misma_suscripcion)) {
			log_info(logger, "El ACK recibido es de un mensaje que no se encuentra en memoria.");
		} else {
			list_iterate(cola_localized, actualizar_suscripto);
				borrar_mensajes_confirmados(LOCALIZED_POKEMON, cola_localized, lista_suscriptores_localized);;
		}			
		break;

		case CAUGHT_POKEMON:
		if(!list_any_satisfy(lista_suscriptores_caught, es_la_misma_suscripcion)) {
			log_info(logger, "El ACK recibido es de un mensaje que no se encuentra en memoria.");
		} else {
			list_iterate(cola_caught, actualizar_suscripto);
				borrar_mensajes_confirmados(CAUGHT_POKEMON, cola_caught, lista_suscriptores_caught);
		}
		break;

		case APPEARED_POKEMON:
		if(!list_any_satisfy(lista_suscriptores_appeared, es_la_misma_suscripcion)) {
			log_info(logger, "El ACK recibido es de un mensaje que no se encuentra en memoria.");
		} else {
			list_iterate(cola_appeared, actualizar_suscripto);
				borrar_mensajes_confirmados(APPEARED_POKEMON, cola_appeared, lista_suscriptores_appeared);
		}			
		break;

		case NEW_POKEMON:
		if(!list_any_satisfy(lista_suscriptores_appeared, es_la_misma_suscripcion)) {
			log_info(logger, "El ACK recibido es de un mensaje que no se encuentra en memoria.");
		} else {
			list_iterate(cola_new, actualizar_suscripto);
				borrar_mensajes_confirmados(NEW_POKEMON, cola_new, lista_suscriptores_new);
		}
		break;

		default:
		log_error(logger, "...El mensaje de id %d no se encuentra disponible en actualizar mensajes confirmados.", mensaje_confirmado -> id_mensaje);
		break;
	}
}

void borrar_mensajes_confirmados(op_code tipo_lista, t_list* cola_mensajes, t_list* suscriptores) {

	t_mensaje* mensaje_a_borrar;
	void borrar_mensaje_recibido_por_todos(void* mensaje) {
		t_mensaje* un_mensaje = mensaje;

		bool es_el_mismo_mensaje(void* msj) {
			t_mensaje* mensaje_buscado = msj;
			return (mensaje_buscado -> id_mensaje) == (un_mensaje -> id_mensaje);
		}

		if(mensaje_recibido_por_todos(un_mensaje, suscriptores)){
			mensaje_a_borrar = list_find(cola_mensajes, es_el_mismo_mensaje);

			if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
				eliminar_mensaje(mensaje_a_borrar);
			} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")) {
				limpiar_buddy((t_memoria_buddy*) mensaje_a_borrar -> payload);
			}
		}
	}

	list_iterate(cola_mensajes, borrar_mensaje_recibido_por_todos);
}

bool mensaje_recibido_por_todos(void* mensaje, t_list* suscriptores) {
	t_mensaje* un_mensaje = mensaje;

	void* id_suscriptor(void* un_suscriptor) {
		t_suscripcion* suscripto = un_suscriptor;
		uint32_t* id = &(suscripto -> id_proceso);
		return id;
	}

	t_list* lista_id_suscriptores = list_map(suscriptores, id_suscriptor);

	bool suscriptor_recibio_mensaje(void* suscripto){
		uint32_t* un_suscripto = suscripto;

		bool es_el_mismo_suscripto(void* id_suscripto){
			uint32_t* alguna_suscripcion = id_suscripto;
			return (*alguna_suscripcion) == (*un_suscripto);
		}

		return list_any_satisfy(un_mensaje -> suscriptor_recibido, es_el_mismo_suscripto);
	}

	return list_all_satisfy(lista_id_suscriptores, suscriptor_recibio_mensaje);
}

uint32_t eliminar_suscriptor_de_enviados_sin_confirmar(t_mensaje* mensaje, uint32_t suscriptor){

	bool es_el_mismo_suscriptor(void* un_suscripto) {
		return suscriptor == *(uint32_t*) un_suscripto;
	}

	list_remove_by_condition(mensaje -> suscriptor_enviado, es_el_mismo_suscriptor);

	/*if(!list_any_satisfy(mensaje -> suscriptor_enviado, es_el_mismo_suscriptor)){
		log_info(logger, "...Se eliminó al suscriptor de la lista de enviados sin confirmar.");
	} else {
		log_error(logger, "...No se eliminó al suscriptor confirmado.");
	}*/
	return suscriptor;
}

void agregar_suscriptor_a_enviados_confirmados(t_mensaje* mensaje, uint32_t confirmacion){
	list_add(mensaje -> suscriptor_recibido, &confirmacion);
}

void enviar_mensajes(t_mensaje* un_mensaje, t_list* lista_suscriptores){ // hilo

	void mandar_mensaje(void* suscriptor) {
		t_suscripcion* un_suscriptor = suscriptor;
		main_hilo_mensaje(un_suscriptor, un_mensaje);
	}
	list_iterate(lista_suscriptores, mandar_mensaje);
}

void main_hilo_mensaje(t_suscripcion* un_suscriptor, t_mensaje* un_mensaje) {

	void* mensaje_a_enviar = preparar_mensaje(un_mensaje);
	uint32_t tamanio_mensaje = size_mensaje(mensaje_a_enviar, un_mensaje -> codigo_operacion);

	enviar_mensaje(un_mensaje -> codigo_operacion, mensaje_a_enviar, un_suscriptor -> socket, tamanio_mensaje);

	actualizar_ultima_referencia(un_mensaje);
	agregar_suscriptor_a_enviados_sin_confirmar(un_mensaje, un_suscriptor -> id_proceso);

	log_info(logger, "Se envia un mensaje al suscriptor %d", un_suscriptor -> id_proceso);
}

bool tiene_el_mensaje(t_list* enviados, uint32_t un_suscripto) {
	bool mensaje_enviado;

	bool es_el_mismo_suscripto(void* suscripto) {
		return *(uint32_t*)suscripto == un_suscripto;
	}

	mensaje_enviado  = list_any_satisfy(enviados, es_el_mismo_suscripto);

	return mensaje_enviado;
}

void agregar_suscriptor_a_enviados_sin_confirmar(t_mensaje* mensaje_enviado, uint32_t un_suscriptor) {
	list_add(mensaje_enviado -> suscriptor_enviado, &un_suscriptor);
}

//--------------------- MEMORIA --------------------//

void dump_de_memoria(){

	log_info(logger, "Se solicito el dump de cache.");
	iniciar_logger_broker(config_broker -> memory_log, "broker");

    //Muestra el inicio del dump.
	time_t t;
	struct tm *tm;
	char fechayhora[25];
	t = time(NULL);
	tm = localtime(&t);
	strftime(fechayhora, 25, "%d/%m/%Y %H:%M:%S", tm);

	log_info(logger_memoria, "Dump: %s\n", fechayhora);

	sem_wait(&muteadito);
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		list_iterate(memoria_con_particiones, dump_info_particion);
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		list_iterate(memoria_cache, dump_info_buddy);

	} else {
		log_error(logger_memoria, "...No se reconoce el algoritmo de memoria a implementar en dump de memoria.");
	}
	sem_post(&muteadito);
	numero_particion = 0;
}

void iniciar_logger_broker(char* file, char* program_name) {

	if(file == NULL){
		printf("No se pudo encontrar el path del config.");
		return exit(-1);
	}

	logger_memoria = log_create(file, program_name, 1, LOG_LEVEL_INFO);

	if (logger_memoria == NULL){
		printf("ERROR EN LA CREACION DEL LOGGER/n");
		exit(-2);
	}
}


void guardar_en_memoria(t_mensaje* mensaje, void* mensaje_original) {

	if(mensaje->tamanio_mensaje > config_broker -> size_memoria){
		log_error(logger,"No ha sido posible guardar el mensaje, ya que es mas grande que la memoria");
	} else{
		void* contenido = armar_contenido_de_mensaje(mensaje_original, mensaje -> codigo_operacion);

		if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")) {
			uint32_t exponente = determinar_exponente(mensaje);
			recorrer_segun_algoritmo(exponente, mensaje, contenido);

		} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")) {
			guardar_particion(mensaje, contenido);
		}
		//free(contenido);
	}
}

void recorrer_segun_algoritmo(uint32_t exponente, t_mensaje* mensaje, void* contenido) {
	t_memoria_buddy* buddy_victima; // tanto vacia como reemplazable

	if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre,"FF")){
		buddy_victima = recorrer_first_fit(exponente);
		guardar_buddy(buddy_victima, mensaje, contenido);

	} else if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre, "BF")){
		buddy_victima = recorrer_best_fit(exponente);
		guardar_buddy(buddy_victima, mensaje, contenido);
	}
}

void* armar_contenido_de_mensaje(void* mensaje, uint32_t codigo){
	void* contenido;

	switch(codigo){
		case GET_POKEMON:
		contenido = armar_contenido_get(mensaje);
		break;
		case CATCH_POKEMON:
		contenido = armar_contenido_catch(mensaje);
		break;
		case LOCALIZED_POKEMON:
		contenido = armar_contenido_localized(mensaje);
		break;
		case APPEARED_POKEMON:
		contenido = armar_contenido_appeared(mensaje);
		break;
		case NEW_POKEMON:
		contenido = armar_contenido_new(mensaje);
		break;
		case CAUGHT_POKEMON:
		contenido = armar_contenido_caught(mensaje);
		break;
		default:
		log_error(logger, "...No se puede armar el contenido del mensaje.");
		break;
	}
	return contenido;
}


void* armar_contenido_get(t_get_pokemon* mensaje){
	uint32_t tamanio_pokemon = strlen(mensaje -> pokemon);
	void* contenido = malloc(tamanio_pokemon);

	memcpy(contenido, (mensaje -> pokemon), tamanio_pokemon);

	return contenido;
}

void* armar_contenido_catch(t_catch_pokemon* mensaje){
	uint32_t tamanio_pokemon = strlen(mensaje -> pokemon);
	uint32_t tamanio = tamanio_pokemon + (sizeof(uint32_t) * 2);
	void* contenido = malloc(tamanio);
	uint32_t offset = 0;

	memcpy(contenido + offset, (mensaje -> pokemon), tamanio_pokemon);
	offset += tamanio_pokemon;

	memcpy(contenido + offset, &(mensaje -> posicion[0]), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(contenido + offset, &(mensaje -> posicion[1]), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	return contenido;
}

void* armar_contenido_localized(t_localized_pokemon* mensaje){
	uint32_t tamanio_pokemon = strlen(mensaje -> pokemon);
	uint32_t tamanio_lista = list_size(mensaje -> posiciones) * sizeof(uint32_t);
	uint32_t tamanio = tamanio_pokemon + tamanio_lista + sizeof(uint32_t);
	void* contenido = malloc(tamanio);
	uint32_t offset = 0;

	memcpy(contenido + offset, (mensaje -> pokemon), tamanio_pokemon);
	offset += tamanio_pokemon;

	tamanio_lista /= 2;
	memcpy(contenido + offset, &tamanio_lista, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	void grabar_numero(void* un_numero){
		memcpy(contenido + offset,(uint32_t*) un_numero, sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}

	list_iterate(mensaje -> posiciones, grabar_numero);

	return contenido;
}


void* armar_contenido_caught(t_caught_pokemon* mensaje){
	void* contenido = malloc(sizeof(uint32_t));

	memcpy(contenido, &(mensaje -> resultado), sizeof(uint32_t));

	return contenido;
}

void* armar_contenido_appeared(t_appeared_pokemon* mensaje){
	uint32_t tamanio_pokemon = strlen(mensaje -> pokemon);
	uint32_t tamanio = tamanio_pokemon + (sizeof(uint32_t) * 2);
	void* contenido = malloc(tamanio);
	uint32_t offset = 0;

	memcpy(contenido + offset, (mensaje -> pokemon), tamanio_pokemon);
	offset += tamanio_pokemon;

	memcpy(contenido + offset, &(mensaje -> posicion[0]), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(contenido + offset, &(mensaje -> posicion[1]), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	return contenido;
}

void* armar_contenido_new(t_new_pokemon* mensaje){
	uint32_t tamanio_pokemon = strlen(mensaje -> pokemon);
	uint32_t tamanio = tamanio_pokemon + (sizeof(uint32_t) * 3);
	void* contenido = malloc(tamanio);
	uint32_t offset = 0;
	memcpy(contenido + offset, (mensaje -> pokemon), tamanio_pokemon);
	offset += tamanio_pokemon;
	memcpy(contenido + offset, &(mensaje -> posicion[0]), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(contenido + offset, &(mensaje -> posicion[1]), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(contenido + offset, &(mensaje -> cantidad), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	return contenido;
}

// --------------------- BUDDY SYSTEM --------------------- //

void arrancar_buddy(){
	memoria_cache = list_create();
	t_memoria_buddy* root = crear_raiz();
	list_add(memoria_cache, root);
}

t_memoria_buddy* crear_raiz() {
	t_memoria_buddy* nodo = malloc(sizeof(t_memoria_buddy));

	nodo -> tamanio_exponente = config_broker -> size_memoria;
	nodo -> tamanio_mensaje = 0;
	nodo -> base = 0;
	nodo -> ocupado = 0;
	nodo -> ultima_referencia = 0;
	nodo -> tiempo_de_carga = 0;
	nodo -> codigo_operacion = 0;
	nodo -> contenido = 0;
	nodo -> posicion = 0;

	return nodo;
}

uint32_t determinar_exponente(t_mensaje* mensaje) {

	if(mensaje -> tamanio_mensaje > config_broker -> size_min_memoria){
		return obtenerPotenciaDe2(mensaje -> tamanio_mensaje);

	}else{
		return config_broker -> size_min_memoria;
	}
}

uint32_t obtenerPotenciaDe2(uint32_t n) {

	if( (n & (n - 1)) == 0){
		return n;
	}
	int res = 0;

	for (int i=n; i>=1; i--) {
		if ((i & (i-1)) == 0) {
			res = i;
			break;
		}
	}

	return (res*2);
}

void guardar_buddy(t_memoria_buddy* buddy, t_mensaje* mensaje, void* contenido) {

	t_memoria_buddy* buddy_a_ubicar;

	if(buddy != NULL) {

		buddy_a_ubicar = armar_buddy(buddy -> tamanio_exponente, buddy -> base, mensaje, 1, contenido, buddy -> posicion);
		uint32_t indice = remover_buddy(buddy);
		list_add_in_index(memoria_cache, indice, buddy_a_ubicar);
		guardar_contenido_de_mensaje(buddy_a_ubicar -> base, contenido, buddy_a_ubicar -> tamanio_exponente);

	} else {
		reemplazar_buddy(mensaje, contenido);
	}
}

void reemplazar_buddy(t_mensaje* mensaje, void* contenido) {

	t_memoria_buddy* buddy_a_eliminar;

	if(string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "FIFO")){
		buddy_a_eliminar = seleccionar_victima_fifo();

	} else if (string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "LRU")){
		buddy_a_eliminar = seleccionar_victima_lru();
	}

	limpiar_buddy(buddy_a_eliminar);

	uint32_t exponente = determinar_exponente(mensaje);
	recorrer_segun_algoritmo(exponente, mensaje, contenido);
}

t_memoria_buddy* seleccionar_victima_fifo() {

	bool buddy_ocupado(void* buddy) {
		t_memoria_buddy* un_buddy = buddy;

		return (un_buddy -> ocupado) != 0;
	}

	t_list* memoria_cache_duplicada = list_filter(memoria_cache, buddy_ocupado);

	bool fue_cargada_antes(void* buddy1, void* buddy2) {
		t_memoria_buddy* un_buddy = buddy1;
		t_memoria_buddy* otro_buddy = buddy2;

		return (un_buddy -> tiempo_de_carga) < (otro_buddy -> tiempo_de_carga);
	}

	t_list* memoria_cache_ordenada = list_sorted(memoria_cache_duplicada, fue_cargada_antes);
	t_memoria_buddy* buddy_victima = list_get(memoria_cache_ordenada, 0);

	if(buddy_victima == NULL) {
		log_error(logger, "No hay buddy victima obtenido por fifo.");
	}
	return buddy_victima;
}

t_memoria_buddy* seleccionar_victima_lru() {

	bool buddy_ocupado(void* buddy) {
		t_memoria_buddy* un_buddy = buddy;

		return (un_buddy -> ocupado) != 0;
	}

	t_list* memoria_cache_duplicada = list_filter(memoria_cache, buddy_ocupado);

	bool fue_referenciada_antes(void* buddy1, void* buddy2) {
		t_memoria_buddy* un_buddy = buddy1;
		t_memoria_buddy* otro_buddy = buddy2;
		return (un_buddy -> ultima_referencia) < (otro_buddy -> ultima_referencia) ;
	}

	t_list* memoria_cache_ordenada = list_sorted(memoria_cache_duplicada, fue_referenciada_antes);
	t_memoria_buddy* buddy_victima = list_get(memoria_cache_ordenada, 0);
	if(buddy_victima == NULL) {
		log_error(logger, "No hay buddy victima obtenido por lru.");
	}
	return buddy_victima;
}

bool son_buddies_hermanos(t_memoria_buddy* un_buddy, t_memoria_buddy* otro_buddy) {
	uint32_t base_mas_chica;
	uint32_t base_mas_grande;
	if(un_buddy -> base < otro_buddy -> base) {
		base_mas_chica = un_buddy -> base;
		base_mas_grande = otro_buddy -> base;
	} else {
		base_mas_grande = un_buddy -> base;
		base_mas_chica = otro_buddy -> base;
	}
	return un_buddy -> tamanio_exponente == otro_buddy -> tamanio_exponente &&
	base_mas_chica + un_buddy -> tamanio_exponente == base_mas_grande &&
	base_mas_chica % ((un_buddy -> tamanio_exponente)*2) == 0;
}

void limpiar_buddy(t_memoria_buddy* buddy) {

	t_memoria_buddy* buddy_vacio = armar_buddy(buddy -> tamanio_exponente, buddy -> base, NULL, 0, NULL, buddy -> posicion);
	uint32_t indice;
	t_mensaje* mensaje_a_eliminar;

	if(buddy->ocupado == 1){
		mensaje_a_eliminar = encontrar_mensaje_buddy(buddy -> base, buddy -> codigo_operacion);
		eliminar_de_message_queue(mensaje_a_eliminar, buddy -> codigo_operacion);
	}

	indice = remover_buddy(buddy);

	bool es_el_hermano(void* otro_buddy) {
		t_memoria_buddy* un_buddy = otro_buddy;
		return son_buddies_hermanos(buddy_vacio, un_buddy);
	}

	t_memoria_buddy* buddy_hermano = list_find(memoria_cache, es_el_hermano);
	if(buddy_hermano != NULL) {
		consolidar_buddy(indice, buddy_vacio);
	} else {
		if(!ya_fue_consolidado(buddy_vacio)){
			list_add_in_index(memoria_cache, indice, buddy_vacio);
		}
	}
}

bool ya_fue_consolidado(t_memoria_buddy* buddy){
	bool tiene_la_misma_base(void* un_buddy){
		t_memoria_buddy* otro_buddy = un_buddy;
		if(otro_buddy){
			return (otro_buddy->base == buddy-> base);
		}
		return false;
	}
	return list_any_satisfy(memoria_cache, tiene_la_misma_base);
}


void consolidar_buddy(uint32_t indice, t_memoria_buddy* buddy) {

	bool es_el_hermano(void* otro_buddy) {
		t_memoria_buddy* un_buddy = otro_buddy;
		return son_buddies_hermanos(buddy, un_buddy);
	}

	t_memoria_buddy* buddy_hermano = list_find(memoria_cache, es_el_hermano);

	if(buddy_hermano != NULL && !buddy_hermano -> ocupado) {
		uint32_t base_padre;
		if(buddy -> posicion) {
			base_padre = buddy_hermano -> base;
		} else {
			base_padre = buddy -> base;
		}

		indice = remover_buddy(buddy_hermano);

		uint32_t posicion_padre;
		if(base_padre % (buddy -> tamanio_exponente * 4)) {
			posicion_padre = 1;
		} else {
			posicion_padre = 0;
		}
		t_memoria_buddy* buddy_padre = armar_buddy(buddy -> tamanio_exponente * 2, base_padre, NULL, 0, NULL, posicion_padre);

		list_add_in_index(memoria_cache, indice, buddy_padre);
		log_info(logger, "Se consolidaron los buddies de bases %d y %d", base_padre, base_padre + buddy -> tamanio_exponente);
		limpiar_buddy(buddy_padre);
	} else {
		list_add_in_index(memoria_cache, indice, armar_buddy(buddy -> tamanio_exponente, buddy -> base, NULL, 0, NULL, buddy -> posicion));
	}
}

t_memoria_buddy* armar_buddy(uint32_t exponente, uint32_t base, t_mensaje* mensaje, uint32_t ocupacion, void* contenido, uint32_t posicion) {

	t_memoria_buddy* nuevo_buddy = malloc(sizeof(t_memoria_buddy));

	if(mensaje != NULL){
		nuevo_buddy -> tamanio_exponente = exponente;
		nuevo_buddy -> tamanio_mensaje = mensaje -> tamanio_mensaje;
		nuevo_buddy -> base = base;
		nuevo_buddy -> ocupado = ocupacion;
		nuevo_buddy -> codigo_operacion = mensaje -> codigo_operacion;
		nuevo_buddy -> contenido = contenido;
		nuevo_buddy -> posicion = posicion;
		mensaje -> payload = nuevo_buddy;

	} else {
		nuevo_buddy -> tamanio_exponente = exponente;
		nuevo_buddy -> tamanio_mensaje = 0;
		nuevo_buddy -> base = base;
		nuevo_buddy -> ocupado = 0;
		nuevo_buddy -> codigo_operacion = 0;
		nuevo_buddy -> contenido = 0;
		nuevo_buddy -> posicion = posicion;
	}

	return nuevo_buddy;
}

uint32_t remover_buddy(t_memoria_buddy* buddy_a_remover) {

	uint32_t indice = encontrar_indice(buddy_a_remover);

	list_remove(memoria_cache, indice);

	return indice;
}

t_memoria_buddy* recorrer_best_fit(uint32_t exponente) {

	bool es_de_menor_tamanio(void* un_buddy, void* otro_buddy){
		t_memoria_buddy* buddy1 = un_buddy;
		t_memoria_buddy* buddy2 = otro_buddy;

		return (buddy1 -> tamanio_exponente) <= (buddy2 -> tamanio_exponente);
	}

	t_list* lista_ordenada = list_sorted(memoria_cache, es_de_menor_tamanio);

	bool es_particion_apta(void* buddy3) {
		t_memoria_buddy* un_buddy = buddy3;

		return (un_buddy -> ocupado) == 0 && (un_buddy -> tamanio_exponente) >= exponente;
	}

	t_memoria_buddy* buddy = list_find(lista_ordenada, es_particion_apta);

	if(buddy != NULL) {
		if(buddy -> tamanio_exponente > exponente) {
			dividir_buddy(buddy);
			return recorrer_best_fit(exponente);
		}
	}
	return buddy;
}

void dividir_buddy(t_memoria_buddy* buddy_a_dividir) {

	if(buddy_a_dividir -> tamanio_exponente > 1) {
		uint32_t indice_removido = remover_buddy(buddy_a_dividir);

		uint32_t exponente_hijos = (buddy_a_dividir -> tamanio_exponente) / 2;
		uint32_t base_izquierda = buddy_a_dividir -> base;
		uint32_t base_derecha = base_izquierda + exponente_hijos;

		t_memoria_buddy* buddy_izquierdo = armar_buddy(exponente_hijos, base_izquierda, NULL, 0, NULL, 0);
		t_memoria_buddy* buddy_derecho = armar_buddy(exponente_hijos, base_derecha, NULL, 0, NULL, 1);

		list_add_in_index(memoria_cache, indice_removido, buddy_izquierdo);
		list_add_in_index(memoria_cache, indice_removido + 1, buddy_derecho);

	} else {

		log_info(logger, "No puedo dividirme mas, mi tamanio es %d", buddy_a_dividir -> tamanio_exponente);
	}
}

//--------------------- PARTICIONES --------------------//

void reemplazar_particion_de_memoria(t_mensaje* mensaje, void* contenido_mensaje){

	t_memoria_dinamica* particion_a_reemplazar = seleccionar_particion_victima_de_reemplazo();

	t_memoria_dinamica* particion_vacia = armar_particion(particion_a_reemplazar -> tamanio, particion_a_reemplazar -> base, NULL, 0, contenido_mensaje);

	uint32_t indice = encontrar_indice(particion_a_reemplazar);
	t_mensaje* mensaje_a_eliminar = encontrar_mensaje(particion_a_reemplazar -> base, particion_a_reemplazar -> codigo_operacion);

	particion_a_reemplazar = list_replace(memoria_con_particiones, indice, particion_vacia);
	eliminar_mensaje(mensaje_a_eliminar);

	//free(particion_a_reemplazar);
	log_info(logger, "Se libera una partición y se intenta ubicar nuevamente el mensaje.");
	
	guardar_particion(mensaje, contenido_mensaje);
}

t_memoria_dinamica* seleccionar_particion_victima_de_reemplazo(){

	t_memoria_dinamica* particion_victima;
	t_list* memoria_ordenada;

	bool particion_ocupada(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return (una_particion -> ocupado) != 0;
	}
	
	t_list* memoria_duplicada = list_filter(memoria_con_particiones, particion_ocupada);

	bool fue_cargada_antes(void* particion1, void* particion2) {
		t_memoria_dinamica* una_particion = particion1;
		t_memoria_dinamica* otra_particion = particion2;

		return (una_particion -> tiempo_de_carga) < (otra_particion -> tiempo_de_carga);
	}

	bool fue_referenciada_antes(void* particion1, void* particion2) {
		t_memoria_dinamica* una_particion = particion1;
		t_memoria_dinamica* otra_particion = particion2;
		return (una_particion -> ultima_referencia) < (otra_particion -> ultima_referencia);
	}

	if(string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "FIFO")) {
		memoria_ordenada = list_sorted(memoria_duplicada, fue_cargada_antes);
		particion_victima = list_get(memoria_ordenada, 0);
	} else if (string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "LRU")) {
		memoria_ordenada = list_sorted(memoria_duplicada, fue_referenciada_antes);
		particion_victima = list_get(memoria_ordenada, 0);
	}

	return particion_victima;
}

uint32_t obtener_id(t_memoria_dinamica* particion){
	uint32_t id = 0;
	t_mensaje* mensaje = encontrar_mensaje(particion -> base, particion -> codigo_operacion);
	id = mensaje -> id_mensaje;

	return id;
}

uint32_t obtener_id_buddy(t_memoria_buddy* particion){
	uint32_t id = 0;
	t_mensaje* mensaje = encontrar_mensaje_buddy(particion -> base, particion -> codigo_operacion);
	id = mensaje -> id_mensaje;

	return id;
}

t_mensaje* encontrar_mensaje(uint32_t base_de_la_particion_del_mensaje, op_code codigo){

	t_mensaje* un_mensaje;
	bool tiene_la_misma_base(void* un_mensaje){
		t_mensaje* msj = un_mensaje;
		t_memoria_dinamica* particion = msj -> payload;
		return particion -> base == base_de_la_particion_del_mensaje;
	}

	switch(codigo){
		case GET_POKEMON:
		un_mensaje = (t_mensaje*) list_find(cola_get, tiene_la_misma_base);
		break;
		case CATCH_POKEMON:
		un_mensaje = (t_mensaje*) list_find(cola_catch, tiene_la_misma_base);
		break;
		case LOCALIZED_POKEMON:
		un_mensaje = (t_mensaje*) list_find(cola_localized, tiene_la_misma_base);
		break;
		case CAUGHT_POKEMON:
		un_mensaje = (t_mensaje*) list_find(cola_caught, tiene_la_misma_base);
		break;
		case APPEARED_POKEMON:
		un_mensaje = (t_mensaje*) list_find(cola_appeared, tiene_la_misma_base);
		break;
		case NEW_POKEMON:
		un_mensaje = (t_mensaje*) list_find(cola_new, tiene_la_misma_base);
		break;
		default:
		log_error(logger, "...No se puede reconocer el mensaje de esta partición.");
		break;
	}
	return un_mensaje;
}

t_mensaje* encontrar_mensaje_buddy(uint32_t base_del_buddy_del_mensaje, op_code codigo){

	t_mensaje* un_mensaje;

	bool tiene_la_misma_base(void* un_mensaje){
		t_mensaje* msj = un_mensaje;
		t_memoria_buddy* buddy = msj -> payload;
		return buddy -> base == base_del_buddy_del_mensaje;
	}

	switch(codigo){
		case GET_POKEMON:
		un_mensaje = list_find(cola_get, tiene_la_misma_base);
		break;
		case CATCH_POKEMON:
		un_mensaje = list_find(cola_catch, tiene_la_misma_base);
		break;
		case LOCALIZED_POKEMON:
		un_mensaje = list_find(cola_localized, tiene_la_misma_base);
		break;
		case CAUGHT_POKEMON:
		un_mensaje = list_find(cola_caught, tiene_la_misma_base);
		break;
		case APPEARED_POKEMON:
		un_mensaje = list_find(cola_appeared, tiene_la_misma_base);
		break;
		case NEW_POKEMON:
		un_mensaje = list_find(cola_new, tiene_la_misma_base);
		break;
		default:
		log_error(logger, "...No se puede reconocer el mensaje de esta partición BS.");
		break;
	}
	return un_mensaje;
}


void iniciar_memoria_particiones(t_list* memoria_de_particiones){

	t_memoria_dinamica* particion_de_memoria = armar_particion((config_broker->size_memoria), 0, NULL, 0, NULL);
	list_add(memoria_de_particiones, particion_de_memoria);
}


void guardar_particion(t_mensaje* un_mensaje, void* contenido_mensaje){
	uint32_t posicion_a_ubicar = 0;
	t_memoria_dinamica* particion_a_ubicar;
	t_memoria_dinamica* nueva_particion;

	uint32_t tamanio_a_buscar = 0;
	if((un_mensaje -> tamanio_mensaje) < (config_broker -> size_min_memoria)){
		tamanio_a_buscar = (config_broker -> size_min_memoria);
	} else {
		tamanio_a_buscar = un_mensaje -> tamanio_mensaje;
	}
	
	if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre, "FF")){
		posicion_a_ubicar = encontrar_primer_ajuste(tamanio_a_buscar);

		if(posicion_a_ubicar == -1){
			reemplazar_particion_de_memoria(un_mensaje, contenido_mensaje);
		} else {
			
			particion_a_ubicar = list_get(memoria_con_particiones, posicion_a_ubicar);
			nueva_particion = armar_particion(un_mensaje -> tamanio_mensaje, particion_a_ubicar -> base, un_mensaje, 1, contenido_mensaje);
			ubicar_particion(posicion_a_ubicar, nueva_particion);

			guardar_contenido_de_mensaje(nueva_particion -> base, contenido_mensaje, nueva_particion -> tamanio);
		}
	} else if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre,"BF")){
		posicion_a_ubicar = encontrar_mejor_ajuste(tamanio_a_buscar);

		if(posicion_a_ubicar == -1){
			reemplazar_particion_de_memoria(un_mensaje, contenido_mensaje);

		} else {
			particion_a_ubicar = list_get(memoria_con_particiones, posicion_a_ubicar);
			nueva_particion = armar_particion(un_mensaje -> tamanio_mensaje, particion_a_ubicar -> base, un_mensaje, 1, contenido_mensaje);
			ubicar_particion(posicion_a_ubicar, nueva_particion);
			
			guardar_contenido_de_mensaje(nueva_particion -> base, contenido_mensaje, nueva_particion -> tamanio);
		}
	}
	
}

void guardar_contenido_de_mensaje(uint32_t offset, void* contenido, uint32_t tamanio){

	if(contenido != NULL){
		memcpy(memoria + offset, contenido, tamanio);
		log_info(logger, "Se guardo un mensaje en la particion que comienza en la posicion %d de tamaño %d.", offset, tamanio);

	} else {
		log_info(logger, "Se libera una partición que comienza en la posicion %d de tamaño %d.", offset, tamanio);
	}
}

void liberar_particion_dinamica(t_memoria_dinamica* particion_vacia){
	free(particion_vacia);
}

void ubicar_particion(uint32_t posicion_a_ubicar, t_memoria_dinamica* particion){

	t_memoria_dinamica* particion_reemplazada = list_replace(memoria_con_particiones, posicion_a_ubicar, particion);

	uint32_t total = particion_reemplazada -> tamanio_part;

	if(particion -> tamanio_part < total) {
		uint32_t nueva_base = (particion_reemplazada -> base) + particion -> tamanio_part;
		t_memoria_dinamica* nueva_particion_vacia = armar_particion(total - particion ->tamanio_part, nueva_base, NULL, 0, NULL);
		list_add_in_index(memoria_con_particiones, posicion_a_ubicar + 1, nueva_particion_vacia);
	}

	liberar_particion_dinamica(particion_reemplazada);
}

t_memoria_dinamica* armar_particion(uint32_t tamanio, uint32_t base, t_mensaje* mensaje, uint32_t ocupacion, void* contenido) {

	t_memoria_dinamica* nueva_particion = malloc(sizeof(t_memoria_dinamica));

	if(mensaje != NULL) {
		nueva_particion -> tamanio = tamanio;
		if(tamanio < (config_broker -> size_min_memoria)) {
			nueva_particion -> tamanio_part = config_broker -> size_min_memoria;
		} else {
			nueva_particion -> tamanio_part = tamanio;
		}
		nueva_particion -> base = base;
		nueva_particion -> ocupado = ocupacion;
		nueva_particion -> codigo_operacion = mensaje -> codigo_operacion;
		nueva_particion -> contenido = contenido;
		mensaje -> payload = nueva_particion;
	} else {
		nueva_particion -> tamanio = tamanio;
		nueva_particion -> tamanio_part = tamanio;
		nueva_particion -> base = base;
		nueva_particion -> ocupado = 0;
		nueva_particion -> codigo_operacion = 0;
		nueva_particion -> contenido = contenido;
		nueva_particion -> tiempo_de_carga = 0;
		nueva_particion -> ultima_referencia = 0;
	}

	return nueva_particion;
}


uint32_t encontrar_primer_ajuste(uint32_t tamanio){
	uint32_t indice_seleccionado = 0;

	bool es_particion_vacia(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return !(una_particion -> ocupado);
	}
	t_list* lista_duplicada = list_filter(memoria_con_particiones, es_particion_vacia);
	bool tiene_tamanio_suficiente(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return tamanio <= (una_particion -> tamanio_part);
	}

	t_memoria_dinamica* posible_particion = list_find(lista_duplicada, tiene_tamanio_suficiente);

	if(posible_particion!=NULL){
		indice_seleccionado = encontrar_indice(posible_particion);
	} else {
		indice_seleccionado = -1;
	}

	return indice_seleccionado;
}

uint32_t encontrar_mejor_ajuste(uint32_t tamanio){
	uint32_t indice_seleccionado = 0;

	bool es_de_menor_tamanio(void* una_particion, void* otra_particion){
		t_memoria_dinamica* particion1 = una_particion;
		t_memoria_dinamica* particion2 = otra_particion;
		return (particion1 -> tamanio_part) < (particion2 -> tamanio_part);
	}

	bool tiene_tamanio_suficiente(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return tamanio <= (una_particion -> tamanio_part);
	}
	t_list* particiones_en_orden_creciente = list_sorted(memoria_con_particiones, es_de_menor_tamanio);

	bool es_particion_vacia(void* part){
		t_memoria_dinamica* part2 = part;
		return !(part2 -> ocupado);
	}

	t_list* particiones_ordenadas = list_filter(particiones_en_orden_creciente, es_particion_vacia);
	t_memoria_dinamica* posible_particion = list_find(particiones_ordenadas, tiene_tamanio_suficiente);

	if(posible_particion != NULL){
		indice_seleccionado = encontrar_indice(posible_particion);
	} else {
		indice_seleccionado = -1;
	}

	list_destroy(particiones_en_orden_creciente);
	//list_destroy(particiones_ordenadas);
	return indice_seleccionado;
}

uint32_t encontrar_indice(void* memory){
	uint32_t indice_disponible = 0;
	uint32_t indice_buscador = 0;
	t_list* indices = list_create();
	t_list* memoria_duplicada;
	t_memoria_buddy* buddy;


	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* posible_particion = memory;
		memoria_duplicada = list_duplicate(memoria_con_particiones);

		void obtener_indices(void* particion){
			t_memoria_dinamica* particion_a_transformar = particion;
			t_indice* un_indice = malloc(sizeof(t_indice));
			un_indice -> indice = indice_buscador;
			un_indice -> base = particion_a_transformar -> base;
			list_add(indices, un_indice);
			indice_buscador++;
		}

		bool es_la_particion(void* indice){
			t_indice* otro_indice = indice;
			return (otro_indice -> base) == (posible_particion -> base);
		}

		list_iterate(memoria_duplicada, obtener_indices);
		t_indice* indice_elegido = list_find(indices, es_la_particion);

		if(indice_elegido!=NULL){
			indice_disponible = indice_elegido -> indice;
		} else {
			indice_disponible = 0;
			log_error(logger, "El indice no pudo obtenerse correctamente.");
		}

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		memoria_duplicada = list_duplicate(memoria_cache);

		buddy = memory;

		void obtener_indices_buddy(void* un_buddy){
			t_memoria_buddy* buddy_a_transformar = un_buddy;
			t_indice* un_indice = malloc(sizeof(t_indice));
			un_indice -> indice = indice_buscador;
			un_indice  -> base = buddy_a_transformar -> base;
			list_add(indices, un_indice);
			indice_buscador ++;
		}
		bool es_el_buddy(void* indice){
			t_indice* otro_indice = indice;
			return (otro_indice-> base) == (buddy -> base);
		}

		list_iterate(memoria_duplicada, obtener_indices_buddy);
		t_indice* indice_elegido = list_find(indices, es_el_buddy);

		if(indice_elegido != NULL){
			indice_disponible = indice_elegido -> indice;
		} else {
			indice_disponible = 0;
			log_error(logger, "El indice de buddy no pudo obtenerse correctamente.");
		}

	} else {
		log_error(logger, "No se reconoce el algoritmo de memoria");
	}

	list_destroy(indices);
	list_destroy(memoria_duplicada);

	return indice_disponible;

}

//--------------------------------CONSOLIDACION_P--------------------------------//

void consolidar_particiones_dinamicas(t_list* memoria){
	t_list* memoria_duplicada = list_duplicate(memoria);
	
	uint32_t contador = 0;
	void consolidar_particiones_contiguas(void* particion){

		if(tiene_siguiente(contador)){
			if(ambas_estan_vacias(contador, contador + 1)){
				consolidar_particiones(contador, contador + 1);
			}
		}
		contador++;
	}

	list_iterate(memoria_duplicada, consolidar_particiones_contiguas);

	//list_destroy(memoria_duplicada);
}

bool tiene_siguiente(uint32_t posicion){
	return (list_size(memoria_con_particiones) - 1) > posicion;
}

bool ambas_estan_vacias(uint32_t una_posicion, uint32_t posicion_siguiente){

	t_memoria_dinamica* una_particion       = list_get(memoria_con_particiones, una_posicion);
	t_memoria_dinamica* particion_siguiente = list_get(memoria_con_particiones, posicion_siguiente);
	uint32_t resultado = 0;

	if(una_particion != NULL){
		if(particion_siguiente!= NULL){
			resultado = (!(una_particion -> ocupado)) && (!(particion_siguiente -> ocupado));
		} else {
			log_error(logger, "La segunda particion a consolidar no fue encontrada.");
		}
	} else {
		log_error(logger, "La primer particion a consolidar no fue encontrada.");
	}

	return resultado;
}

void consolidar_particiones(uint32_t primer_elemento, uint32_t elemento_siguiente){
	t_memoria_dinamica* una_particion = list_get(memoria_con_particiones, primer_elemento);
	t_memoria_dinamica* particion_siguiente = list_get(memoria_con_particiones, elemento_siguiente);

	uint32_t tamanio_particion_consolidada    = (una_particion -> tamanio_part) + (particion_siguiente -> tamanio_part);
	t_memoria_dinamica* particion_consolidada = armar_particion(tamanio_particion_consolidada, (una_particion -> base), NULL, 0, NULL);

	ubicar_particion(primer_elemento, particion_consolidada);
	log_info(logger, "Se consolidan las particiones de base %d y base %d en una particion de tamanio %d", una_particion -> base, particion_siguiente -> base, tamanio_particion_consolidada);
	list_remove(memoria_con_particiones, elemento_siguiente);
}

void compactar_particiones_dinamicas(t_list* memoria){
	t_list* particiones_vacias;
	t_list* particiones_ocupadas;

	uint32_t posicion_lista = 0;

	bool es_particion_vacia(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return !(una_particion -> ocupado);
	}

	bool es_particion_ocupada(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return (una_particion -> ocupado);
	}
	particiones_vacias = list_filter(memoria, es_particion_vacia);
	particiones_ocupadas = list_filter(memoria, es_particion_ocupada);
	
	list_clean(memoria);
	list_add_all(memoria, particiones_ocupadas);
	list_add_all(memoria, particiones_vacias);

	void actualizar_base(void* particion){
		t_memoria_dinamica* una_particion = particion;
		una_particion -> base = obtener_nueva_base(una_particion, posicion_lista);
		posicion_lista++;
	}

	list_iterate(memoria, actualizar_base);

	log_info(logger, "Se inicia la compactacion");
	compactar_memoria_cache(memoria);
	
	list_destroy(particiones_vacias);
	list_destroy(particiones_ocupadas);

	particiones_liberadas = 0;

}

uint32_t obtener_nueva_base(t_memoria_dinamica* una_particion, uint32_t indice_tope){
	uint32_t nueva_base = 0;
	t_list* memoria_duplicada = list_duplicate(memoria_con_particiones);

	t_memoria_dinamica* particion_buscada;

	if(indice_tope > 0){
		particion_buscada = list_get(memoria_duplicada, (indice_tope-1));
		if(particion_buscada!=NULL){
			nueva_base = (particion_buscada -> base) + (particion_buscada -> tamanio);
		} else {
			log_error(logger, "...No se puede actualizar la base para compactar :/");
		}
	} else {
		nueva_base = 0;
	}
	//list_destroy(memoria_duplicada);
	return nueva_base;
}

void compactar_memoria_cache(t_list* lista_particiones){

	void reescribir_memoria(void* particion){
		t_memoria_dinamica* una_particion = particion;
		if(una_particion -> ocupado){
			guardar_contenido_de_mensaje((una_particion -> base), (una_particion -> contenido),(una_particion -> tamanio));
		}
	}

	list_iterate(lista_particiones, reescribir_memoria);
}

void eliminar_mensaje(void* mensaje){
	t_mensaje* un_mensaje = mensaje;
	liberar_mensaje_de_memoria(un_mensaje);
	free(un_mensaje -> payload);
	free(un_mensaje);
}

void liberar_mensaje_de_memoria(t_mensaje* mensaje){

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		bool es_la_particion(void* particion){
			t_memoria_dinamica* una_particion = particion;
			return (una_particion -> base) == ((t_memoria_dinamica*) (mensaje->payload))-> base;
		}

		t_memoria_dinamica* particion_a_liberar = list_find(memoria_con_particiones, es_la_particion);

		uint32_t indice = encontrar_indice(particion_a_liberar);

		t_memoria_dinamica* particion_vacia = armar_particion(particion_a_liberar -> tamanio, particion_a_liberar -> base, NULL, 0, NULL);
		particion_a_liberar = list_replace(memoria_con_particiones, indice, particion_vacia);

		guardar_contenido_de_mensaje(particion_a_liberar -> base, NULL, particion_a_liberar -> tamanio);

		eliminar_de_message_queue(mensaje, mensaje -> codigo_operacion);

		particiones_liberadas++;

		if(config_broker -> frecuencia_compactacion == 0 || (particiones_liberadas == (config_broker -> frecuencia_compactacion))){
			compactar_particiones_dinamicas(memoria_con_particiones);
		}		

		consolidar_particiones_dinamicas(memoria_con_particiones);

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){

	} else {
		log_error(logger, "...Algoritmo no reconocido al liberar mensaje.");
	}


}

t_mensaje* eliminar_de_message_queue(t_mensaje* mensaje, op_code codigo){
	if(mensaje == NULL) {
		log_info(logger, "Se intento remover un mensaje nulo de la message queue");
		return 0;
	} else {

		if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")){
			t_memoria_buddy* un_buddy = mensaje -> payload;
			guardar_contenido_de_mensaje(un_buddy -> base, NULL, un_buddy -> tamanio_exponente);

		} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"PARTICIONES")){
			t_memoria_dinamica* una_particion = mensaje -> payload;
			guardar_contenido_de_mensaje(una_particion -> base, NULL, una_particion -> tamanio_part);
		}
	}
	bool es_el_mismo_mensaje(void* msj){
		t_mensaje* el_mensaje = msj;
		return (el_mensaje -> id_mensaje) == (mensaje -> id_mensaje);
	}

	t_mensaje* un_mensaje;

	switch(codigo){
		case GET_POKEMON:
		un_mensaje = list_remove_by_condition(cola_get, es_el_mismo_mensaje);
		break;
		case CATCH_POKEMON:
		un_mensaje = list_remove_by_condition(cola_catch, es_el_mismo_mensaje);
		break;
		case APPEARED_POKEMON:
		un_mensaje = list_remove_by_condition(cola_appeared, es_el_mismo_mensaje);
		break;
		case LOCALIZED_POKEMON:
		un_mensaje = list_remove_by_condition(cola_localized, es_el_mismo_mensaje);
		break;
		case CAUGHT_POKEMON:
		un_mensaje = list_remove_by_condition(cola_caught, es_el_mismo_mensaje);
		break;
		case NEW_POKEMON:
		un_mensaje = list_remove_by_condition(cola_new, es_el_mismo_mensaje);
		break;
		default:
		log_error(logger, "...No se pudo eliminar el mensaje de la message queue.");
		break;
	}

	return un_mensaje;
}

void dump_info_particion(void* particion){
	t_memoria_dinamica* una_particion = particion;
	
	char* ocupado = malloc(sizeof(char));
	ocupado = "L";

	if(una_particion -> ocupado != 0) {
		ocupado = "X";
	}

	uint32_t* base = memoria + (una_particion -> base);
	uint32_t tamanio = 0;

	tamanio = una_particion -> tamanio_part;

	if(una_particion -> ocupado != 0) {
		uint32_t valor_lru = (uint32_t)una_particion -> ultima_referencia;
		char* cola_del_mensaje = obtener_cola_del_mensaje(una_particion);
		uint32_t id_del_mensaje = obtener_id(una_particion);

//		free(cola_del_mensaje);

		log_info(logger_memoria, "Particion %d: %p.  [%s] Size: %d b LRU:%u Cola:%s ID:%d", numero_particion, base, ocupado, tamanio, valor_lru, cola_del_mensaje, id_del_mensaje);
	} else {

		log_info(logger_memoria, "Particion %d: %p.  [%s] Size: %d", numero_particion, base, ocupado, tamanio);
	}
//	free(ocupado);
	numero_particion++;
}

void dump_info_buddy(void* buddy){
	t_memoria_buddy* un_buddy = buddy;
	char* ocupado = malloc(sizeof(char));
	ocupado = "L";

	if(un_buddy -> ocupado != 0) {
		ocupado = "X";
	}

	uint32_t* base = memoria + (un_buddy -> base);
	uint32_t tamanio = un_buddy -> tamanio_exponente;
	
	
	if(un_buddy -> ocupado != 0) {
		uint32_t valor_lru = (uint32_t)un_buddy -> ultima_referencia;
		char* cola_del_mensaje = obtener_cola_del_mensaje_buddy(un_buddy);
		uint32_t id_del_mensaje = obtener_id_buddy(un_buddy);

		log_info(logger_memoria, "Buddy %d: %p.  [%s] Size: %d b LRU:%u Cola:%s ID:%d", numero_particion, base, ocupado, tamanio, valor_lru, cola_del_mensaje, id_del_mensaje);

		//free(cola_del_mensaje);
	} else {

		log_info(logger_memoria, "Buddy %d: %p.  [%s] Size: %d", numero_particion, base, ocupado, tamanio);
	}
	numero_particion++;
	//free(ocupado);
}

char* obtener_cola_del_mensaje(t_memoria_dinamica* una_particion){
	char* una_cola;
	switch(una_particion -> codigo_operacion){
		case GET_POKEMON:
		una_cola = malloc(strlen("COLA_GET") + 1);
		una_cola = "COLA_GET";
		break;
		case CATCH_POKEMON:
		una_cola = malloc(strlen("COLA_CATCH") + 1);
		una_cola = "COLA_CATCH";
		break;
		case LOCALIZED_POKEMON:
		una_cola = malloc(strlen("COLA_LOCALIZED") + 1);
		una_cola = "COLA_LOCALIZED";
		break;
		case APPEARED_POKEMON:
		una_cola = malloc(strlen("COLA_APPEARED") + 1);
		una_cola = "COLA_APPEARED";
		break;
		case CAUGHT_POKEMON:
		una_cola = malloc(strlen("COLA_CAUGHT") + 1);
		una_cola = "COLA_CAUGHT";
		break;
		case NEW_POKEMON:
		una_cola = malloc(strlen("COLA_NEW") + 1);
		una_cola = "COLA_NEW";
		break;
		default:
		una_cola = malloc(strlen("No existe en ninguna cola de mensajes") + 1);
		una_cola = "No existe en ninguna cola de mensajes";
		break;
	}

	return una_cola;
}

char* obtener_cola_del_mensaje_buddy(t_memoria_buddy* un_buddy){
	char* una_cola;
	switch(un_buddy -> codigo_operacion){
		case GET_POKEMON:
		una_cola = malloc(strlen("COLA_GET") + 1);
		una_cola = "COLA_GET";
		break;
		case CATCH_POKEMON:
		una_cola = malloc(strlen("COLA_CATCH") + 1);
		una_cola = "COLA_CATCH";
		break;
		case LOCALIZED_POKEMON:
		una_cola = malloc(strlen("COLA_LOCALIZED") + 1);
		una_cola = "COLA_LOCALIZED";
		break;
		case APPEARED_POKEMON:
		una_cola = malloc(strlen("COLA_APPEARED") + 1);
		una_cola = "COLA_APPEARED";
		break;
		case CAUGHT_POKEMON:
		una_cola = malloc(strlen("COLA_CAUGHT") + 1);
		una_cola = "COLA_CAUGHT";
		break;
		case NEW_POKEMON:
		una_cola = malloc(strlen("COLA_NEW") + 1);
		una_cola = "COLA_NEW";
		break;
		default:
		una_cola = malloc(strlen("No existe en ninguna cola de mensajes") + 1);
		una_cola = "No existe en ninguna cola de mensajes";
		break;
	}

	return una_cola;
}

uint64_t timestamp(void) {
	struct timeval valor;
	gettimeofday(&valor, NULL);
	unsigned long long result = ((unsigned long long )valor.tv_sec) * 1000 + ((unsigned long long) valor.tv_usec) / 1000;
	uint64_t tiempo = result;
	return tiempo;
}


t_memoria_buddy* recorrer_first_fit(uint32_t exponente) {

	bool es_particion_apta(void* buddy1) {
		t_memoria_buddy* un_buddy = buddy1;
		return (un_buddy -> ocupado) == 0 && (un_buddy -> tamanio_exponente) >= exponente;
	}
	t_memoria_buddy* buddy = list_find(memoria_cache, es_particion_apta);

	if(buddy != NULL) {
		if(buddy -> tamanio_exponente > exponente) {
			dividir_buddy(buddy);
			return recorrer_first_fit(exponente);
		}
	}
	return buddy;
}




// -------------- FIN DE PROGRAMA -------------- //

void iniciar_semaforos_broker() { //REVISAR INICIALIZCIONES
	sem_init(&semaforo, 0, 1);
	sem_init(&muteadito, 0, 1);
	sem_init(&mx_suscripciones, 0, 1);
	sem_init(&muteadin, 0, 1);
}

void terminar_hilos_broker(){
	pthread_detach(hilo_envio_mensajes);
}

void liberar_semaforos_broker(){
	sem_destroy(&semaforo);
	sem_destroy(&muteadito);
	sem_destroy(&mx_suscripciones);
}

