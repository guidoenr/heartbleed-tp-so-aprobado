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

//typedef struct {
//	op_code codigo_operacion;
//	t_buffer* buffer;
//	uint32_t id_mensaje;
//	uint32_t id_mensaje_correlativo;
//}t_paquete;


void* serializar_paquete(t_paquete* paquete, uint32_t* bytes) {

	uint32_t malloc_size = paquete -> buffer -> size + sizeof(op_code) + sizeof(uint32_t) * 4;
	(*bytes) = malloc_size; //tamaño del paquete

	void* stream = malloc(*bytes);
	//log_info(logger,"te estas acercando baby");
	uint32_t offset=0 ;
			//RECIBE 		//DE DONDE COPIAS

	memcpy(stream+offset, &bytes, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	//log_info(logger, "done offset");

	memcpy(stream+offset, &(paquete -> codigo_operacion), sizeof(paquete -> codigo_operacion));
	offset += sizeof(paquete -> codigo_operacion);

	//log_info(logger, "done codgio");

	memcpy(stream+offset,&(paquete->id_mensaje),sizeof(uint32_t));
	offset += sizeof(uint32_t);

	//log_info(logger, "done idmensaje");

	memcpy(stream+offset,&(paquete->id_mensaje_correlativo),sizeof(uint32_t));
	offset += sizeof(uint32_t);

	//log_info(logger, "done idmencorrelativo");

	memcpy(stream + offset, &(paquete -> buffer -> size), sizeof(paquete -> buffer -> size));
	offset += sizeof(paquete -> buffer -> size);

	//log_info(logger, "done buffer 1");

	memcpy(stream + offset, paquete -> buffer -> stream, paquete -> buffer -> size);
	offset += paquete -> buffer -> size;
	//log_info(logger, "done buffer2");


	log_info(logger, "bytes: %d", *bytes);
	log_info(logger, "cod op a enviar %d", paquete -> codigo_operacion);
	log_info(logger, "tam a enviar %d", malloc_size);


	return stream;
}


t_paquete* deserealizar_paquete(void* stream_recibido, uint32_t* bytes){

	t_paquete* paquete_deserializado = malloc(sizeof(t_paquete));

	uint32_t offset=0;

	memcpy( bytes ,stream_recibido+offset,sizeof(uint32_t));
	offset+= sizeof(uint32_t);

	memcpy(	&(paquete_deserializado->codigo_operacion),stream_recibido+offset,sizeof(op_code));
	offset+= sizeof(op_code);

	memcpy(&(paquete_deserializado->id_mensaje),stream_recibido+offset,sizeof(uint32_t));
	offset+= sizeof(uint32_t);

	memcpy(&(paquete_deserializado->id_mensaje_correlativo),stream_recibido+offset,sizeof(uint32_t));
	offset+= sizeof(uint32_t);

	memcpy( &(paquete_deserializado->buffer),stream_recibido+offset,sizeof(paquete_deserializado->buffer->size));
	offset+= sizeof(paquete_deserializado->buffer->size);


	log_info(logger,"Se recibio todo esto: %d %d %d",paquete_deserializado->codigo_operacion,paquete_deserializado->id_mensaje,paquete_deserializado->id_mensaje_correlativo);

	return paquete_deserializado;

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
		log_error(logger,"error de conexion por socket");
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(op_code codigo_op, void* mensaje, uint32_t socket_cliente, uint32_t size_mensaje) {
	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer -> size 	 = size_mensaje;
	buffer -> stream = malloc(buffer -> size);
	buffer -> stream = mensaje;
	log_info(logger,"Armando paquete");

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete -> buffer = malloc(size_mensaje);

	paquete -> codigo_operacion = codigo_op;
	paquete -> buffer -> size = buffer -> size;
	paquete -> buffer -> stream = buffer -> stream;
    paquete -> id_mensaje = -1;
    paquete -> id_mensaje_correlativo = -1;


	uint32_t* size_serializado;//log_info(logger, "no me rompo baby");
	void* stream = serializar_paquete(paquete, &size_serializado);
	log_info(logger,"Paquete serializado con tamaño :%d",(size_serializado));


	send(socket_cliente, stream, (&size_serializado) , 0);





	log_info(logger,"Paquete enviado");
	free(buffer -> stream);
	free(buffer);
	free(paquete);
	free(stream);
	free(paquete->buffer->stream);
	free(paquete->buffer);

}

/*void* recibir_mensaje(uint32_t socket_cliente, uint32_t* size) {
	log_info(logger, "Recibiendo mensaje.");
	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
	log_info(logger, "Tamano de paquete recibido: %d", *size);
	void* buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	log_info(logger, "Mensaje recibido: %s", buffer);
	return buffer;
}*/

void iniciar_servidor(char *IP, char *PUERTO) {
	uint32_t socket_servidor;

    struct addrinfo huint32_ts, *servinfo, *p;

    memset(&huint32_ts, 0, sizeof(huint32_ts));
    huint32_ts.ai_family = AF_UNSPEC;
    huint32_ts.ai_socktype = SOCK_STREAM;
    huint32_ts.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &huint32_ts, &servinfo);

    for (p = servinfo; p != NULL; p = p -> ai_next) {
        if ((socket_servidor = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol)) == -1)
            continue;

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
	uint32_t cod_op;

	if(recv(*socket, &cod_op, sizeof(uint32_t), MSG_WAITALL) == -1)
		cod_op = -1;

	log_info(logger,"Se conecto un cliente con socket: %d",*socket);
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

t_get_pokemon* deserealizar_get_pokemon(void* stream){
	t_get_pokemon* mensaje_get_pokemon = malloc(sizeof(t_get_pokemon));
	memcpy(&(mensaje_get_pokemon->id_mensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_get_pokemon->pokemon), stream, sizeof(char*));
	stream += sizeof(char*);
	return mensaje_get_pokemon;
}

t_catch_pokemon* deserealizar_catch_pokemon(void* stream){
	t_catch_pokemon* mensaje_catch_pokemon = malloc(sizeof(t_catch_pokemon));
	memcpy(&(mensaje_catch_pokemon->id_mensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_catch_pokemon->pokemon), stream, sizeof(char*));
	stream += sizeof(char*);
	memcpy(&(mensaje_catch_pokemon->posicion), stream, (sizeof(uint32_t)*2));
	stream += (sizeof(uint32_t) * 2);
	return mensaje_catch_pokemon;
}

t_localized_pokemon* deserealizar_localized_pokemon(void* stream){
	t_localized_pokemon* mensaje_localized_pokemon = malloc(sizeof(t_localized_pokemon));
	memcpy(&(mensaje_localized_pokemon->id_mensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_localized_pokemon->pokemon), stream, sizeof(char*));
	stream += sizeof(char*);
	//Falta el memcpy de la lista de entrenadores
	return mensaje_localized_pokemon;
}

t_caught_pokemon* deserealizar_caught_pokemon(void* stream){
	t_caught_pokemon* mensaje_caught_pokemon = malloc(sizeof(t_caught_pokemon));
	memcpy(&(mensaje_caught_pokemon->id_mensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_caught_pokemon->id_mensaje_correlativo), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_caught_pokemon->resultado), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	return mensaje_caught_pokemon;
}

t_appeared_pokemon* deserealizar_appeared_pokemon(void* stream){
	t_appeared_pokemon* mensaje_appeared_pokemon = malloc(sizeof(t_appeared_pokemon));
	memcpy(&(mensaje_appeared_pokemon->id_mensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_appeared_pokemon->id_mensaje_correlativo), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_appeared_pokemon->pokemon), stream, sizeof(char*));
	stream += sizeof(char*);
	memcpy(&(mensaje_appeared_pokemon->posicion), stream, (sizeof(uint32_t)*2));
	stream += (sizeof(uint32_t) * 2);
	return mensaje_appeared_pokemon;
}

t_new_pokemon* deserealizar_new_pokemon(void* stream){
	t_new_pokemon* mensaje_new_pokemon = malloc(sizeof(t_new_pokemon));
	memcpy(&(mensaje_new_pokemon->id_mensaje), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(mensaje_new_pokemon->pokemon), stream, sizeof(char*));
	stream += sizeof(char*);
	memcpy(&(mensaje_new_pokemon->posicion), stream, (sizeof(uint32_t)*2));
	stream += (sizeof(uint32_t) * 2);
	memcpy(&(mensaje_new_pokemon->cantidad), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	return mensaje_new_pokemon;
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


