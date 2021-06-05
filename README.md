# Delibird

Deilibir is a program wich simulets basic OS operations (planification, memory and filesystem instructions and functionalities)
Delibird es un programa que simula operaciones basicas de un sistema operativo (en lineas generales planificacion, memoria y file system).

## Content

There are 3 basic modules:
 - Team: simulates the processor
 - Broker: simulates the memory
 - GameCard: simulates the files system

They comunicate with each other with sockets simulating the bgehaviour of an OS.

The Gameboy modulo is used to send instructions, test each module separately and also al together!

For further information check de assigment "Delibird - v1.3.pdf"


## Tools

MAde in Ubutntu with C.
Realizado en Ubuntu con C.

## Exec

to compile u need to follow this steps:
1) Execute instalarCommons.sh 
2) Execute "compilarTodo.sh" if u want to try it in one computer or execute "compilar*.sh" files (* = Broke/Gameboy/Gamecard/Team) in diferent computers (NOTE: u have to change de .config files of each module with the IP and PORT of the computer).
3) Excecute each module or the one u want to test
4) Execute commands from a terminal with the Gameboy. (Wath commands??? check it in "Delibird - v1.3-pdf" !!!)
