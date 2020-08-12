#!/bin/sh

cd conexiones/Debug

make 

cd ../..

cd gameboy/Default
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../conexiones/Debug
make ./gameboy