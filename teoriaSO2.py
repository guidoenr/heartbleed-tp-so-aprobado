-------------------------------------- CLASE 01 - "MEMORIA" -------------------------------------- 
Un programa tiene que cargarse en memoria desde disco, por lo general.. y se tiene que colocar 
con las estructuras necesarias de un proceso para poder ejecutarlo.
La CPU directamente a la hora de ejecutar va a acceder a memoria principal o bien a los registros 
que tiene disponible. Cuando hacemos un acceso a un registro es una operacion muy rapida, es el 
equivalente a un ciclo de CPU o menos.
Si acceder a memoria fue como agarrar algo que tengo una biblioteca, acceder a disco es como irme
de viaje y volver a agarrar algo del disco.
Mejorar la ejecucion de un programa se hace generalmente via hardware, es decir, con componente.
Por ejemplo la Memoria Cache, que se coloca entre la memoria RAM y la CPU para tener un acceso mas 
rapido a la informacion.
EL DISCO ES LENTO. hay que usarlo lo menos posible. 

accesos a registros > accesos a memoria principal

;dir memoria    ;instruccion 
0x231af01       pushl %ebp
98123aba21      molv %esp,%esp
h12baxc241      "" 
 ..]

el instruction POINTER apunta a una direccion de memoria y en base a eso hace la instruccion
que esta al lado(codigo). Si no hay una instruccion, rompe.

Ahora "cuando se generan o se setean estas direcciones de memoria en mi sistema?"
                                    ↓↓↓↓↓
"Vinculacion de Direcciones":(3 momentos en el que se cargan a memoria)
• Compilacion: En estas maquinas no se corren varias cosas al mismo tiempo.. si se conoce a priori
la posicion que va a ocupar un proceso en la memoria, se puede generar codigo absoluto con
referencias absolutas a memoria, si cambia la posicion del proceso hay que recompilar el codigo.
aca la posicion de memoria no cambia.
En breve: cuando yo compile , ya genere las direcciones de memoria.
Esto solamente sirve para maquinas donde solo se corre un proceso, porque sino existe el problema
que si corren 2 procesos al mismo tiempo, esas direcciones se pisan.
Entonces para solucionarlo, podrias re-compilar y ahi te ahorras el problema.

• Carga: si no se conoce la posicion del proceso en memoria en tiempo de compilacion se debe generar
codigo reubicable
Cuando compilo tengo direcciones, pero cuando lo cargo actualizo esas direcciones , entonces de
esa forma yo podria cargar diferentes procesos sin la necesidad de recompilar mi codigo.
Si yo quisiera SWAPEAR un proceso, cuando lo traigo de SWAP, tendria que ubicarlo en la misma 
posicion de antes, entonces es un escenario de mierda, no es algo que habitualmente suceda.
Es muy complejo recalcular todas las direcciones devuelta, puede generar muchos errores.

• Ejecucion: si el proceso puede cambiar de posicion durante su ejecucion, la vinculacion se 
retrasa hasta el momento de ejecucion. Necesita soporte hardware para el mapeo de direcciones.
Si genero las direcciones en tiempo de ejecucion, tendriamos que buscar donde esta esa posicion 
y ejecutarlo. Tengo la ventaja de que puedo mover al proceso por toda la memoria, no habria muchos
problemas.
Tambien nos da un PLUS de seguridad, si yo se que tal programa se corre siempre en tal lado, 
su heap esta en tal lado, y su stack en tal lado ,algun programa malicioso podria explotar un BUG 
en ese lado. En ejecucion las direcciones son "dinamicas", porque el proceso se va moviendo durante
toda la memoria, por lo tanto ese bug no existiria porque no sabria las direcciones puesto que son 
random digamos.

"segmentation faulT": ESTA DIRECCION NO TE PERTENECE.

"Espacio de Direcciones":
• Direccion logica: direccion que genera el proceso, tambien se conoce como dir virtual
• Direcion fisica: direccion que percibe la unidad de memoria

Ambas son iguales en los esquemas de vinculacion en tiempo de compilacion y de carga, pero 
difieren en el esquema de vinculacion en tiempo de ejecucion. 

"Base y Limite":
Un par de registros base y limite definen el espacio de direcciones logicas. Puede aplicar
a todo el proceso, o tenerlo por porciones.
Base: donde arranca mi proceso o porcion de proceso
Limite: donde termina mi proceso o porcion de proceso
;(1)

"MMU": Memory Management Unit: es la que transforma las direcciones virtuales en fisicas para
poder buscarlas, digamos que es la traduccion de direcciones.
Gracias a esto siempre vamos a conocer las direcciones logicas y no las fisicas.
El seg fault 0341232x1 hace referencia a una direccion logica
;(2)

"Asignacion de espacio contiguo (continuo es en realidad)"
• La memoria principal se encuentra dividida en dos partes:
    - (kernel) : normalmente en posiciones bajas de la memoria junto al vector de interrupciones
    mientras mas chicos es el kernel mejor, ocupa menos memoria. 
    - zona para procesos de usuario: normalmente en posiciones altas de memoria.

• La zona para procesos de usuarios se encuentra dividda a su vez en particiones que se asignaran
a los procesos
    - Particionamiento estatico: las particiones se establecen en el momento de arranque del SO 
    y permanecen fijas durante todo el tiempo.
    Prendo la maquina, arranca el SO  y esas particiones permanecen fija con su tamaño, esto no 
    implica que sean iguales los tamaños.
    - Particionamiento dinamico: las particiones cambian de acuerdo a los requisitos de 
    los procesos.
    Voy asignado y desasignando, mi memoria es un gran "hueco" en la que yo voy asignando y desasignando
    partes.

PARTICIONAMIENTO ESTATICO: 
ejemplo:

"Asignacion estatica"
;(3)
• Los registros de reubicacion se usan para proteger los procesos de usuario unos de otros y del codigo 
y datos del SO.
    - El registro base contiene la direccion fisica mas baja a la que puede acceder el proceso.
    - El registro limite contiene el tamaño de la zona de memoria accesible por el procesos.

•Compartir memoria no es sencillo, directamente no se puede, porque una particion es asignada
para cada proceso (uno solo)

"Asignacion Dinamica":
;(4)
Ahora el tamaño y ubicacion de las particiones no es fijo, sino que cambia a lo largo del tiempo.
Cuando llega un proceso se le asigna memoria de un hueco lo suficientemente grande para que quepa.
• Con el espacio sobrante del hueco se crea una nueva particion libre
• La comparticion se puede conseguir mediante solapamiento de particiones.

"ALGORITMOS DE ASIGNACION DINAMICA":
•First-fit = se asigna el primer hueco lo suficientemente grande, no hay overhead.
•Best-fit = se asigna el hueco mas pequeño que es lo sufuicientemente grande, hay que buscar en la
lista de huecos, pero ese pedacito pequeño que deja no le podria servir a nadie al ser muy pequeño.
•Worst-fit = se asigna el hueco mas grande, habria que buscar en la lista completa los huecos
salvo si esta ordenada por tamaño. La ventaja de esto, es que si busco el hueco mas grande es mas probable
que lo que no use ese proceso, lo pueda usar otro proceso.

"COMPACTACION": cuando se empiezan a tener huecos que dejan los procesos que finalizan , esos 
huecos que tengo es fragmentacion externa: memoria disponible que le sirve a los proceso.
la compactacion junta to esos cachitos en un hueco grande, y tambien lo que hace es juntar 
todos los procesos. (queda todo el espacio libre en un lado y toda la parte ocupada en otro lado)

"FRAGMENTACION EXTERNA VS INTERNA"
Interna: dentro de una particion donde un unico proceso deberia acceder, me sobra y lo puede usar unicamente
ese proceso.
Externa: quedan muchos huecos chiquitos digamos que no estan contiguos, entonces la memoria 
no "tendria" espacio para un nuevo proceso, pero en realidad si lo tiene (si juntas todos)
pero al no poder juntarlos contiguamentes , se desperdicia espacio digamos. Esto es fragmentacion
externa.

"SEGMENTACION": es el representante de asignacion dinamica. No usa una unica particion, sino que 
parte al proceso en diferentes partes o segmentos. 
Un segmento es una unidad logica tal como un programa principal, procedimiento, funcion, metodo, 
objeto, variables locales, globales ,bloque comun, pila, tabla de simbolos, arrays, etc.
Ahora ya no asigno una unica porcion, sino que asigno varias porciones.
Lo que tiene esto de bueno: imaginemos un micro y 3 autos, que es mas facil estacionar? 
y los 3 autos, porque no tienen que estar continuos estacionados.
Algo parecido pasa aca con los procesos, porque lo puedo partir y no hace falta que esten contiguos.
La segmentacion usa la tabla de segmentos, y lo que hace, es tener la direccion donde arranca,
el limite y vamos a tener N veces. (numero de segmento que va a estar dado por el subindicie de esa tabla)
lo normal es que sean tablas chicas, hay varios base-limite, porque ahora el proceso esta partido
no como antes que estaban contiguos.

Esquema=;
direccionLogica= <numero de segmento, desplazamiento>
tablaDeSegmentos= contiene informacion sobre la ubicacion de los segmentos en memoria, cada entrada
tiene una base que indica la direccion fisica en el que comeinza el segmento y un limite, que especifica
la longitud del segmento
    en cada entrada de la tabla de segmentos hay
    •privilegios de lectura/escritura/ejecucion
    •set de instrucciones permitidas
    •nivel de privilegio

STBR= Registro base de la tabla de segmentos: apunta a la localizacion en memoria de la tabla de segmentos
STLR= Registro de longitud de la tabla de segmentos: indica el umero de segmentos usados por un programa

"obs": el numero de segmento S es legal si S < STLR

;(5)
;(6)
;(7)

"PAGINACION": el gran representante de lo que es particiones fijas, al reves de segmentacion.
lo que se hace aca es tomar anuestro proceso y dividrlo en porciones del mismo tamaño, y a esas 
porciones a las cuales dividimos ese proceso la llamamos paginas.
La memoria la dividimos en porciones del mismo tamaño , se le llaman frames o marcos.
Para ejecutar un programa de tamaño N paginas, hace falta encontrar N frames libres y cargar 
el programa, no importa si esos marcos estan contiguos o no, no me importa una verga.
Se usa uan tabla de paginas para transformar las direciones logicas en fisicas.
En este esquema hay fragmentacion interna, pero solamente puede haber en la ultima pagina.. y es 
una fragmentacion interna muy chica.. es muy parecido a los blocks del filesystem.
Mi tabla de paginas es una POR proceso. Y mi pcb en principio tiene un puntero al registro base 
de mi tabla de paginas, para decirme a partir de aca va tu tabla de paginas.
Este esquema impide la fragmentacion EXTERNA, solamente existe interna.
;(8)

"TLB": Trasnlation look.aside buffer: es una pequeña memoria cache, lo que guarda son traducciones.
guardar una pagina me ahorra un acceso , y guardar una traducion me ahorra 2 numeritos. entonces 
me conviene guardar una traduccion. ej= "<esta pagina, esta en este marco>"
Entonces antes de ir a memoria a buscar un marco, chequeo esta tabla.. 
La tabla se carga cuando voy a buscar algo a la TLB, no lo encuentro, entonces hago la  forma tradicional
y me traigo ese marco y pagina para meterlo en el TLB.
Se guarda esa posicion hasta que cambia de contexto.
;(9)
Lo primero es bucar la pagina en el TLB, si encuentro la pagina y el frame.. me ahorro el acceso
a la tabla de paginas en memoria, tengo un "TLB hit". (tiene un ratio de 95% a 99%)
Si no lo encuentro, tengo un "TLB miss" , voy a la tabla de paginas y hago lo tradicional y actualizo 
la TLB.
;(10)

"BIT DE VALIDEZ": si es 1 me dice que la pagina de ese proceso, corresponde, si es 0, es invalido.. esa pagina
no existe. 
;(11)

"TABLA DE PAGINAS MULTINIVEL- PAGINACION JERARQUICA": Las tablas de paginas son de tamaño fijo y muy grande. Como hacemos
para no desperdiciar memoria? este esquema va como piña. Suponete que corres 200 procesos, en los esquemas
tradicionales tendrias 200 tablas de paginas, una para cada proceso, que es una locura de memoria en tabla
de paginas.
Divido el espacio de direcciones logicas en multiples tablas de paginas, Entonces tengo una tabla de 
paginas de tabla de paginas.
Queda una paginacion multinivel, lo bueno de esto es que en las tablas de niveles intermedios 
yo voy a crear las tablas de paginas solo si las necesito.
ahorra MUCHISIMO ESPACIO.
la desventaja es que tengo mas accesos a memoria.
si mi pagina multinivel es de N niveles voy a tener N+1 accesos a memoria, entonces , cada nivel que 
yo agrego tengo mas accesos a memoria.. pero si mi TLB funciona bien, voy a reducirlos.
;(12)
;(13)

"TABLA DE PAGINAS INVERTIDA": nosotros antes teniamos nuestra tabla de paginas donde deciamos 
partimos al proceso en paginas, partimos a la memoria en marcos(todo eso lo seguimos teniendo) y 
deciamos le creamos a cada proceso su tabla de paginas. En este esquema, lo que tenemos es una 
sola tabla de pagina que este indexada por marcos.. entonces aca se accede invertidamente, el 
indice es el marco.
;(14)
aca la busqueda es ma compleja porque tengo que recorrer toda la tabla, soluciono este problema 
acercandome al marco que necesito. Se usa una funcion de HASH, es una funcion matematica.
f(t) = salida(t)
donde f(t) es el identificador de proceso y la pagina.
salida(t) va a ser un valor acotado entre 0 y (cantidad de marcos -1)
esta funcion tiene dispersion, significa que me devuelve valores que estan por toda la memoria
con repeticiones que se den lo menos posible.
Si se repetin las posiciones, digo que tengo una "colisión", una opcion para solucionarlo podria ser
uso el registro siguiente, ej si tu frame es 3000 le sumo +1.

"SEGMENTACION PAGINADA": Lo mas comun en PCS.
lo que hace es combinar segmentacion con paginacion..
Lo bueno de segmentacion era que el proecso lo podias tener dividido en segmentos, en el cual en el uno
estan las funciones, en el otro el stack, otro el heap, etc.. en paginacion esto era imposible, podria ser
que una pagina tenga pedacitos de cada cosa juntos.
Paginacion esta bueno porque tiene particiones chiquitas, soluciono problemas de acceso a memoria con la TLB
y demas, pero este no puede hacer que el heap, el stack, el codigo esten en paginas diferentes.
Este esquema toma lo mejor de los dos:
Lo que hacemos aca es segmentar el proceso , y cada segmento lo paginamos(dividimos en paginas)..
y cada paginas de las que tengo las escribimos en frames de memoria.
Nuestra direccion ahora tiene ;<segmento,pagina,resto>
con nuestro segmento vamos a la tabla de segmentos, que en este caso no me da una base y un limite
,me da un puntero a la tabla de paginas.. y una vez con la pagina, si le mando el numero de pagina
que esta en mi direccion voy a obtener el frame de memoria.
NO HAY FRAG EXTERNA

;(15)

-------------------------------------- CLASE 02: MEMORIA VIRTUAL -------------------------------------- 
"INTRODUCCION": nosotros vamos a tener nuestros procesos en memoria y en SWAP, esto es la idea de 
memoria virtual: una combinacion entre la memoria principal y el disco.
Ejemplo cuando jugas al fifa no tenes los 50.000 estadios guardados en memoria, sino que los tenes en SWAP
SWAPPEAR me permite aumentar el grado de multiprogramacion, el mapa que estas jugando esta en memoria.
esto totalmente transparente para el proceso, no sabe si esta en memoria virtual o en disco.
es una especie de "engaño" al CPU.
pensa que podes tener muchos mas procesos y memoria, y podes tener procesos mas GRANDES que mi memoria,
permitiendo sacar procesos de memoria al disco y sacar procesos del disco y meterlos en memoria.
TERRIBLE.
Mi proceso va a tener parte en memoria principal y en swap, pero el proceso no lo sabe.. es totalmente
transparente.
En resumen, un ejemplo terrible : suponete el CSGO, cuando se ejecuta, solamente te va a traer las paginas
que va a necesitar, no te va a traer la pagina que tiene un de_mirage a memoria cuando estas jugando de_overpass,
el swapping es LAZY = traigo lo que necesito, que esta terrible, porque estas mintiendo al sistema haciendole creer que 
tenes mas memoria.
;(1)

"PRINCIPIO DE LOCALIDAD": Localidad o vecindad : es un set de paginas que se usan en conjunto y la idea
es que va a ser un fenomeno que lo que nos va a decir es que en base a lo ultimo que fue haciendo mi proceso
nosotros vamos a poder predecir las instrucciones o datos que vamos a usar a futuro.
;WHO CONTROLS THE PAST CONTROLS THE FUTURE
si yo estoy ejecutando una funcion y esa funcion itera un vector , entonces yo voy a tener la pagina
que tiene ese codigo mas la pagina de stack que tiene ese vector, mas la pagina de heap que tiene uan 
parte que estoy usando ahi.. bueno todas esasa paginas se usan en conjunto
si vamos mirando todo esto, podemos ir prediciendo cuales me conviene tener en memoria principal.

• Localidad temporal: las paginas no necesariamente son continuas pero se llaman en conjunto.
Esta es siempre mejor pero es mas dificl de estimar, porque no puedo saber mirando una pagina de codigo
saber cuales va a necesitar.
Esta localidad SIEMPRE es mejor, pero es mas dificl de estimar.

• Localidad espacial: si yo estoy referenciando una determinada posicion de memoria es muy probable que
proximanmente accedamos a secciones cercanas. si estoy en la pagina 1 donde arranca el if, probablemente despues
use la pagina 2 que es donde termina el IF.
esta es mas facil de estimbar, porque decis "che bueno, la pagina que viene es la que uso".
;(2)

"FALLO DE PAGINA":  (del inglés page fault) es una excepción arrojada cuando un programa informático requiere una dirección que no se
encuentra en la memoria principal actualmente y tiene que ir a buscarla a disco.

"paginas": PORCIONES DEL PROCESO.
"PAGINACION POR DEMANDA": La idea de esto es que las paginas se cargan en memoria fisica solo cuando se
necesitan(de esa forma me ahorro memoria), el swapping es 'Lazy'. Se mueven solo las paginas que se necesitan.
se agregan un par de bits mas, la idea de paginacion por demanda es que yo voy a cargar las paginas
en memoria solo cuando las necesito ( me ahorro memoria ). 
se agregan los "BITS DE MODIFICADO" y "BIT DE PRESENCIA".

"PRESENCIA": un bit en 0, significa que NO esta en memmoria principal, pero si en SWAP
"FRAME": dice que la pagina en algun momento estuvo en memoria principal y uso el frame2
"MODIFICADO": respecto a la copia que yo tengo en swap, esto esta modificado, porque en memoria principal dice otra

ejemplo para entender: F:7, P:1, M:1. tengo la pagina en memoria en el frame 7 y esta diferente a lo que tengo en swap
ejemplo para entender: F:2, P:0, M:0. no tenog la pagina en memoria, pero alguna vez estuvo y en el frame 2.
ejemplo para entender: F:2, P:1, M:0. tengo la pagina en memoria en el frame 2 y esta igual a lo que hay en disco.

obs: cuando hiberno mi pc se va todo a swap
- cuando uso un disco SSD , puedo suspender y levantar al toque porque basicamente como el disco es rapido
el swap tambien.
;(3)como se lee la tabla:
-la pagina 1 uso en algun momento el frame 2 , pero no lo esta usando mas 
-la pagina 2 esta usando el frame 7 y esta modificado

"POLITICAS DE ASIGNACION":
ASIGNACION_FIJA= le doy a un proceso una asignacion fija de marcos, por ej digo a todos mis procesos les doy 10 frames
ASIGNACION_DINAMICA= le doy a un proceso una asignacion de frames dinamica, que puede ir variando con el tiempo
"POLITICAS DE SUSTITUCION":
SUSTITUCION_LOCAL= mi proceso va a correr el algoritmo contra sus propios marcos, entonces a la hora de remplazar 
elije una victima dentro de sus marcos
SUSTICTUCION_GLOBAL= lo mismo pero puede elegir un marcos dentro de todos los procesos. puede liberar marcos propios
o de otros procesos.

combinaciones posibles:

FIJA-LOCAL.
DINAMICA-GLOBAL: el problema que tiene es que un proceso que no esta usando sus marcos se los pueden sacar
DINAMICA-LOCAL: podria ser posible cuando el proceso se vaya achicando.

"THRASHING / HiPER-PAGINACION": En sistemas operativos, se denomina hiperpaginación 
a la situación en la que se utiliza una creciente cantidad de recursos para hacer una cantidad de 
trabajo cada vez menor. A menudo, se refiere a cuando se cargan y descargan sucesiva y constantemente
partes de la imagen de un proceso desde y hacia la memoria principal y la memoria virtual o espacio
de intercambio. En un estado normal, esto permite que un proceso bloqueado y no listo para correr
deje lugar en memoria principal a otro proceso listo. Cuando se produce hiperpaginación, los ciclos
del procesador se utilizan en llevar y traer páginas (o segmentos, según sea el caso) y el 
rendimiento general del sistema se degrada notablemente. 
Al no tener suficiente memoria para que los procesos tengan su localidad, paso mucho mas tiempo atendiendo
fallos de pagina que ejecutando.
Si hablo 2000 pestañas de chrome, cuando em quedo sin memoria, y quiero ver otra cosa la maquina deja de
responderme. esto es un caso tipico de thrasing.

TRASHING=puede suceder con "SUSTITUCION LOCAL- ASIGNACION FIJA":
ej: instruccion que usa op1 y op2. Trato de ejecutar esa instruccion, no la tengo en memoria principal, 
entonces la traigo.. cuando la ejecuto no tengo el operando1, entonces lo voy a buscar y lo traigo,
despues ejecuto y no tengo el operando2, lo busco ..y entonces podria pasar que la instruccion se valla
de memoria porque le robaron frames y entra en una especie de ciclo, con lo cual la tendria que ir a buscar
nuevamente.
ocurre porque no tengo suficiente memoria para que los procesos tengan su localidad, entonces paso mas tiempo
atendiendo fallos de pagina que ejecutando.

"THRASING-SOLUCION": Lo que hago es que si mi proceso tiene muy pocos frames, voy a tener muchos fallos
de pagina.. entonces lo que tengo que hacer es asignarle frames.. pero si le asigno demasiados frames,
mis fallos de pagina no van a bajar, osea nunca voy a llegar a que mis fallos de paginas sean 0.
Si tiene muchos marcos, les saco entonces.. se busca un punto de equilibrio entre cantidad de marcos.

"PAGINAS GRANDES - MENOS FALLOS vs PAGINAS-CHICAS - MEJOR APROX": Ejemplo con que el PROCESO entra en una sola pagina,
vos en un fallo de pagina podrias traer todo tu proceso.. tendrias cosas que tal vez no necesitas en memoria..
y estarias desaprovechadno mucha memoria, guardando cosas al pedo digamos.
entonces si tenes paginas chicas tenes mas fallos de pagina, pero aprovechas un toque mas la memoria, porque tenes en
memoria solamente lo que esta usando tu proceso.

"ARCHIVOS MAPEADOS":El mapeo de memoria de un archivo usa el sistema de memoria virtual del sistema
operativo para acceder a los datos en el sistema de archivos directamente, en lugar de utilizar 
funciones normales de E/S. El mapeo de memoria típicamente mejora el rendimiento de E/S porque no 
implica una llamada al sistema por separado para cada acceso y no requiere copiar datos entre búfers
– la memoria se accede directamente tanto por el núcleo como por la aplicación de usuario.
Para leer archivos con fopen, para reducir las syscall es mejor hacer un archivo mapeado a memoria.
en vez de hacer fwrite, fwopen, haces un gran malloc=() y tenes todo el archivo ahi.

https://rico-schmidt.name/pymotw-3/mmap/

"BUDDY SYSTEM": mecanismo de asignacion continua que tiene ambas fragmentaciones, pero ninguna de las dos
es un problema.
;(5)
parte de una particion que es multiplo de 2, lo que busca es quizas no ser tan optimo respecto al tema de
uso de espacio.. no le molesta desperdiciar o demas, pero es muy rapido para asignar y para compactar.
entonces se parte de una potencia de 2 , cuando nosotros recibimos un pedido , tomamos esa particion y 
la partimos en 2, tomo 1, la parto en 2, y asi.. cuando llego al punto que no puedo partir mas porque 
ya no entraria, asigno eso. Entonces asigno ese pedacito de memoria, si viene otro proceso y me pide 64,
busco una de las particiones divididas y le asigno una libre.
cuando dos porciones contiguas, "BUDDIES", el 64 y 64 son buddys, cuando estan las dos libres lo que hago 
es consolidar, es decir unirlas, y ahora tengo una de 128.
generalmente es mejor tener una lista de particiones libres, donde pongo todas las que no estan en uso.

"PAGE FAULT": se producen cuando cargamos los frames o bien cuando remplazamos

"ALGORITMO OPTIMO": mira adelante y se fija cual no se va a volver a usar, o la que mas lejos en el tiempo
se va a usar.


-------------------------------------- CLASE 03: FILESYSTEM -------------------------------------- 
FileSystem: modulo del sistema operativo que se ocupa manejar todo lo que es el almacenamiento secundario 
y terciaro. Administra espacio libre, espacios a los archivos, etc etc etc... resumido es el representante
de todo los archivos y todo lo que sea considerado almacenamiento en nuestro S.O.
La administracion del filesystem la hace el S.O, el S.O lo que hace es buscar una forma generica de acceder
a cada filesystem que soporta.
Los diferentes sistemas operativos tienen que poder reconocer los distintos filesystem para poder trabajar
con ellos.
Ejemplo de pendrive formateado en cierto S.O y que en otro no lo lee.
EJ: NTCS- FAT32, son distintos tipos de filesystem.


"PARTICIONAMIENTO": division logica, que puede ser dentro de un disco o juntar varios disco y tratarlos como
particion

"VOLUMEN": particion con formato en particular.
;(0)
"ARCHIVOS": coleccion de informacion relacionada para el que crea ese archivo.
    .Nombre
    .ID 
    .Tipo (.exe,.py,.txt,..
    .Ubicacion
    .Tamaño 
    .Proteccion
    .Metadata

↓↓↓↓↓
"FCB": File Control Block(se guarda dentro de la metadata del mismo filesystem), estructura que nos permite
guardar toda esta informacion + diferentes datos que estan relacionados con el archivo en si, esto se
guarda dentro del disco, aunque podria estar en memoria cuando lo necesite.
Se guarda dentro del encabezado de nuestro filesystem.
Mientras el archivo este en el disco, tiene que haber un FCB por cada archivo que exista.
al borrar un archivo marcas como libres los blocks de ese archivo .Mientras no hayas pisado esos blocks, 
podes recuperar el archivo.. siempre y cuando no hayas pisado esos blocks.
Por eso hay aplicaciones que recuperan archivos facilmente.
Cuando formateas un pendrive tenes dos modos, un modo es el modo rapido que lo que hace basicamente es 
limpiar la tabla y CHAU, si usas FAT chau fat/ bitvector.
Si usas el formateo fisico y demas, lo que esta haciendo es reescribirte los blocks y las tablas, que lo
formatea totalmente.
Un FCB podría guardar información acerca de los permisos de los usuarios, en formato
Propietario-Grupo-Universo, una lista de control de accesos o bien el propietario del
archivo, permitiendo que ese usuario sea el único que puede accederlo.
El FCB se guarda dentro de la metadata del filesystem (por ejemplo, en la tabla de
inodos en el caso de UFS), salvo casos particulares como FAT, que la distribuyen en las
entradas de directorio o bien en la metadata de cada archivo.
"FCB" → Linux le dice I-NODO

"METODOS DE ACCESO A ARCHIVOS"
-Secuencial : un registro despues de otro, tengo que pasar por todos los registros para leer el que quiero.
-Directo : acceder a cualquier parte sin tener que recorrer lo anterior.
-Indexado : accedo por indice y me acelera la busqueda
-Hashed : se usa la funcion de hash para acceder directamente al bloque, es como memoria virtual. es el mas veloz
corro una funcion y me da como respuesta "busca en este bloque"

"BLOQUEOS/LOCKS": Me permite regular el acceso a un archivo , no quiero que dos programas accedan simultaneamente
, queremos mantener la integridad del archivo. 
Al usar locks vas un paso mas alla que los semaforos, ademas de que te sirvan para que dos procesos escriban
simultaneamente en el archivo, tambien te sirve para poder leer informacion que este actualizada.

LOCK COMPARTIDO para las lecturas.
LOCK EXCLUSIVO: para las escrituras.

entonces cuando haces una syscall, decis "dame el lock de lectura".
;OBS: importante para el TP, tenes que trabar los archivos segun si lo queres leer o escribir.
;no es necsario un doble block, podes lockearlos solamente si lo vas a escribir.

LOCK OBLIGATORIO: el S.O se ocupa de garantizarme la integridad sobre el archivo, quiere decir que si yo quiero
acceder a un archivo que alguien tiene con un lock exclusivo, no me va a dejar.
LOCK SUGERIDO: ademas de los locks, le agrego dentro de mi set de syscalls opciones para quien esta programando
pregunte, entonces el lock sugerido me dice "hay otro que esta usando este archivo en modo exclusivo, que hacemos?"
entonces la integridad la garantiza quien programa, es el quien decide si leer el archivo cuando alguien mas lo 
esta usando, corriendo los riesgos de ver algo desactualizado.


"OPEN FILE TABLE":es una tabla global, que dice que archivos estan abiertos en mi sistema..tambien dice cuantos procesos
tienen abierto ese archivo..
Tambien tiene un contador de aperturas, que es un valor que me va a decir cuantos procesos estan referenciando ese archivo 
cuando hacemos un fclose() bajariamos ese contador, entonces cuando este contador llega a 0 significa que nadie mas necesita 
ese archivo y entonces limpiamos esta tabla.

"FILE DESCRIPTOR TABLE": Tabla local que es por c/ proceso , el PCB de un archivo tiene el puntero a una file descriptor Table
que dice los archivos abiertos tambien ,pero es propia.
Cuando nosotros abrimos un archivo, primero lo vamos a buscar en la tabla de archivos global, si esta en la global, perfecto
tenemos un puntero a esa entrada y ya.. si no esta en la global, agregamos ese archivo a la global y a nuestra tabla 
particular.
Esta tabla tiene cosas particulares de ese archivo pero solamente para ese proceso, la tabla global, tiene cosas generales
como por ejemplo un lock= y/n , porque es general para todos.
por eso esta bueno hacer el fclose(file) siempre a un archivo despues de usarlo, asi actualizas esta tabla.

obs=
"para linux todo es un archivo": linux trata muchas cosas como si fuera un archivo.. para linux siempre
lo que estan haciendo es leer o escribir un archivo. 

"FILES DESCRIPTORS": linux tiene reservado siempre el 0, 1, 2 que son para el teclado, pantalla y standarderror.
en el tp te da el 5, porque primero levantas la config y despues el logger.

"DIRECTORIOS": tipo de archivo, en mi FCB me dice 0 si es archivo regular o 1 si es directorio. 
Cuando borro y creo archivos, lo que estoy haciendo es editando el archivo directorio. 
Un directorio deberia permitirme:
-crear y borrar archivos
-modificarlos
-buscar archivos
-lsitar archivos en ese directorio
-renombrar archivos

Generalmente los grafos de directorios son aciclicos, pueden referenciar a un archivo desde lugares distintos

"IMPLEMENTACION DE DIRECTORIOS": cuando tenemos directorios lo que buscamos es que el acceso sea RAPIDO.
Un directorio minimanente tiene que tener el id del archivo, el nbombre y un punto al FCB.
cuando hacemos un ls -parametro en linux , pero si alguien me hace un ls para ver algo especifico del archivo
tendria que ir a el FCB  de ese archivo, traerlo y cargalo en mi dir.

"Lista lineal": tabla donde cada archivo es una entrada y tengo la info que necesite y quizas le meto
algunas cosas de metadata en la entrada , que tambien estan en la FCB, pero estan replicadas por un tema de velocidad.
imaginate que esa tabla tiene unas 7000 entradas, si yo borro una entrada me quedan huecos en el medio,
esos huecos todos los archivos lo pueden ir llenando y demas, pero por mas que yo lo intente mantener ordenado
cualquier creacion o eliminacion de archivo me lo desordena. Si quiero chequear que el archivo no este duplicado,
tendria que buscar toda la tabla denuevo y fijarme.

"Lista enlazada": es mucho mas facil de ordenar, podria tner varias formas de ordenarlas. Por ejemplo , mi lista
tiene diferentes punteros, uno por si lo queres ordenar por fecha, otro por tamaño, otro por nombre ,etc.
Es mas dificil de construir y todo eso pero es mucho mas sencillo, entonces cargo esta lista en memoria cuando
abro ese directorio.

"PROTECCION DE ARCHIVOS":
• Tipos de acceso:
        -Proteccion completa(solo el dueño puede acceder a sus archivos)
        -Acceso libre (ninguna proteccion)
        -Acceso controlado (LECTURA|ESCRITURA|EJECUCION|ADICION|BORRADO|LISTADO)

"ACL- ACCES CONTROL LIST": Por cada archivo armamos una pequeña matriz de usuarios y permismos y en base a eso
decimos si o no, entonces el usuario X puede hacer esto, esto y esto.. etc
armar esto es pesado, si yo creo un nuevo archivo, tomo todos los usuarios que tengo dentro de mi registro de usuarios
y a cada usuario le asigno distintos permisos.
Otro problema es que cuando creo un nuevo usuario ese usuario tengo que agregarlo en todas las listas de todos 
los archivos.
Windows usa esto, pero a nivel directorios y a nivel grupos de usuario.

"PROPIETARIO - GRUPO - UNIVERSO": usado por linux , propietario es quien creo el archivo, grupo es basicamente
el grupo al que pertenece el propietario, universo es "el resto".
;(0.5)
obs= si no tengo por ejemplo permisos en un directorio de escritura, no solamente no puede escribir archivos
sino que tambien no puedo ni crear ni borrar archivos, puesto que no puedo editar esa entrada de directorios.

"ESQUEMAS PARA ASIGNACION DE ESPACIO LIBRE"
"Asignacion Contigua": se asignan bloques contiguos en el disco, lo cual se mueve menos el cabezal de lectura
y demas.
El bloque logico se forma juntando varios blocks fisicos (del disco) ,  que lo hace el filesystem.
Si asigno siempre contiguo y elimino archivos, quedan huecos .. si quiero asignar 7 bloques, no tengo 7 bloques
de corrido, con lo cual tendria que compactar y al estar todo en disco es mas lento.
Que pasaria si mi archivo azul quisiera crecer 4 bloques mas al final, en principio no tengo 4 bloques mas 
para darle al final.. aunque realmente tengo ese espacio, pero no estan juntos.
hacer acceso directo es facil, secuencial tambien (sabiendo donde termina y donde empieza los blocks de mi archivo)
EJ: la grabacion de los CDS se hace de forma contigua, por eso se graba una vez y no lo volves a grabar mas.
Aca hay fragmentacion EXTERNA tanto como interna, hay bloques libres que no uso ningun bloque y no los puede 
usar otro bloque.
;(1)
"Asignacion enlazada": los bloques pueden estar en cualquier lado y cada bloque tiene un puntero al siguiente.
El problema aca es la velocidad de acceso, porque los bloques estan separados.
Otro problema es que si se te rompe un bloque de un archivo, perdes el resto.
A la hora de funcionamiento no es la mejor.
FAT usa esto con asteroides, cuando fragmenta junta los blocks para leerlo mas rapido.
;(2)
"Asignacion Indexada": tenemos un indice por cada archivo. Mi entrada a directorio no apunta a un bloque, sino
a un indice.. entonces todos estos punteros me los llevo al bloque indice. Si quiero hacer lectura secuencial,
sigue siendo medio lenteja porque los bloques estan dispesos peeeero para acceso directo es mucho mas rapida.
Los indices se almacenan en disco, conceptualmente estos indices son bloques de disco , lo cual ocupa espacio
en disco.
EXTENDEND usa esto.
indice = i-nodo ;
;(3)

EN TODAS HAY FRAGMENTACION INTERNA, pensa que el ultimo block puede llenarse como que no.
Al ser los blocks de tamaño fijo pasa esto.

"GESTION DE ESPACIO LIBRE": 
-"LISTA ENLAZADA": Se usa en asignacion contigua , son bloques libres encadenados. Hay que leer el 1er
bloque antes de asignarlo para obtener el puntero al siguiente bloque.

-"BITMAP": tengo un bit que me dice si esta en 1 me dice que esta ocupado, en 0 libre, el bitmap debe estar
en la RAM para ser eficiente, pero se tiene que guardar en disco.
Se puede usar tanto par asignacion indexada como enlazada.

"Incosistencia": en un FCB la metadata dice tal fecha, pero si voy al directorio dice otra.
el bitvector me dice el block esta en uso, pero no esta en uso.
Estos dos ejemplos representan la inconsistencia.
Windows hace algo parecido cuando hace un scandisk al inicio.

"Journaling": lleva un historial de transacciones, se bloquean las estructuras afectadas para que nadie mas
pueda usarlas durante la transaccion. Se escriben en el journal los pasos necesarios para deshacer la transaccion.

"ENTRADA DE DIRECTORIO": el directorio es un archivo cuyo contenido son entradas, esaas entradas se pueden 
pensar como filas en una tablita.. y esa tabla tiene el nombre de archivo y algo que me permita acceder a ese
archivo.
Es como un archivo.txt pero es del tipo directorio, y tal como el archivo.txt tiene cosas adentro, el archivo 
directorio tambien.. y lo que tiene adentro son entradas de directorios, que la entrada de directorio me dicen 
cuantos archivos tengo en ese directorio en especifico.

"FILES SYSTEMS DE VERDAD:"
"FAT"(File Alocation Table): es una variante de la lista enlazada,.. el numero que acompaña a fat indica el Tamaño 
de puntero de direccionamiento que va a usar.
Lo que hace fat es , todos los punteros que tendriamos en los blocks de archivos se los lleva a una tabla,
entonces cuando mira los blocks estan libres, entonces lo bueno es que esa tabla la puedo cargar a memoria principal
y cuando la leo esta ahi.
Todo lo que es acceso secuencial y directo es bastante mas rapido, todo el reocorrido entre punteros lo hago con
entradas en memoria, entonces con eso soluciono el tema de velocidad.
no usa FCB, la info administrativa de los archivos se guarda en las entradas de directorio.
FAT no usa bitvector, si la entrada dice 000h quiere decir que esta libre , si dice ffff8 ahi termina el archivo.
ffff7 -> cluster dañado
ffff6 -> cluster reservados para la raiz.
Los directorios son estaticos por lo que la cantidad de archivos estaba limitada. Esto cambia en FAT32, 
el tamaño maximo de los archivos esta limitado por la entrada de directorio (4GB maximo).
Para obtener un bloque libre se recorre la tabla.
La tabla esta en memoria y en disco, si miro la tabla me muestra espacios en blancos, que son los 0..
lo unico que tengo que hacer es recorrer la tabla hasta encontrar un 0 y eso significa que un bloque esta vacio.
FAT llama a los blocks como clusters.
;(4)

"EXT2": Extended 2.
Filesystem que usa asignacion indexada. Usamos indices pero se los llaman i-nodos , que tienen un comportamiento
particular.
Este maneja el concepto de grupos de bloques y cada grupo de bloques funciona como una especie de mini filesystem,
como cilindros. Si yo consigo que mis archivos estan en el mismo cilindro, leerlos es mas rapido.
Si pensas en el disco como un conjunto de circulos concentricos, a la proyeccion de esos circulos concentricos
se le llama cilindro. Entonces al manejar grupos de cilindros, bloques que estan en el mismo cilindro, entonces
al asignar un bloque a un archivo voy a tratar de asignarle el bloque que este en el mismo cilindro.
Maneja un superbloque que describe el sistema de archivos. (aca esta la info de cada fs)
Usa bitmap para los bloques y otro bitmap para los i-nodos, que el i-nodo va a ser el FCB. Siempre estan ahi,
nosotros lo que hacemos no es crearlos, sino asignarles cosas.
EXT2 maneja FCBS que estan creados adentro de cada grupo, entonces tenemos una cantidad limitada, los i-nodos
no se crean.. sino que se asignan.
EXT2 va asignando los punteros en orden, no asignaria un puntero indirecto triple que justo entra el archivo
va asignado los que necesita en orden, primero los directos, despues los dobles, etc.

;(5)

"I-nodos": son los FCB que usa extended, que tiene los indices y toda la informacion de metadata administrativa.
El clasico i-nodo tiene 12 punteros directos, 1 indireccion simple, 1 doble y 1 triple.
Las indirecciones apuntan a un bloque de punteros, depende de a cuantos bloques distintos de punteros apuntemos
define el nivel de indireccion.
Los directos apuntan a bloques de datos.
;(6)

"Enlaces": Accesos directos.
Acceso a un mismo archivo de dos lugares, si un lugar lo modifica, el otro tambien lo ve modificado.
;(7)

"HARD LINK": tengo dos directorios diferentes, en uno creo un archivo y me toca por ejemplo el inodo 12345,a ese 
inodo se le asigna un determinado bloque y ese bloque tiene un contenido. Si yo qusiera ese archivo compartirlo con
otro directorio tengo que usar un enlace.
Lo que hace el enlace, es en otro directorio distitno agregar otra entrada que tiene el numero de inodo del archivo 
que ya existe, y es el mismo archivo, no es una copia ni nada. Si uno de esos dos lugares lo modifica, el otro lugar
lo ve modificado.

"SOFT LINK": Acceso directo clasico que conocemos. Aca se crea otro archivo ,pero no una entrada de directorio.
ni un archivo.
se crea otra especie de archivo , que es diferente al tipo archivo o directorio que ya conociamos 
que se llamaba "symbolic link" .
Entonces esto representa que el contenido de este archivo no lo interpretamos como texto, lo interpretamos como una respuesta
hacia donde quiero acceder.
Cuando accedo, en vez de abrir esa ruta, se trata de acceder a esa ruta.

"HARD VS SOFT LINK": A symbolic or soft link is an actual link to the original file, whereas a
hard link is a mirror copy of the original file. If you delete the original file, the soft link has
no value, because it points to a non-existent file. But in the case of hard link, it is entirely
opposite. Even if you delete the original file, the hard link will still has the data of the 
original file. Because hard link lo apunta de diferentes lados.

Soft LINK
    can cross the file system,
    allows you to link between directories,
    has different inode number and file permissions than original file,
    permissions will not be updated,
    has only the path of the original file, not the contents.

A hard Link
    can’t cross the file system boundaries (i.e. A hardlink can only work on the same filesystem),
    can’t link directories,
    has the same inode number and permissions of original file,
    permissions will be updated if we change the permissions of source file,
    has the actual contents of original file, so that you still can view the contents, even if the original file moved or removed.


Podriamos tener links a cosas que no existen pero que podrian existir mas adelante, podriamos tener
links a directorios tambien..
Con hard link haces un acceso directamente ,pero con soft link haces mas accesos.

"MAGIC NUMBER": distinguir un filesystem
el Metadata.bin de mi TP es el superbloque

"FAT ;N": N me dice cuantas entradas tiene la tabla.
en FAT el TAM maximo de mi FS = TAM maximo de archivo
ej: bloques de 1kb
punteros de 4bytes  → 32bits
↓↓↓↓↓
TAMfs= 2^32 * 2^10 = 2^42 = 4TB


FAT 12 tiene maximo 4096 entradas, podria tener menos. Cada entrada pesa 12
FAT 16 tiene maximo 2^16 entradas. Cada entrada pesa 16
FAT 32 tiene 28 bits de esos 32, los otros 4 me los meto en el orto. Cada entrada pesa 32 bits

"AGRANDAR CLUSTERS VS AGRANDAR PUNTERO"
Agrandar clusters : tengo mas fragmentacion interna, porque podria sobrarme mucho espacio en un cluster
Agrandar puntero : 

"Punteros en filesystem": apuntan a la minima unidad de asignacion, es decir, apuntan a un block.

-------------------------------------- CLASE 04- EJERCICIOS -------------------------------------- 









-------------------------------------- CLASE 05 -------------------------------------- 
PARCIAL

"La principal ventaja de utilizar paginacion jerarquica es": ahorrar memoria, al dividir en subniveles
solamente tengo en memoria principal esos niveles que estoy usando..

"La paginacion y la segmentacion pueden requerir el uso de la compactacion para evitar
la fragmentacion, Sin embargo la segmentacion paginada no requiere compactacion": La paginacion
no requiere compactacion, ya por eso es falsa. La segmentacion si puede requerir compactacion.
La segmentacion paginada no requiere compactacion, esto usa marcos a nivel fisico, entonces no 
necesito compactar.

"Ventajas de un inodo con punteros directos e indirectos": Con los directos puede referenciar 
archivos pequeños y con los indirectos puedo referenciar archivos mas grandes.
Podria haber fragmentacion si tengo un indrecto triple por un unico bloque, porque desperdicio
punteros y blocks.
Si hablamos de "cuanto" PODRIAS decir tamaño de bloque -1, porque usa 1 solo byte digamos.

"Ventajas desventajas que plantea la estrategia de proteccion de archivos propietario-grupo-universo
vs Matriz de accesos": Lo que tiene de bueno es que p-g-u ocupa menos espacio y a su vez , 
es mas facil mantenerlo. Si creo un nuevo usuario , ese usuario va a ser parte del universo 
para el restro de los archivos.
Lo bueno de la matriz de acceso es que es super granular, entonces yo puedo decir para este archivo
en particular hace tal cosa.

"EJ CONTENIDO DE UN ARCHIVO DEL TIPO DIRECTORIO QUE TIENE DOS HARDLINSK Y UN SOFT LINK,. TODOS
APUNTANDO A UN ARCHIVO FILE.TXT QUE SE ENCUENTRA EN DICHO DIRECTORIO"

directorio
ENTRADAS_DE_DIRECTORIO=

file.txt        inodo 2            metadata
hardlink.txt    inodo 2            metadata
softlink.txt    inodo 3            metadata'

el softlin kno tiene que ser necesariamente el mismo inodo que los hard link
y el nombre de los hardlink no debe ser el mimsmo necesarimente tambien


-------------------------------------- CLASE 06- ENTRADA SALIDA -------------------------------------- 
"Entrada salida": comuinicacion del sistema operativo y el exterior, el exterior es cualquier cosa que se 
comunique.
Un teclado por ejemplo, un parlante, un joystick.. lo que sea.
El SO sabe como tratar todo esto, y para que esto funciona, el SO tiene que soportar todos los mecanismos 
para poder identificar esto.

"Modulo de ES": es el modulo que se resume en pedazos de codigo.
Sus funciones son:
- Interfaz con los dispositivos
- Interfaz con el usuario: mis estimulos con los dispositivos tienen que poder ser traducidos.
- Inicializacion 
- seguridad
- Planificar E/S 
- Gestion de dispositivos 
- Asignacion y monitoreo: si la impresora fallo, se instalo bien, etc..

"Clasificaicones E/S":
"Segun tipo de dispositivos":
- Acceso secuencial o aleatorio
- Compartible o dedicado 
- Por velocidad de operacion 
- Lecutra, Escrita, LECTURA/ESCRITURA 

"Orientada al bit vs orientada al caracter vs control":

"Sincrona vs Asincrona":
- "Sincrona": tengo que mantener cierto sincronismo entre mi codigo y mi IO, entonces yo no deberia 
seguir ejecutando el codigo si no tengo la respuesta de esa entrada salida.
Por ej: leo determiando block de un archivo y en la siguiente linea opero sobre esa porcion de memoria , 
si la entrada salida no finalizo, no puedo continuar. En un caso asi necesito que mi IO sea sincrona
Ej: lectura de archivos y cosas del fs, yo si no termine de leer un archivo, no quiero que mi proceso 
siga corriendo.
- "Asincrona": tengo ciertas operaciones que yo puedo hacer y mi proceso podria continuar la ejecucion sin 
mayores problemas. Se deja una funcion asociada que se usa cuando termina la entrada salida .
Nosotros simulamos operaciones asincronas con hilos, ees decir, hago algo y a la vez otra cosa, sin importarme 
si algo termino o no.

"Bloqueante vs No bloqueante":
- "Bloqueante": se piensa que es lo que va a pasar cuando hago la IO, si hago una IO bloquenate mi proceso 
va a intentar hacerla y si fuese necesario bloquear al proceso mientras se hace esa entrada salida .
El tipico ejemplo es de las clases de planificacion, que cuando un proceso tiene que hacer ES tenes 
que esperar que terminan las demas, encolarlo y esperar a que termine 
- "No bloqueante": yo quiero hacer una IO y me va a demorar un ratito, entonces una  IO bloqueante bloquea 
el proceso para que espere mientra se hace esa IO , la no bloqueante me devuelve un error..
cuando hacemos un recv por ejemplo, lo podemos hacer bloqueante , que quiere decir? que el proceso no va a 
seguir hasta que le llegue algo
y en un forma no bloqueante, podria seguir digamos. 
ej: el crear_conexion() en el tp, es no bloqueante.. que queire decir? que no te frena.
obs= el fopen en C es no bloqueante , si quiero abrir un archivo que no esta creado, me retorna un error .


SINCRONA-NO-BLOQUENATE= ejemplo el malloc.

"FORMAS DE REALIZAR LA IO":
"Programada":  aca no tengo interrupciones entonces voy trayendome de a porciones y durante ese tiempo 
tengo una espera activa donde mi cpu pregunta si podia seguir. Estaba pensado para programas con 1 solo 
proceso en ejecucion.
"Interrupciones":
"DMA": 

"DRIVER": cada periferico necesita un driver, la idea del driver es que el fabricante es quien mejor 
conoce a su dispositivo, con lo cual que mejor que cada fabricante prepare una porcion de software,
con una parte de bajo nivel que interactua con el dispitivo fisicamente (ej= move el brazo de aca 
para alla, move el cabezal, etc ) y la otra parte que nos tiene que hacer de interface con el S.O 
entonces cuando a mi dispositivo le decis "leer" tiene que hacer esto, si le decis "escribir" tiene 
que hacer esto..

"DISCOS RIGIDOS": vamos a usar este dispositivo como ejemplo.