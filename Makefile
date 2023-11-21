CPP := $(wildcard *.cpp)
OBJ := $(CPP:.cpp=.o)

main: $(OBJ)
	g++ -std=c++17 -g -Wall -o $@ $^ 

%.o: %.cpp
	g++ -g -Wall -o $@ -c $^

clean:; find $(CURDIR) -name '*.o' -delete -o -name 'main' -delete
.PHONY: main clean

