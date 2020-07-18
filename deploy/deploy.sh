#!/bin/sh

cd /home/utnso/workspace/

chown -R utnso:utnso tp-2020-1c-heartbleed/

echo -e "Clonando las commons \e[35m"
git clone "https://github.com/sisoputnfrba/so-commons-library"

chown -R utnso:utnso so-commons-library/

cd so-commons-library
make all

echo -e "Compilando broker \e[35m"
cd /home/utnso/workspace/tp-2020-1c-heartbleed/broker/Debug
make all

echo -e "Compilando filesystem \e[35m"
cd /home/utnso/workspace/tp-2020-1c-heartbleed/game_card/Debug
make all

echo -e "Compilando team \e[35m"
cd /home/utnso/workspace/tp-2020-1c-heartbleed/team/Debug
make all

echo -e "Compilando gameboy \e[35m"
cd /home/utnso/workspace/tp-2020-1c-heartbleed/game_boy/Debug
make all
mv game_boy gameboy




