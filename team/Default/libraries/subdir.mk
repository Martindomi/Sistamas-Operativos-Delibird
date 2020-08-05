################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/entrenadores.c \
../libraries/planificacion.c \
../libraries/utils.c 

OBJS += \
./libraries/entrenadores.o \
./libraries/planificacion.o \
./libraries/utils.o 

C_DEPS += \
./libraries/entrenadores.d \
./libraries/planificacion.d \
./libraries/utils.d 


# Each subdirectory must supply rules for building sources it contributes
libraries/%.o: ../libraries/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2020-1c-Elite-Four/conexiones" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


