#-----Macros---------------------------------
LIB = ../../lib/libJunior.a
INCLUDES = -I../../include

# set up compiler and options
CXX = g++
CXXFLAGS = -g $(INCLUDES)

#-----File Dependencies----------------------

SRC1 = sender.cpp
SRC2 = receiver.cpp

OBJ1 = $(addsuffix .o, $(basename $(SRC1)))
OBJ2 = $(addsuffix .o, $(basename $(SRC2)))

all: sender receiver
sender: $(OBJ1)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ1) -lm $(LIB)
	
receiver: $(OBJ2)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ2) -lm $(LIB)

#-----Other stuff----------------------------
clean:
	rm -f $(OBJ1)
	rm -f $(OBJ2)