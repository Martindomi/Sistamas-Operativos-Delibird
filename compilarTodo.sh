#!/bin/sh

cd conexiones/Debug

make 

cd ../..

cd gameboy/Default
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2020-1c-Elite-Four/conexiones/Debug
make ./gameboy

cd ../..

cd broker/Default
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2020-1c-Elite-Four/conexiones/Debug
make ./broker

cd ../.. 

cd gamecard/Default
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2020-1c-Elite-Four/conexiones/Debug
make ./gamecard

cd ../..

cd team/Default
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2020-1c-Elite-Four/conexiones/Debug
make ./team

cd ../..