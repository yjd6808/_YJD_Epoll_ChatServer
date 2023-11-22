CPP := $(wildcard *.cpp)
OBJ := $(CPP:.cpp=.o)

main: $(OBJ)
	g++ -std=c++17 -g -Wall -o $@ $^ 

%.o: %.cpp
	g++ -g -Wall -o $@ -c $^

# find의 -o는 or을 의미한다.
clean:; find $(CURDIR) -name '*.o' -delete -o -name 'main' -delete
.PHONY: main clean


