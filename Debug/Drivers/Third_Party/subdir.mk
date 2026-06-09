################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Third_Party/HC_SR04_driver.c \
../Drivers/Third_Party/MCP2515_driver.c \
../Drivers/Third_Party/MPU6050_driver.c 

OBJS += \
./Drivers/Third_Party/HC_SR04_driver.o \
./Drivers/Third_Party/MCP2515_driver.o \
./Drivers/Third_Party/MPU6050_driver.o 

C_DEPS += \
./Drivers/Third_Party/HC_SR04_driver.d \
./Drivers/Third_Party/MCP2515_driver.d \
./Drivers/Third_Party/MPU6050_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Third_Party/%.o Drivers/Third_Party/%.su Drivers/Third_Party/%.cyclo: ../Drivers/Third_Party/%.c Drivers/Third_Party/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F412Rx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Third_Party

clean-Drivers-2f-Third_Party:
	-$(RM) ./Drivers/Third_Party/HC_SR04_driver.cyclo ./Drivers/Third_Party/HC_SR04_driver.d ./Drivers/Third_Party/HC_SR04_driver.o ./Drivers/Third_Party/HC_SR04_driver.su ./Drivers/Third_Party/MCP2515_driver.cyclo ./Drivers/Third_Party/MCP2515_driver.d ./Drivers/Third_Party/MCP2515_driver.o ./Drivers/Third_Party/MCP2515_driver.su ./Drivers/Third_Party/MPU6050_driver.cyclo ./Drivers/Third_Party/MPU6050_driver.d ./Drivers/Third_Party/MPU6050_driver.o ./Drivers/Third_Party/MPU6050_driver.su

.PHONY: clean-Drivers-2f-Third_Party

