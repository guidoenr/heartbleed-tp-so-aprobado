#!/bin/sh
echo -e "\e[92m-------------------------------"
echo -e "\e[92mPrueba unitaria para el filesystem"
echo -e "\e[92m-------------------------------"
sleep 2
cd /home/utnso/workspace/tp-2020-1c-heartbleed/game_card

echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mVerificar que no existan archivos dentro del mismo"
cd Montaje/Files
pwd
ll
sleep 3
cd /home/utnso/workspace/tp-2020-1c-heartbleed/game_card

echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mEjecutar el script new_pikachu.sh"
sleep 1
bash deploy_gc/new_pikachu.sh
sleep 7

echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mSe creo la carpeta Pikachu y su metadata indica que el tamaño es 7 bytes"
ls -l Montaje/Files 
xxd Montaje/Files/Pikachu/Metadata.bin
sleep 3

echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mEjecutar el script new_pokemon_varios"
sleep 1
bash deploy_gc/new_pokemons_varios.sh
sleep 50


echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mEl tamaño de Pikachu se actualizo a 13 bytes"
xxd Montaje/Files/Pikachu/Metadata.bin
sleep 3

echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mSe creo la carpeta Charmander y su metadata indique que posee dos bloques y su tamaño es 70 bytes"
ls -l  Montaje/Files
xxd Montaje/Files/Charmander/Metadata.bin
sleep 3

echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mEjecutar el script catch_charmander.sh"
sleep 1
bash deploy_gc/catch_charmander.sh
sleep 7

echo -e "\e[95m------------------------------------------------------------"
echo -e "\e[96mVerificar que el archivo Charmander ahora indique que posee solo un bloque y su tamaño es 61 bytes"
xxd Montaje/Files/Charmander/Metadata.bin
sleep 3


echo -e "\e[92mScript finished"
echo -e "\e[92m--------------------"
echo -e "\e[97mMontaje eliminado"

