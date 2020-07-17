#!/bin/sh

cd /home/utnso/workspace/tp-2020-1c-heartbleed/game_card
echo -e "Verificar que no existan archivos dentro del mismo \e[36mC"
ls -l Montaje/Files
echo -e "Ejecutar el script new_pikachu.sh \e[36m"
bash new_pikachu.sh
echo -e "Se creo la carpeta Pikachu y su metadata indica que el tamaño es 7 bytes \e[36m"
ls -l Montaje/Files 
xxd Montaje/Files/Pikachu/Metadata.bin
echo -e "Ejecutar el script new_pokemon_varios.sh \e[36m"
bash new_pokemons_varios.sh
echo -e "El tamaño del archivo Pikachu se haya actualizado a 13 bytes \e[36m"
echo -e "Se creo la carpeta Charmander y su metadata indique que posee dos bloques y su tamaño es 70 bytes \e[36m"
ls -l  Montaje/Files
xxd Montaje/Files/Charmander
echo -e "Ejecutar el script catch_charmander.sh \e[36m"
bash catch_charmander.sh
echo -e "Verificar que el archivo Charmander ahora indique que posee solo un bloque y su tamaño es 61 bytes \e[36m"
xxd Montaje/Files/Charmander/Metadata.bin
