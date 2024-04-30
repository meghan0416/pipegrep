CC = g++
CCFLAGS = -g -std=c++11
INCLUDES =
LIBRARIES = -lboost_system -lboost_thread -lpthread -lrt 
EXECUTABLES = pipegrep

pipegrep: pipegrep.o buffer.o
	$(CC) $(CCFLAGS) $(INCLUDES) -o pipegrep pipegrep.o buffer.o $(LIBRARIES)

buffer.o: buffer.h

# Rule for generating .o file from .cpp file
%.o: %.cpp
	$(CC) $(CCFLAGS) $(INCLUDES) -c $^ 

# All files to be generated
all: pipegrep

# Clean the directory
clean: 
	rm -f $(EXECUTABLES)  *.o *.gch
