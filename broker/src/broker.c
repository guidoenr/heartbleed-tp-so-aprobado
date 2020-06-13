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
	t_paquete* msg = malloc(sizeof(t_paquete));
	t_ack* confirmacion_mensaje = malloc(sizeof(t_ack));

	log_info(logger,"Codigo de operacion %d",(op_code) cod_op);
	msg = recibir_mensaje(cliente_fd, &size);

	switch (cod_op) {
		case GET_POKEMON:
			//msg = malloc(sizeof(t_get_pokemon));
			//msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(GET_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CATCH_POKEMON:
			//msg = malloc(sizeof(t_catch_pokemon));
			//msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(CATCH_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case LOCALIZED_POKEMON:
			//msg = malloc(sizeof(t_localized_pokemon));
			//msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(LOCALIZED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case CAUGHT_POKEMON:
			//msg = malloc(sizeof(t_caught_pokemon));
			//msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(CAUGHT_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case APPEARED_POKEMON:
			//msg = malloc(sizeof(t_caught_pokemon));
			//msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(APPEARED_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case NEW_POKEMON:
			//msg = malloc(sizeof(t_new_pokemon));
			//msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(NEW_POKEMON, size, msg, cliente_fd);
			free(msg);
			break;
		case SUBSCRIPTION:
			//msg = malloc(sizeof(t_suscripcion));
			//msg = recibir_mensaje(cliente_fd, &size);
			agregar_mensaje(SUBSCRIPTION, size, msg, cliente_fd);
			free(msg);
			break;
		case ACK:
			//msg = malloc(sizeof(t_ack));
			//msg = recibir_mensaje(cliente_fd, &size);
			confirmacion_mensaje        = 	deserealizar_ack(msg); //REVISAR LO QUE RECIBE EL DESEREALIZAR
			actualizar_mensajes_confirmados(confirmacion_mensaje);
			free(confirmacion_mensaje);
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

	void* buffer;
	log_info(logger, "Recibiendo mensaje.");
	sem_wait(&semaforo);

	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);

	log_info(logger, "Tamano de paquete recibido: %s", size);

	buffer = malloc(*size);

	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	sem_post(&semaforo);

	log_info(logger, "Mensaje recibido: %s", buffer);


	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete = deserealizar_paquete(buffer,size);

    return paquete;
}

void agregar_mensaje(uint32_t cod_op, uint32_t size, t_paquete* paquete, uint32_t socket_cliente){
	log_info(logger, "Agregando mensaje");
	log_info(logger, "Size: %d", size);
	log_info(logger, "Socket_cliente: %d", socket_cliente);
	log_info(logger, "Payload: %s", (char*) paquete);

	t_mensaje* mensaje    = malloc(sizeof(t_mensaje));
	uint32_t nuevo_id     = generar_id_univoco();
	paquete -> id_mensaje = nuevo_id;
	mensaje -> id_mensaje = nuevo_id;

	//Este id habría que ver como pasarlo al id de un t_get por ejemplo.

	mensaje -> payload 		       = paquete -> buffer -> stream;
	mensaje -> codigo_operacion    = cod_op;
	mensaje -> suscriptor_enviado  = list_create();
	mensaje -> suscriptor_recibido = list_create();


	sem_post(&mutex_id);
	send(socket_cliente, &(paquete -> id_mensaje) , sizeof(uint32_t), 0); //Avisamos,che te asiganmos un id al mensaje
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
			case SUBSCRIPTION:
				recibir_suscripcion(mensaje);
				break;
			default:
				log_info(logger, "El codigo de operacion es invalido");
				exit(-6);
	}
}

//-----------------------SUSCRIPCIONES------------------------//
void recibir_suscripcion(t_mensaje* mensaje){

	t_suscripcion* mensaje_suscripcion = malloc(sizeof(t_suscripcion));
	mensaje_suscripcion -> id_proceso  = malloc(sizeof(char*));

	mensaje_suscripcion 			   = deserealizar_suscripcion(mensaje -> payload);
	op_code cola_a_suscribir		   = mensaje_suscripcion -> cola_a_suscribir;
	t_suscriptor* suscripcion    	   = armar_suscripcion_a_guardar(mensaje_suscripcion);

	log_info(logger, "Se recibe una suscripción.");

		switch (cola_a_suscribir) {
			 case GET_POKEMON:
				suscribir_a_cola(lista_suscriptores_get, suscripcion, cola_a_suscribir);
				break;
			 case CATCH_POKEMON:
				suscribir_a_cola(lista_suscriptores_catch, suscripcion, cola_a_suscribir);
				break;
			 case LOCALIZED_POKEMON:
				suscribir_a_cola(lista_suscriptores_localized, suscripcion, cola_a_suscribir);
				break;
			 case CAUGHT_POKEMON:
				suscribir_a_cola(lista_suscriptores_caught, suscripcion, cola_a_suscribir);
				break;
			 case APPEARED_POKEMON:
				suscribir_a_cola(lista_suscriptores_appeared,suscripcion, cola_a_suscribir);
				break;
			 case NEW_POKEMON:
				suscribir_a_cola(lista_suscriptores_new, suscripcion, cola_a_suscribir);
				break;
			 default:
				log_info(logger, "Ingrese un codigo de operacion valido");
				break;
		 }

	 free(suscripcion -> emisor);
	 free(mensaje_suscripcion -> id_proceso);
	 free(mensaje_suscripcion);
	 free(suscripcion);

}

t_suscriptor* armar_suscripcion_a_guardar(t_suscripcion* mensaje_suscripcion){
	t_suscriptor* nueva_suscripcion = malloc(sizeof(t_suscriptor));
	nueva_suscripcion -> emisor 	= malloc(sizeof(char*));
	nueva_suscripcion -> emisor     = mensaje_suscripcion -> id_proceso;
	nueva_suscripcion -> socket     = mensaje_suscripcion -> socket;
	nueva_suscripcion -> temporal   = mensaje_suscripcion -> tiempo_suscripcion;
	return nueva_suscripcion;
}

//Ver de agregar threads.
void suscribir_a_cola(t_list* lista_suscriptores, t_suscriptor* suscripcion, op_code cola_a_suscribir){

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
		t_suscriptor* otra_suscripcion = una_suscripcion;
		return otra_suscripcion -> emisor == suscripcion -> emisor;
	}


	if(suscripcion -> temporal != 0){
		sleep(suscripcion -> temporal);
		list_remove_by_condition(lista_suscriptores, es_la_misma_suscripcion);
		//list_remove_and_destroy_by_condition(lista_suscriptores, es_la_misma_suscripcion, destruir_suscripcion);
		log_info(logger, "La suscripcion fue anulada correctamente.");
	}

}

//REVISAR FUERTE
void destruir_suscripcion(void* suscripcion) {
	t_suscriptor* una_suscripcion = suscripcion;
	free(una_suscripcion -> emisor);
	free((t_suscriptor*)suscripcion);
	free(una_suscripcion);
}


//REVISAR SI LOS HISTORIALES ESTÁN BIEN RELACIONADOS
//REVISAR EL PARAMETRO QUE RECIBE
void informar_mensajes_previos(t_suscriptor* una_suscripcion, op_code cola_a_suscribir){

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

t_ack* deserealizar_ack(void* stream){
	t_ack* acknowledgment = malloc(sizeof(t_ack));
	memcpy(&(acknowledgment -> id_mensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(acknowledgment -> tipo_mensaje), stream, sizeof(op_code));
	stream += sizeof(op_code);
	memcpy(&(acknowledgment -> id_proceso), stream, sizeof(char*));
	stream += sizeof(char*);
	memcpy(&(acknowledgment -> socket), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	return acknowledgment;
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
		t_suscriptor* suscripto = un_suscriptor;
		return suscripto -> emisor;
	}

	lista_id_suscriptores = list_map(suscriptores, id_suscriptor);

	bool mensaje_recibido_por_todos(void* mensaje){
		t_mensaje* un_mensaje = mensaje;

		bool suscriptor_recibio_mensaje(void* suscripto){
			char* un_suscripto = suscripto;

			bool es_el_mismo_suscripto(void* id_suscripto){
				char* alguna_suscripcion = id_suscripto;
				return string_equals_ignore_case(alguna_suscripcion, un_suscripto);
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
	list_destroy_and_destroy_elements(un_mensaje -> suscriptor_enviado, eliminar_suscriptor);
	list_destroy_and_destroy_elements(un_mensaje -> suscriptor_recibido, eliminar_suscriptor);
	free(un_mensaje -> payload);
	free(un_mensaje);
	free((t_mensaje*) mensaje);
}

void eliminar_suscriptor(void* un_suscriptor){
	char* id_suscripto = un_suscriptor;
	free(id_suscripto);
	free((char*)un_suscriptor);
}

void eliminar_suscriptor_de_enviados_sin_confirmar(t_mensaje* mensaje, char* suscriptor){

	bool es_el_mismo_suscriptor(void* un_suscripto){
		char* suscripto = un_suscripto;
		return string_equals_ignore_case(suscripto, suscriptor);
	}

	list_remove_by_condition(mensaje -> suscriptor_enviado, es_el_mismo_suscriptor);
}

void agregar_suscriptor_a_enviados_confirmados(t_mensaje* mensaje, char* confirmacion){
	list_add(mensaje -> suscriptor_recibido, confirmacion);
}
//Se le pasa por parametro la cola y la lista de sus suscriptores segun se necesite.
//Por ejemplo:
//enviar_mensajes(cola_get, lista_suscriptores_get);
void enviar_mensajes(t_list* cola_de_mensajes, t_list* lista_suscriptores){

	void mensajear_suscriptores(void* mensaje){
			t_mensaje* un_mensaje = mensaje;

			void mandar_mensaje(void* suscriptor){
				t_suscriptor* un_suscriptor = suscriptor;

				if(no_tiene_el_mensaje(un_mensaje, un_suscriptor -> emisor)){
					enviar_mensaje(un_mensaje -> codigo_operacion, un_mensaje -> payload, un_suscriptor -> socket, 24);// HAy que sacar el 24 por el size real
					agregar_suscriptor_a_enviados_sin_confirmar(un_mensaje, un_suscriptor -> emisor);
				}
			}
			list_iterate(lista_suscriptores, mandar_mensaje);
		}
	list_iterate(cola_de_mensajes, mensajear_suscriptores);
}

bool no_tiene_el_mensaje(t_mensaje* mensaje, char* un_suscripto){
	bool mensaje_enviado;
	bool mensaje_recibido;

	bool es_el_mismo_suscripto(void* suscripto){
		char* id_suscripcion = suscripto;
		return id_suscripcion == un_suscripto;
	}

	mensaje_enviado  = list_any_satisfy(mensaje -> suscriptor_enviado, es_el_mismo_suscripto);
	mensaje_recibido = list_any_satisfy(mensaje -> suscriptor_recibido, es_el_mismo_suscripto);

	return !mensaje_enviado && !mensaje_recibido;
}

void agregar_suscriptor_a_enviados_sin_confirmar(t_mensaje* mensaje_enviado, char* un_suscriptor){
	list_add(mensaje_enviado -> suscriptor_enviado, un_suscriptor);
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
		   exponente = obtenerPotenciaDe2(config_broker -> size_min_memoria);
	       recorrer(memoria_cache, exponente, mensaje -> payload);
	}

	if(string_equals_ignore_case(config_broker -> algoritmo_memoria,"PARTICIONES")){
		 //i
		 //
		 //int m = sizeof(tamanio_bloque) / sizeof(tamanio_proceso);
		 //int n =1;

	}
	  uint32_t tamanio_bloque  = config_broker -> size_min_memoria;
      uint32_t tamanio_proceso = sizeof(mensaje -> payload) + sizeof(uint32_t);
	  //firstFit(blockSize,processSize);

}

uint32_t obtenerPotenciaDe2(uint32_t tamanio_proceso)
{
	uint32_t contador = 0 ;
	while((2^contador) < tamanio_proceso){
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
    if(nodo == NULL) {
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

/*
void firstFit(int blockSize[], int m,
              int processSize[], int n)
{
    // Stores block id of the
    // block allocated to a process
    int allocation[n];

    // Initially no block is assigned to any process
    memset(allocation, -1, sizeof(allocation));

    // pick each process and find suitable blocks
    // according to its size ad assign to it
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
        	     //13               4
            if (blockSize[j] >= processSize[i])
            {
                // allocate block j to p[i] process
                allocation[i] = j;

                // Reduce available memory in this block.
               ///si tengo algo menor a4 asignar 4, si no el tam_del proceso
                blockSize[j] -= processSize[i];

                break;
            }
        }
    }


}


*/








