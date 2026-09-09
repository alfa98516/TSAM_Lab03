all: puzzlesolver
CXXFLAGS = -Wpedantic -std=c++23
puzzlesolver: puzzlesolver.cpp
	g++ ${CXXFLAGS} puzzlesolver.cpp -o $@

clean:
	rm -f puzzlesolver
