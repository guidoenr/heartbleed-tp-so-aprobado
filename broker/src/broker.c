#include "broker.h"

int main(void) {
	iniciar_programa();

	/*enviar_mensajes(cola_get, lista_suscriptores_get);
	enviar_mensajes(cola_catch, lista_suscriptores_catch);
	enviar_mensajes(cola_localized, lista_suscriptores_localized);
	enviar_mensajes(cola_caught, lista_suscriptores_caught);
	enviar_mensajes(cola_appeared, lista_suscriptores_appeared);
	enviar_mensajes(cola_new, lista_suscriptores_new);*/

	//uint32_t thread = pthread_create(&hilo_mensaje, NULL, gestionar_mensaje, NULL);

	terminar_programa(logger);
	return 0;
}

void iniciar_programa(){
	id_mensaje_univoco = 0;
	leer_config();
	iniciar_logger(config_broker->log_file, "broker");
	reservar_memoria();
	iniciar_semaforos();
	crear_colas_de_mensajes();
    crear_listas_de_suscriptores();
	log_info(logger, "IP: %s", config_broker -> ip_broker);
	iniciar_servidor(config_broker -> ip_broker, config_broker -> puerto);

}

void reservar_memoria(){
	//La memoria en sí tiene que ser un void*
	//La memoria_cache sería la lista auxiliar para esa memoria?
	memoria_cache = malloc(config_broker ->  size_memoria);
	if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")){
		arrancar_buddy();
	}

    if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"PARTICIONES")){
		memoria_con_particiones = list_create();
        iniciar_memoria_particiones(memoria_con_particiones);
        //Al principio la lista tiene un unico elemento que es la memoria entera.
	}
}

void iniciar_semaforos(){
    sem_init(&semaforo, 0, 1);
    sem_init(&mutex_id, 0, 1);
}


void destruir_semaforos(){
	sem_destroy(&semaforo);
	sem_destroy(&mutex_id);
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

	config = config_create("broker.config");

	if(config == NULL){
		    	printf("No se pudo encontrar el path del config.");
		    	exit(-2);
	}
	config_broker -> size_memoria 			   = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> size_min_memoria 		   = config_get_int_value(config, "TAMANO_MEMORIA");
	config_broker -> algoritmo_memoria 		   = config_get_string_value(config, "ALGORITMO_MEMORIA");
	config_broker -> algoritmo_reemplazo 	   = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	config_broker -> algoritmo_particion_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	config_broker -> ip_broker 				   = config_get_string_value(config, "IP_BROKER");
	config_broker -> puerto 				   = config_get_string_value(config, "PUERTO_BROKER");
	config_broker -> frecuencia_compactacion   = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
	config_broker -> log_file 				   = config_get_string_value(config, "LOG_FILE");

}

void liberar_config(t_config_broker* config) {
	free(config -> algoritmo_memoria);
	free(config -> algoritmo_reemplazo);
	free(config -> algoritmo_particion_libre);
	free(config -> ip_broker);
	free(config -> puerto);
	free(config -> log_file);
	free(config);
}

void terminar_programa(t_log* logger){
	liberar_listas();
	liberar_config(config_broker);
	liberar_logger(logger);
	liberar_memoria_cache();
	destruir_semaforos();
	config_destroy(config);
}

void liberar_memoria_cache(){
	free(memoria_cache);
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
	void* msg;
	recibir_paquete(cliente_fd, &size, codigo_op, msg);
	cod_op = (*codigo_op);
	log_info(logger,"Codigo de operacion %d", cod_op);
	void* mensaje_e_agregar = deserealizar_paquete(msg, *codigo_op, size);

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
			recibir_suscripcion(mensaje_e_agregar);
			break;
		case ACK:
			actualizar_mensajes_confirmados(mensaje_e_agregar);
			break;
		case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
	free(codigo_op);
//REVISAR DONDE Y CUANDO HACER EL FREE DE LOS MENSAJES QUE SE AGREGARON
}


void agregar_mensaje(uint32_t cod_op, uint32_t size, void* mensaje, uint32_t socket_cliente){
	log_info(logger, "Agregando mensaje");
	log_info(logger, "Size: %d", size);
	log_info(logger, "Socket_cliente: %d", socket_cliente);
	log_info(logger, "Payload: %s", (char*) mensaje);

	uint32_t malloc_size = size + sizeof(uint32_t) * 2 + sizeof(op_code);
	t_mensaje* mensaje_a_agregar = malloc(malloc_size);
	uint32_t nuevo_id     = generar_id_univoco();

	//HAY QUE ARREGLAR ESTO
	//PREGUNTAR POR EL VOID*

	//mensaje -> id_mensaje = nuevo_id;
	mensaje_a_agregar -> id_mensaje = nuevo_id;

	//Este id habría que ver como pasarlo al id de un t_get por ejemplo.

	mensaje_a_agregar -> payload   = mensaje;
	mensaje_a_agregar -> codigo_operacion    = cod_op;
	mensaje_a_agregar -> suscriptor_enviado  = list_create();
	mensaje_a_agregar -> suscriptor_recibido = list_create();


	sem_post(&mutex_id);
	send(socket_cliente, &(nuevo_id) , sizeof(uint32_t), 0); //Avisamos,che te asiganmos un id al mensaje
	sem_post(&mutex_id);


	// En memori deberia guardarse solo el contenido, no todo el mensaje.
	guardar_en_memoria(mensaje);

	sem_wait(&semaforo);
	encolar_mensaje(mensaje, cod_op);
	sem_post(&semaforo);
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
				list_add(cola_get, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes get.");
				break;
			case CATCH_POKEMON:
				list_add(cola_catch, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes catch.");
				break;
			case LOCALIZED_POKEMON:
				list_add(cola_localized, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes localized.");
				break;
			case CAUGHT_POKEMON:
				list_add(cola_caught, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes caught.");
				break;
			case APPEARED_POKEMON:
				list_add(cola_appeared, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes appeared.");
				break;
			case NEW_POKEMON:
				list_add(cola_new, mensaje);
				log_info(logger, "Mensaje agregado a cola de mensajes new.");
				break;
			default:
				log_info(logger, "El codigo de operacion es invalido");
				exit(-6);
	}
}

//-----------------------SUSCRIPCIONES------------------------//
void recibir_suscripcion(t_suscripcion* mensaje_suscripcion){

	op_code cola_a_suscribir		   = mensaje_suscripcion -> cola_a_suscribir;

	log_info(logger, "Se recibe una suscripción.");

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
				log_info(logger, "Ingrese un codigo de operacion valido");
				break;
		 }

}


//Ver de agregar threads.
void suscribir_a_cola(t_list* lista_suscriptores, t_suscripcion* suscripcion, op_code cola_a_suscribir){

	//esto es solo para probar, una vez que funciones se saca
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
			log_error(logger, "Se desconoce la cola a suscribir.");
			break;
	}

	log_info(logger, "EL cliente fue suscripto a la cola de mensajes: %s.", cola);
	list_add(lista_suscriptores, suscripcion);
	//Nombre malisimo, hay que revisar.
	//informar_mensajes_previos(suscripcion,cola_a_suscribir);


	bool es_la_misma_suscripcion(void* una_suscripcion){
		t_suscripcion* otra_suscripcion = una_suscripcion;
		return otra_suscripcion -> id_proceso == suscripcion -> id_proceso;
	}


	if(suscripcion -> tiempo_suscripcion != 0){
		sleep(suscripcion -> tiempo_suscripcion);
		list_remove_by_condition(lista_suscriptores, es_la_misma_suscripcion);
		//list_remove_and_destroy_by_condition(lista_suscriptores, es_la_misma_suscripcion, destruir_suscripcion);
		log_info(logger, "La suscripcion fue anulada correctamente.");
	}

}

//REVISAR FUERTE
void destruir_suscripcion(void* suscripcion) {
	free(suscripcion);
}


//REVISAR SI LOS HISTORIALES ESTÁN BIEN RELACIONADOS
//REVISAR EL PARAMETRO QUE RECIBE
void informar_mensajes_previos(t_suscripcion* una_suscripcion, op_code cola_a_suscribir){

	switch(cola_a_suscribir){
		case GET_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(GET_POKEMON, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes get del historial");
			break;
		case CATCH_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(CATCH_POKEMON, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes catch del historial");
			break;
		case LOCALIZED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(LOCALIZED_POKEMON, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes localized del historial");
			break;
		case CAUGHT_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(CAUGHT_POKEMON, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes caught del historial");
			break;
		case NEW_POKEMON: //GAME_CARD SUSCRIPTO
			descargar_historial_mensajes(NEW_POKEMON, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes new del historial");
			break;
		case APPEARED_POKEMON: //TEAM SUSCRIPTO
			descargar_historial_mensajes(APPEARED_POKEMON, una_suscripcion -> socket);
			log_info(logger, "El proceso suscripto recibe los mensajes appeared del historial");
			break;
		default:
			log_info(logger, "No se pudo descargar el historial de mensajes satisfactoriamente.");
			break;
	}
}

void descargar_historial_mensajes(op_code tipo_de_mensaje, uint32_t socket_cliente){
	//Se recurre a la variable memoria_cache.
	//Se puede usar la lista auxiliar de mensajes para hacerlo (con el tipo de mensaje).
	//Habria que filtrar por codigo de operación los mensajes en la cache.
	//Despues enviar todos los mensajes de cada cola a la que fue suscripta.
}


/*Para team y game_card
t_ack* armar_confirmacion_de_recepcion(t_paquete* paquete){
	t_ack* confirmacion_mensaje = malloc(sizeof(t_ack));
	confirmacion_mensaje -> id_proceso = malloc(sizeof(char*));
	confirmacion_mensaje -> id_proceso = "3";//Este valor en realidad viene por config (para gamecard siempre 1)
	confirmacion_mensaje -> id_mensaje = paquete -> id_mensaje;
	confirmacion_mensaje -> tipo_mensaje = paquete -> codigo_operacion;
	//Esta confirmacion despues se envia con la funcion enviar_ensaje con op_code 8 (ACK)
	return confirmacion_mensaje;
}*/


//---------------------------MENSAJES---------------------------//

//REVISAR
void actualizar_mensajes_confirmados(t_ack* mensaje_confirmado){
	op_code cola_de_mensaje_confirmado = mensaje_confirmado -> tipo_mensaje;

	void actualizar_suscripto(void* mensaje){
		t_mensaje* mensaje_ok = mensaje;
		if(mensaje_ok -> id_mensaje == mensaje_confirmado -> id_mensaje){
			eliminar_suscriptor_de_enviados_sin_confirmar(mensaje_ok, mensaje_confirmado -> id_proceso);
			agregar_suscriptor_a_enviados_confirmados(mensaje_ok, mensaje_confirmado -> id_proceso);
		}
	}

	switch(cola_de_mensaje_confirmado){
		case GET_POKEMON:
			list_iterate(cola_get, actualizar_suscripto);
			break;
		case CATCH_POKEMON:
			list_iterate(cola_catch, actualizar_suscripto);
			break;
		case LOCALIZED_POKEMON:
			list_iterate(cola_localized, actualizar_suscripto);
			break;
		case CAUGHT_POKEMON:
			list_iterate(cola_caught, actualizar_suscripto);
			break;
		case APPEARED_POKEMON:
			list_iterate(cola_appeared, actualizar_suscripto);
			break;
		case NEW_POKEMON:
			list_iterate(cola_new, actualizar_suscripto);
			break;
		default:
			log_info(logger, "El mensaje no se encuentra disponible");
			break;
	}
	//Se chequea si un mensaje fue recibido por todos los suscriptores.
	//Si es así, se elimina el mensaje.
	eliminar_mensajes_confirmados();

}
//REVISAR SI ESTÁ BIEN UBICADA CUANDO SE INVOCA
void eliminar_mensajes_confirmados(){

	borrar_mensajes_confirmados(GET_POKEMON, cola_get, lista_suscriptores_get);
	borrar_mensajes_confirmados(CATCH_POKEMON, cola_catch, lista_suscriptores_catch);
	borrar_mensajes_confirmados(LOCALIZED_POKEMON, cola_localized, lista_suscriptores_localized);
	borrar_mensajes_confirmados(CAUGHT_POKEMON, cola_caught, lista_suscriptores_caught);
	borrar_mensajes_confirmados(APPEARED_POKEMON, cola_appeared, lista_suscriptores_appeared);
	borrar_mensajes_confirmados(NEW_POKEMON, cola_new, lista_suscriptores_new);

}
// HAY QUE REVISARLO PERO MINIMO 5 VECES O_o
void borrar_mensajes_confirmados(op_code tipo_lista, t_list* cola_mensajes, t_list* suscriptores){

	t_list* lista_id_suscriptores = list_create();

	void* id_suscriptor(void* un_suscriptor){
		t_suscripcion* suscripto = un_suscriptor;
		uint32_t* id = &(suscripto -> id_proceso);
		return id;
	}

	lista_id_suscriptores = list_map(suscriptores, id_suscriptor);

	bool mensaje_recibido_por_todos(void* mensaje){
		t_mensaje* un_mensaje = mensaje;

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

	switch(tipo_lista){
	case GET_POKEMON:
		list_remove_and_destroy_by_condition(cola_get, mensaje_recibido_por_todos, eliminar_mensaje);
		break;
	case CATCH_POKEMON:
		list_remove_and_destroy_by_condition(cola_catch, mensaje_recibido_por_todos, eliminar_mensaje);
		break;
	case LOCALIZED_POKEMON:
		list_remove_and_destroy_by_condition(cola_localized, mensaje_recibido_por_todos, eliminar_mensaje);
		break;
	case CAUGHT_POKEMON:
		list_remove_and_destroy_by_condition(cola_caught, mensaje_recibido_por_todos, eliminar_mensaje);
		break;
	case APPEARED_POKEMON:
		list_remove_and_destroy_by_condition(cola_appeared, mensaje_recibido_por_todos, eliminar_mensaje);
		break;
	case NEW_POKEMON:
		list_remove_and_destroy_by_condition(cola_new, mensaje_recibido_por_todos, eliminar_mensaje);
		break;
	default:
		log_error(logger, "El mensaje no fue eliminado correctamente");
		break;
	}
	log_info(logger, "Los mensajes confirmados por todos los suscriptores fueron eliminados.");
	list_destroy(lista_id_suscriptores);

}
// REVISAR TAMBIÉN
void eliminar_mensaje(void* mensaje){
	t_mensaje* un_mensaje = mensaje;
	list_destroy(un_mensaje -> suscriptor_enviado);
	list_destroy(un_mensaje -> suscriptor_recibido);
	free(un_mensaje -> payload);
	free(un_mensaje);
	free((t_mensaje*) mensaje);
}


void eliminar_suscriptor_de_enviados_sin_confirmar(t_mensaje* mensaje, uint32_t suscriptor){

	bool es_el_mismo_suscriptor(void* un_suscripto){
		uint32_t* suscripto = un_suscripto;
		return suscriptor == (*suscripto);
	}

	list_remove_by_condition(mensaje -> suscriptor_enviado, es_el_mismo_suscriptor);
}

void agregar_suscriptor_a_enviados_confirmados(t_mensaje* mensaje, uint32_t confirmacion){
	list_add(mensaje -> suscriptor_recibido, &confirmacion);
}
//Se le pasa por parametro la cola y la lista de sus suscriptores segun se necesite.
//Por ejemplo:
//enviar_mensajes(cola_get, lista_suscriptores_get);
void enviar_mensajes(t_list* cola_de_mensajes, t_list* lista_suscriptores){

	void mensajear_suscriptores(void* mensaje){
			t_mensaje* un_mensaje = mensaje;

			void mandar_mensaje(void* suscriptor){
				t_suscripcion* un_suscriptor = suscriptor;

				if(no_tiene_el_mensaje(un_mensaje, un_suscriptor -> id_proceso)){
					uint32_t tamanio_mensaje = size_mensaje(un_mensaje -> payload, un_mensaje -> codigo_operacion);
					enviar_mensaje(un_mensaje -> codigo_operacion, un_mensaje -> payload, un_suscriptor -> socket, tamanio_mensaje);
					agregar_suscriptor_a_enviados_sin_confirmar(un_mensaje, un_suscriptor -> id_proceso);
				}
			}
			list_iterate(lista_suscriptores, mandar_mensaje);
		}
	list_iterate(cola_de_mensajes, mensajear_suscriptores);
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
void eliminar_particion_de_memoria(){
	char* algoritmo_de_reemplazo = config_broker -> algoritmo_reemplazo;
	if(string_equals_ignore_case(algoritmo_de_reemplazo, "FF")){
		//eliminar con fifo
	} else {
		//eliminar con lru
	}
}

void compactar_memoria(){
	//Esto debería ser un hilo que periódicamente haga la compactación?
	//uint32_t frecuencia_de_compactacion = config_broker -> frecuencia_compactacion;

}

//Una vez guardado habría que actualizar que el campó del payload del mensaje sea un puntero a
// la posición de la memoria_cache donde se encuentra el contenido del mensaje y su tamanio.
void guardar_en_memoria(t_mensaje* mensaje){

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")){
	   uint32_t exponente = 0;
	   if(sizeof(mensaje -> payload) > config_broker -> size_min_memoria)
		   exponente = obtenerPotenciaDe2(sizeof(mensaje -> payload));
	   else
		   exponente = config_broker -> size_min_memoria;

	  uint32_t pudoGuardarlo = recorrer(memoria_cache, exponente, mensaje -> payload);
	  if(pudoGuardarlo)
	  {
		  log_info(logger,"no hay memoria suficiente para guardarlo");
		  exit(-7);
	  }
	}

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"PARTICIONES")){


	}


}

uint32_t obtenerPotenciaDe2(uint32_t tamanio_proceso)
{
	uint32_t contador = 0 ;
	while((2^contador) <= tamanio_proceso){
      contador ++;
	}
	return (2^contador) ;
}


struct t_node* crear_nodo(uint32_t tamanio)
{
  // Allocate memory for new node
  struct t_node* node = (struct t_node*)malloc(sizeof(struct t_node));

  // Assign data to this node
  node -> bloque = malloc(sizeof(t_memoria_buddy));
  node -> bloque -> tamanio = tamanio;
  node -> bloque -> payload = NULL;
  node -> bloque -> libre = 1;

  // Initialize left and right children as NULL
  node -> izquierda = NULL;
  node -> derecha = NULL;
  return(node);
}


void arrancar_buddy(){
	struct node *root = crear_nodo(config_broker -> size_memoria);
	list_add(memoria_cache, root);
}



void asignar_nodo(struct t_node* node,void* payload){
    node -> bloque -> payload = payload;
    node ->  bloque -> libre = 0;
    list_add(memoria_cache, node);
}


uint32_t recorrer(struct t_node* nodo, uint32_t exponente, void* payload){
    if(nodo == NULL && exponente < config_broker -> size_min_memoria) {
        return 0;
    }

    if (nodo->bloque->tamanio == exponente && nodo->bloque->libre == 1) {
        asignar_nodo(nodo, payload);
        return 1;
    }

    if (nodo -> izquierda == NULL) {
        nodo -> izquierda = crear_nodo(nodo -> bloque -> tamanio / 2);
    }

    if (nodo -> izquierda -> bloque -> tamanio > exponente) {
        return 0;
    }

    int asignado = recorrer(nodo -> izquierda, exponente, payload);
    if(asignado == 0) {
    	if (nodo -> derecha == NULL) {
    	        nodo -> derecha = crear_nodo(nodo -> bloque -> tamanio / 2);
    	    }
        asignado = recorrer(nodo -> derecha,exponente, payload);
    }

    return asignado;
}

void arrancar_memoria(){
    if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"BS")){
        //ARMO LA ESTRUCTURA INICIAL DEL BS, SIN GUARDAR NADA TODAVIA. DESPUÉS CUANDO TENGA QUE
        //GUARDAR ALGO SE REUTILIZA LA ESTRUCTURA QUE SE DEFINIÓ ACÁ.
    }

    if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"PARTICIONES")){
		t_list* memoria_con_particiones = list_create();
        iniciar_memoria_particiones(memoria_con_particiones);
        //Al principio la lista tiene un unico elemento que es la memoria entera.
	}
}

/*TAMANIO_MENSAJE SE REFIERE AL TAMAÑO DE LA PARTICION O AL TAMAÑO DEL MENSAJE QUE SE GUARDA EN ELLA?

typedef struct {
	uint32_t tamanio_mensaje;
	void* 	 payload;
	uint32_t base;
    uint32_t ocupado;
} t_memoria_dinamica;
*/


void iniciar_memoria_particiones(t_list* memoria_de_particiones){
    /*tamanio_mensaje+payload+base+ocupado+base*/
    uint32_t size_particion = sizeof(uint32_t) * 3 + (config_broker ->size_memoria);
    t_memoria_dinamica* particion_de_memoria = malloc(size_particion);
    particion_de_memoria -> tamanio_mensaje = 0;
    particion_de_memoria -> payload         = memoria_cache;
    particion_de_memoria -> base            = 0;
    list_add(memoria_de_particiones, particion_de_memoria);
}
//DEBERIA ARMAR UNA PARTICION PARA GUARDAR ANTES O CHEQUEARLO A PARTIR DEL MENSAJE?
void guardar_particion(t_mensaje* un_mensaje){
    uint32_t tamanio = size_mensaje(un_mensaje -> payload, un_mensaje -> codigo_operacion);
    uint32_t posicion_a_ubicar = 0;
    if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre,"FF")){
        //ENCONTRAR EL PRIMER AJUSTE DEBERIA DEVOLVERME EL INDICE DEL PRIMER ELEMENTO DE LA LISTA
        //EN EL CUAL HAY UNA PARTICION DISPONIBLE, Y -1 EN CASO DE QUE NO HAYA LUGAR SUFICIENTE.
        posicion_a_ubicar = encontrar_primer_ajuste(tamanio);

        if(posicion_a_ubicar == -1){
            log_error(logger, "No hay suficiente tamaño para ubicar el mensaje en memoria.");
            exit(-20);
        }

        ubicar_particion(posicion_a_ubicar, tamanio, un_mensaje->payload);

    }
    if(string_equals_ignore_case(config_broker -> algoritmo_particion_libre,"BF")){
        posicion_a_ubicar = encontrar_mejor_ajuste(tamanio);

        if(posicion_a_ubicar == -1){
            log_error(logger, "No hay suficiente tamaño para ubicar el mensaje en memoria.");
            exit(-21);
        }

        ubicar_particion(posicion_a_ubicar, tamanio, un_mensaje->payload);
    }

}

void liberar_particion_dinamica(t_memoria_dinamica* particion_vacia){
    free(particion_vacia -> payload);
    free(particion_vacia);
}
// EN ubicar_particion FALTA UBICAR EN LA MEMORIA DEL MALLOC ENORME EL MENSAJE COMO CORRESPONDE.
void ubicar_particion(uint32_t posicion_a_ubicar, uint32_t tamanio, void* payload){
        uint32_t size_particion = sizeof(uint32_t) * 3 + (tamanio);
        t_memoria_dinamica* nueva_particion = malloc(size_particion);
        nueva_particion -> tamanio_mensaje  = tamanio;
        nueva_particion -> base             = posicion_a_ubicar;
        nueva_particion -> payload          = payload ;
        nueva_particion -> ocupado          = 1; // CUANDO = 0 ESTA VACIO Y CUANDO ES !=0 ESTA OCUPADO

        t_memoria_dinamica* particion_reemplazada = list_replace(memoria_con_particiones, posicion_a_ubicar, nueva_particion);

        uint32_t total = particion_reemplazada -> tamanio_mensaje;
        if(tamanio < total)
        {
        uint32_t size_particion_vacia = sizeof(uint32_t) * 3 + (total - tamanio);
        t_memoria_dinamica* nueva_particion_vacia = malloc(size_particion_vacia);
        nueva_particion_vacia -> tamanio_mensaje = total - tamanio;
        nueva_particion_vacia -> base            = posicion_a_ubicar + tamanio + 1;
        nueva_particion_vacia -> payload         = NULL;
        nueva_particion_vacia -> ocupado         = 0;

        list_add_in_index(memoria_con_particiones, posicion_a_ubicar + 1, nueva_particion_vacia);
        }

        liberar_particion_dinamica(particion_reemplazada);

}
//REVISAR MUCHISIMO POSTA

uint32_t encontrar_primer_ajuste(uint32_t tamanio){
    uint32_t indice_seleccionado = 0;
    uint32_t indice_buscador = -1;

    bool tiene_tamanio_suficiente(void* particion){
        t_memoria_dinamica* una_particion = particion;
        return tamanio <= (una_particion -> tamanio_mensaje) && una_particion -> ocupado == 0;
    }

    t_memoria_dinamica* posible_particion = list_find(memoria_con_particiones, tiene_tamanio_suficiente);
    indice_seleccionado = encontrar_indice(posible_particion);

    return indice_seleccionado;
}

typedef struct {
    uint32_t indice;
    uint32_t tamanio;
} t_indice;

uint32_t encontrar_mejor_ajuste(uint32_t tamanio){
    uint32_t indice_seleccionado = 0;


    bool es_de_menor_tamanio(void* una_particion, void* otra_particion){
        t_memoria_dinamica* particion1 = una_particion;
        t_memoria_dinamica* particion2 = otra_particion;
        return (particion1 -> tamanio_mensaje) < (particion2 -> tamanio_mensaje);
    }

    bool tiene_tamanio_suficiente(void* particion){
        t_memoria_dinamica* una_particion = particion;
        return tamanio <= (una_particion -> tamanio_mensaje) && una_particion -> ocupado == 0 ;
    }

    t_list* particiones_en_orden_creciente = list_create();
    particiones_en_orden_creciente = list_sorted(memoria_con_particiones, es_de_menor_tamanio);

    t_memoria_dinamica* posible_particion = list_find(particiones_en_orden_creciente, tiene_tamanio_suficiente);
    indice_seleccionado = encontrar_indice(posible_particion);
    //REVISAR ESTO
    list_destroy_and_destroy_elements(particiones_en_orden_creciente, destruir_particion);

    return indice_seleccionado;
}

void destruir_particion(void* una_particion){
    t_memoria_dinamica* particion = una_particion;
    free(particion -> payload);
    free(particion);
    free((t_memoria_dinamica*) una_particion);
}

uint32_t encontrar_indice(t_memoria_dinamica* posible_particion){
    uint32_t indice_disponible = 0;
    uint32_t indice_buscador = -1;

    void* obtener_indices(void* particion){
        t_memoria_dinamica* particion_a_transformar = particion;
        t_indice* un_indice;
        un_indice -> indice = indice_buscador + 1;
        un_indice -> tamanio = particion_a_transformar -> tamanio_mensaje;
        return un_indice;
    }

    bool es_el_tamanio_necesario(void* indice){
        t_indice* otro_indice = indice;
        return (otro_indice -> tamanio) == (posible_particion -> tamanio_mensaje);
    }

    t_list* indices = list_create();
    indices = list_map(memoria_con_particiones, obtener_indices);
    t_indice* indice_elegido = list_find(indices, es_el_tamanio_necesario);
    //FIJARSE SI ES UN DESTROY A LOS ELEMENTOS TAMBIEN.
    list_destroy(indices);
    indice_disponible = indice_elegido -> indice;
    //free(indice_elegido;)

    return indice_disponible;
}


