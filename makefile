all: puzzlesolver

ifeq ($(UNAME_S),Darwin)
	# do the thing for macos whatever
else
	CXXFLAGS = -Wpedantic -std=c++23
	puzzlesolver: puzzlesolver.cpp
		g++ ${CXXFLAGS} puzzlesolver.cpp -o $@

	clean:
		rm -f puzzlesolver
endif
