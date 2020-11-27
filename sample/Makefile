SRC=add.cpp add2.cpp label.cpp direct_write.cpp dump.cpp bf.cpp
OBJ=$(SRC:.cpp=.o)
EXE=$(SRC:.cpp=)
all: $(EXE)

LDFLAGS=-L ../lib -lxbyak_aarch64
CFLAGS=-g -DNDEBUG -Wall -Wextra -I ../
ifneq ($(DEBUG),1)
CFLAGS+=-O2
endif

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

%: %.o
	$(CXX) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXE) $(OBJ)

.PHONY: clean

.SECONDARY: $(OBJ)
