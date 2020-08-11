/-----------------------------------OPERATIVOS - CLASE 1----------------------------------/
INTRODDUCION_SISTEMAS_OPERATIVOS=;

KERNEL= es la capa del sistema opertavio que esta mas en contacto con el hardware, siempre
vamos a decir que el kernel esta siempre en memoria.
Si mi kernel es grande, usa gran memoria.. esto es lo que aparece cuando dice "minimo para funcionar
necesito tanta memoria". 
Es el nucleo de mi sistema operativo, "recompilar el kernel" seria modificarlo para que este ocupe
lo menos memoria posible.
Ej al leer un archivo, tengo que hacer que mi disco se mueva, mueva el cabezal, busque ese archivo
lo traiga y demas.. para hacer esto se hacen "system calls", que son una serie de servicios que 
me brindan los s.o y que me permiten acceder a servicios del kernel.

"SYSTEM_CALLS": lo mejor a nivel perfomance que vamos a tener, son un poco rusticas pero son 
realmente muy rapidas.
Al decir que son rusticas es que es muy raro que alguien escriba una syscall a mano para 
realizar una operacion en mi S.O.
estas estan encapsuladas en funciones de C, como por ejemplo malloc=(sizeof(...)),  malloc detras
tiene varias syscalls.
Las bibliotecas de sistemas son los set de funciones que agrupan syscalls, como por ejemplo 
las funciones de fclose, fopen, etc.. 

"SYSTEM CALLS VS BILIOTECAS DE SISTEMA": el problema aca es que las syscalls varian segun el S.O 
si yo uso un malloc en windows, linux, o lo que sea.. siempre me devuelve memoria, como lo hace por atras?
ni me preocupa, eso es la gracia de las bilbiotecas y permite que funcione en varias maquinas distintas.
En cambio al usar syscalls, podrian variar en los s.o y podria tener problemas.
Windows implementa un set de syscals diferentes al que linux, hacen lo mismo capaz.. pero de otra
forma.
Las syscalls capaz sirven mas a alguien que capaz quiere diseñar un software que quiere tener un 
contacto tal vez mas directo con el s.o. Ejemplo diseñar el sistema de movimiento de un cabezal.

"Cuanto mas cerca al kernel estas, mas poder tenes.. y mas responsabilidad tambien".

"MODOS DE EJECUCION": Pensalo como un flag que me dice si estoy en modo usuario o modo kernel.

"MODO USUARIO": Cuando estoy en este modo tengo un set de instrucciones que puede usar, puedo 
tocar varios registros, puedo ejecutar ciertas instreucciones , etc.
Pero hayt instrucciones que no, si quiero desabilitar las interrupciones y estoy en modo usuario
no lo podria hacer digamos

"MODO KERNEL": Mas set de instrucciones con mas poder, aca si podria deshabilitar las interrupcioens.
El SO inicia en modo kernel, una de las cosas que hace el so cuando arranca ademas de boootear y etc
es tambien pasar a modo usuario.
Ej= escribir algo en disco se hace en modo kernel, yo no puedo decirle al disco movete a tal lado y 
hace tal cosa.

"CAMBIO DE MODO":
• el cambio a modo usuario, es sencillo.. si estoy en modo kernel puedo hacer 
lo que yo quiera
• Ahora ,pasar de usuario a modo kernel.. no lo puedo hacer tan facil, yo quiero evitar que un 
programa este en modo usuario y diga "che bueno tengo que pasara a modo kernel" porque capaz 
hace quilombo.
Para evitar esto tengo dos formas de cambiar a modo kernel 
    1. Syscall con interrupcion: si ejecuto una instruccion que lo que hace es forzar una 
    interrupcion , yo no voy a llamar a cualquier interrupcion, voy a llamar cierta interrupcion
    y esa interrupcion va a tener codigo para atender una syscall.

    2. Fast Syscall:  llamo a una instruccion que se llama syscall y automaticamente
    estoy pasando a modo kernel y diciendole al SO que ejecute una instruccion.
El resultado final es cedo el flag, y le doy el poder al so para ejecute esa syscall que le voy
a pedir.


"TIPOS DE ESTRUCTURA DE S0"

"Monolitico": es la mas comun en el dia a dia. Todo mi SO esta ahi, es como un gran codigo
donded yo lo compile donde cualquier cosa le puede pedir cualqueir cosa a quien sea.
Lo bueno que tiene el monolitico es el mas rapido, porque como ttodo le puede pedir cualquier
cosa a cualquiera, no tengo ningun pasamanos en el medio.
Lo malo es que es mas propenso a errores, pensa que cualquier cosa que rompa hace que me rompa
todo el sistema.

"Multi-capa": es una especie para poder distinguir ciertas responsabilidades, entonces aca 
dividis todo en capa. Una capa se ocupa de los usuarios, otro de lo que es la interaccion 
con software, etc. 
Los pedidos vienen de las capas de arriba y van hacia las de abajo y asi..
El problema que trae me genera bastantes pasamanos, si eventualmente el equipo que mantiene 
una capa toca algo , puede traer algunos problemas.

"Microkkernel": tengo una buena parte de mi SO que la necesito si o si, y necesito que ejecute
como modo kernel.. pero hay un monton de cosas que yo meto en el kernel pero que podria tenerlas
afuera.
Imaginate el codigo que haga que el disco busque un archivo ,mueva el cabezal y eso... esto definitivamente
va en el kernel pero pensa por ejemplo cosas que no necesito que corran en modo kernel , lo sacas
a modo usuario.. entonces ganas en el lado de la estabilidad , penas que la tablita que me administra
los archivitos abiertos, esta en modo usuario.. si ese se cae, no hay drama , lo levantas denuevo.
El problema aca es que el modo kernel es lento, porque tengo que pasar del modo kernel a modo 
usuario varias veces. 
el microkernel es el menos usado.

"Maquinas vrituales":es un software que emula un hardware, 
    Tipo 2: es como bajarse el virtualbox, levantas la imagen de una VM y para nosotros mientras
    ejecutamos esa vm, el hardware que usa es uno.. pero atras de la maquina hay otra cosa.
    Se usan mucho para pruebas de aplicaciones, para emular distintos hardwares.


el "HYPERVISOR": es el software que me permite usar la vm.

/-----------------------------------OPERATIVOS - CLASE 2----------------------------------/
PROCESOS-HILOS=;

"PROCESO": Programa en ejecucion.. cuando yo tengo el programa es como el molde del proceso
en ejecucion.
Es muy comun correr varias veces un programa y tener varios procesos para el mismo programa.
Cuando un proceso esta en ejecucion neceesitamos un par de cosas, esas cosas son.
    ESTATICAS=
    -El codigo: codigo de un programa.c , es estatico.
    -Datos: variables globales
    DINAMICAS=
    -Stack: la pila de mi proceso , yo voy a ir guardando retornos de funcion , variables locales
    y parametros que funcionan como variables locales 
    Cuando voy ejecutando y metiendo en diferentes funciones , las meto aca , el main por ejemplo
    esta en el Stack
    -Heap: la porcion de memoria destinada al proceso donde yo guardo todo lo que es memoria dinamica 
    cuando yo hago un malloc(tam) ese tam esta en el heap.
    El puntero a ese mega, capaz esta en el stack si es una variable local. 
    ejemplo: char* pepito = "pepito" el puntero 'p' esta en el stack y el contenido en el heap.

Es normal que el heap y el stack crezcan. 
Si el stack crece demasiado, puede ser un error.. ejemplo una recursividad mal cortada..
al stack generalmente se le pone un tamaño, entonces cuando se pasa tira un stackoverflow
pensa que al meterle cosas locales y retornos de funcion no seria muy normal que eso crezca 
mucho.

"EJEMPLO DE STACK:"
int main(int arg, char* argv){
    hacer_algo(3);
}

void hacer_algo(int tope){
    if (tope>0){
        hacer_algo(tope-1);
    }
}
voy apilando y desapilando en base que quiere ejecutar.


"SCOPE O ALCANCE DE LA FUNCION": es el segmento de interes de la funcion digamos, es decir, las 
cosas a las que puedo acceder.
;(1)
"ESTADOS DE UN PROCESO": el estado va cambiando cuando cambia la ejecucion del proceso.
"Nuevo-new": el proceso se esta creando.. esta creacion lo que implica es que mi sistema esta
creando las estructuras necesarias para que ese proceso corra. (stack,heap,etc..)

"Ready": cuando el proceso es admitido ,pasa a ready.. el proceso esta esperando que le asignen
CPU., es decir, esta listo para arrancar o para seguir pero esperando que se le asigne la CPU.

"Ejecucion": cuando se le asigna la CPU, pasa al estado running/ ejecucion.
En esta instancia estamos corriendo sus instrucciones, los registros de CPU estan cargados
con informacion de ese proceso y demas.
    "a un proceso en ejecucion se lo puede sacar por varios motivos y cuando se lo saca
    comunmente es porque se quiere meter otro de mas prioridad, otra opcion seria que el
    proceso voluntariamente dejara la cpu"
    "Waiting": el proceso esta en espera por algun tipo de evento, el clasico es io,
    como tiene que esperar un tiempo muy grande.. entonces lo mando a waiting. Si 
    la espera fuera muy chiquita, lo mantengo en ejecucion, cuando ocurre el evento
    lo pongo a ready denuevo
    "Terminado": el proceso viene a este estado cuando termina.

obs= puede ocurrir que un proceso de ready pase a terminado, es por ejemplo cuando finalizas 
un proceso en windows.

obs= en la transicion de new a ready, (admitted) en algunos sistemas no tan comunes , lo que pasa
es que algunos procesos yo decido no admitirlos durante un tiempo.. imaginate un sistema que 
solamente quiere tener dentro de todo mi ciclo ready-waiting-running solamente 10 procesos.
Cuando el proceso esta en admitted, estaria esperando que el grado de multiprogramacion se 
actualize, y puede pasar cuando un proceso pasa de running a terminated.

obs= ready-runing-waiting usan memoria principal, new-terminated a disco.
terminated como que libera los recursos que tiene asignado.

"GRADO DE MULTIPROGRAMACION": mi sistema puede tener en memoria hasta N procesos. Para lograr 
esto tengo que tener una especie de mecanismo control policial pidiendome permisos para poder 
entrar o no.. el criterio podria tener criterios de prioridad y demas.
en resumen , es cuantos procesos puedo tener ejecutando en mi ciclo ready-waiting-runing
si yo miro un intervalo de tiempo , voy a tener varios procesos que ejecutaron, en un intervalo.
en resumen, el grado de multiprogramacion es cuantos procesos puedo tener en memoria.

"GRADO DE MULTIPROCESAMIENTO": esto me lo define el hardware, es la cantidad de procesos que yo
puedo tener ejecutando en simultaneo.
si yo miro un instante, voy a tener varios procesos corriendo. 

;(2)
"OTRAS FORMAS DE ESTADOS": mismo esquema pero blocked = waiting 
se agregan estos dos estados cuando decido que algunos procesos que estan en memoria, tienen 
que liberar esa memoria, o bien me estan ocupando espacio en mi grado de multiprogramacion.
Entonces yo tengo dos opciones, o matar procesos.. o bajarlos a "SWAP" que es disco.
"Ready suspended"
"Blocked suspenden"
son los que estan en disco.

"Tipos de Planificaciones - Schedulers":
Extra_largo_plazo= consiste en configuracion del administrador, entonces se le pregunta al administrador 
que prioridad uso, cuanto grado de multiprogramacion, etc. Se hace una vez cada tanto.
Largo_plazo= ya haablamos de un modulo del sistema operativo, que controla el grado de multiprogramacion 
voy a tener en algun lado una funcion que me dice "este proceso pasa de new a ready, este de tal a tal ..etc"
Mediano_plazo= se ocupa de algo similar a largo plazo pero a la hora de suspender procesos, dice
"a este lo suspendo, a este lo traigo devuelta"
Corto_plazo= controla el grado de multiprocesamiento, quienes pasan de ready a running e ir mirando 
que el estado running nunca quede vacio. Este es el que mas se ejecuta y casi siempre el que se busca 
optimizar.

"PCB": Procces Control Block.. se lo conoce tambien como la imagen del proceso o "todo lo que apunta"
Es el block que contiene informacion asociada a cada proceso, informacion como la de abajo 
o bien punteros a estos datos. El pcb se guarda en memoria.
- ID 
- Estado del proceso: blocked / ready
- Contador de programa : por cual linea va mi codigo 
- Registros de la cpu 
- Informacion de planificacion de CPU 
- Informacion de gestion de memoria re
- Informacion contable: cuanto tiempo lleva ejecutando 
- Informacion de estad de IO 
- Punteros 
obs= el heap, data y todo lo demas es apuntado por el PCB.

"Cambio de contexto": cuando se cambia el proceso , lo que hacemos es guardar el estado de ese 
proceso y a su vez tenemos que cargar el estado de el nuevo proceso.. en resumen, es leer los registros 
de cpu que estaba usando y guardarlos y escribir los nuevos registros de cpu.
El tiempo de cambio de contexto se llama "OVERHEAD".
Cuando mi cambio de contexto consiste en cambiar un proceso por otro, tambien tenemos cambio de proceso 
Puede haber casos donde cambio de contexto para atender una interrupcion y despues sigue ejecutando 
el mismo proceso, cuando pasa eso, solamente hay cambio de contexto. 
Cuando hay cambio de contexto podria haber cambio de modo, y esto ocurro cuando tengo que 
escribir en el PCB porque lo tengo que hacer en el modo kernel.
;(3)
https://youtu.be/CRW16WWRpUk?list=PL6oA23OrxDZDQEFo7aKBceotX48vLmQYA&t=2435
CLASE DE ADRIANO - PCB - CAMBIO DE CONTEXTO - demas

"HILOS - THREADS": la idea del hilo es para querer ejecutar varias veces el mismo codigo o porciones del 
mismo proceso. 
es una forma de dividir o estructurar mi proceso, los diferentes hilos pueden ejecutar el mismo set de codigo 
pero no tienen porque estar en el mismo momento al mismo tiempo.
Los hilos comparten recursos como codigo,datos y memoria.
El cambio de contexto entre hilos es mucho mas rapido, yo no tengo proteccion entre hilos lo que podria pasar
seria que el hilo le pise la pila a otro hilo.
generalmente a los hilos se les pasa una funcion para ejecutar, es muy raro que se le vuelva a pasar todo 
el main.


;(4)
"TCB": Thread Control Block: es una estructura aparte para cada uno de los hilos, qeu va a tener el puntero 
al stack, algunos registros, el IP de c/u de los hilos y puntero al PCB ( el pcb tmb tiene punteros a los tlb)

"DIFERENCIAS CON PROCESOS":
- Permiten paralelismo dentro de un proceso o aplicacion: ¿porque usaria hilos? si yo quiero ejecutar el codigo
10 veces , quizas me sirva en vez de crear 10 procesos, crear 10 hilos y cada uno de esos hilos hace una partecita.

- Permiten la comuniucacion privada sin solicitar intervencion del SO.
- Mayor eficiencia en el cambio de un hilo a otro, que de un proceso a otro 
- Mayor eficicencia en la creacion de un Hilo que en la creacion de un Proceso hijo 
- Un proceso multihilo puede recuperarse ed la muerte de un hilo.
- Cuando un proceso muere, todos sus hilos tambien, pues los recursos de proceso son tomados por el 
sistema operativo.

"IPC": inter procces comunication: ejemplo los sockets del TP.
 
"CONCURRENCIA": todos comparten eventualmente el mismo recurso , si yo miro en un intervalo de tiempo 
un monton de tipitos usaron el recurso este.. pero si yo miro un instante en particular solamente hay 
1 utilizandola

"PARALELISMO": si miro en algun instante en particular voy a poder tener varios, si yo tengo paralelismo 
yo tambien tengo concurriencia, pero no viceversa.
;(5)

"KLT o HILOS NIVEL KERNEL o KERNEL LEVEL THREADS": cuando yo hablo de KLTS, hablo de hilos nativos o pesados .
La idea de esto es que el sistema operativo los conoce, osea que los puede controlar, crear, lo que quiera.
Como el S0 los conoce , puede poner uno en cada cpu , osea los toma como si fueran procesos distintos.
Si un hilo se bloquea, el resto puede continuar.

"ULT - User level Threads - Green Threads": no le dejo la administracion de mis hilos al SO, sino que tengo 
mis bibliotecas que me manejan hilos.. el SO no los conoce, entonces todo lo que pase , para el SO, lo hizo 
un unico proceso. 
El cambio de contexto es mas rapido, porque es llamar a funciones.. pero si un hilo se bloquea, todo el proceso 
se va a bloquear.

"Memory Leaks": memoria que pido y no devuelvo.



/-----------------------------------OPERATIVOS - CLASE 3----------------------------------/

"PLANIFICACIÓN - SCHEDULER": Definen el orden de los procesos , ordenar en el tiempo 
en base a diferente criterios. Ej: vacaciones, tengo tiempo limitado por 
el coronavirus.. entonces puedo definir que hago en base a todo eso. 
Algo parecido se hace en la CPU con los procesos.
En resumen, es ogobernar el orden de ejecucion de los procesos.

"Planificador de procesos": modulo del SO que se encarga de mover los 
procesos entre las distintas colas de planificacion.

"Proceso limitado por E/S": (io bound) es aquel que pasa mas tiempo haciendo
E/S que usando la CPU, tiene rafagas de CPU cortas.
Ej: un software que te pide muchos datos para ejecutar y en el medio tmb

"Proceso limtado por CPU": (CPU  bound) es aquel que pasa mas tiempo 
procesando que haciendo E/S (tiene ragas de CPU largas)
Ej algoritmos que hacen calculos matematicos.

"Planificador sin desalojo": planifica solo cuando un proceso deja la CPU voluntariamente.
"Planificador con desalojo": planifica cuando un proceso entra a la cola de listos y  lo de arriba 

- La ejecucion de un procesos consiste en una alternancia entre rafaga 
de CPu y rafagas de E/S.

El planificador escoge un proceso de entre los que estan en memoria listos 
para ejecutarse y le asigna CPU al proceso elegido
La decision de planificacion puede ocurrir:
    1. Cuando un proceso deja la CPu voluntariamente
    2. Cuando un proceso entra a la cola de listos

DISPATCHER: modulo que cede la CPU al proceso elegido por el planificador 
de CPU. Para esto debe hacer
    - cambio de contexto
    - cambiar la maquina a modo usuario
    - saltar al punto apropiado del programa para continuar con su ejecucion
latencia de dispacher: tiempo que trada en detener un proceso y poner otro en ejecucion


CRITERIOS DE PLANIFICACION: (nunca se pueden optimizar todos)
- "Utilizacion de la CPU" : mantener la CPU tan ocupada como sea posible,
si mi criterio es ese busco un algoritmo que haga eso. EJ - ROUND ROBIN
- "Rendimiento" : buscar que se completen la mayor cantidad de procesos por unidad de tiempo.
- "Tiempo de retorno" : tiempo transcurrido desde que el proceso entra al sistema
hasta que termina. Si hay muchos procesos el tiempo de retorno es alto, 
entonces voy a tener que buscar algun balance que miniminice el tiempo de 
retorno. EJ: OTRA VEZ, ROUND ROBIN NO VA BIEN.
- "Tiempo de espera": tiempo que un proceso pasa en la cola de listos
- "Tiempo de respuesta": tiempo que tarda un proceso desde que se le presenta
una solicitud hasta que produce la primera respuesta (minizar/hacerlo previsible)
Ej: las paginas webs. Amazon pierde plata si el tiempo de respuesta es grande,
pues los usuarios cerrarian la pestaña antes de que termine de abrirse, suponiendo
que tarda en abrirse unos 10segundos..

....................................................
ALGORITMOS:
- "FIFO" : Ineficiente, es facil de ver.. primero que entra primero que sale
ejecuta cada proceso al completo, sin importar la rafaga (milisegundos) que tarden
El tiempo de espera medio es poco previsible, puede ser muy alto.
La sociedad entera usa este algoritmo ineficiente sin saberlo.

- "SJF(Shortest Job First) SIN DESALOJO" : asigna la CPU al proceso que dura menos tiempo
no funciona porque no conocemos la rafaga.. es el que menos tiempo de espera me da 
el gran problema que trae es el starvation, y ademas debemos saber de antemano
cuanto tiempo va a demorar. Paa poder usar SJF necesito saber ese tiempo.
Cuando el tiempo es igual, se suele desempatar por fifo.
SJF minimiza mucho el tiempo de espera medio, pero tiene un problema .. 
en el ejemplo de la foto P1 ejecutra primero a pesar de que es el mas largo, y 
lo hace porque llego primero.. pero su rafaga era la mas larga..
;(6)

- "SJF(Shortest Job First) CON DESALOJO" : es igual que el de arriba, pero con una diferencia 
aca este algoritmo lo que hace es: cuando llega un nuevo proceso a mi cola ready, y tengo uno ejecutando 
actualmente, evaluo.. me combiene que siga ejecutando el que estaba? o me combiene desalojarlo porque 
el que entro nuevo dura menos tiempo?
El gran problema de este es que tiene mucho "overhead", estoy todo el tiempo cambiando de procesos 
y por otro lado, tiene starvation, que significa? si a mi me siguen llegando procesos cortos, voy a tener 
procesos largos que nunca van a ejecutar.
obs= los SJF son medio raros, pero uno nunca conoce cuanto va a tardar en ejecutar un proceso.. pensa que 
tenes un codigo que si entra al if() hace algo , pero si entra al else() hace algo totalmente distinto y capaz 
tarda mas o menos, entonces esa rafaga se podria aproximar si se quisiera, pero nunca se sabria con certeza.
;(7)

- "SJF - Estimacion ": la rafaga no se conoce ,por lo tanto como no se puede 
calcular.. se estima con una formula , usando la rafaga que use antes.
Tn = longitud de la n-esima rafaga de CPU
Tn+1 = valor predicho para la siguiente rafaga de CPu
alfa = esta entre 0 y 1

Tn+1 = alfa.Tn + (1 - alfa).Tn

"Si el alfa tiende a ser muy grande" , le doy mas importancia a la longitud de la rafaga anterior que 
a mis estimaciones.
"Si el alfa tiende a 0", no le estoy dando mucha importancia a la rafaga anterior.
y le estoy dando mas importancia a mis estimaciones anteriores.
Si yo confio en mis estimaciones , no quiero que una rafaga muy diferente a mi ejecucion normal, 
no quiero que me arruine mi estimacion 

obs=se va a usar casi siempre el Alfa = 0.5 que es el promedio entre ambas.
"WHO CONTROLS THE PAST CONTROL THE FUTURE"

- "Prioridades (generalizacion)": a cada proceso le voy a poner una prioridad, que va a ser un numero.
La cpu se asigna al proceso que tenga la prioridad mas alta (el numero mas bajo)
Windows es alreves, en linux tiene 0 la mayor prioridad y en windows 35 por ej
un problema que tiene de starvation es que los procesos de mas baja prioridad 
podrian no ejecutarse nunca.
la solucion es el aging : conforme el tiempo pasa, se aumenta la prioridad de 
los procesos que esperan mucho en el sistema.
starvation : los procesos que mas baja prioridad tienen, tienden a no ejecutarse nunca

- "HRRN(Highest response ratio next)" : adaptacion del SJF que permite romper el starvation
el que tiene el mas ratio, es el que va a ejecutar.. entonces aca entra en juego el tiempo que lleva
esperando un proceso.
S= tiempo de servicio de la proxima rafaga
W= espera actual
R.R = (S+W) / S
cuanto mayor sea la espera, respecto al tramaño de su rafaga, mayor sera su prioridad para ejecutar 
ejemplo: una espera a la cantidad de productos que va a llevar, el que lleva un carrito gigante
va a esperar millones de horas.

- "ROUND ROBIN (Fifo por turnos)": vamos a definir un tiempo llamado quantum
primero que entra primero que sale, el quantum es un tiempo que se le da al proceso
para terminarlo, si no alcanza a terminarse, se lanza una interrupcion.
ejemplo: "te cobro 10 productos, despues te mando a la fila devuelta"
si el quantum es muy grande, ponele que llevas 1000 productos, nunca se va a interrumpir a nadie
puesto que quien mierda va a llevar 1000 productos viejo.
si quantum muy chico, es mucho overhead, pensa, te cobro 1 producto , te mando denuevo, cobro otro producto
y te mando denuevo.. es decir, estaria interrumpiendo a todos los procesos.
si el quantum no le alcanza , se lo interrumpe y se lo manda a ready, 
despues si no le alcanza el otro quantum , denuevo lo mismo.
"como elegir un buen quantum": tiene que ser lo suficientemente largo para no molestar a todos los procesos 
y tambien lo suficientemente corto para que si un proceso se pasa, frenarlo.
yo miro mi sistema, miro las rafagas de mis procesos y busco un quantum, que al 80% de las rafagas de mis 
procesos, deje pasarlas sin interrumpirlas
EL FIN DE QUANTUM MATA TODO.

"Feedback": configuracion de varias colas o algoritmos, es el famoso colas multinivel con prioridades , donde 
por ejemplo la de mas prioridad es RR2, la de menor prioridad un fifo, etc

obs= un s.o nunca va a tener un algoritmo pelado, siempre va a tener mezclas entre ambos o mejoras.
--------------------------------------------------------------------------------------------------------
CLASE 4: 29/04 - SINCRONIZACION
--------------------------------------------------------------------------------------------------------
Race Condition: el resultado de la ejecucion de los procesos depende
del orden en que se ejecutan. Es aleatorio, lo corro una vez pasa algo,
lo corro otra vez y pasa otra cosa.

VARIABLE GLOBAL x;
hilo1 - x+1
hilo2 - x*5

resultados??? anda a saberlos papito.. depende del planificador
y como se ejecute en cada PC.
Recursos criticos: recursos que se comparten entre hilos/procesos
Region critica: donde se usa esa variable(en que linea x ej)

CONDICIONES DE BERNSTEIN:
- vamos a distinguir 3 tipos de procesos: independientes(no interactuan entre si, ej team y gamecard)
,cooperativos(interactuan de alguna forma) y competitivos(estan compitiendo por alguno recurso )
- determinan si dos procesos se pueden ejecutar concurrentemente
- dados dos procesos a y b, sus conjuntos de lectura se definen como R y su escritura como W
- esos dos procesos se pueden ejectuar concurrente si:
    Wa ^ Wb = {}
    Wa ^ Rb = {}
    Ra ^ Wb = {}

SECCION CRITICA: cada proceso posee un fragmento de codigo, denominado
seccion critica, que no debe intercalarse con las secciones criticas de los
demas procesos.
    -la ejecucion de las secciones criticas debe ser mutuamente exclusiva
    para evitar inconsistencia de datos
    -para solucionarlo , diseñas un protocolo.
|
v

protocolo:
- cuando ingreso pido permiso (me pueden decir que no)
- cuando nos vamos, avisamos que nos vamos

-------------------------------
SEMAFOROS: herramienta de sincronizacion que no requiere espera activa 
(pero puede usarla)
va a tener una variable entera..
dos operaciones estandar modifican s : wait() y signal(), ademas
se lo inicializa con un valor positivo o cero.
solo puede accederse al semaforo a traves de las dos operaciones atomicas

    -semaforo contador: tiene un valor entero que puede variar
    en un dominio no acotado
    -semaforo binario: tiene un entero que puede variar entre 1-0
    (libre - ocupado)
    -semaforo mutex: garantiza mutua exclusion, es un caso particular
    del binario..

uso de un mutex:
Semaphore S = 1; // en codigo se le llama a la funcion init();
wait(S);
    seccion critica
signal(S);
------------
IMPLEMENTACION.
- se debe garantizar que dos procesos no ejecuten wait y signal
sobre el mismo semaforo al mismo tiempo
- la operacion wait puede implementarse con espera activa
    - si la seccion critica es corta la espera activa tmb lo sera
    - las aplicaciones pueden pasar mucho tiempo en secciones criticas
        y por lo tanto, no es una buena solucion
    -se desaprovecha la CPU

NO USAR SLEEP

NORMALMENTE SE USA UN SEMAFORO DISTINTO PARA CADA RECURSO COMPARTIDO
QUE QUIERO USAR ..

siempre hay un hilo que lee, otro que escribe.. y ahi siempre se usa
mutex

blockear hilos que no hacen nada = lo podes hacer con un wait() 
y el semaforo inicianilazdo en 0, y se lo levanta cuando se le pune 1


wait(decrementa el semaforo) --
signal(aumenta el semaforo) ++

3 problemas a solucionar en ejercicios de este tipo:
    -Mutua exclusion
    -Orden
    -Limites



------------
PRODUCTOR - CONSUMIDOR (buffer infinito)
https://youtu.be/DfoqRiffLN8?list=PL6oA23OrxDZDQEFo7aKBceotX48vLmQYA&t=2321
(productor) -> (cola) -> (consumidor)

Productor               Consumidor 

                        wait (mensajes)
msj = crear ()          wait(mutex)
wait(mutex)             msg = retirar(cola)
insertar(msj,cola)      signal(mutex)
signal(mutex)           consumir(msg)
signal(mensajes)

mutex = 1
mensajes = 0

--------------------------------------------------------------------------------------------------------
CLASE 5: 06/05
--------------------------------------------------------------------------------------------------------
fue practica

--------------------------------------------------------------------------------------------------------
CLASE 5: 13/05
--------------------------------------------------------------------------------------------------------
DEADLOCK Y REPASO
;juan b justo y santa fe (autos en cuadrado)
"DEADLOCK": bloqueo entre dos o mas procesos(o hilos) que cumple con las condiciones: 
    - "mutua exclusion": o hay uno o hay otro, pero no puede haber dos autos en la misma posicion 
    cuando hay un auto en un lugar , no puede haber otro en el mismo lugar 
    - "espera y retencion": mientras estoy esperando por un siguiente recurso (siguiente pedaso
        de calle) estoy esperando un recurso mientras estoy detienienndo al otro
    - "no ocurre desaolojo": los autos no se mueven, a menos que halla un policia que los saque 
    o un auto que se vaya
    - "espera circular": autos esperan que el siguiente se mueva y el sigueinte al siguiente etc 
obs= cuando se cumplen estas cuatro condiciones, decimos que nuestros procesos estan en deadlock.
obs= para resolver el deadlock tengo que atacar a los procesos que estan dentro del deadlockk, no el 
auto que esta afuera de la calle digamos esperando que  se resuelva el deadlock.


"ESTRATEGIAS": 
    - "No hacer nada": es costoso y peligroso combatir el deadlock, entonces a veces es mejor no hacer nada
        reinicio la compu. Ej si tengo una pc que la esta usando alguien todo el tiempo ,y abre muchos programas 
        y dedmas y todo eso entra en deadlock, la persona dice "bueno la proxima abro menos cosas", reinicio
        la pc y se soluciona el deadlock.
        Entonces , en resumen, no hacemos nada cuando el deadlock es muy peligroso de combatirlo o muy costoso.
    - "Prevencion": prevenir alguna de las cuatro condiciones: hago que alguna de las 4 condiciones nunca se de.
            "mutua exclusion": no dejando que mis procesos tengan mutua exclusion, si dejo que se pisen
            estoy teniendo un problema, no se apunta mucho a esto.. lo que se busca es no generar mutuas
            exclusiones donde no hace falta, y no mucho mas que eso.. esta prevencion no se hace mucho.
            pensa que la mutua exclusion es un problema que resolves en sincronizacion.
            "espera": si algun proceso me pide recursos, tengo que hacer que no espere.. si no tengo 
            los recursos para darle, lo puedo finalizar o decirle esos recursos no estan.
            o tener una cantidad ridicula de recursos de forma tal que mis procesos nunca esperen, entonces
            cuando arranca les doy todos los recursos que pueden llegar a necesitar.
            pero de esta forma soy ineficiente, les doy mas de lo que necesitan.. peeero soluciona el DEADLOCK
            "retencion": podria hacer que un proceso me tenga que devolver el recurso antes
            de darle otro.. por lo general la espera y retencion se resuelve dandoles los recursos que necesita
            al principio y listo (si puedo obviamente)
            "desalojo": garantizar que exista algun problema de desalojo, pero me trae el problema de que en algunos casos  
            de que voy a poder desalojar procesos y a veces no, para la memoria ram es facil de desalojar, porque 
            lo que esta en memoria lo puedo volcar a disco y ya esta, garantizo que esta bien guardado.
            Para el resto de los recursos es complicado el tema del desalojo, imagiante que estoy escribiendo 
            un archivo en cierto byte, y un proceso me lo pide.. entonce es algo complicado guardarse 
            todo eso antes de darlo.
            "espera circular": si lo pensamos en 4 esquinas como 4 recursos, tengo un auto que necesita la 
            E1 otro que necesita la E2, etc.. si rompes ese ciclo, por ejemplo, si ya estas en la E4 no podes
            ir a la E1.. entonces lo que se hace es numerar mis recursos, entonces se piden los recursos en orden 
            si me pediste un recurso de tipo 4 no me podes pedir un recurso de tipo 1, pero si lo necesitas 
            primero pedime el de E1 y despues el de la E4, va a haber mucha demora pero no va a haber espera 
            circular.
        ;;prevenir cualquiera de estas 4 es muy complicado, depende de mucho el sistema de cual prevenir
        • es una decision de diseño
    
    - "Evasion": tampoco llegamos deadlock, la idea de atras de la evasion es que nosotros vamos a tener un algortimo 
    llamado algoritmo del banquero , lo que hace es que cada vez que un proceso me pide un recurso, lo que hace 
    es una simulacion.. y esa simulacion lo que hace es "ver que pasaria si le doy ese recurso a ese proceso", entonces 
    dada la simulacion me pregunto "tengo la posibilidad de que lleguemos a deadlock?" si la respuesta es si: 
    le negamos el recurso, si la respuesta es no: se lo damos.
    Esta opcion es pesimista, porque analiza el peor de los casos, entonce esto dice.. si el peor de los casos , me garantiza 
    que no hay deadlock, entonces nunca vamos a tenerlo.
    "Estado Seguro": ninguna chance de que halla deadlock 
    "Estado Inseguro": posibilidad de que pase deadlock (es posible que pase, no quiere decir que si o si va a pasar)

    - "Deteccion": cada una cierta cantidad de tiempo, buscamos deadlocks.. si hay deadlock, vemos que hacemos 
    y sino hay deadlock, todo sigue como si nada.

"LIVELOCK": es una especie de bloqueo donde los procesos no estan bloqueados, pero sin embargo ninguno progresa 
imaginatelo en la calle donde los 4 autitos estan esperando en la esquina y frenan todos porque saben que van 
a chocar.
el livelock es mas dificl de detectar, a pesar de que si miras la ejecucion, los procesos estan ejecutando y uno podria 
notar que estan tardando demasiado tiempo en realizar las tareas, usan siempre los mismos recursos, etc.
es muy manual esta deteccion.
los procesos intentan que no se den los deadlocks, llegar a un livelock ocurre normalmente cuando se trata de prevenir 
un deadlock.
Los procesos pueden llegar a starvation: algun proceso podria nunca terminar de ejecutar o no ejecutar nunca,
porque otro con mas prioridades los postergan.
aca es algo parecido ,no por un tema de prioridad.





no se puede realizar una operacion en HW sin hacer syscall.. si fuera en modo usuario , romperia

;- web browser.. mejor hilos o procesos?
multi procesos es mas estable que multi-hilo.. si flasha el plugin de adobe flash, usando hilos,
romperia en todas las otras ventanas.. 
tambien da seguridad, puesto que no comparten el heap como en los hilos..
- conviene usar un proceso por cada pestaña abierta o uno por cada dominio ej www.utn.so visitado?
ponele que tenes abierta en un hilo el youtube.com , si se te cae un video, se te caerian todas las 
demas ventanas .. 
es mas estable usar procesos.

;- tengo un SO de un telefono con procesos corriendo de diferentes prioridades, tales como lladas
;notificaciones de apps, mensajes de texto, y aplicaciones donde varias pueden ejecutar al mismo tiempo
;pero solo una se muestra por pantalla. Como configuraria un algoritmo de cultas multinivel para 
;optimizarr el planificador
se solucionaria con colas de diferentes prioridades..  o con una sola cola que asigna el cpu
segun el proceso que necesite 
"las llamadas son una cola, con algoritmo prioridades donde la llamada que yo pongo en atencion
tiene la prioridad maxima y el resto tienen igual prioridad, y desempatan por fifo"
"notificaciones , aplicaciones, mensajes y demas.. cola de baja prioridad a la cual si esepera
mas de un determinado tiempo (ej 5 seg) la subo de prioridad para que se ejecuten y roimper el 
starvation"
"aplicaciones se pensaria como uan cola intermedia donde quella que este en ejecucion es la que
mas prioridad tiene..."
    }
"dividirlo en dos colas, lo que esta en pantalla lo pongo en una cola, lo otro en la otra
a las de primer plano le pongo un quantum grande y al resto le pongo un quantum chiquito"


;porque la sentencia variable++ no es considerada segura en un ambiente multi hilos? 
race condition 

;que desventaja traeria que el compilador rodee automaticamente cada aparicion de una sentencia
;similar de semaforos mutex?
agrega un overhead que no seria necesario 

;podria ocurrir deadlock con dos semaforos mutex distintos entre dos procesos? 

wait(muitex1)
wait(mutex2)
..
signal


wait(mutex2)
wait(mutex1)
..
signal

"uno retiene el mutex2 mientra retiene el mutex1 y el otro es viceversa"


camibo de modo: en el instante 0 , tenes una syscall




