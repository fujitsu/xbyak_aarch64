SRC=add.cpp add2.cpp label.cpp direct_write.cpp dump.cpp
OBJ=$(SRC:.cpp=.o)
EXE=$(SRC:.cpp=)
all: $(EXE)

LDFLAGS=-L ../lib -lxbyak_aarch64
CFLAGS=-std=c++11 -O3 -DNDEBUG -Wall -Wextra -I ../xbyak_aarch64

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

%: %.o
	$(CXX) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXE) $(OBJ)

.PHONY: clean

.SECONDARY: $(OBJ)
