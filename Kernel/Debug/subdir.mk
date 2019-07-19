################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../API_Kernel.c \
../GossipingKernel.c \
../Planificador.c 

OBJS += \
./API_Kernel.o \
./GossipingKernel.o \
./Planificador.o 

C_DEPS += \
./API_Kernel.d \
./GossipingKernel.d \
./Planificador.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2019-1c-GGWP/BiblioCompartida" -I"/home/utnso/workspace/tp-2019-1c-GGWP/BiblioCompartida/Debug" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


