#!/bin/sh

cd conexiones/Debug

make 

cd ../..

cd team/Default
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../conexiones/Debug
make ./team