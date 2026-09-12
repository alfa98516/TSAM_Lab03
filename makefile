all: puzzlesolver
CXXFLAGS = -Wpedantic -std=c++2b
puzzlesolver: main.cpp
	g++ ${CXXFLAGS} main.cpp -o $@

clean:
	rm -f puzzlesolver
