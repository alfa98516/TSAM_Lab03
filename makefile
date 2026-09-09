all: puzzlesolver
CXXFLAGS = -Wpedantic -std=c++2b
puzzlesolver: puzzlesolver.cpp
	g++ ${CXXFLAGS} puzzlesolver.cpp -o $@

clean:
	rm -f puzzlesolver
