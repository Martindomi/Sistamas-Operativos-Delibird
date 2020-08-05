#!/bin/sh

cd conexiones/Debug

make 

cd ../..

cd broker/Default
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../conexiones/Debug
make ./broker