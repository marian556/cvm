################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/libcvm/compiler.cpp \
../src/libcvm/expression.cpp \
../src/libcvm/program.cpp \
../src/libcvm/statement.cpp \
../src/libcvm/vm.cpp 

OBJS += \
./src/libcvm/compiler.o \
./src/libcvm/expression.o \
./src/libcvm/program.o \
./src/libcvm/statement.o \
./src/libcvm/vm.o 

CPP_DEPS += \
./src/libcvm/compiler.d \
./src/libcvm/expression.d \
./src/libcvm/program.d \
./src/libcvm/statement.d \
./src/libcvm/vm.d 


# Each subdirectory must supply rules for building sources it contributes
src/libcvm/%.o: ../src/libcvm/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I/home/marian/boost_1_69_0 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


