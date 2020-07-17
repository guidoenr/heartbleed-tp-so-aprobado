#include "Utils.h"

void iniciar_logger(char* file, char* program_name) {

	if(file == NULL){
			printf("No se pudo encontrar el path del config.");
			return exit(-1);
	}

	logger = log_create(file, program_name, 1, LOG_LEVEL_INFO);

	if (logger == NULL){
		printf("ERROR EN LA CREACION DEL LOGGER/n");
		exit(-2);
	}
}

uint32_t crear_conexion(char *ip, char* puerto) {
	struct addrinfo huint32_ts;
	struct addrinfo *server_info;

	memset(&huint32_ts, 0, sizeof(huint32_ts));
	huint32_ts.ai_family = AF_UNSPEC;
	huint32_ts.ai_socktype = SOCK_STREAM;
	huint32_ts.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &huint32_ts, &server_info);

	uint32_t socket_cliente = socket(server_info -> ai_family, server_info -> ai_socktype, server_info -> ai_protocol);

	if(connect(socket_cliente, server_info -> ai_addr, server_info -> ai_addrlen) == -1){
		//log_error(logger,"error de conexion por socket");
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(op_code codigo_op, void* mensaje, uint32_t socket_cliente, uint32_t size_mensaje) {

    uint32_t* size_serializado = malloc(sizeof(uint32_t));
    (*size_serializado) = size_mensaje + (sizeof(uint32_t) *2) + sizeof(op_code);


	void* stream = malloc((*size_serializado));
	stream = serializar_paquete(mensaje, size_mensaje, codigo_op, size_serializado);
	uint32_t size_paquete = (*size_serializado);
    log_info(logger,"...Paquete serializado con tamaño :%d", size_paquete);
    //revisar tema hilos y semaforos
	send(socket_cliente, stream, size_paquete, 0);

	log_info(logger,"...Paquete enviado");
	free(size_serializado);
	free(stream);
}

void* recibir_paquete(uint32_t socket_cliente, uint32_t* size, op_code* codigo_operacion){

    //log_info(logger, "Recibiendo mensaje.");

    //sem_wait(&semaforo);

    recv(socket_cliente, codigo_operacion, sizeof(op_code), MSG_WAITALL);
    op_code codigo = (*codigo_operacion);
    //log_info(logger, "Codigo de operacion recibido: %d", codigo);

    recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);

    uint32_t tamanio_mensaje = (*size)-sizeof(uint32_t)-sizeof(op_code);
    //log_info(logger, "Tamanio de paquete recibido: %d", tamanio_mensaje);

	void* stream = malloc(tamanio_mensaje);
	(*size) = tamanio_mensaje;
    recv(socket_cliente, stream, tamanio_mensaje, MSG_WAITALL);
    //log_info(logger, "recibir_paquete stream: %d", *(int*) (stream +4));

    //sem_post(&semaforo);
    //log_info(logger, "Mensaje recibido: %s", stream);
    return stream;

}

void* deserealizar_paquete(void* stream, op_code codigo_operacion, uint32_t tamanio_mensaje){

    void* mensaje_deserializado = malloc(tamanio_mensaje);

    switch(codigo_operacion){
        case GET_POKEMON:
        mensaje_deserializado = deserealizar_get_pokemon(stream, tamanio_mensaje);
        break;
        case CATCH_POKEMON:
        mensaje_deserializado = deserealizar_catch_pokemon(stream, tamanio_mensaje);
        break;
        case LOCALIZED_POKEMON:
        mensaje_deserializado = deserealizar_localized_pokemon(stream, tamanio_mensaje);
        break;
        case CAUGHT_POKEMON:
        mensaje_deserializado = deserealizar_caught_pokemon(stream, tamanio_mensaje);
        break;
        case APPEARED_POKEMON:
        mensaje_deserializado = deserealizar_appeared_pokemon(stream, tamanio_mensaje);
        break;
        case NEW_POKEMON:
        mensaje_deserializado = deserealizar_new_pokemon(stream, tamanio_mensaje);
        break;
        case SUBSCRIPTION:
        mensaje_deserializado = deserealizar_suscripcion(stream, tamanio_mensaje);
        break;
        case ACK:
        mensaje_deserializado = deserealizar_ack(stream, tamanio_mensaje);
        break;
        default:
        log_error(logger, "No fue posible deserializar el mensaje correctamente.");
        break;
    }
    return mensaje_deserializado;
}

uint32_t size_new_pokemon(t_new_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon -> pokemon) + 1;
}

uint32_t size_appeared_pokemon(t_appeared_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon -> pokemon) + 1;
}

uint32_t size_get_pokemon(t_get_pokemon* pokemon){
    return sizeof(uint32_t) + strlen(pokemon -> pokemon) + 1;
}
uint32_t size_localized_pokemon(t_localized_pokemon* pokemon){
    return sizeof(uint32_t) + strlen(pokemon -> pokemon) + 1;
}

uint32_t size_ack(t_ack* confirmacion){
    return sizeof(uint32_t) * 2 + sizeof(op_code);
}

uint32_t size_suscripcion(t_suscripcion* suscripcion){
    return sizeof(uint32_t) * 3 + sizeof(op_code);
}

uint32_t size_catch_pokemon(t_catch_pokemon* pokemon){
    return sizeof(uint32_t) * 3 + strlen(pokemon -> pokemon) + 1;
}

uint32_t size_caught_pokemon(t_caught_pokemon* pokemon){
    return sizeof(uint32_t) * 3;
}

// y localized?

uint32_t size_mensaje(void* mensaje, op_code codigo){
	uint32_t tamanio = 0;
	switch(codigo){
	case GET_POKEMON:
		tamanio = size_get_pokemon(mensaje);
		break;
	case CATCH_POKEMON:
		tamanio = size_catch_pokemon(mensaje);
		break;
	case LOCALIZED_POKEMON:
		tamanio = size_localized_pokemon(mensaje);
		break;
	case CAUGHT_POKEMON:
		tamanio = size_caught_pokemon(mensaje);
		break;
	case APPEARED_POKEMON:
		tamanio = size_appeared_pokemon(mensaje);
		break;
	case NEW_POKEMON:
		tamanio = size_new_pokemon(mensaje);
		break;
	case SUBSCRIPTION:
		tamanio = size_suscripcion(mensaje);
		break;
	case ACK:
		tamanio = size_ack(mensaje);
		break;
	default:
		log_error(logger, "No pudo calcularse el tamaño del mensaje.");
		break;
	}
	return tamanio;
}

void iniciar_servidor(char *IP, char *PUERTO) {
	uint32_t socket_servidor;

    struct addrinfo huint32_ts, *servinfo, *p;

    memset(&huint32_ts, 0, sizeof(huint32_ts));
    huint32_ts.ai_family = AF_UNSPEC;
    huint32_ts.ai_socktype = SOCK_STREAM;
    huint32_ts.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &huint32_ts, &servinfo);

    for (p = servinfo; p != NULL; p = p -> ai_next) {
        if ((socket_servidor = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1){
           uint32_t flag = 1;
        	    setsockopt(socket_servidor, SOL_SOCKET,SO_REUSEPORT,&flag,sizeof(flag));
        	continue;
        }

        if (bind(socket_servidor, p -> ai_addr, p -> ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);
    log_info(logger, "Puerto: %s", PUERTO);
    log_info(logger, "Servidor levantado.");
    while(1)
    	esperar_cliente(socket_servidor);
}

void esperar_cliente(uint32_t socket_servidor) {

	struct sockaddr_in dir_cliente;

	uint32_t tam_direccion = sizeof(struct sockaddr_in);
	uint32_t socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	pthread_create(&thread, NULL, (void*)serve_client, &socket_cliente);
	pthread_detach(thread);

}

void serve_client(uint32_t* socket) {
	uint32_t cod_op = 0;

	//log_info(logger,"Se conecto un cliente con socket: %d",*socket);
	process_request(cod_op, *socket);
	close(*socket);
}

void liberar_conexion(uint32_t socket_cliente) {
	close(socket_cliente);
}

void liberar_logger(){
	if(logger != NULL){
		log_destroy(logger);
	}
}

char* concatenar(char* str1,char* str2){
	char* new_str ;
	if((new_str = malloc(strlen(str1)+strlen(str2)+1)) != NULL){
	    new_str[0] = '\0';
	    strcat(new_str,str1);
	    strcat(new_str,str2);
	} else {
	    log_error(logger,"error al concatenar");
	}

	return new_str;
}

void* serializar_paquete(void* mensaje, uint32_t size_mensaje, op_code codigo, uint32_t* size_serializado){
    uint32_t malloc_size   = (*size_serializado);
    void* paquete_a_enviar;

    switch(codigo){
        case GET_POKEMON:
        paquete_a_enviar = serializar_get_pokemon(mensaje, size_mensaje, size_serializado);
        break;
        case CATCH_POKEMON:
        paquete_a_enviar = serializar_catch_pokemon(mensaje, size_mensaje, size_serializado);
        break;
        case LOCALIZED_POKEMON:
        paquete_a_enviar = serializar_localized_pokemon(mensaje, size_mensaje, size_serializado);
        break;
        case CAUGHT_POKEMON:
        paquete_a_enviar = serializar_caught_pokemon(mensaje, size_mensaje, size_serializado);
        break;
        case APPEARED_POKEMON:
        paquete_a_enviar = serializar_appeared_pokemon(mensaje, size_mensaje, size_serializado);
        break;
        case NEW_POKEMON:
        paquete_a_enviar = serializar_new_pokemon(mensaje, size_mensaje, size_serializado);
        break;
        case SUBSCRIPTION:
        paquete_a_enviar = serializar_suscripcion(mensaje, size_mensaje, size_serializado);
        break;
        case ACK:
        paquete_a_enviar = serializar_ack(mensaje, size_mensaje, size_serializado);
        break;
        default:
        log_error(logger, "No se pudo serializar correctamente.");
        exit(-8);
        break;
    }
    return paquete_a_enviar;
}

void* serializar_get_pokemon(void* mensaje_get, uint32_t size_mensaje, uint32_t* size_serializado){
    t_get_pokemon* mensaje_a_enviar = mensaje_get;
    uint32_t tamanio_pokemon	    = strlen(mensaje_a_enviar -> pokemon) + 1;

    uint32_t malloc_size = (*size_serializado);

    void* stream = malloc(malloc_size);
	uint32_t offset = 0;


	op_code codigo_operacion = GET_POKEMON;
    memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
    //log_info(logger,"Serializacion codigo de operacion: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    //log_info(logger,"Serializacion size: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje), sizeof(uint32_t));
    //log_info(logger,"Serializacion idmensaje: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &tamanio_pokemon, sizeof(uint32_t));
    //log_info(logger,"Serializacion tamaniopokemon: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, mensaje_a_enviar -> pokemon, tamanio_pokemon);
    //log_info(logger,"Serializacion pokemon %s:", (char*) stream+ offset);
	offset += tamanio_pokemon;

	log_info(logger, "...Codigo de operacion a enviar: %d", GET_POKEMON);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);
    liberar_mensaje_get(mensaje_a_enviar);
	return stream;
}

t_get_pokemon* deserealizar_get_pokemon(void* stream, uint32_t size_mensaje){
	t_get_pokemon* mensaje_get_pokemon = malloc(sizeof(t_get_pokemon));
    uint32_t tamanio_pokemon = 0;
    uint32_t offset = 0 ;

    memcpy(&(mensaje_get_pokemon -> id_mensaje), stream + offset, sizeof(uint32_t));
    //log_info(logger,"deserealizado idmensaje %d:", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(&tamanio_pokemon, stream + offset, sizeof(uint32_t));
    //log_info(logger,"deserealizado tamanio pokemon %d:", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    mensaje_get_pokemon -> pokemon = malloc(tamanio_pokemon);

	memcpy(mensaje_get_pokemon -> pokemon,stream + offset, tamanio_pokemon);
	//log_info(logger, "Se deserializo un mensaje get del pokemon: %s", mensaje_get_pokemon -> pokemon);
	offset += tamanio_pokemon;

	return mensaje_get_pokemon;
}


void* serializar_catch_pokemon(void* mensaje_catch, uint32_t size_mensaje, uint32_t* size_serializado){
    t_catch_pokemon* mensaje_a_enviar = mensaje_catch;
    uint32_t tamanio_pokemon          = strlen(mensaje_a_enviar -> pokemon) + 1;

    uint32_t malloc_size = (*size_serializado);

    void* stream = malloc(malloc_size);
	uint32_t offset = 0;

	op_code codigo_operacion = CATCH_POKEMON;
	memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
	//log_info(logger,"Sereliazacion codigo de operacion: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    //log_info(logger,"Sereliazacion size: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion idmensaje: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &tamanio_pokemon, sizeof(uint32_t));
    //log_info(logger,"Sereliazacion tamaniopokemon: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, mensaje_a_enviar -> pokemon, tamanio_pokemon);
    //log_info(logger,"Sereliazacion pokemon: %s", (char*) (stream+ offset));
	offset += tamanio_pokemon;

    memcpy(stream + offset, &(mensaje_a_enviar -> posicion[0]), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion posicion x: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> posicion[1]), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion posicion y: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

	log_info(logger, "...Codigo de operacion a enviar: %d", CATCH_POKEMON);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);

    liberar_mensaje_catch(mensaje_a_enviar);
	return stream;

}

t_catch_pokemon* deserealizar_catch_pokemon(void* stream, uint32_t size_mensaje){
	t_catch_pokemon* mensaje_catch_pokemon = malloc(sizeof(t_catch_pokemon));
	uint32_t tamanio_pokemon = 0;
	uint32_t offset = 0;

    memcpy(&(mensaje_catch_pokemon -> id_mensaje), stream + offset, sizeof(uint32_t));
    //log_info(logger,"deserealizado idmensaje %d:", *(int*) (stream+ offset));
    stream += sizeof(uint32_t);

    memcpy(&tamanio_pokemon, stream + offset, sizeof(uint32_t));
    //log_info(logger,"deserealizado tamanio pokemon %d:", *(int*) (stream+ offset));
    stream += sizeof(uint32_t);

    mensaje_catch_pokemon -> pokemon = malloc(tamanio_pokemon);

	memcpy(mensaje_catch_pokemon -> pokemon, stream + offset, tamanio_pokemon);
	//log_info(logger, "Se deserializo un mensaje catch del pokemon: %s", mensaje_catch_pokemon -> pokemon);
	stream += tamanio_pokemon;

	memcpy(&(mensaje_catch_pokemon -> posicion[0]), stream + offset, sizeof(uint32_t));
	//log_info(logger, "deserealizado posicion x: %d",*(int*) (stream+ offset));
	stream += sizeof(uint32_t);

    memcpy(&(mensaje_catch_pokemon -> posicion[1]), stream + offset, sizeof(uint32_t));
    //log_info(logger, "deserealizado posicion y: %d",*(int*) (stream+ offset));
	stream += sizeof(uint32_t);

	return mensaje_catch_pokemon;

}

void* serializar_appeared_pokemon(void* mensaje_appeared, uint32_t size_mensaje, uint32_t* size_serializado){
    t_appeared_pokemon* mensaje_a_enviar = mensaje_appeared;
    uint32_t tamanio_pokemon             = strlen(mensaje_a_enviar -> pokemon) + 1;

    uint32_t malloc_size = (*size_serializado);

    void* stream = malloc(malloc_size);
	uint32_t offset = 0;

	op_code codigo_operacion = APPEARED_POKEMON;
	memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
	//log_info(logger,"Sereliazacion codigo de operacion: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    //log_info(logger,"Sereliazacion size: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion idmensaje: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje_correlativo), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion idmensaje correlativo: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &tamanio_pokemon, sizeof(uint32_t));
    //log_info(logger,"Sereliazacion tamaniopokemon: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, mensaje_a_enviar -> pokemon, tamanio_pokemon);
    //log_info(logger,"Sereliazacion pokemon: %s", (char*) stream+ offset);
	offset += tamanio_pokemon;

    memcpy(stream + offset, &(mensaje_a_enviar -> posicion[0]), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion posicion x: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> posicion[1]), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion posicion y: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

	log_info(logger, "...Codigo de operacion a enviar: %d", APPEARED_POKEMON);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);

    liberar_mensaje_appeared(mensaje_a_enviar);
	return stream;

}


t_appeared_pokemon* deserealizar_appeared_pokemon(void* stream, uint32_t size_mensaje){
    t_appeared_pokemon* mensaje_appeared_pokemon = malloc(sizeof(t_appeared_pokemon));
    uint32_t tamanio_pokemon = 0;
    uint32_t offset = 0;

	memcpy(&(mensaje_appeared_pokemon -> id_mensaje), stream + offset, sizeof(uint32_t));
	//log_info(logger,"deserealizado idmensaje: %d", *(int*) (stream+ offset));
	stream += sizeof(uint32_t);

	memcpy(&(mensaje_appeared_pokemon -> id_mensaje_correlativo), stream + offset, sizeof(uint32_t));
	//log_info(logger,"deserealizado idmensaje correlativo: %d", *(int*) (stream+ offset));
	stream += sizeof(uint32_t);

    memcpy(&tamanio_pokemon, stream + offset, sizeof(uint32_t));
    //log_info(logger,"deserealizado tamanio pokemon: %d", *(int*) (stream+ offset));
    stream += sizeof(uint32_t);

    mensaje_appeared_pokemon -> pokemon = malloc(tamanio_pokemon);

	memcpy(mensaje_appeared_pokemon -> pokemon, stream + offset, tamanio_pokemon);
	//log_info(logger, "Se deserializo un mensaje appeared del pokemon: %s", mensaje_appeared_pokemon -> pokemon);
	stream += tamanio_pokemon;

	memcpy(&(mensaje_appeared_pokemon -> posicion[0]), stream + offset, sizeof(uint32_t));
	//log_info(logger, "deserealizado posicion x: %d",*(int*) (stream+ offset));
	stream += sizeof(uint32_t);

    memcpy(&(mensaje_appeared_pokemon -> posicion[1]), stream + offset, sizeof(uint32_t));
    //log_info(logger, "deserealizado posicion y: %d",*(int*) (stream+ offset));
	stream += sizeof(uint32_t);

	return mensaje_appeared_pokemon;

}


void* serializar_new_pokemon(void* mensaje_new, uint32_t size_mensaje, uint32_t* size_serializado){
    t_new_pokemon* mensaje_a_enviar      = mensaje_new;
    uint32_t tamanio_pokemon             = strlen(mensaje_a_enviar -> pokemon) + 1;

    uint32_t malloc_size = (*size_serializado);

    void* stream = malloc(malloc_size);
	uint32_t offset = 0;

	op_code codigo_operacion = NEW_POKEMON;
	memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
	//log_info(logger,"Sereliazacion codigo de operacion: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    //log_info(logger,"Sereliazacion size: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion idmensaje: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &tamanio_pokemon, sizeof(uint32_t));
    //log_info(logger,"Sereliazacion tamaniopokemon: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, mensaje_a_enviar -> pokemon, tamanio_pokemon);
    //log_info(logger,"Sereliazacion pokemon: %s", (char*) stream+ offset);
	offset += tamanio_pokemon;

    memcpy(stream + offset, &(mensaje_a_enviar -> posicion[0]), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion posicion x: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> posicion[1]), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion posicion y: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> cantidad), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion cantidadpokemon: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

	log_info(logger, "...Codigo de operacion a enviar: %d", NEW_POKEMON);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);

    liberar_mensaje_new(mensaje_a_enviar);
	return stream;

}


t_new_pokemon* deserealizar_new_pokemon(void* stream, uint32_t size_mensaje){
	t_new_pokemon* mensaje_new_pokemon = malloc(sizeof(t_new_pokemon));
    uint32_t tamanio_pokemon = 0;
    uint32_t offset = 0;

	memcpy(&(mensaje_new_pokemon -> id_mensaje), stream + offset, sizeof(uint32_t));
	//log_info(logger,"deserealizado idmensaje: %d", *(int*) (stream+ offset));
	stream += sizeof(uint32_t);

    memcpy(&tamanio_pokemon, stream + offset, sizeof(uint32_t));
    //log_info(logger,"deserealizado tamanio pokemon: %d", *(int*) (stream+ offset));
    stream += sizeof(uint32_t);

    mensaje_new_pokemon -> pokemon = malloc(tamanio_pokemon);

	memcpy(mensaje_new_pokemon -> pokemon, stream + offset, tamanio_pokemon);
	//log_info(logger, "Se deserializo un mensaje new del pokemon: %s", mensaje_new_pokemon -> pokemon);
	stream += tamanio_pokemon;

	memcpy(&(mensaje_new_pokemon -> posicion[0]), stream + offset, sizeof(uint32_t));
	//log_info(logger, "deserealizado posicion x: %d",*(int*) (stream+ offset));
	stream += sizeof(uint32_t);

    memcpy(&(mensaje_new_pokemon -> posicion[1]), stream + offset, sizeof(uint32_t));
    //log_info(logger, "deserealizado posicion y: %d",*(int*) (stream+ offset));
	stream += sizeof(uint32_t);

	memcpy(&(mensaje_new_pokemon -> cantidad), stream + offset, sizeof(uint32_t));
	//log_info(logger, "deserealizado cantidad: %d",*(int*) (stream+ offset));
	stream += sizeof(uint32_t);

    return mensaje_new_pokemon;

}

void* serializar_caught_pokemon(void* mensaje_caught, uint32_t size_mensaje, uint32_t* size_serializado){
    t_caught_pokemon* mensaje_a_enviar   = mensaje_caught;

    uint32_t malloc_size = (*size_serializado);

    void* stream = malloc(malloc_size);
	uint32_t offset = 0;

  	op_code codigo_operacion = CAUGHT_POKEMON;
	memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
	//log_info(logger,"Sereliazacion codigo de operacion: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    //log_info(logger,"Sereliazacion size: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion idmensaje: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje_correlativo), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion idmensaje correlativo: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> resultado), sizeof(uint32_t));
    //log_info(logger,"Sereliazacion resultado caught: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

	log_info(logger, "...Codigo de operacion a enviar: %d", GET_POKEMON);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);

    liberar_mensaje_caught(mensaje_a_enviar);
	return stream;
}

t_caught_pokemon* deserealizar_caught_pokemon(void* stream, uint32_t size_mensaje){
	t_caught_pokemon* mensaje_caught_pokemon = malloc(sizeof(t_caught_pokemon));
	uint32_t offset = 0;

    memcpy(&(mensaje_caught_pokemon -> id_mensaje), stream + offset, sizeof(uint32_t));
    //log_info(logger,"deserealizado idmensaje: %d", *(int*) (stream+ offset));
	stream += sizeof(uint32_t);

	memcpy(&(mensaje_caught_pokemon -> id_mensaje_correlativo), stream + offset, sizeof(uint32_t));
	//log_info(logger,"deserealizado idmensaje correlativo: %d", *(int*) (stream+ offset));
	stream += sizeof(uint32_t);

	memcpy(&(mensaje_caught_pokemon -> resultado), stream + offset, sizeof(uint32_t));
	//log_info(logger, "Se deserializo un mensaje caught de resultado: %d", mensaje_caught_pokemon -> resultado);
	stream += sizeof(uint32_t);

    return mensaje_caught_pokemon;

}

//REVISAR
void* serializar_localized_pokemon(void* mensaje_localized, uint32_t size_mensaje, uint32_t* size_serializado){
	t_localized_pokemon* mensaje_a_enviar = mensaje_localized;
    uint32_t tamanio_pokemon	          = strlen(mensaje_a_enviar -> pokemon) + 1;

    uint32_t malloc_size = (*size_serializado);
    
    void* stream = malloc(malloc_size+ sizeof(uint32_t));
	uint32_t offset = 0;

    op_code codigo_operacion = LOCALIZED_POKEMON;
    memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
    log_info(logger,"Serialiazacion codigo de operacion: %d", *(int*) (stream + offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    log_info(logger,"Serialiazacion size: %d", *(int*) (stream + offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje), sizeof(uint32_t));
    log_info(logger,"Serialiazacion idmensaje: %d", *(int*) (stream + offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &tamanio_pokemon, sizeof(uint32_t));
    log_info(logger,"Serialiazacion tamanio pokemon: %d", *(int*) (stream + offset));
	offset += sizeof(uint32_t);
    
    memcpy(stream + offset, mensaje_a_enviar -> pokemon, tamanio_pokemon);
    log_info(logger,"Serialiazacion pokemon %s:", (char*) stream + offset);
	offset += tamanio_pokemon;

	memcpy(stream + offset, &(mensaje_a_enviar->tamanio_lista), sizeof(uint32_t));
	    log_info(logger,"Serializacion tamanio de la lista %d:", *(int*) (stream + offset));
		offset += sizeof(uint32_t);

	void serializar_numero(void* numero){
    	uint32_t* un_numero = numero;

    	memcpy(stream + offset, un_numero, sizeof(uint32_t));
    	offset += sizeof(uint32_t);
    }

    list_iterate(mensaje_a_enviar -> posiciones, serializar_numero);

    log_info(logger, "...Codigo de operacion a enviar: %d", LOCALIZED_POKEMON);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);
    liberar_mensaje_localized(mensaje_a_enviar);
	return stream;
}

t_localized_pokemon* deserealizar_localized_pokemon(void* stream, uint32_t size_mensaje){

	t_localized_pokemon* mensaje_localized_pokemon = malloc(sizeof(t_localized_pokemon));
    uint32_t tamanio_pokemon = 0;
    uint32_t cantidad_coordenadas = 0;
    uint32_t offset = 0;

    memcpy(&(mensaje_localized_pokemon -> id_mensaje), stream + offset, sizeof(uint32_t));
    log_info(logger,"Deserealizado idmensaje: %d", *(int*) (stream + offset));
	offset += sizeof(uint32_t);

    memcpy(&tamanio_pokemon, stream + offset, sizeof(uint32_t));
    log_info(logger,"Deserealizado tamanio pokemon: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    mensaje_localized_pokemon -> pokemon = malloc(tamanio_pokemon);

	memcpy(mensaje_localized_pokemon -> pokemon, stream + offset, tamanio_pokemon);
	log_info(logger, "Se deserializo un mensaje localized del pokemon: %s", mensaje_localized_pokemon -> pokemon);
	offset += tamanio_pokemon;

	memcpy(&(mensaje_localized_pokemon ->tamanio_lista), stream + offset, sizeof(uint32_t));
	log_info(logger,"Deserealizado tamanio_lista: %d", *(int*) (stream + offset));
	offset += sizeof(uint32_t);

    memcpy(&cantidad_coordenadas, stream + offset, sizeof(uint32_t));
    uint32_t tamanio_lista_a_recibir = (*(int*) (stream+ offset)) * 2 * sizeof(uint32_t);
    log_info(logger,"Deserealizado tamanio lista: %d",  tamanio_lista_a_recibir);
    offset += sizeof(uint32_t);

    mensaje_localized_pokemon -> posiciones = list_create();
    list_add(mensaje_localized_pokemon -> posiciones, &cantidad_coordenadas);

    uint32_t cantidad_elementos = cantidad_coordenadas * 2;
    uint32_t posicion[cantidad_elementos];
    uint32_t i = 0;

    for(i = 0; i < cantidad_elementos; i ++){
    	memcpy(&(posicion[i]), stream + offset, sizeof(uint32_t));
    	offset+= sizeof(uint32_t);
    	list_add(mensaje_localized_pokemon -> posiciones, &posicion[i]);
    }

	return mensaje_localized_pokemon;
}

void* serializar_ack(void* mensaje_ack, uint32_t size_mensaje, uint32_t* size_serializado){
    t_ack* mensaje_a_enviar = mensaje_ack;

    uint32_t malloc_size = (*size_serializado);

    void* stream = malloc(malloc_size);
	uint32_t offset = 0;

	op_code codigo_operacion = ACK;
	memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
	log_info(logger,"Serializacion codigo de operacion: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    log_info(logger,"Serializacion size: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_mensaje), sizeof(uint32_t));
    log_info(logger,"Serializacion idmensaje: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> tipo_mensaje), sizeof(op_code));
    log_info(logger,"Serializacion tipomensaje confirmado: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_proceso), sizeof(uint32_t));
    log_info(logger,"Serializacion idproceso: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

	log_info(logger, "...Codigo de operacion a enviar: %d", ACK);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);

    liberar_ack(mensaje_a_enviar);
	return stream;
}

t_ack* deserealizar_ack(void* stream, uint32_t size_mensaje){
    t_ack* confirmacion_mensaje = malloc(sizeof(t_ack));
    uint32_t offset = 0;

    memcpy(&(confirmacion_mensaje -> id_mensaje), stream + offset, sizeof(uint32_t));
    log_info(logger, "Se deserializo un mensaje ack para el mensaje de id: %d", confirmacion_mensaje -> id_mensaje);
    stream += sizeof(uint32_t);

	memcpy(&(confirmacion_mensaje -> tipo_mensaje), stream + offset, sizeof(op_code));
	log_info(logger, "deserealizado tipo mensaje: %d", *(int*) (stream + offset));
	stream += sizeof(op_code);

	memcpy(&(confirmacion_mensaje -> id_proceso), stream + offset, sizeof(uint32_t));
	log_info(logger, "deserealizado id proceso: %d", *(int*) (stream + offset));
	stream += sizeof(uint32_t);

    return confirmacion_mensaje;
}

void* serializar_suscripcion(void* mensaje_suscripcion, uint32_t size_mensaje, uint32_t* size_serializado){
    t_suscripcion* mensaje_a_enviar = mensaje_suscripcion;

    uint32_t malloc_size = (*size_serializado) + sizeof(uint32_t)*2;

    void* stream = malloc(malloc_size);
	uint32_t offset = 0;

	op_code codigo_operacion = SUBSCRIPTION;
	memcpy(stream + offset, &codigo_operacion, sizeof(op_code));
	log_info(logger,"Sereliazacion codigo de operacion: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, size_serializado, sizeof(uint32_t));
    log_info(logger,"Sereliazacion size: %d", *(int*) (stream+ offset));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> socket), sizeof(uint32_t));
    log_info(logger,"Sereliazacion socket: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> tiempo_suscripcion), sizeof(uint32_t));
    log_info(logger,"Sereliazacion tiempo suscripcion: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

    memcpy(stream + offset, &(mensaje_a_enviar -> cola_a_suscribir), sizeof(op_code));
    log_info(logger,"Sereliazacion cola: %d", *(int*) (stream+ offset));
	offset += sizeof(op_code);

    memcpy(stream + offset, &(mensaje_a_enviar -> id_proceso), sizeof(uint32_t));
    log_info(logger,"Sereliazacion idproceso: %d", *(int*) (stream+ offset));
	offset += sizeof(uint32_t);

	log_info(logger, "...Codigo de operacion a enviar: %d", SUBSCRIPTION);
	log_info(logger, "...Tamaño a enviar: %d", malloc_size);

    liberar_suscripcion(mensaje_a_enviar);
	return stream;
}

t_suscripcion* deserealizar_suscripcion(void* stream, uint32_t size_mensaje){
    t_suscripcion* mensaje_suscripcion = malloc(sizeof(t_suscripcion));
    uint32_t offset = 0;

    memcpy(&(mensaje_suscripcion -> socket), stream + offset, sizeof(uint32_t));
	log_info(logger, "deserealizado socket: %d", *(int*) (stream + offset));
    stream += sizeof(uint32_t);

    memcpy(&(mensaje_suscripcion -> tiempo_suscripcion), stream + offset, sizeof(uint32_t));
    log_info(logger, "deserealizado tiempo suscripcion: %d", *(int*) (stream + offset));
    stream += sizeof(uint32_t);

	memcpy(&(mensaje_suscripcion -> cola_a_suscribir), stream + offset, sizeof(op_code));
	log_info(logger, "deserealizado cola: %d", *(int*) (stream + offset));
	stream += sizeof(op_code);

	memcpy(&(mensaje_suscripcion -> id_proceso), stream + offset, sizeof(uint32_t));
    log_info(logger, "Se deserializo un mensaje de suscripcion del proceso con id: %d", mensaje_suscripcion -> id_proceso);
	stream += sizeof(uint32_t);

    return mensaje_suscripcion;
}

void liberar_mensaje_get(t_get_pokemon* mensaje_get){
    free(mensaje_get -> pokemon);
    free(mensaje_get);
}

void liberar_mensaje_catch(t_catch_pokemon* mensaje_catch){
    free(mensaje_catch -> pokemon);
    free(mensaje_catch);
}

void liberar_mensaje_appeared(t_appeared_pokemon* mensaje_appeared){
    free(mensaje_appeared -> pokemon);
    free(mensaje_appeared);
}

void liberar_mensaje_new(t_new_pokemon* mensaje_new){
    free(mensaje_new -> pokemon);
    free(mensaje_new);
}

void liberar_mensaje_caught(t_caught_pokemon* mensaje_caught){
    free(mensaje_caught);
}

void liberar_ack(t_ack* mensaje_ack){
    free(mensaje_ack);
}

void liberar_suscripcion(t_suscripcion* mensaje_suscripcion){
    free(mensaje_suscripcion);
}

void liberar_mensaje_localized(t_localized_pokemon* mensaje_localized){
    free(mensaje_localized -> pokemon);
    list_destroy(mensaje_localized -> posiciones);
    free(mensaje_localized);
}
