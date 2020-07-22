#!/bin/sh
cd /home/utnso/workspace/tp-2020-1c-heartbleed/game_boy/Debug/
./gameboy BROKER CAUGHT_POKEMON 1 OK
./gameboy BROKER CAUGHT_POKEMON 2 FAIL

./gameboy BROKER NEW_POKEMON Pikachu 2 3 1

./gameboy BROKER CATCH_POKEMON Onyx 4 5

