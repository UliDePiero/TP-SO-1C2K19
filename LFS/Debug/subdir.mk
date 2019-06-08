################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../API_LFS.c \
../Compactador.c \
../FileSystem.c \
../Lissandra.c 

OBJS += \
./API_LFS.o \
./Compactador.o \
./FileSystem.o \
./Lissandra.o 

C_DEPS += \
./API_LFS.d \
./Compactador.d \
./FileSystem.d \
./Lissandra.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-GGWP/BiblioCompartida" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


