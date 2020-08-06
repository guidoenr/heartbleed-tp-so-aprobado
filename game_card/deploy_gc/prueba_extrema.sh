#!/bin/sh
cd /home/utnso/workspace/tp-2020-1c-heartbleed/game_boy/Debug/

echo -e "\e[92m------------------------------------------------------"
echo -e "\e[92mTe mando unos 6 pro players, tendrian que llenarse 6 blocks "
sleep 1

./gameboy GAMECARD NEW_POKEMON Luken 1 1 1 1
./gameboy GAMECARD NEW_POKEMON Simple 1 1 1 2
./gameboy GAMECARD NEW_POKEMON Meyern 1 1 1 3
./gameboy GAMECARD NEW_POKEMON Kennys 1 1 1 4
./gameboy GAMECARD NEW_POKEMON Goncho 1 1 1 5
./gameboy GAMECARD NEW_POKEMON jks 1 1 1 6


sleep 35
echo -e "\e[92m------------------------------------------------------"
echo -e "\e[92mTe mando el catch para todos los wachines, deberian liberarse todos los blocks"
sleep 1

./gameboy GAMECARD CATCH_POKEMON Luken 1 1 1
./gameboy GAMECARD CATCH_POKEMON Simple 1 1 2
./gameboy GAMECARD CATCH_POKEMON Meyern 1 1 3
./gameboy GAMECARD CATCH_POKEMON Kennys 1 1 4
./gameboy GAMECARD CATCH_POKEMON Goncho 1 1 5
./gameboy GAMECARD CATCH_POKEMON jks 1 1 6


sleep 35

echo -e "\e[92m------------------------------------------------------"
echo -e "\e[92mVuelvo a crear todos los mismos en otro orden y tiene que pasar lo mismo"
./gameboy GAMECARD NEW_POKEMON jks 1 1 1 6
./gameboy GAMECARD NEW_POKEMON Goncho 1 1 1 5
./gameboy GAMECARD NEW_POKEMON Kennys 1 1 1 4
./gameboy GAMECARD NEW_POKEMON Meyern 1 1 1 3
./gameboy GAMECARD NEW_POKEMON Simple 1 1 1 2
./gameboy GAMECARD NEW_POKEMON Luken 1 1 1 1
