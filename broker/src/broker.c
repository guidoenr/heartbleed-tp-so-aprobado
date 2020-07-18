#include "broker.h"

int main(void) {
	iniciar_programa();


	terminar_programa(logger);
	return 0;
}

void sig_handler(void* signo) {
    uint32_t* un_signo = signo;
	if((*un_signo) == SIGUSR1){
        dump_de_memoria();
    }
}

void iniciar_programa(){
	id_mensaje_univoco = 0;
	particiones_liberadas = 0;
	numero_particion = 1;
	nodo_id = 0;

	iniciar_semaforos_broker();

	leer_config();
	iniciar_logger(config_broker->log_file, "broker");
	reservar_memoria();
	crear_colas_de_mensajes();
	crear_listas_de_suscriptores();
	iniciar_servidor(config_broker -> ip_broker, config_broker -> puerto);
	crear_hilo_signal();
	

    //crear_hilo_envio_mensajes(); --> tiene como main enviar_mensajes
    //crear_hilo_por_mensaje(); --> en el serve client

	log_info(logger, "IP: %s", config_broker -> ip_broker);

}

void* main_hilo_signal(void* variable){

	signal(SIGUSR1, sig_handler);

	return NULL;
}

void crear_hilo_signal(){
	uint32_t err = pthread_create(&hilo_signal, NULL, main_hilo_signal, NULL);
	if(err != 0) {
		log_error(logger, "El hilo no pudo ser creado!!");
	}
}

void reservar_memoria(){

	memoria = malloc(config_broker ->  size_memoria);
	log_error(logger, "Base: %d", memoria);
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")){
		arrancar_buddy();
	}

    if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"PARTICIONES")){
		memoria_con_particiones = list_create();
        iniciar_memoria_particiones(memoria_con_particiones);
	}

}

void crear_colas_de_mensajes(){

	 cola_new 		= list_create();
	 cola_appeared  = list_create();
	 cola_get 		= list_create();
	 cola_localized = list_create();
	 cola_catch 	= list_create();
	 cola_caught 	= list_create();
}

void crear_listas_de_suscriptores(){

	 lista_suscriptores_new 	  = list_create();
	 lista_suscriptores_appeared  = list_create();
	 lista_suscriptores_get 	  = list_create();
	 lista_suscriptores_localized = list_create();
	 lista_suscriptores_catch 	  = list_create();
	 lista_suscriptores_caught 	  = list_create();
}

void leer_config() {

	t_config* config;

	config_broker = malloc(sizeof(t_config_broker));

	config = config_create("Debug/broker.config");

	if(config == NULL){
		    	printf("No se pudo encontrar el path del config.");
		    	exit(-2);
	}
	config_broker -> size_memoria 			   = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> size_min_memoria 		   = config_get_int_value(config, "TAMANO_MINIMO_PARTICION");
	config_broker -> algoritmo_memoria 		   = config_get_string_value(config, "ALGORITMO_MEMORIA");
	config_broker -> algoritmo_reemplazo 	   = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	config_broker -> algoritmo_particion_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	config_broker -> ip_broker 				   = config_get_string_value(config, "IP_BROKER");
	config_broker -> puerto 				   = config_get_string_value(config, "PUERTO_BROKER");
	config_broker -> frecuencia_compactacion   = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
	config_broker -> log_file 				   = config_get_string_value(config, "LOG_FILE");
	config_broker -> memory_log 			   = config_get_string_value(config, "DUMP_CACHE");
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

void terminar_programa(t_log* logger){
	liberar_listas();
	liberar_config(config_broker);
	liberar_logger(logger);
	liberar_memoria_cache();
	liberar_semaforos_broker();
	config_destroy(config);
}

void liberar_memoria_cache(){
	free(memoria);
}

void liberar_listas(){
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
	op_code* codigo_op = malloc(sizeof(op_code));
	void* stream = recibir_paquete(cliente_fd, &size, codigo_op);
	cod_op = (*codigo_op);
	log_info(logger,"Codigo de operacion %d", cod_op);
	void* mensaje_e_agregar = deserealizar_paquete(stream, *codigo_op, size);

	switch (*codigo_op) {
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
			sem_wait(&muteadito);
			recibir_suscripcion(mensaje_e_agregar);
			sem_post(&muteadito);
			break;
		case ACK:
			actualizar_mensajes_confirmados(mensaje_e_agregar);
			break;
		case 0:
			log_error(logger,"...No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
	free(codigo_op);
	free(stream);
}

void agregar_mensaje(uint32_t cod_op, uint32_t size, void* mensaje, uint32_t socket_cliente){
	//log_info(logger, "...Agregando mensaje");
	//log_info(logger, "...Size: %d", size);
	//log_info(logger, "...Socket_cliente: %d", socket_cliente);
	//log_info(logger, "...Payload: %s", (char*) mensaje);
	t_mensaje* mensaje_a_agregar = malloc(sizeof(t_mensaje));
	uint32_t nuevo_id     = generar_id_univoco();


	mensaje_a_agregar -> id_mensaje = nuevo_id;

    if(cod_op == APPEARED_POKEMON){
    	t_appeared_pokemon* mensaje_pokemon = mensaje;
    	mensaje_a_agregar -> id_correlativo = mensaje_pokemon -> id_mensaje_correlativo;
    } else if(cod_op == LOCALIZED_POKEMON){
    	t_localized_pokemon* mensaje_pokemon = mensaje;
    	mensaje_a_agregar -> id_correlativo = mensaje_pokemon -> id_mensaje_correlativo;
    } else if(cod_op == CAUGHT_POKEMON){
    	t_caught_pokemon* mensaje_pokemon = mensaje;
    	mensaje_a_agregar -> id_correlativo = mensaje_pokemon -> id_mensaje_correlativo;
    } else {
    	mensaje_a_agregar -> id_correlativo = 0;
    }

    if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
         mensaje_a_agregar -> payload =  malloc(sizeof(t_node));
         ((t_node*)(mensaje_a_agregar->payload))->bloque = malloc(sizeof(t_memoria_buddy));
    } else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
        t_memoria_dinamica* particion = malloc(sizeof(t_memoria_dinamica));
		mensaje_a_agregar -> payload = particion;
    } else {
        log_error(logger, "...No se reconoce el algoritmo de memoria.");
    }

	mensaje_a_agregar -> codigo_operacion    = cod_op;
	mensaje_a_agregar -> suscriptor_enviado  = list_create();
	mensaje_a_agregar -> suscriptor_recibido = list_create();
    mensaje_a_agregar -> tamanio_mensaje     = obtener_tamanio_contenido_mensaje(mensaje, cod_op);

    if(cod_op == LOCALIZED_POKEMON){
    	t_localized_pokemon* localized = mensaje;
    	mensaje_a_agregar -> tamanio_lista_localized = localized -> tamanio_lista;
    }


	sem_wait(&mutex_id);
	send(socket_cliente, &(nuevo_id) , sizeof(uint32_t), 0); //Avisamos,che te asiganmos un id al mensaje
	sem_post(&mutex_id);

	if(puede_guardarse_mensaje(mensaje_a_agregar)){
		guardar_en_memoria(mensaje_a_agregar, mensaje);
	}

	sem_wait(&semaforo);
	encolar_mensaje(mensaje_a_agregar, cod_op);
	sem_post(&semaforo);
}

bool puede_guardarse_mensaje(t_mensaje* un_mensaje){
	uint32_t no_se_repite_correlativo = 1;

	bool tiene_mismo_id_correlativo(void* mensaje){
		t_mensaje* msg = mensaje;
		return (msg -> id_correlativo) == (un_mensaje -> id_correlativo);
	}

	switch(un_mensaje -> codigo_operacion){
		case LOCALIZED_POKEMON:
			if(list_find(cola_localized, tiene_mismo_id_correlativo)){
				no_se_repite_correlativo = 0;
			} else {
				no_se_repite_correlativo = 1;
			}
			break;
		case CAUGHT_POKEMON:
			if(list_find(cola_caught, tiene_mismo_id_correlativo)){
				no_se_repite_correlativo = 0;
			} else {
				no_se_repite_correlativo = 1;
			}
			break;
		case APPEARED_POKEMON:
			if(list_find(cola_localized, tiene_mismo_id_correlativo)){
				no_se_repite_correlativo = 0;
			} else {
				no_se_repite_correlativo = 1;
			}
			break;
		default:
			no_se_repite_correlativo = 1;
			break;
	}

	return no_se_repite_correlativo;
}

uint32_t obtener_tamanio_contenido_mensaje(void* mensaje, uint32_t codigo){
	uint32_t tamanio;
	t_get_pokemon* get;
	t_catch_pokemon* catch;
	t_localized_pokemon* localized;
	t_new_pokemon* new;
	t_appeared_pokemon* appeared;

	switch(codigo){
	case GET_POKEMON:
		get = mensaje;
		tamanio = strlen(get -> pokemon) + sizeof(uint32_t);
		break;
	case CATCH_POKEMON:
		catch = mensaje;
		tamanio = strlen(catch -> pokemon) + (3*sizeof(uint32_t));
		break;
	case LOCALIZED_POKEMON:
		localized = mensaje;
		tamanio = strlen(localized -> pokemon) + (list_size(localized -> posiciones) * sizeof(uint32_t))+ sizeof(uint32_t);
		break;
	case CAUGHT_POKEMON:
		tamanio = sizeof(uint32_t)*2;
		break;
	case APPEARED_POKEMON:
		appeared = mensaje;
		tamanio = strlen(appeared -> pokemon) + (3*sizeof(uint32_t));
		break;
	case NEW_POKEMON:
		new = mensaje;
		tamanio = strlen(new -> pokemon) + (4*sizeof(uint32_t));
		break;
	default:
		log_error(logger, "...No se puede obtener el tamaño del contenido del mensaje.");
		tamanio = 0;
		break;
	}
	return tamanio;
}

uint32_t generar_id_univoco(){
	pthread_mutex_t mutex_id_univoco = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex_id_univoco);
	id_mensaje_univoco++;
	pthread_mutex_unlock(&mutex_id_univoco);

	pthread_mutex_destroy(&mutex_id_univoco);

	return id_mensaje_univoco;
}

void encolar_mensaje(t_mensaje* mensaje, op_code codigo_operacion){

	switch (codigo_operacion) {
			case GET_POKEMON:
				establecer_tiempo_de_carga(mensaje);
				sem_wait(&mx_cola_get);
				list_add(cola_get, mensaje);
				sleep(3);
				sem_wait(&mx_suscrip_get);
				enviar_mensajes(cola_get, lista_suscriptores_get);
				sem_post(&mx_suscrip_get);
				sem_post(&mx_cola_get);
				log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes get.");
				break;
			case CATCH_POKEMON:
				establecer_tiempo_de_carga(mensaje);
				sem_wait(&mx_cola_catch);
				list_add(cola_catch, mensaje);
				sleep(3);
				sem_wait(&mx_suscrip_catch);
				enviar_mensajes(cola_catch, lista_suscriptores_catch);
				sem_post(&mx_suscrip_catch);
				sem_post(&mx_cola_catch);
				log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes catch.");
				break;
			case LOCALIZED_POKEMON:
				establecer_tiempo_de_carga(mensaje);
				sem_wait(&mx_cola_localized);
				list_add(cola_localized, mensaje);
				sleep(3);
				sem_wait(&mx_suscrip_localized);
				enviar_mensajes(cola_localized, lista_suscriptores_localized);
				sem_post(&mx_suscrip_localized);
				sem_post(&mx_cola_localized);
				log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes localized.");
				break;
			case CAUGHT_POKEMON:
				establecer_tiempo_de_carga(mensaje);
				sem_wait(&mx_cola_caught);
				list_add(cola_caught, mensaje);
				sleep(3);
				sem_wait(&mx_suscrip_caught);
				enviar_mensajes(cola_caught, lista_suscriptores_caught);
				sem_post(&mx_suscrip_caught);
				sem_post(&mx_cola_caught);
				log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes caught.");
				break;
			case APPEARED_POKEMON:
				establecer_tiempo_de_carga(mensaje);
				sem_wait(&mx_cola_appeared);
				list_add(cola_appeared, mensaje);
				sleep(3);
				sem_wait(&mx_suscrip_appeared);
				enviar_mensajes(cola_appeared, lista_suscriptores_appeared);
				sem_post(&mx_suscrip_appeared);
				sem_post(&mx_cola_appeared);			
				log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes appeared.");
				break;
			case NEW_POKEMON:
				establecer_tiempo_de_carga(mensaje);
				sem_wait(&mx_cola_new);
				list_add(cola_new, mensaje);
				sleep(3);
				sem_wait(&mx_suscrip_new);
				enviar_mensajes(cola_new, lista_suscriptores_new);
				sem_post(&mx_suscrip_new);
				sem_post(&mx_cola_new);
				log_info(logger, "Un nuevo mensaje fue agregado a la cola de mensajes new.");
				break;
			default:
				log_error(logger, "...El codigo de operacion es invalido");
				exit(-6);
	}
}

//-----------------------SUSCRIPCIONES------------------------//
void recibir_suscripcion(t_suscripcion* mensaje_suscripcion){

	op_code cola_a_suscribir = mensaje_suscripcion -> cola_a_suscribir;

	log_info(logger, "Se recibe una suscripción.");

		switch (cola_a_suscribir) {
			 case GET_POKEMON:
			 	sem_wait(&mx_suscrip_get);
				suscribir_a_cola(lista_suscriptores_get, mensaje_suscripcion, cola_a_suscribir);
				sem_post(&mx_suscrip_get);
				break;
			 case CATCH_POKEMON:
				sem_wait(&mx_suscrip_catch);
				suscribir_a_cola(lista_suscriptores_catch, mensaje_suscripcion, cola_a_suscribir);
				sem_post(&mx_suscrip_catch);
				break;
			 case LOCALIZED_POKEMON:
			 	sem_wait(&mx_suscrip_localized);
				suscribir_a_cola(lista_suscriptores_localized, mensaje_suscripcion, cola_a_suscribir);
				sem_post(&mx_suscrip_localized);
				break;
			 case CAUGHT_POKEMON:
			 	sem_wait(&mx_suscrip_caught);
				suscribir_a_cola(lista_suscriptores_caught, mensaje_suscripcion, cola_a_suscribir);
				sem_post(&mx_suscrip_catch);
				break;
			 case APPEARED_POKEMON:
			 	sem_wait(&mx_suscrip_appeared);
				suscribir_a_cola(lista_suscriptores_appeared, mensaje_suscripcion, cola_a_suscribir);
				sem_post(&mx_suscrip_appeared);
				break;
			 case NEW_POKEMON:
			 	sem_wait(&mx_suscrip_new);
				suscribir_a_cola(lista_suscriptores_new, mensaje_suscripcion, cola_a_suscribir);
				sem_post(&mx_suscrip_new);
				break;
			 default:
				log_info(logger, "...La suscripcion no se puede realizar.");
				break;
		}

}

void suscribir_a_cola(t_list* lista_suscriptores, t_suscripcion* suscripcion, op_code cola_a_suscribir){

	char* cola;
	switch(cola_a_suscribir){
		case GET_POKEMON:
			cola = "cola get";
			break;
		case CATCH_POKEMON:
			cola = "cola catch";
			break;
		case LOCALIZED_POKEMON:
			cola = "cola localized";
			break;
		case CAUGHT_POKEMON:
			cola = "cola caught";
			break;
		case APPEARED_POKEMON:
			cola = "cola appeared";
			break;
		case NEW_POKEMON:
			cola = "cola new";
			break;
		default:
			log_error(logger, "...Se desconoce la cola a suscribir.");
			break;
	}

	log_info(logger, "EL cliente fue suscripto a la cola de mensajes: %s.", cola);
	list_add(lista_suscriptores, suscripcion);

	informar_mensajes_previos(suscripcion, cola_a_suscribir);

	bool es_la_misma_suscripcion(void* una_suscripcion){
		t_suscripcion* otra_suscripcion = una_suscripcion;
		return otra_suscripcion -> id_proceso == suscripcion -> id_proceso;
	}

	if(suscripcion -> tiempo_suscripcion != 0){
		sleep(suscripcion -> tiempo_suscripcion);
		list_remove_by_condition(lista_suscriptores, es_la_misma_suscripcion);
		log_info(logger, "La suscripcion temporal fue anulada correctamente.");
	}

}

void destruir_suscripcion(void* suscripcion) {
	free(suscripcion);
}

//REVISAR EL PARAMETRO QUE RECIBE
void informar_mensajes_previos(t_suscripcion* una_suscripcion, op_code cola_a_suscribir){

	switch(cola_a_suscribir){
		case GET_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(GET_POKEMON, una_suscripcion -> socket);
			//log_info(logger, "...El proceso suscripto recibe los mensajes get del historial");
			break;
		case CATCH_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(CATCH_POKEMON, una_suscripcion -> socket);
			//log_info(logger, "...El proceso suscripto recibe los mensajes catch del historial");
			break;
		case LOCALIZED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(LOCALIZED_POKEMON, una_suscripcion -> socket);
			//log_info(logger, "...El proceso suscripto recibe los mensajes localized del historial");
			break;
		case CAUGHT_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(CAUGHT_POKEMON, una_suscripcion -> socket);
			//log_info(logger, "...El proceso suscripto recibe los mensajes caught del historial");
			break;
		case NEW_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(NEW_POKEMON, una_suscripcion -> socket);
			//log_info(logger, "...El proceso suscripto recibe los mensajes new del historial");
			break;
		case APPEARED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(APPEARED_POKEMON, una_suscripcion -> socket);
			//log_info(logger, "...El proceso suscripto recibe los mensajes appeared del historial");
			break;
		default:
			log_error(logger, "...No se pudo descargar el historial de mensajes satisfactoriamente.");
			break;
	}
}

void descargar_historial_mensajes(op_code tipo_mensaje, uint32_t socket_cliente){

	 void mandar_mensajes_viejos(void* mensaje){
        t_mensaje* un_mensaje = mensaje;
        uint32_t size = 0;

        void* mensaje_a_enviar = preparar_mensaje(un_mensaje);
		size = size_mensaje(mensaje_a_enviar, tipo_mensaje);
        enviar_mensaje(tipo_mensaje, mensaje_a_enviar, socket_cliente, size);
		actualizar_ultima_referencia(un_mensaje);
		//free(mensaje_a_enviar);
    }

    switch(tipo_mensaje){
		case GET_POKEMON:
			sem_wait(&mx_cola_get);
			list_iterate(cola_get, mandar_mensajes_viejos);
			sem_post(&mx_cola_get);
			break;
		case CATCH_POKEMON:
			sem_wait(&mx_cola_catch);
			list_iterate(cola_catch, mandar_mensajes_viejos);
			sem_post(&mx_cola_catch);
			break;
		case LOCALIZED_POKEMON:
			sem_wait(&mx_cola_localized);
			list_iterate(cola_localized, mandar_mensajes_viejos);
			sem_post(&mx_cola_localized);
			break;
		case CAUGHT_POKEMON:
			sem_wait(&mx_cola_caught);
			list_iterate(cola_caught, mandar_mensajes_viejos);
			sem_post(&mx_cola_caught);
			break;
		case APPEARED_POKEMON:
			sem_wait(&mx_cola_appeared);
			list_iterate(cola_appeared, mandar_mensajes_viejos);
			sem_post(&mx_cola_appeared);
			break;
		case NEW_POKEMON:
			sem_wait(&mx_cola_new);
			list_iterate(cola_new, mandar_mensajes_viejos);
			sem_post(&mx_cola_new);
			break;
		default:
			log_info(logger, "...No se puede enviar el mensaje pedido.");
			break;
	}

//    log_info(logger, "...Los mensajes previos a la suscripcion fueron informados al nuevo cliente suscripto.");

}


void* preparar_mensaje(t_mensaje* un_mensaje){
	void* mensaje_armado;

	switch(un_mensaje -> codigo_operacion){
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
		mensaje_armado = preparar_mensaje_catch(un_mensaje);
		break;
		default:
		log_error(logger, "... El broker no puede preparar el mensaje para enviarlo a otro modulo.");
		break;
	}

	return mensaje_armado;
}

t_get_pokemon* preparar_mensaje_get(t_mensaje* mensaje){
		uint32_t tamanio;
		t_get_pokemon* mensaje_get = malloc(sizeof(t_get_pokemon));
		if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
			t_memoria_dinamica* particion_del_mensaje = mensaje -> payload;	
			mensaje_get -> id_mensaje = mensaje -> id_mensaje;
			tamanio = particion_del_mensaje -> tamanio;
			mensaje_get -> pokemon = malloc(tamanio);
			memcpy(mensaje_get -> pokemon, particion_del_mensaje -> contenido, tamanio);
		} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
			t_node* buddy_del_mensaje = mensaje -> payload;
			mensaje_get -> id_mensaje = mensaje -> id_mensaje;
			tamanio = buddy_del_mensaje -> bloque -> tamanio_mensaje;
			mensaje_get -> pokemon = malloc(tamanio);
			memcpy(mensaje_get -> pokemon, buddy_del_mensaje -> bloque -> contenido, tamanio);
		} else {
			log_error(logger, "No se reconoce el algoritmo de memoria.");
		}
	
		return mensaje_get;
}

t_catch_pokemon* preparar_mensaje_catch(t_mensaje* un_mensaje){
	uint32_t tamanio;
	t_catch_pokemon* mensaje_catch = malloc(sizeof(t_catch_pokemon));
	void* contenido_a_enviar;

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_catch -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  particion_del_mensaje -> tamanio - sizeof(uint32_t) * 2;
		contenido_a_enviar = particion_del_mensaje -> contenido;
		mensaje_catch -> pokemon = malloc(tamanio);
		memcpy(mensaje_catch -> pokemon, contenido_a_enviar, tamanio);
		memcpy(&(mensaje_catch -> posicion[0]), contenido_a_enviar + tamanio , sizeof(uint32_t));
		memcpy(&(mensaje_catch -> posicion[1]), contenido_a_enviar + tamanio + sizeof(uint32_t), sizeof(uint32_t));
		
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		t_node* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_catch -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  buddy_del_mensaje -> bloque -> tamanio_mensaje - sizeof(uint32_t) * 2;
		contenido_a_enviar = buddy_del_mensaje -> bloque -> contenido;
		mensaje_catch -> pokemon = malloc(tamanio);
		memcpy(mensaje_catch -> pokemon, contenido_a_enviar, tamanio);
		memcpy(&(mensaje_catch -> posicion[0]), contenido_a_enviar + tamanio , sizeof(uint32_t));
		memcpy(&(mensaje_catch -> posicion[1]), contenido_a_enviar + tamanio + sizeof(uint32_t), sizeof(uint32_t));
	} else {
		log_error(logger, "No se reconoce el algoritmo de memoria.");
	}

	return mensaje_catch;
}

t_localized_pokemon* preparar_mensaje_localized(t_mensaje* un_mensaje){
	uint32_t tamanio;
	t_localized_pokemon* mensaje_localized = malloc(sizeof(t_localized_pokemon));
	uint32_t offset = 0;
	void* contenido_a_enviar;
	uint32_t posicion[un_mensaje->tamanio_lista_localized];
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_localized -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_localized -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		tamanio = (particion_del_mensaje -> tamanio) - ((un_mensaje -> tamanio_lista_localized)*sizeof(uint32_t));
		contenido_a_enviar = particion_del_mensaje -> contenido;
		mensaje_localized -> pokemon = malloc(tamanio);
		memcpy(mensaje_localized -> pokemon, contenido_a_enviar, tamanio);
		mensaje_localized -> tamanio_lista = un_mensaje -> tamanio_lista_localized;
		mensaje_localized -> posiciones = list_create();
		offset+=tamanio;
		for(int i=0;i<(un_mensaje -> tamanio_lista_localized);i++){
			memcpy(&(posicion[i]), contenido_a_enviar + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			list_add(mensaje_localized -> posiciones, &posicion[i]);
		}
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		t_node* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_localized -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_localized -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		tamanio = (buddy_del_mensaje -> bloque -> tamanio_mensaje) - ((un_mensaje -> tamanio_lista_localized)*sizeof(uint32_t));
		contenido_a_enviar = buddy_del_mensaje -> bloque -> contenido;
		mensaje_localized -> pokemon = malloc(tamanio);
		memcpy(mensaje_localized -> pokemon, contenido_a_enviar, tamanio);
		mensaje_localized -> tamanio_lista = un_mensaje -> tamanio_lista_localized;
		mensaje_localized -> posiciones = list_create();
		offset+=tamanio;
		for(int i=0;i<(un_mensaje -> tamanio_lista_localized);i++){
			memcpy(&(posicion[i]), contenido_a_enviar + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			list_add(mensaje_localized -> posiciones, &posicion[i]);
		}
	} else {
		log_error(logger, "No se reconoce el algoritmo de memoria.");
	}
	return mensaje_localized;
}

t_caught_pokemon* preparar_mensaje_caught(t_mensaje* un_mensaje){
	t_caught_pokemon* mensaje_caught = malloc(sizeof(t_caught_pokemon));
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_caught -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_caught -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		memcpy(&(mensaje_caught -> resultado), particion_del_mensaje -> contenido, sizeof(uint32_t));
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		t_node* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_caught -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_caught -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		memcpy(&(mensaje_caught -> resultado), buddy_del_mensaje -> bloque -> contenido, sizeof(uint32_t));
	} else {
		log_error(logger, "No se reconoce el algoritmo de memoria.");
	}
	return mensaje_caught;
}

t_new_pokemon* preparar_mensaje_new(t_mensaje* un_mensaje){
	uint32_t tamanio;
	t_new_pokemon* mensaje_new = malloc(sizeof(t_new_pokemon));
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_new -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  particion_del_mensaje -> tamanio - sizeof(uint32_t) * 3;
		mensaje_new -> pokemon = malloc(tamanio);
		memcpy(mensaje_new -> pokemon, particion_del_mensaje -> contenido, tamanio);
		memcpy(&(mensaje_new -> posicion[0]), (particion_del_mensaje -> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_new -> posicion[1]), (particion_del_mensaje -> contenido) + tamanio + sizeof(uint32_t), sizeof(uint32_t));
		memcpy(&(mensaje_new -> cantidad), ((particion_del_mensaje -> contenido) + tamanio + sizeof(uint32_t)*2), sizeof(uint32_t));
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		t_node* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_new -> id_mensaje = un_mensaje -> id_mensaje;
		tamanio =  buddy_del_mensaje  -> bloque-> tamanio_mensaje - sizeof(uint32_t) * 3;
		mensaje_new -> pokemon = malloc(tamanio);
		memcpy(mensaje_new -> pokemon, buddy_del_mensaje -> bloque -> contenido, tamanio);
		memcpy(&(mensaje_new -> posicion[0]), (buddy_del_mensaje  -> bloque-> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_new -> posicion[1]), (buddy_del_mensaje -> bloque -> contenido) + tamanio + sizeof(uint32_t), sizeof(uint32_t));
		memcpy(&(mensaje_new -> cantidad), ((buddy_del_mensaje  -> bloque-> contenido) + tamanio + sizeof(uint32_t)*2), sizeof(uint32_t));
	} else {
		log_error(logger, "No se reconoce el algoritmo de memoria.");
	}
	return mensaje_new;
}

t_appeared_pokemon* preparar_mensaje_appeared(t_mensaje* un_mensaje){
	uint32_t tamanio;
	t_appeared_pokemon* mensaje_appeared = malloc(sizeof(t_appeared_pokemon));
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* particion_del_mensaje = un_mensaje -> payload;	
		mensaje_appeared -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_appeared -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		tamanio =  particion_del_mensaje -> tamanio - sizeof(uint32_t) * 2;
		mensaje_appeared -> pokemon = malloc(tamanio);
		memcpy(mensaje_appeared -> pokemon, particion_del_mensaje -> contenido, tamanio);
		memcpy(&(mensaje_appeared -> posicion[0]), (particion_del_mensaje -> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_appeared -> posicion[1]), ((particion_del_mensaje -> contenido) + tamanio + sizeof(uint32_t)), sizeof(uint32_t));
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		t_node* buddy_del_mensaje = un_mensaje -> payload;
		mensaje_appeared -> id_mensaje = un_mensaje -> id_mensaje;
		mensaje_appeared -> id_mensaje_correlativo = un_mensaje -> id_correlativo;
		tamanio =  buddy_del_mensaje -> bloque -> tamanio_mensaje - sizeof(uint32_t) * 2;
		mensaje_appeared -> pokemon = malloc(tamanio);
		memcpy(mensaje_appeared -> pokemon, buddy_del_mensaje -> bloque -> contenido, tamanio);
		memcpy(&(mensaje_appeared -> posicion[0]), (buddy_del_mensaje -> bloque -> contenido) + tamanio, sizeof(uint32_t));
		memcpy(&(mensaje_appeared -> posicion[1]), ((buddy_del_mensaje -> bloque -> contenido) + tamanio + sizeof(uint32_t)), sizeof(uint32_t));
	} else {
		log_error(logger, "No se reconoce el algoritmo de memoria.");
	}
	return mensaje_appeared;
}

void actualizar_ultima_referencia(t_mensaje* un_mensaje){

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* particion = un_mensaje -> payload;
		particion -> ultima_referencia = timestamp();
	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		t_node* buddy = un_mensaje -> payload;
		buddy -> bloque -> ultima_referencia = timestamp();
	  } else {
		log_error(logger, "...No se reconoce el algoritmo de memoria.");
	}

}

void establecer_tiempo_de_carga(t_mensaje* un_mensaje){

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* una_particion = un_mensaje -> payload;
		una_particion -> tiempo_de_carga = timestamp();
		} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		log_warning(logger,"%d", timestamp());
		uint64_t variable = timestamp();
		log_warning(logger,"%d",variable);
		((t_node*)(un_mensaje->payload))-> bloque -> tiempo_de_carga = variable;
	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria.");
	}
	actualizar_ultima_referencia(un_mensaje);
}


//---------------------------MENSAJES---------------------------//

void actualizar_mensajes_confirmados(t_ack* mensaje_confirmado){

	void actualizar_suscripto(void* mensaje){
		t_mensaje* mensaje_ok = mensaje;
		if(mensaje_ok -> id_mensaje == mensaje_confirmado -> id_mensaje){
			eliminar_suscriptor_de_enviados_sin_confirmar(mensaje_ok, mensaje_confirmado -> id_proceso);
			agregar_suscriptor_a_enviados_confirmados(mensaje_ok, mensaje_confirmado -> id_proceso);
		} 
	}

	bool es_la_misma_suscripcion(void* una_suscripcion){
		t_suscripcion* otra_suscripcion = una_suscripcion;
		return otra_suscripcion -> id_proceso == mensaje_confirmado -> id_proceso;
	}

	switch(mensaje_confirmado -> tipo_mensaje){
		case GET_POKEMON:
			if(!list_any_satisfy(lista_suscriptores_get, es_la_misma_suscripcion)){
				log_info(logger, "Me llego un ACK de un mensaje que no esta :(");
			} else {
				sem_wait(&mx_cola_get);
				list_iterate(cola_get, actualizar_suscripto);
				sem_wait(&mx_suscrip_get);
				borrar_mensajes_confirmados(GET_POKEMON, cola_get, lista_suscriptores_get);
				sem_post(&mx_suscrip_get);
				sem_post(&mx_cola_get);
			}
			break;

		case CATCH_POKEMON:
			
			if(!list_any_satisfy(lista_suscriptores_catch, es_la_misma_suscripcion)){
				log_info(logger, "Me llego un ACK de un mensaje que no esta :(");
			} else {
				sem_wait(&mx_cola_catch);
				list_iterate(cola_catch, actualizar_suscripto);
				sem_wait(&mx_suscrip_catch);
				borrar_mensajes_confirmados(CATCH_POKEMON, cola_catch, lista_suscriptores_catch);
				sem_post(&mx_suscrip_catch);
				sem_post(&mx_cola_catch);
			}			
			break;

		case LOCALIZED_POKEMON:
			if(!list_any_satisfy(lista_suscriptores_localized, es_la_misma_suscripcion)){
				log_info(logger, "Me llego un ACK de un mensaje que no esta :(");
			} else {
				sem_wait(&mx_cola_localized);
				list_iterate(cola_localized, actualizar_suscripto);
				sem_wait(&mx_suscrip_localized);
				borrar_mensajes_confirmados(LOCALIZED_POKEMON, cola_localized, lista_suscriptores_localized);
				sem_post(&mx_suscrip_localized);
				sem_post(&mx_cola_localized);
			}			
			break;

		case CAUGHT_POKEMON:
			if(!list_any_satisfy(lista_suscriptores_caught, es_la_misma_suscripcion)){
					log_info(logger, "Me llego un ACK de un mensaje que no esta :(");
			} else {
				sem_wait(&mx_cola_caught);
				list_iterate(cola_caught, actualizar_suscripto);
				sem_wait(&mx_suscrip_caught);
				borrar_mensajes_confirmados(CAUGHT_POKEMON, cola_caught, lista_suscriptores_caught);
				sem_post(&mx_suscrip_caught);
				sem_post(&mx_cola_caught);
			}
			break;

		case APPEARED_POKEMON:
			if(!list_any_satisfy(lista_suscriptores_appeared, es_la_misma_suscripcion)){
					log_info(logger, "Me llego un ACK de un mensaje que no esta :(");
			} else {
				sem_wait(&mx_cola_appeared);
				list_iterate(cola_appeared, actualizar_suscripto);
				sem_wait(&mx_suscrip_appeared);
				borrar_mensajes_confirmados(APPEARED_POKEMON, cola_appeared, lista_suscriptores_appeared);
				sem_post(&mx_suscrip_appeared);
				sem_post(&mx_cola_appeared);
			}			
			break;

		case NEW_POKEMON:
			if(!list_any_satisfy(lista_suscriptores_appeared, es_la_misma_suscripcion)){
					log_info(logger, "Me llego un ACK de un mensaje que no esta :(");
			} else {
				sem_wait(&mx_cola_new);
				list_iterate(cola_new, actualizar_suscripto);
				sem_wait(&mx_suscrip_new);
				borrar_mensajes_confirmados(NEW_POKEMON, cola_new, lista_suscriptores_new);
				sem_post(&mx_suscrip_new);
				sem_post(&mx_cola_new);
			}
			break;

		default:
			log_error(logger, "...El mensaje de id %d no se encuentra disponible.", mensaje_confirmado -> id_mensaje);
			break;
	}
	//Se chequea si un mensaje fue recibido por todos los suscriptores.
	//Si es así, se elimina el mensaje.
	
}

void borrar_mensajes_confirmados(op_code tipo_lista, t_list* cola_mensajes, t_list* suscriptores){

	t_list* cola_duplicada = list_duplicate(cola_mensajes);
	t_mensaje* mensaje_a_borrar;
	void borrar_mensaje_recibido_por_todos(void* mensaje){
		t_mensaje* un_mensaje = mensaje;

		bool es_el_mismo_mensaje(void* msj){
			t_mensaje* mensaje_buscado = msj;
			return (mensaje_buscado -> id_mensaje) == (un_mensaje -> id_mensaje);
		}

		if(mensaje_recibido_por_todos(un_mensaje, suscriptores)){
			mensaje_a_borrar = list_find(cola_mensajes, es_el_mismo_mensaje);
			eliminar_mensaje(mensaje_a_borrar);
		}
	}

	list_iterate(cola_duplicada, borrar_mensaje_recibido_por_todos);
	//log_info(logger, "...Los mensajes confirmados por todos los suscriptores fueron eliminados.");


}

bool mensaje_recibido_por_todos(void* mensaje, t_list* suscriptores){
	t_mensaje* un_mensaje = mensaje;
	t_list* lista_id_suscriptores = list_create();

	void* id_suscriptor(void* un_suscriptor){
		t_suscripcion* suscripto = un_suscriptor;
		uint32_t* id = &(suscripto -> id_proceso);
		return id;//Revisar si devuelve un id o un (*id)-
	}

	lista_id_suscriptores = list_map(suscriptores, id_suscriptor);

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

void eliminar_suscriptor_de_enviados_sin_confirmar(t_mensaje* mensaje, uint32_t suscriptor){

	bool es_el_mismo_suscriptor(void* un_suscripto){
		uint32_t* suscripto = un_suscripto;
		return suscriptor == (*suscripto);
	}

	list_remove_by_condition(mensaje -> suscriptor_enviado, es_el_mismo_suscriptor);

	if(!list_any_satisfy(mensaje -> suscriptor_enviado, es_el_mismo_suscriptor)){
		//log_info(logger, "...Se eliminó al suscriptor de la lista de enviados sin confirmar.");
	} else {
		//log_error(logger, "...No se eliminó al suscriptor confirmado.");
	}

}

void agregar_suscriptor_a_enviados_confirmados(t_mensaje* mensaje, uint32_t confirmacion){
	list_add(mensaje -> suscriptor_recibido, &confirmacion);

	bool es_el_mismo_suscriptor(void* un_suscripto){
		uint32_t* suscripto = un_suscripto;
		return confirmacion == (*suscripto);
	}

	if(list_any_satisfy(mensaje -> suscriptor_recibido, es_el_mismo_suscriptor)){
		//log_info(logger, "...Se agregó al suscriptor a la lista de enviados que recibieron el mensaje.");
	} else {
		//log_error(logger, "...No se agregó al suscriptor confirmado.");
	}
}

void enviar_mensajes(t_list* cola_de_mensajes, t_list* lista_suscriptores){ // hilo
	// sem_wait(&mensajes_a_enviar); --> le das post cada vez q un mensaje es agregado a memoria

	void mensajear_suscriptores(void* mensaje){
			t_mensaje* un_mensaje = mensaje;

			void mandar_mensaje(void* suscriptor){
				t_suscripcion* un_suscriptor = suscriptor;

				if(no_tiene_el_mensaje(un_mensaje, un_suscriptor -> id_proceso)){

					t_envio_mensaje* datos_de_mensaje = malloc(sizeof(t_envio_mensaje));

					datos_de_mensaje -> suscriptor = un_suscriptor;
					datos_de_mensaje -> mensaje = un_mensaje;

					uint32_t err = pthread_create(&hilo_envio_mensajes, NULL, main_hilo_mensaje, datos_de_mensaje);
						if(err != 0) {
							log_error(logger, "El hilo no pudo ser creado!!");
					}
				}
			}
			list_iterate(lista_suscriptores, mandar_mensaje);
	}

	list_iterate(cola_de_mensajes, mensajear_suscriptores);
}

void* main_hilo_mensaje(void* unos_datos_de_mensaje) {

	t_envio_mensaje* datos_de_mensaje = unos_datos_de_mensaje;
	void* mensaje_a_enviar;
	
	mensaje_a_enviar = preparar_mensaje(datos_de_mensaje -> mensaje);
	uint32_t tamanio_mensaje = size_mensaje(mensaje_a_enviar, datos_de_mensaje -> mensaje -> codigo_operacion);

	enviar_mensaje(datos_de_mensaje -> mensaje -> codigo_operacion, mensaje_a_enviar, datos_de_mensaje -> suscriptor -> socket, tamanio_mensaje);
	actualizar_ultima_referencia(datos_de_mensaje -> mensaje);
	agregar_suscriptor_a_enviados_sin_confirmar(datos_de_mensaje -> mensaje, datos_de_mensaje -> suscriptor -> id_proceso);

	log_error(logger, "...No se reconoce el algoritmo de memoria. ");
	
	return NULL;
}

bool no_tiene_el_mensaje(t_mensaje* mensaje, uint32_t un_suscripto){
	bool mensaje_enviado;
	bool mensaje_recibido;

	bool es_el_mismo_suscripto(void* suscripto){
		uint32_t* id_suscripcion = suscripto;
		return (*id_suscripcion) == un_suscripto;
	}

	mensaje_enviado  = list_any_satisfy(mensaje -> suscriptor_enviado, es_el_mismo_suscripto);
	mensaje_recibido = list_any_satisfy(mensaje -> suscriptor_recibido, es_el_mismo_suscripto);

	return !mensaje_enviado && !mensaje_recibido;
}

void agregar_suscriptor_a_enviados_sin_confirmar(t_mensaje* mensaje_enviado, uint32_t un_suscriptor){
	list_add(mensaje_enviado -> suscriptor_enviado, &un_suscriptor);
}

//--------------MEMORIA-------------//

void dump_de_memoria(){

	log_info(logger, "Se solicitó el dump de cache.");
    //Se tiene que crear un nuevo archivo y loggear ahi todas las cosas.
    iniciar_logger(config_broker -> memory_log, "broker");

    //Muestra el inicio del dump.
    time_t t;
    struct tm *tm;
    char fechayhora[25];
    t = time(NULL);
    tm = localtime(&t);
    strftime(fechayhora, 25, "%d/%m/%Y %H:%M:%S", tm);
    printf ("Hoy es: %s\n", fechayhora);

    log_info(logger_memoria, "Dump: %s", fechayhora);

    if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
    //Se empieza a loguear partición por partición --> considero la lista de particiones en memoria ("memoria auxiliar").
	sem_wait(&mx_memoria_particiones);
	list_iterate(memoria_con_particiones, dump_info_particion);
	sem_post(&mx_memoria_particiones);
    } else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
    //Ver que se necesita hacer para el BS
    list_iterate(memoria_cache, dump_info_buddy);

    } else {
        log_error(logger_memoria, "(? No se reconoce el algoritmo de memoria a implementar.");
    }
}


void guardar_en_memoria(t_mensaje* mensaje, void* mensaje_original){

	void* contenido = armar_contenido_de_mensaje(mensaje_original, mensaje -> codigo_operacion);

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")){
		uint32_t exponente = 0;
		if(mensaje -> tamanio_mensaje > config_broker -> size_min_memoria){
				exponente = obtenerPotenciaDe2(mensaje->tamanio_mensaje);
		}else{
		  exponente = config_broker -> size_min_memoria;
		}

		  t_node* primer_nodo = malloc(sizeof(t_node));
		  primer_nodo = (t_node*)memoria_cache->head->data;
		  uint32_t pudoGuardarlo =chequear_memoria();
		  if(pudoGuardarlo){
			 if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre,"FF")){
							 pudoGuardarlo = recorrer_first_fit(primer_nodo, exponente,  contenido, mensaje);
		     }
			 if(string_equals_ignore_case(config_broker ->algoritmo_particion_libre, "BF")){
							  pudoGuardarlo = recorrer_best_fit(primer_nodo,exponente, contenido, mensaje);
			 }
		 }
		 if(!pudoGuardarlo){
		  reemplazo_buddy(exponente, contenido, mensaje);
	     }
	}
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){

	guardar_particion(mensaje, contenido);

    } 

	free(contenido);

}

uint32_t chequear_memoria(){
	uint32_t size_memoria=0;
     void sumar_buddy(void* buddy){
    	 t_node* un_buddy = buddy;
    	 if(un_buddy -> bloque->ocupado)
    	 size_memoria = size_memoria + un_buddy -> bloque->tamanio_exponente;
     }

	list_iterate(memoria_cache,sumar_buddy);
	if(size_memoria == config_broker->size_memoria){
		log_error(logger,"no hay memoria");
		return 0;
	}

   return 1;
}

void reemplazo_buddy(uint32_t exponente, void* contenido, t_mensaje* mensaje){
	    consolidar_buddy(memoria_cache);
		t_node* buddy_victima = seleccionar_particion_victima_de_reemplazo_buddy();
		if(buddy_victima -> bloque->tamanio_exponente < exponente){
			reemplazo_buddy(exponente, contenido, mensaje);
		}

		uint32_t posicion = encontrar_indice(buddy_victima);
	    list_remove(memoria_cache, posicion);
		t_node* buddy_nuevo = armar_buddy(exponente,buddy_victima->bloque->base,mensaje,buddy_victima->bloque->ocupado, contenido);
		buddy_nuevo -> bloque->id = buddy_victima->bloque->id;
		buddy_nuevo -> bloque->padre = buddy_victima->bloque->padre;
		uint32_t pudoGuardarlo = 0;
	    if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre,"FF")){
		   pudoGuardarlo = recorrer_first_fit(buddy_nuevo, exponente,  contenido, mensaje);
	    }
		if(string_equals_ignore_case(config_broker ->algoritmo_particion_libre, "BF")){
		    	  pudoGuardarlo = recorrer_best_fit(buddy_nuevo,exponente, contenido, mensaje);
		}
		if(!pudoGuardarlo){
			reemplazo_buddy(exponente,contenido,mensaje);
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
        contenido = armar_contenido_localized(mensaje);//--> Corregir
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
    uint32_t tamanio = tamanio_pokemon + sizeof(uint32_t);
    void* contenido = malloc(tamanio);
    uint32_t offset = 0;

    op_code codigo = GET_POKEMON;
    memcpy(contenido + offset, &codigo, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(contenido + offset, (mensaje -> pokemon), tamanio_pokemon);
    offset += tamanio_pokemon;

    return contenido;
}


void* armar_contenido_catch(t_catch_pokemon* mensaje){
    uint32_t tamanio_pokemon = strlen(mensaje -> pokemon);
    uint32_t tamanio = tamanio_pokemon + (sizeof(uint32_t) * 2) + sizeof(uint32_t);
    void* contenido = malloc(tamanio);
    uint32_t offset = 0;

    op_code codigo = CATCH_POKEMON;
    memcpy(contenido + offset, &codigo, sizeof(uint32_t));
    offset += sizeof(uint32_t);

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

	op_code codigo = LOCALIZED_POKEMON;
	memcpy(contenido + offset, &codigo, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(contenido + offset, (mensaje -> pokemon), tamanio_pokemon);
	offset += tamanio_pokemon;

	void grabar_numero(void* un_numero){
		uint32_t* numero = un_numero;
		memcpy(contenido + offset, numero, sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}

	list_iterate(mensaje -> posiciones, grabar_numero);

    return contenido;
}


void* armar_contenido_caught(t_caught_pokemon* mensaje){
    void* contenido = malloc((sizeof(uint32_t)*2));
    uint32_t offset = 0;
    op_code codigo = CAUGHT_POKEMON;
    memcpy(contenido + offset, &codigo, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(contenido + offset, &(mensaje -> resultado), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    return contenido;
}

void* armar_contenido_appeared(t_appeared_pokemon* mensaje){
    uint32_t tamanio_pokemon = strlen(mensaje -> pokemon);
    uint32_t tamanio = tamanio_pokemon + (sizeof(uint32_t) * 3);
    void* contenido = malloc(tamanio);
    uint32_t offset = 0;

    op_code codigo = APPEARED_POKEMON;
    memcpy(contenido + offset, &codigo, sizeof(uint32_t));
    offset += sizeof(uint32_t);

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
    uint32_t tamanio = tamanio_pokemon + (sizeof(uint32_t) * 4);
    void* contenido = malloc(tamanio);
    uint32_t offset = 0;

    op_code codigo = NEW_POKEMON;
    memcpy(contenido + offset, &codigo, sizeof(uint32_t));
    offset += sizeof(uint32_t);

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

//--------------PARTICIONES-------------//


void reemplazar_particion_de_memoria(t_mensaje* mensaje, void* contenido_mensaje){

	t_memoria_dinamica* particion_a_reemplazar;

    particion_a_reemplazar = seleccionar_particion_victima_de_reemplazo();
    log_error(logger, "Base: %d", particion_a_reemplazar -> base);

    t_memoria_dinamica* particion_vacia = armar_particion(particion_a_reemplazar -> tamanio, particion_a_reemplazar -> base, NULL, 0, contenido_mensaje);

    uint32_t indice = encontrar_indice(particion_a_reemplazar);
    t_mensaje* mensaje_a_eliminar = encontrar_mensaje(particion_a_reemplazar -> base, particion_a_reemplazar -> codigo_operacion);
   	sem_wait(&mx_memoria_particiones);
    particion_a_reemplazar = list_replace(memoria_con_particiones, indice, particion_vacia);
	sem_post(&mx_memoria_particiones);
    eliminar_mensaje(mensaje_a_eliminar);
    //free(particion_a_reemplazar);
    log_info(logger, "Se libera una partición y se intenta ubicar nuevamente el mensaje.");

    guardar_particion(mensaje, contenido_mensaje);
}



t_memoria_dinamica* seleccionar_particion_victima_de_reemplazo(){

    t_memoria_dinamica* particion_victima;
    t_list* memoria_ordenada = list_create();
    t_list* memoria_duplicada = list_create();

    bool particion_ocupada(void* particion){
           t_memoria_dinamica* una_particion = particion;
           return (una_particion -> ocupado) != 0;
    }
	
	sem_wait(&mx_memoria_particiones);
    memoria_duplicada = list_filter(memoria_con_particiones, particion_ocupada);
	sem_post(&mx_memoria_particiones);

    bool fue_cargada_antes(void* particion1, void* particion2){
    	t_memoria_dinamica* una_particion = particion1;
    	t_memoria_dinamica* otra_particion = particion2;

    	return (una_particion -> tiempo_de_carga) < (otra_particion -> tiempo_de_carga) ;
    }

    bool fue_referenciada_antes(void* particion1, void* particion2){
		t_memoria_dinamica* una_particion = particion1;
		t_memoria_dinamica* otra_particion = particion2;
		return (una_particion -> tiempo_de_carga) < (otra_particion -> tiempo_de_carga) ;
    }

    if(string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "FIFO")){
		memoria_ordenada = list_sorted(memoria_duplicada, fue_cargada_antes);
    	particion_victima = list_get(memoria_ordenada, 0);
	} else if (string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "LRU")){
		memoria_ordenada = list_sorted(memoria_duplicada, fue_referenciada_antes);
		particion_victima = list_get(memoria_ordenada, 0);
	}
    log_error(logger,"Tamanio: %d", particion_victima -> tamanio_part);
    log_error(logger,"Base: %d", particion_victima -> base);

    return particion_victima;
}

t_node* seleccionar_particion_victima_de_reemplazo_buddy(){

    t_node* buddy_victima;
    t_list* memoria_ordenada = list_create();
	t_list* memoria_duplicada = list_create();

	bool buddy_ocupado(void* buddy){
	   t_node* un_buddy = buddy;
	   return (un_buddy -> bloque -> ocupado) != 0;
	}

	memoria_duplicada = list_filter(memoria_cache, buddy_ocupado);

	 bool fue_cargada_antes(void* buddy1, void* buddy2){
		t_node* un_buddy = buddy1;
		t_node* otro_buddy = buddy2;

		return (un_buddy -> bloque -> tiempo_de_carga) < (otro_buddy -> bloque -> tiempo_de_carga);
	}

	bool fue_referenciada_antes(void* buddy1, void* buddy2){
		t_node* un_buddy = buddy1;
		t_node* otro_buddy = buddy2;
		return (un_buddy -> bloque -> tiempo_de_carga) < (otro_buddy -> bloque -> tiempo_de_carga) ;

	}

    if(string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "FIFO")){
   		memoria_ordenada = list_sorted(memoria_duplicada, fue_cargada_antes);
       	buddy_victima = list_get(memoria_ordenada, 0);
   	} else if (string_equals_ignore_case(config_broker -> algoritmo_reemplazo, "LRU")){
   		memoria_ordenada = list_sorted(memoria_duplicada, fue_referenciada_antes);
   		buddy_victima = list_get(memoria_ordenada, 0);
   	}

    return buddy_victima;
}



uint32_t obtener_id(t_memoria_dinamica* particion){
    uint32_t id = 0;
    t_mensaje* mensaje = encontrar_mensaje(particion -> base, particion -> codigo_operacion);
    id = mensaje -> id_mensaje;
    return id;
}

uint32_t obtener_id_buddy(t_node* buddy){
    uint32_t id = 0;
    t_mensaje* mensaje = encontrar_mensaje_buddy(buddy -> bloque -> base, buddy -> bloque -> codigo_operacion);
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
        	sem_wait(&mx_cola_get);
			un_mensaje = list_find(cola_get, tiene_la_misma_base);
			sem_post(&mx_cola_get);
			break;
		case CATCH_POKEMON:
			sem_wait(&mx_cola_catch);
			un_mensaje = list_find(cola_catch, tiene_la_misma_base);
			sem_post(&mx_cola_catch);
			break;
		case LOCALIZED_POKEMON:
			sem_wait(&mx_cola_localized);
			un_mensaje = list_find(cola_localized, tiene_la_misma_base);
			sem_post(&mx_cola_localized);
			break;
		case CAUGHT_POKEMON:
			sem_wait(&mx_cola_caught);
			un_mensaje = list_find(cola_caught, tiene_la_misma_base);
			sem_post(&mx_cola_caught);
			break;
		case APPEARED_POKEMON:
			sem_wait(&mx_cola_appeared);
			un_mensaje = list_find(cola_appeared, tiene_la_misma_base);
			sem_wait(&mx_cola_appeared);
			break;
		case NEW_POKEMON:
			sem_wait(&mx_cola_new);
			un_mensaje = list_find(cola_new, tiene_la_misma_base);
			sem_post(&mx_cola_new);
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
        t_node* buddy = msj -> payload;
        return buddy -> bloque -> base == base_del_buddy_del_mensaje;
    }

    switch(codigo){
        case GET_POKEMON:
        	sem_wait(&mx_cola_get);
			un_mensaje = list_find(cola_get, tiene_la_misma_base);
			sem_post(&mx_cola_get);
			break;
		case CATCH_POKEMON:
			sem_wait(&mx_cola_catch);
			un_mensaje = list_find(cola_catch, tiene_la_misma_base);
			sem_post(&mx_cola_catch);
			break;
		case LOCALIZED_POKEMON:
			sem_wait(&mx_cola_localized);
			un_mensaje = list_find(cola_localized, tiene_la_misma_base);
			sem_post(&mx_cola_localized);
			break;
		case CAUGHT_POKEMON:
			sem_wait(&mx_cola_caught);
			un_mensaje = list_find(cola_caught, tiene_la_misma_base);
			sem_post(&mx_cola_caught);
			break;
		case APPEARED_POKEMON:
			sem_wait(&mx_cola_appeared);
			un_mensaje = list_find(cola_appeared, tiene_la_misma_base);
			sem_wait(&mx_cola_appeared);
			break;
		case NEW_POKEMON:
			sem_wait(&mx_cola_new);
			un_mensaje = list_find(cola_new, tiene_la_misma_base);
			sem_post(&mx_cola_new);
		    break;
		default:
            log_error(logger, "...No se puede reconocer el mensaje de esta partición.");
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
            log_error(logger, "...No hay suficiente tamaño para ubicar el mensaje en memoria.");
            reemplazar_particion_de_memoria(un_mensaje, contenido_mensaje);
        } else {
			sem_wait(&mx_memoria_particiones);
        	particion_a_ubicar = list_get(memoria_con_particiones, posicion_a_ubicar);
			nueva_particion = armar_particion(un_mensaje -> tamanio_mensaje, particion_a_ubicar -> base, un_mensaje, 1, contenido_mensaje);
			ubicar_particion(posicion_a_ubicar, nueva_particion);
			sem_post(&mx_memoria_particiones);
			guardar_contenido_de_mensaje(nueva_particion -> base, contenido_mensaje, nueva_particion -> tamanio);

        }

    }

    if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre,"BF")){
        posicion_a_ubicar = encontrar_mejor_ajuste(tamanio_a_buscar);

        if(posicion_a_ubicar == -1){
            log_error(logger, "...No hay suficiente tamaño para ubicar el mensaje en memoria.");
            reemplazar_particion_de_memoria(un_mensaje, contenido_mensaje);
        } else {
			sem_wait(&mx_memoria_particiones);
        	particion_a_ubicar = list_get(memoria_con_particiones, posicion_a_ubicar);
			nueva_particion = armar_particion(un_mensaje -> tamanio_mensaje, particion_a_ubicar -> base, un_mensaje, 1, contenido_mensaje);
			ubicar_particion(posicion_a_ubicar, nueva_particion);
			sem_post(&mx_memoria_particiones);
			guardar_contenido_de_mensaje(nueva_particion -> base, contenido_mensaje, nueva_particion -> tamanio);
        }


    }

}

void guardar_contenido_de_mensaje(uint32_t offset, void* contenido, uint32_t tamanio){

	if(contenido != NULL){
		sem_wait(&mx_memoria_cache);
		memcpy(memoria + offset, contenido, tamanio);
		sem_post(&mx_memoria_cache);
		log_info(logger, "Se guardó un mensaje en la particion de tamaño %d que comienza en la posicion %d.", tamanio, offset);
	} else {
		log_info(logger, "Se libera una partición de tamaño %d que comienza en la posicion %d.", tamanio, offset);
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

t_memoria_dinamica* armar_particion(uint32_t tamanio, uint32_t base, t_mensaje* mensaje, uint32_t ocupacion, void* contenido){

	t_memoria_dinamica* nueva_particion = malloc(sizeof(t_memoria_dinamica));

	if(mensaje != NULL){
		nueva_particion = (t_memoria_dinamica*) mensaje->payload;
		nueva_particion -> tamanio = tamanio;
		if(tamanio < (config_broker -> size_min_memoria)){
			nueva_particion -> tamanio_part = config_broker -> size_min_memoria;
		} else {
			nueva_particion -> tamanio_part = tamanio;
		}
		nueva_particion -> base = base;
		nueva_particion -> ocupado = ocupacion;
		nueva_particion -> codigo_operacion = mensaje -> codigo_operacion;
		nueva_particion -> contenido = contenido;
		nueva_particion = mensaje -> payload;
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
t_node* armar_buddy(uint32_t tamanio, uint32_t base, t_mensaje* mensaje, uint32_t ocupacion, void* contenido){

	t_node* nuevo_buddy = malloc(sizeof(t_node));

	if(mensaje != NULL){
		nuevo_buddy = (t_node*) mensaje->payload;
		nuevo_buddy -> bloque-> tamanio_exponente = tamanio;
		nuevo_buddy -> bloque-> tamanio_mensaje = mensaje -> tamanio_mensaje;
		nuevo_buddy -> bloque-> base = base;
		nuevo_buddy -> bloque-> ocupado = ocupacion;
		nuevo_buddy -> bloque-> codigo_operacion = mensaje -> codigo_operacion;
		nuevo_buddy -> bloque-> contenido = contenido;
		nuevo_buddy = mensaje -> payload;
    }
    return nuevo_buddy;
}

uint32_t encontrar_primer_ajuste(uint32_t tamanio){
    uint32_t indice_seleccionado = 0;

    bool es_particion_vacia(void* particion){
            t_memoria_dinamica* una_particion = particion;
            return !(una_particion -> ocupado);
    }
	sem_wait(&mx_memoria_particiones);
    t_list* lista_duplicada = list_filter(memoria_con_particiones, es_particion_vacia);
	sem_post(&mx_memoria_particiones);
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
	sem_wait(&mx_memoria_particiones);
    t_list* particiones_en_orden_creciente = list_sorted(memoria_con_particiones, es_de_menor_tamanio);
	sem_post(&mx_memoria_particiones);
    bool es_particion_vacia(void* part){
    	t_memoria_dinamica* part2 = part;
    	return !(part2 -> ocupado);
    }

    t_list* particiones_ordenadas = list_filter(particiones_en_orden_creciente, es_particion_vacia);
    t_memoria_dinamica* posible_particion = list_find(particiones_ordenadas, tiene_tamanio_suficiente);

    if(posible_particion!=NULL){
    	indice_seleccionado = encontrar_indice(posible_particion);
    } else {
    	indice_seleccionado = -1;
    }

    list_destroy(particiones_en_orden_creciente);
    return indice_seleccionado;
}

void destruir_particion(void* una_particion){
   t_memoria_dinamica* particion = una_particion;
   free(particion);
}

uint32_t encontrar_indice(void* memory){
	uint32_t indice_disponible = 0;
	uint32_t indice_buscador = 0;
	t_list* indices = list_create();
   	t_list* memoria_duplicada;
   	t_node* buddy;


	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* posible_particion = memory;
        sem_wait(&mx_memoria_particiones);
	       memoria_duplicada = list_duplicate(memoria_con_particiones);
	    sem_post(&mx_memoria_particiones);
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
		sem_wait(&mx_memoria_cache);
			memoria_duplicada = list_duplicate(memoria_cache);
		sem_post(&mx_memoria_cache);
	    buddy = memory;

		void obtener_indices_buddy(void* un_buddy){
					t_node* buddy_a_transformar = un_buddy;
					t_indice* un_indice = malloc(sizeof(t_indice));
					un_indice -> indice = indice_buscador;
					un_indice  -> base = buddy_a_transformar -> bloque -> base;
					list_add(indices, un_indice);
					indice_buscador++;
		}
		bool es_el_buddy(void* indice){
				t_indice* otro_indice = indice;
				return (otro_indice-> base) == (buddy->bloque -> base);
	    }
				list_iterate(memoria_duplicada, obtener_indices_buddy);
				t_indice* indice_elegido = list_find(indices, es_el_buddy);

				if(indice_elegido!=NULL){
					indice_disponible = indice_elegido -> indice;
				} else {
					indice_disponible = 0;
					log_error(logger, "El indice no pudo obtenerse correctamente.");
				}
	} else {
		log_error(logger, "No se reconoce el algoritmo de memoria (?");
	}

	list_destroy(indices);
	list_destroy(memoria_duplicada);

	return indice_disponible;

}

//--------------------------------CONSOLIDACION_P--------------------------------//


void consolidar_particiones_dinamicas(t_list* memoria){
	t_list* memoria_duplicada = list_create();
	sem_wait(&mx_memoria_particiones);
	memoria_duplicada = list_duplicate(memoria);
	sem_post(&mx_memoria_particiones);

	uint32_t contador = 0;
    void consolidar_particiones_contiguas(void* particion){

        if(tiene_siguiente_buddy(contador)){
        	if(ambas_estan_vacias(contador, contador + 1)){
        		consolidar_particiones(contador, contador + 1);
        	}
        }
        contador++;
    }

    list_iterate(memoria_duplicada, consolidar_particiones_contiguas);

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
	sem_wait(&mx_memoria_particiones);
    t_memoria_dinamica* una_particion = list_get(memoria_con_particiones, primer_elemento);
    t_memoria_dinamica* particion_siguiente = list_remove(memoria_con_particiones, elemento_siguiente);

    uint32_t tamanio_particion_consolidada    = (una_particion -> tamanio_part) + (particion_siguiente -> tamanio_part);
    t_memoria_dinamica* particion_consolidada = armar_particion(tamanio_particion_consolidada, (una_particion -> base), NULL, 0, NULL);

    ubicar_particion(primer_elemento, particion_consolidada);
	sem_post(&mx_memoria_particiones);
    log_info(logger, "Se consolidan las particiones de base %d y base %d en una particion de tamanio %d", una_particion -> base, particion_siguiente -> base, tamanio_particion_consolidada);
}

void compactar_particiones_dinamicas(t_list* memoria){
    t_list* particiones_vacias = list_create();
    t_list* particiones_ocupadas = list_create();

    uint32_t posicion_lista = 0;


	bool es_particion_vacia(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return !(una_particion -> ocupado);
	}

	bool es_particion_ocupada(void* particion){
		t_memoria_dinamica* una_particion = particion;
		return (una_particion -> ocupado);
	}
	sem_wait(&mx_memoria_particiones);
	particiones_vacias = list_filter(memoria, es_particion_vacia);
	particiones_ocupadas = list_filter(memoria, es_particion_ocupada);
	
	list_clean(memoria);
	list_add_all(memoria, particiones_ocupadas);
	list_add_all(memoria, particiones_vacias);
	sem_post(&mx_memoria_particiones);

	void actualizar_base(void* particion){
		t_memoria_dinamica* una_particion = particion;
		una_particion -> base = obtener_nueva_base(una_particion, posicion_lista);
		posicion_lista++;
	}

	sem_wait(&mx_memoria_particiones);
	list_iterate(memoria, actualizar_base);
	sem_post(&mx_memoria_particiones);

	compactar_memoria_cache(memoria);
	
	list_destroy(particiones_vacias);
	list_destroy(particiones_ocupadas);

	particiones_liberadas = 0;

}

uint32_t obtener_nueva_base(t_memoria_dinamica* una_particion, uint32_t indice_tope){
	uint32_t nueva_base = 0;
	sem_wait(&mx_memoria_particiones);
	t_list* memoria_duplicada = list_duplicate(memoria_con_particiones);
	sem_post(&mx_memoria_particiones);
	t_memoria_dinamica* particion_buscada;

	if(indice_tope > 0){
		particion_buscada = list_get(memoria_duplicada, (indice_tope-1));
		if(particion_buscada!=NULL){
			nueva_base = (particion_buscada -> base) + (particion_buscada -> tamanio);
		} else {
			log_error(logger, "No se puede actualizar la base para compactar :/");
		}
	} else {
		nueva_base = 0;
	}
	//nueva_base ++;
	return nueva_base;
}

void compactar_memoria_cache(t_list* lista_particiones){

    void reescribir_memoria(void* particion){
        t_memoria_dinamica* una_particion = particion;
        if(una_particion -> ocupado){
        	guardar_contenido_de_mensaje((una_particion -> base), (una_particion -> contenido),(una_particion -> tamanio));
        }
    }
	sem_wait(&mx_memoria_particiones);
	list_iterate(lista_particiones, reescribir_memoria);
	sem_post(&mx_memoria_particiones);
}

void eliminar_mensaje(void* mensaje){
	t_mensaje* un_mensaje = mensaje;
	list_destroy(un_mensaje -> suscriptor_enviado);
	list_destroy(un_mensaje -> suscriptor_recibido);
	liberar_mensaje_de_memoria(un_mensaje);
	free(un_mensaje -> payload);
	free(un_mensaje);
}

void liberar_mensaje_de_memoria(t_mensaje* mensaje){

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "PARTICIONES")){
		t_memoria_dinamica* particion_buscada = mensaje -> payload;
		bool es_la_particion(void* particion){
			t_memoria_dinamica* una_particion = particion;
			return (una_particion -> base) == (particion_buscada -> base);
		}
		sem_wait(&mx_memoria_particiones);
		t_memoria_dinamica* particion_a_liberar = list_find(memoria_con_particiones, es_la_particion);
		sem_post(&mx_memoria_particiones);
		uint32_t indice = encontrar_indice(particion_a_liberar);

		t_memoria_dinamica* particion_vacia = armar_particion(particion_a_liberar -> tamanio, particion_a_liberar -> base, NULL, 0, NULL);
		sem_wait(&mx_memoria_particiones);
		particion_a_liberar = list_replace(memoria_con_particiones, indice, particion_vacia);
		sem_post(&mx_memoria_particiones);

		eliminar_de_message_queue(mensaje, mensaje -> codigo_operacion);
		log_info(logger, "El mensaje fue eliminado correctamente.");

		
		consolidar_particiones_dinamicas(memoria_con_particiones);

		if(config_broker -> frecuencia_compactacion == 0 || (particiones_liberadas == (config_broker -> frecuencia_compactacion))){
			compactar_particiones_dinamicas(memoria_con_particiones);
		}

		sem_wait(&sem_particion_liberada);
		particiones_liberadas++;
		sem_post(&sem_particion_liberada);
		

	} else if(string_equals_ignore_case(config_broker -> algoritmo_memoria, "BS")){
		t_node* buddy_buscado = mensaje ->payload;
		bool es_el_buddy(void* buddy){
					t_memoria_dinamica* un_buddy = buddy;
					return (un_buddy->base) == (buddy_buscado->bloque-> base);
	  }
		t_node* buddy_a_liberar = list_find(memoria_cache, es_el_buddy);

				uint32_t indice = encontrar_indice(buddy_a_liberar);

				t_node* buddy_vacio = armar_buddy(buddy_a_liberar -> bloque -> tamanio_exponente, buddy_a_liberar -> bloque -> base, NULL, 0, NULL);

				buddy_a_liberar = list_replace(memoria_cache, indice, buddy_vacio);
				eliminar_de_message_queue(mensaje, mensaje -> codigo_operacion);
				log_info(logger, "El mensaje fue eliminado correctamente.");


	} else {
		log_error(logger, "...No se reconoce el algoritmo de memoria.");
	}

}

void eliminar_de_message_queue(t_mensaje* mensaje, op_code codigo){

	bool es_el_mismo_mensaje(void* msj){
		t_mensaje* el_mensaje = msj;
		return (el_mensaje -> id_mensaje) == (mensaje -> id_mensaje);
	}

	switch(codigo){
	case GET_POKEMON:
		sem_wait(&mx_cola_get);
		list_remove_by_condition(cola_get, es_el_mismo_mensaje);
		sem_post(&mx_cola_get);
		break;
	case CATCH_POKEMON:
		sem_wait(&mx_cola_catch);
		list_remove_by_condition(cola_catch, es_el_mismo_mensaje);
		sem_post(&mx_cola_catch);
		break;
	case APPEARED_POKEMON:
		sem_wait(&mx_cola_appeared);
		list_remove_by_condition(cola_appeared, es_el_mismo_mensaje);
		sem_post(&mx_cola_appeared);
		break;
	case LOCALIZED_POKEMON:
		sem_wait(&mx_cola_localized);
		list_remove_by_condition(cola_localized, es_el_mismo_mensaje);
		sem_post(&mx_cola_localized);
		break;
	case CAUGHT_POKEMON:
		sem_wait(&mx_cola_caught);
		list_remove_by_condition(cola_caught, es_el_mismo_mensaje);
		sem_post(&mx_cola_caught);
		break;
	case NEW_POKEMON:
		sem_wait(&mx_cola_new);
		list_remove_by_condition(cola_new, es_el_mismo_mensaje);
		sem_post(&mx_cola_new);
		break;
	default:
		log_error(logger, "...No se pudo eliminar el mensaje de la message queue.");
		break;
	}
}

void dump_info_particion(void* particion){
    t_memoria_dinamica* una_particion = particion;
    char* ocupado = malloc(sizeof(char));
    ocupado = "L";

    if(una_particion -> ocupado != 0) {
        ocupado = "X";
    }

    uint32_t* base = memoria + (una_particion -> base);//Revisar que apunte al malloc
	//uint32_t* limite = memoria + (una_particion -> base) + (una_particion -> tamanio);
    uint32_t tamanio = una_particion -> tamanio;
    uint32_t valor_lru = una_particion -> ultima_referencia;
    //Relacionar al mensaje con la partición
    char* cola_del_mensaje = obtener_cola_del_mensaje(una_particion);
    uint32_t id_del_mensaje = obtener_id(una_particion);

    //Revisar el tema de la dirección de memoria para loggear-->(&base?).
    log_info(logger_memoria, "Particion %d: %p.  [%s] Size: %d b LRU:%d Cola:%s ID:%d", numero_particion, base, (*ocupado), tamanio, valor_lru, cola_del_mensaje, id_del_mensaje);
    numero_particion++;
    free(cola_del_mensaje);
    //Revisar si hay que liberar la partición!
}

void dump_info_buddy(void* buddy){
    t_node* un_buddy = buddy;
    char* ocupado = malloc(sizeof(char));
    ocupado = "L";

    if(un_buddy -> bloque -> ocupado != 0) {
        ocupado = "X";
    }
    uint32_t* base = memoria + (un_buddy -> bloque -> base);//Revisar que apunte al malloc
	//uint32_t* limite = memoria + (un_buddy -> base) + (un_buddy -> tamanio_exponente);
    uint32_t tamanio = un_buddy  -> bloque-> tamanio_exponente;
    uint32_t valor_lru = un_buddy -> bloque -> ultima_referencia;
    //Relacionar al mensaje con la partición
    char* cola_del_mensaje = obtener_cola_del_mensaje_buddy(un_buddy);
    uint32_t id_del_mensaje = obtener_id_buddy(un_buddy);

    //Revisar el tema de la dirección de memoria para loggear-->(&base?).
    log_info(logger_memoria, "Buddy %d: %p.  [%s] Size: %d b LRU:%d Cola:%s ID:%d", numero_particion, base, (*ocupado), tamanio, valor_lru, cola_del_mensaje, id_del_mensaje);
    numero_particion++;
    free(cola_del_mensaje);
    free(ocupado);
    //Revisar si hay que liberar la partición!
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

char* obtener_cola_del_mensaje_buddy(t_node* un_buddy){
    char* una_cola;
    switch(un_buddy -> bloque -> codigo_operacion){
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
	uint64_t result = ((unsigned long long )valor.tv_sec) * 1000 + ((unsigned long) valor.tv_usec) / 1000;
	return result;
}

//-------------BUDDY_SYSTEM-------------//

uint32_t obtenerPotenciaDe2(uint32_t n)
{
	//pregunto si el size es pot de 2
if( (n & (n - 1)) == 0){
	return n;
}

	  int res = 0;
	    for (int i=n; i>=1; i--)
	    {
	        // If i is a power of 2
	        if ((i & (i-1)) == 0)
	        {
	            res = i;
	            break;
	        }
	    }
	    return (res*2);
}


t_node* crear_nodo(uint32_t tamanio)
{
  // Allocate memory for new node
  t_node* node = (t_node*)malloc(sizeof(t_node));

  // Assign data to this node
  node -> bloque = malloc(sizeof(t_memoria_buddy));
  node -> bloque -> tamanio_exponente = tamanio;
  node->bloque->tamanio_mensaje = 0;
  node -> bloque ->base = 0;
  node -> bloque -> ocupado =0;
  node ->bloque ->codigo_operacion =0;
  node -> bloque->contenido ="";
  node-> bloque->tiempo_de_carga = 0;
  node->bloque -> ultima_referencia = 0;
  node -> bloque -> id = crear_id_nodo();
  // Initialize left and right children as NULL
  node -> izquierda = malloc(sizeof(t_node));
  node -> izquierda -> bloque = malloc(sizeof(t_memoria_buddy));
  node -> izquierda->bloque->padre = node->bloque ->id ;
  node -> derecha = malloc(sizeof(t_node));
  node -> derecha->bloque = malloc(sizeof(t_memoria_buddy));
  node -> derecha -> bloque -> padre = node-> bloque -> id;
  return(node);
}


void arrancar_buddy(){
	memoria_cache = list_create();
	t_node* root = crear_nodo(config_broker -> size_memoria);
	list_add(memoria_cache, root);
}

void asignar_nodo(t_node* node,void* contenido, t_mensaje* mensaje, uint32_t exponente){
    node ->  bloque -> ocupado =1;
    node -> bloque -> tiempo_de_carga =  timestamp();
    node ->  bloque-> ultima_referencia =  timestamp();
    node -> bloque -> tamanio_mensaje = mensaje ->tamanio_mensaje;
    node -> bloque -> codigo_operacion = mensaje -> codigo_operacion;
    node -> bloque -> contenido = contenido;
    mensaje -> payload = node;
    if(list_size(memoria_cache) > 1){
      	t_node* ultimo_buddy= list_get( memoria_cache, list_size(memoria_cache)-1);
      	node->bloque->base =  ultimo_buddy -> bloque ->base +  ultimo_buddy -> bloque->tamanio_exponente + 1;
      	guardar_contenido_de_mensaje(exponente, contenido, mensaje -> tamanio_mensaje);
    }else{
        node -> bloque-> base = 0;
    	guardar_contenido_de_mensaje(0, contenido, exponente);
    }

   chequear_buddy(node);
}

void chequear_buddy(t_node* node){

   if(list_size(memoria_cache) > 1){
        uint32_t posicion_a_ubicar = encontrar_hermano(node->bloque);
        if (posicion_a_ubicar){
        void* unused = list_replace(memoria_cache, posicion_a_ubicar, node->bloque);
        }
        else{
          crear_companieros(node);
        }

   } else{
       crear_companieros(node);
   }
}

uint32_t encontrar_hermano(t_node* buddy){
	uint32_t indice_disponible = 0;
	uint32_t indice_buscador = 0;
	t_list* indices = list_create();

	sem_wait(&mx_memoria_cache);
		t_list* memoria_duplicada = list_duplicate(memoria_cache);
	sem_post(&mx_memoria_cache);

	bool encontrar_al_companero(void* buddy1){
        t_node* un_buddy= buddy1;
        return un_buddy -> bloque ->padre == buddy -> bloque->padre &&  buddy -> bloque->id != un_buddy -> bloque->id;
    }
	void obtener_indices(void* buddy){
				t_node* particion_a_transformar = buddy;
				t_indice* un_indice = malloc(sizeof(t_indice));
				un_indice -> indice = indice_buscador;
				un_indice -> base = particion_a_transformar -> bloque -> base;
				list_add(indices, un_indice);
				indice_buscador++;
	}
	list_iterate(memoria_duplicada, obtener_indices);
	t_indice* indice_elegido = list_find(indices, encontrar_al_companero);

	if(indice_elegido!=NULL){
		indice_disponible = indice_elegido -> indice;
	} else {
		indice_disponible = 0;
		log_error(logger, "El indice no pudo obtenerse correctamente.");
	}

	list_destroy(indices);
	list_destroy(memoria_duplicada);

	return indice_disponible;
}


void crear_companieros(t_node* node){
   list_add(memoria_cache, node);
      if(node->bloque->tamanio_exponente < config_broker-> size_memoria){
         t_node* el_buddy = crear_nodo( node ->bloque ->tamanio_exponente);
         el_buddy -> bloque -> base = node ->bloque->base + node->bloque->tamanio_exponente;
         el_buddy -> bloque -> padre = node -> bloque -> padre;
         list_add(memoria_cache, el_buddy);
      } else{
          log_error(logger_memoria,"se creo un buddy igual a la memoria, no tendra un companero");
      }
}

uint32_t crear_id_nodo(){
   return nodo_id ++;
}

uint32_t recorrer_first_fit(t_node* nodo, uint32_t exponente, void* contenido, t_mensaje*  mensaje){
    if(nodo == NULL || exponente < config_broker -> size_min_memoria) {
        return 0;
    }
    asignado  = 0;
    if (nodo->bloque->ocupado == 0) {
    	if(nodo -> bloque -> tamanio_exponente == exponente){
        asignar_nodo(nodo, contenido, mensaje, exponente);
        asignado = 1;
        return 1;
    	}
    	else {
        	nodo -> izquierda = crear_nodo(nodo -> bloque -> tamanio_exponente / 2);
        	recorrer_first_fit(nodo -> izquierda, exponente, contenido, mensaje);
        }
    }

   if (asignado ==0 &&nodo -> izquierda != NULL && nodo -> izquierda -> bloque -> tamanio_exponente > exponente) {
    	recorrer_first_fit(nodo->izquierda, exponente,contenido,mensaje);
    }

   if(asignado == 0) {
    	if (nodo -> derecha == NULL) {
    		if(nodo -> bloque->tamanio_exponente > exponente) {
    	        nodo -> derecha = crear_nodo(nodo -> bloque -> tamanio_exponente / 2);
    		} else {
    			 nodo -> derecha = crear_nodo(nodo -> bloque -> tamanio_exponente);
    		}
    	    }
        asignado = recorrer_first_fit(nodo -> derecha,exponente, contenido,mensaje);
    } else {
    	return 1;
    }
    return asignado;
}

uint32_t recorrer_best_fit(t_node* nodo, uint32_t exponente, void* contenido, t_mensaje* mensaje){
    if(nodo == NULL || exponente < config_broker -> size_min_memoria) {
        return 0;
    }
    asignado  = 0;
    if (nodo->bloque->tamanio_exponente == exponente && nodo->bloque->ocupado == 0) {
        asignar_nodo(nodo, contenido, mensaje, exponente);
        return 1;
    }

    if (exponente > config_broker-> size_min_memoria && nodo -> izquierda == NULL) {
        nodo -> izquierda = crear_nodo(nodo -> bloque -> tamanio_exponente / 2);
    }

    if (nodo -> izquierda != NULL && nodo -> izquierda -> bloque -> tamanio_exponente > exponente) {
    	recorrer_best_fit(nodo->izquierda, exponente,contenido,mensaje);
    }

    asignado = recorrer_best_fit(nodo -> izquierda, exponente, contenido,mensaje);
    if(asignado == 0) {
    	if (nodo -> derecha == NULL) {
    		if(nodo -> bloque->tamanio_exponente > exponente) {
    	        nodo -> derecha = crear_nodo(nodo -> bloque -> tamanio_exponente / 2);
    		} else {
    			 nodo -> derecha = crear_nodo(nodo -> bloque -> tamanio_exponente);
    		}
    	    }
        asignado = recorrer_best_fit(nodo -> derecha,exponente, contenido,mensaje);
    } else {
    	return 1;
    }
    return asignado;
}
//void consolidacion_buddy_systeam(t_node*  nodo)

void consolidar_buddy(t_list* memoria){
	t_list* memoria_duplicada = list_create();
	sem_wait(&mx_memoria_cache);
	memoria_duplicada = list_duplicate(memoria);
	sem_post(&mx_memoria_cache);

	uint32_t contador = 0;
    void consolidar_buddies_contiguos(void* buddy){

        if(tiene_siguiente(contador)){
        	if(ambas_estan_vacias(contador, contador + 1)){
        		consolidar_particiones(contador, contador + 1);
        	}
        }
        contador++;
    }

    list_iterate(memoria_duplicada, consolidar_buddies_contiguos);

}

bool tiene_siguiente_buddy(uint32_t posicion){
    return (list_size(memoria_cache) - 1) > posicion;
}

bool ambas_estan_vacias_buddy(uint32_t una_posicion, uint32_t posicion_siguiente){

    t_node* buddy       = list_get(memoria_cache, una_posicion);
    t_node* un_buddy    = list_get(memoria_cache, posicion_siguiente);
    uint32_t resultado = 0;

    if(un_buddy != NULL){
    	if(un_buddy!= NULL){
    		resultado = (!(un_buddy -> bloque -> ocupado)) && (!(un_buddy -> bloque -> ocupado));
    	} else {
    		log_error(logger, "La segunda particion a consolidar no fue encontrada.");
    	}
    } else {
    	log_error(logger, "La primer particion a consolidar no fue encontrada.");
    }

    return resultado;
}

void consolidar_buddies(uint32_t primer_elemento, uint32_t elemento_siguiente){
	sem_wait(&mx_memoria_cache);
    t_node* un_buddy = list_get(memoria_con_particiones, primer_elemento);
    t_node* buddy_siguiente = list_remove(memoria_con_particiones, elemento_siguiente);

    uint32_t tamanio_buddy_consolidado= (un_buddy->bloque-> tamanio_exponente) + (buddy_siguiente ->bloque-> tamanio_exponente);
    t_node* buddy_consolidado =armar_buddy(tamanio_buddy_consolidado, (un_buddy->bloque -> base), NULL, 0, NULL);

    //ubicar_particion(primer_elemento, buddy_consolidado);
	sem_post(&mx_memoria_cache);
}


void iniciar_semaforos_broker() {
	//REVISAR INICIALIZCIONES
	sem_init(&mx_memoria_particiones, 0, 1);
	sem_init(&semaforo, 0, 1);
	sem_init(&sem_particion_liberada, 0,1);
	sem_init(&mutex_id, 0, 1);
    sem_init(&mx_cola_get, 0, 1);
	sem_init(&mx_cola_catch, 0, 1);
	sem_init(&mx_cola_localized, 0, 1);
	sem_init(&mx_cola_caught, 0, 1);
    sem_init(&mx_cola_appeared, 0, 1);
    sem_init(&mx_cola_new, 0, 1);
    sem_init(&mx_suscrip_get, 0, 1);
	sem_init(&mx_suscrip_catch, 0, 1);
	sem_init(&mx_suscrip_localized, 0, 1);
	sem_init(&mx_suscrip_caught, 0, 1);
    sem_init(&mx_suscrip_appeared, 0, 1);
    sem_init(&mx_suscrip_new, 0, 1);
    sem_init(&mx_memoria_cache, 0, 1);
    sem_init(&mx_copia_memoria, 0, 1);
    sem_init(&muteadito,0,1);


}

void terminar_hilos_broker(){
	pthread_detach(hilo_envio_mensajes);
	pthread_detach(hilo_signal);
    //pthread_detach(hilo_mensaje);
}

void liberar_semaforos_broker(){
	sem_destroy(&sem_particion_liberada);
	sem_destroy(&mx_cola_get);
    sem_destroy(&mx_cola_catch);
    sem_destroy(&mx_cola_localized);
    sem_destroy(&mx_cola_caught);
    sem_destroy(&mx_cola_appeared);
    sem_destroy(&mx_cola_new);
    sem_destroy(&mx_suscrip_get);
    sem_destroy(&mx_suscrip_catch);
    sem_destroy(&mx_suscrip_localized);
    sem_destroy(&mx_suscrip_caught);
    sem_destroy(&mx_suscrip_appeared);
    sem_destroy(&mx_suscrip_new);
    sem_destroy(&mx_memoria_cache);
    sem_destroy(&mx_copia_memoria);
	sem_destroy(&mx_memoria_particiones);
    sem_destroy(&semaforo);
    sem_destroy(&mutex_id);
}
