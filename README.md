# tp-2020-1c-heartbleed
## GameCard
• La conexion con el broker funciona correctamente, haciendo un `sleep` de *n* segundos leidos desde el archivo config, prueba       conectarse infinitamente ,sin un numero de intentos, hasta que se levante el broker.

• El FileSystem tallgrass levanta sin problemas y sin importar si ya esta creado inicialmente , creando todos los directorios y archivos principales del mismo, tales como el `Metadata.bin` , `Bitmap.bin`(*1). Tambien crea los **blocks** con su respectivo tamaño *64bytes* leido desde el archivo de configuracion. En esta instancia, solamente crea 10, para no realizar muchas syscalls al sistema puesto que el archivo de configuracion propone 5192 blocks, que en total, son **10384** syscalls contando las operaciones `fopen & fclose` por cada block creado. 

• Se suscribe al broker ,enviando un menasje del tipo *SUBSCRIPTION* a las 3 colas, y el mismo espera la respuesta para poder operar los mensajes que le son otorgados.

• La lectura y escritura de archivos funciona correctamente, el manejo de `archivos.bin` se utiliza de igual manera que `archivos.dat` por lo cual es facil operarlos. Al crear un pokemon, hace las validaciones que pide el enunciado, preguntando si existe el directorio incialmente para no re-crearlo. (*2)

• Al recibir un mensaje del estilo *`NEW_POKEMON`* levanta un hilo `funcionHiloNewPokemon(..)` que lo que hace es realizar todas las validaciones, pateando al hilo con un `sleep(TIEMPO_RETARDO_OPERACION)` en caso de que el archivo `t_metadata_file.open` este abierto.
Tambien verifica las posiciones del pokemon en el mapa (*3).
Una vez validado todo , arma un mensaje `appeared_pokemon` y se lo manda al broker. 

• Las funciones para envio de los 3 tipos de mensajes que maneja este modulo estan hechas.

• Los hilos para los demas tipos de mensajes `CATCH` & `GET` realizan *casi* el mismo algoritmo que el de `funcionHiloNewPokemon(...)` no estan desarrollados.


##### TODO:
**(1)**: El bitmap.bin lleva dentro un bitarray, que estoy implementado y viendo como se utiliza para manejar los blocks de mi filesystem.

**(2)**: El enunciado esta confuso en la parte que dice que cada pokemon tiene su archivo indicando sus posiciones, entiendo que el funcionamiento del filesystem es asignar blocks libres para guardar todo ese contenido, pero no aclara que tipo de archivo es. Podria ser un binario, un .txt o lo que fuera, porque habla de lineas.

**(3)**: Las posiciones en el mapa hasta ahora son inchequeables, las funciones que verifican eso retornan un `true` para no trabarse, puesto que aun estoy implementando el bitarray para poder asignarle blocks libres a un determinado pokemon. 
