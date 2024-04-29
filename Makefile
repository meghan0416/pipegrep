CC = g++
CCFLAGS = -g -std=c++11
INCLUDES =
# LIBRARIES = -lboost_system -lboost_thread -lpthread -lrt 
LIBRARIES = -lpthread
EXECUTABLES = producer-consumer

producer-consumer: producer-consumer.o
	$(CC) $(CCFLAGS) $(INCLUDES) -o producer-consumer producer-consumer.o $(LIBRARIES)

# Rule for generating .o file from .cpp file
%.o: %.cpp
	$(CC) $(CCFLAGS) $(INCLUDES) -c $^ 

# All files to be generated
all: producer-consumer

# Clean the directory
clean: 
	rm -f $(EXECUTABLES)  *.o
