#!/bin/sh

cd /home/utnso/workspace/tp-2020-1c-heartbleed/broker/Debug

mv broker.config broker1.config
mv broker1.config broker2.config
mv broker2.config broker.config

echo -e "Configs Switched"


