################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bitarray.c \
../src/consola.c \
../src/iniciarFS.c \
../src/sindicato.c 

OBJS += \
./src/bitarray.o \
./src/consola.o \
./src/iniciarFS.o \
./src/sindicato.o 

C_DEPS += \
./src/bitarray.d \
./src/consola.d \
./src/iniciarFS.d \
./src/sindicato.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2020-2c-Los-Recursa2/shared" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


