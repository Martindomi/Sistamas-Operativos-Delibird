#!/bin/sh

cd ..
echo""
echo "----------------------------------"
echo "Comienzo a descargar commons..."
echo "----------------------------------"
echo""
git clone https://github.com/sisoputnfrba/so-commons-library
echo "Commons descargadas"
cd so-commons-library/
echo "Cominezo a instalar commons..."
make
sudo make install
echo""
echo "----------------------------------"
echo "Commons instaladas"
echo "----------------------------------"
echo""
cd ../tp-2020-1c-Elite-Four