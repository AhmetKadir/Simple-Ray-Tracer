CXX := g++
CXXFLAGS := -std=c++17 -I .

SRC := main.cpp tinyxml2.cpp  # Include tinyxml2.cpp here
OBJ := $(SRC:.cpp=.o)
EXE := main

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(EXE)
