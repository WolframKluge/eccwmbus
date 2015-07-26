#for cross compiling call "make -f <file> CROSS=1

INC= -I .   -I ../include

ifeq "$(CROSS)" "1"
    CC     = arm-linux-gnueabihf-gcc
    CPP    = arm-linux-gnueabihf-g++
else
    CC     = gcc
    CPP    = g++
endif

all:	 eccwmbus

		
eccwmbus: 		eccwmbus.o wmbus.o 
				$(CC) -o eccwmbus eccwmbus.o wmbus.o -lpthread -ldl
				
eccwmbus.o:		../src/wmbus/eccwmbus.c ../include/wmbus/eccwmbus.h 
				$(CC) $(INC) -c ../src/wmbus/eccwmbus.c
							
wmbus.o:		../src/wmbus/wmbus.c
				$(CC) $(INC) -pthread -c ../src/wmbus/wmbus.c

clean: 			
				@rm -f eccwmbus eccwmbus.o wmbus.o
				@echo Clean done
