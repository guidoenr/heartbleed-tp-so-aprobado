#include "game_card.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



int main(void) {
	config = leer_config();
	iniciar_logger("gameCard.log","gamercard");
	int socket_br;
	conectarse(socket_br);

	//int socket_gb = crear_conexion(config -> ip_gameBoy, config -> puerto_gameBoy);
//	//enviar_mensaje(GC_LOCALIZED_POKEMON_BR, "Localized Pokemon", socket_br);
//	//iniciar_servidor(config -> ip_gameCard,config -> puerto_gameCard);
//	//suscribirme_a_colas();
//	t_new_pokemon* luken = malloc(sizeof(t_new_pokemon));
//	luken->cantidad = 20;
//	luken->id_mensaje= 1512;
//	luken->posicion[0]= 1;
//	luken->posicion[1]= 2;
//	luken->pokemon = "luken";
//	printf("tamaño: %d",tamanioNewPokemon(luken));


//
//	enviar_new_pokemon(luken,socket_br);
//	t_new_pokemon* a = recibir_new_pokemon(socket_br, 30);

//	verificarPokemon(pikachu);
//	verificarAperturaPokemon(pikachu);

	terminar_programa(socket,config);
}
void conectarse(int socket){
	socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	int time = config->tiempo_reintento_conexion;
	if (socket == -1 ){
		log_info(logger,"imposible conectar con broker, reintento en: %d",time);
		sleep(time);
		socket=0;
		conectarse(socket); //terrible negrada, pero anda o no nada?
	} else {
		log_info(logger,"conexion exitosa con broker ");
	}
}

void suscribirme_a_colas() {
	suscribirse_a(NEW_POKEMON);
	suscribirse_a(CATCH_POKEMON);
	suscribirse_a(GET_POKEMON);
}

void suscribirse_a(op_code una_cola) {
	uint32_t socket = crear_conexion(config -> ip_broker, config -> puerto_broker);
	char* msgSub = concatenar("Subscription FROM game_card to: ",(char*) una_cola);
	uint32_t size = sizeof(msgSub) + 1;
	enviar_mensaje(SUBSCRIPTION, msgSub ,socket,size);
	//recibir el mensaje del broker TODO

	//duda serve_client? process_request? crear hilo para atender solicitud? ayuda plis TODO TODO TODO

}

t_config_game_card* leer_config() {

	t_config_game_card* config_game_card = malloc(sizeof(t_config_game_card));

	t_config_game_card* config = config_create("Debug/game_card.config");

	config_game_card -> tiempo_reintento_conexion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_CONEXION");
	config_game_card -> tiempo_reintento_operacion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_OPERACION");
	config_game_card -> punto_montaje_tallgrass = strdup(config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS"));
	config_game_card -> ip_broker = strdup(config_get_string_value(config, "IP_BROKER"));
	config_game_card -> puerto_broker = strdup(config_get_string_value(config, "PUERTO_BROKER"));
	config_game_card -> ip_gameBoy= strdup(config_get_string_value(config, "IP_GAMEBOY"));
	config_game_card -> puerto_gameBoy= strdup(config_get_string_value(config, "PUERTO_GAMECARD"));
	config_game_card -> ip_gameCard= strdup(config_get_string_value(config, "IP_GAMEBOY"));
	config_game_card -> puerto_gameCard= strdup(config_get_string_value(config, "PUERTO_GAMECARD"));
	config_game_card -> tiempo_retardo_operacion = config_get_int_value(config,"TIEMPO_RETARDO_OPERACION");

	config_destroy(config);

	return config_game_card;
}

void liberar_config(t_config_game_card* config) {
	free(config -> punto_montaje_tallgrass);
	free(config -> ip_broker);
	free(config -> puerto_broker);
	free(config);
}

void terminar_programa(int conexion,t_config_game_card* config) {
	liberar_config(config);
	liberar_logger(logger);
	liberar_conexion(conexion);
}


void crearMetadata(char* path){

	FILE* file = fopen(path,"wb"); //write-binary
	if (file==NULL){
		FILE* file = fopen(path,"wb");
		fclose(file);
		log_info(logger,"se creo el archivo metadata.bin VACIO");

	}else{

		t_metadata metadata;
		metadata.blocksize = 64;
		metadata.blocks = 5192;
		metadata.magic = "TALL_GRASS";
		int tam = tamanio_de_metadata(metadata);
		fwrite(&metadata,tam,1,file);

		log_info(logger,"se creo el archivo metadata.bin de tamaño: %d",tam);

		fclose(file);

	}

}

void crearMetadataFile(char* path,t_new_pokemon* newPoke){
	FILE* file = fopen(path,"wb");

	t_file_metadata meta;
	meta.directory = 'Y';
	meta.size = 62; //TODO preguntar que es esto?
	meta.blocks[0] = newPoke->posicion[0];
	meta.blocks[1] = newPoke->posicion[1];// recontra TODO ni idea
	meta.open = 'N';

	fwrite(&meta,sizeof(tamanio_de_file_metadata(meta)),1,file);

	fclose(file);

}
int tamanio_de_metadata(t_metadata metadata){
	int stringLong = strlen(metadata.magic)+1 ;
	return stringLong + (sizeof(int) *2) ;
}

int tamanio_file_metadata(t_file_metadata fileMeta){
	return sizeof(char) + sizeof(fileMeta.blocks) + sizeof(int) + sizeof(char); // TODO BLOCKS?
}

void leerMetadata(char* path){
	FILE* file = fopen(path,"rb"); //read-binary

	if (!isFile(path)){
		log_warning(logger,"no existe el archivo a leer");
	}

	t_metadata metadataLeido; //despues vemos como se usa esta verga

	fread(&(metadataLeido.blocksize),sizeof(int),1,file);
	fread(&(metadataLeido.blocks),sizeof(int),1,file);
	fread(&(metadataLeido.magic),13,1,file);

	log_info(logger, "se leyo el metadata con tamaño %d",tamanio_de_metadata(metadataLeido));

	fclose(file);
}

int fileSize(char* path){
	FILE* f = fopen(path,"r");
	fseek(f, 0L, SEEK_END);
	int i = ftell(f);
	fclose(f);
	return i;
}

bool isFile(char* path){
	FILE* f = fopen(path,"rb");
	fclose(f);
	return f != NULL;
}

void crearBitmap(char* path){
	FILE* file = fopen(path,"wb");
	int status;
	status=0; //arranca en 0
	fwrite(&status,sizeof(int),1,file);
	log_info(logger,"se creo el bitmap.bin con status: %d",status);
	fclose(file);
}

int estadoBitmap(char* path){
	FILE* file = fopen(path,"rb");
	int status;
	fread(&status,sizeof(int),1,file);
	fclose(file);
	return status;
}

int isDirectory(char* path){
	t_file_metadata fileMeta;
	FILE* file = fopen(path,"rb");
	fread(&fileMeta.directory,sizeof(char),1,file);
	fclose(file);

	return fileMeta.directory == 'Y';
}

int isOpen(char* path){
	t_file_metadata fileMeta;
	FILE* file = fopen(path,"rb");
	fread(&fileMeta.open,sizeof(char),1,file);
		fclose(file);
		return fileMeta.open == 'Y';
}

void abrirArchivo(char* path){
	FILE* f = fopen(path,"wb");
	t_file_metadata fileMeta;
	fileMeta.open = 'Y';
	fwrite(&fileMeta.open,sizeof(char),1,f);
	fclose(f);
}

void cerrarArchivo(char* path){
	FILE* f = fopen(path,"wb");
		t_file_metadata fileMeta;
		fileMeta.open = 'N';
		fwrite(&fileMeta.open,sizeof(char),1,f);
		fclose(f);
}

void process_request(uint32_t cod_op, uint32_t cliente_fd){ // Cada case depende del que toque ese modulo.
	uint32_t* size;
	void* msg;

	log_info(logger,"Codigo de operacion %d",cod_op);

	switch (cod_op) {
		case GET_POKEMON:
//			msg = malloc(sizeof(t_get_pokemon));
//			msg = recibir_mensaje(cliente_fd, &size);
//			informarAlBroker(msg,cliente_fd,GET_POKEMON);
//			agregar_mensaje(GET_POKEMON, size, msg, cliente_fd);
//			free(msg);
			break;
		case CATCH_POKEMON:
//			msg = malloc(sizeof(t_catch_pokemon));
//			msg = recibir_mensaje(cliente_fd, &size);
//			informarAlBroker(msg,cliente_fd,CATCH_POKEMON);
//			agregar_mensaje(CATCH_POKEMON, size, msg, cliente_fd);
//			free(msg);
			break;
		case NEW_POKEMON:
			//void* recibir_mensaje(uint32_t socket_cliente, uint32_t* size);
			msg = malloc(sizeof(t_new_pokemon));
			msg = recibir_new_pokemon(cliente_fd,size); // retorna un t_new_pokemon
			informarAlBroker(cliente_fd, NEW_POKEMON);
			funcionHiloNewPokemon(msg);


			//agregar_mensaje(NEW_POKEMON, size, msg, cliente_fd); ??
			free(msg);
			break;

		case 0:
			log_info(logger,"No se encontro el tipo de mensaje");
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
}

void informarAlBroker(int socket,op_code codigo){
	char* aviso = concatenar("ACK[gamecard]: ",(char*) codigo);
	int tamanioAviso = sizeof(aviso) + 1;
	enviar_mensaje(codigo,aviso,socket,tamanioAviso);
}

void funcionHiloNewPokemon(t_new_pokemon* pokemon){

	verificarExistenciaPokemon(pokemon);
	verificarAperturaPokemon(pokemon);

//	if (existePokemonEnEsaPosicion()){
//		agregarAPoisiconExistente();
//	} else {
//		agregarAlFinalDelArchivo();
//	}

	//cerrarArchivo();

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

void verificarExistenciaPokemon(t_new_pokemon* newpoke){
	char* montaje = "montaje/Pokemon/";		  			  // esto va a cambiar con el tallgras, pero es un TODO para la entrega 21 maso
	char* path = concatenar(montaje,newpoke->pokemon); 	  // DE TODAS FORMAS DEJO UNA ALGORITMIA FANTASTATICA

	if (existeDirectorio(path)){

		log_info(logger,"existe el dir: %s",path);

	} else{

		mkdir(path, 0777);
		log_info(logger,"se creo el directorio: %s",path);
		char* metaPath = concatenar(path,"/Metadata.bin");

		crearMetadataFile(metaPath,newpoke);
		log_info(logger,"se creo el file_metadata: %s",metaPath);
	}

}

int existeDirectorio(char* path){
	DIR* dir = opendir(path);
	int x = dir;
	closedir(dir);
	return x;
}

void verificarAperturaPokemon(t_new_pokemon* msg){
	char* path = concatenar ("montaje/Pokemon/",msg->pokemon);
	char* path2 = concatenar (path,"/Metadata.bin"); //terrible negrada esto, pero anda o no anda?

	if (isOpen(path2)){ //TODO VER QUE ONDA
		int secs = config->tiempo_retardo_operacion;
		log_info(logger,"no se puede acceder a este pokemon porque esta en uso (OPEN=Y), reintentando en: %d",secs);
		sleep(secs);
		funcionHiloNewPokemon(msg);

	} else {
		log_info(logger,"se puede acceder /n");
	}
}

void enviar_new_pokemon(t_new_pokemon* pokemon, uint32_t socket_cliente) {
	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer -> size = tamanioNewPokemon(pokemon);
	buffer -> stream = malloc(buffer -> size);
	buffer -> stream = pokemon;
	log_info(logger,"Armando paquete New_Pokemon");

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete -> codigo_operacion = NEW_POKEMON;
	paquete -> buffer = buffer;

	uint32_t size_serializado;
	void* stream = serializar_paquete(paquete, &size_serializado);
	log_info(logger,"Paquete serializado con tamaño :%d",size_serializado);
	send(socket_cliente, stream, size_serializado, 0);
	log_info(logger,"Paquete enviado");
	//free(buffer -> stream);
	free(buffer);
	free(paquete);
	free(stream);
}

t_new_pokemon* recibir_new_pokemon(uint32_t socket_cliente, uint32_t* size){
		log_info(logger, "recibiendo new_pokemon");
		recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
		log_info(logger, "tamaño new_pokemon: %d", *size);
		void* buffer = malloc(*size);
		recv(socket_cliente, buffer, *size, MSG_WAITALL);
		log_info(logger, "mensaje recibido: %s", buffer);

		t_new_pokemon* pokemon = deserealizar_new_pokemon(buffer);
		return pokemon;
	}
//
uint32_t tamanioNewPokemon(t_new_pokemon* pokemon){
	return sizeof(uint32_t) * 4 + strlen(pokemon->pokemon) + 1;
}

