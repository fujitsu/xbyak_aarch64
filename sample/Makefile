SRC=add.cpp add2.cpp label.cpp direct_write.cpp dump.cpp
OBJ=$(SRC:.cpp=.o)
EXE=$(SRC:.cpp=.exe)
all: $(EXE)

LDFLAGS=-L ../lib -lxbyak_aarch64
CFLAGS=-g -DNDEBUG -Wall -Wextra -I ../xbyak_aarch64

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

%.exe: %.o
	$(CXX) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXE) $(OBJ)

.PHONY: clean

.SECONDARY: $(OBJ)
