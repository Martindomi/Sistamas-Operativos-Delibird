################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libconexiones.so

dependents:
	-cd /home/utnso/tp-2020-1c-Elite-Four/conexiones/Release && $(MAKE) all

# Tool invocations
libconexiones.so: $(OBJS) $(USER_OBJS) /home/utnso/tp-2020-1c-Elite-Four/conexiones/Release/libconexiones.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2020-1c-Elite-Four/conexiones/Release" -shared -o "libconexiones.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(OBJS)$(C_DEPS) libconexiones.so
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/tp-2020-1c-Elite-Four/conexiones/Release/libconexiones.so:

-include ../makefile.targets
