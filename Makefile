
CXXFLAGS= -I. -std=c++11 -Wall
LFLAGS=

TARGETS= riscv-lut-compiler

OBJ=
all: make.incl $(TARGETS)

make.incl:
	./builddep.py > make.incl

include make.incl


riscv-lut-compiler: $(OBJ)
	$(CXX) $^ -o$@ $(LFLAGS)

clean:
	rm -f $(OBJ) $(TARGETS) make.incl

