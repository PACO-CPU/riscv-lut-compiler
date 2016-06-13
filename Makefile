
CXXFLAGS= -I. -std=c++11 -Wall -g
LFLAGS=-llua -ldl

AUTOGENERATED_FILES=\
  segdata.h \
  lex.BaseWeights.cc \
  lex.BaseInput.cc \
  lex.BaseIntermediate.cc \
  lex.BaseBounds.cc

TARGETS= riscv-lut-compiler

OBJ=
all: make.incl $(TARGETS)

make.incl: $(AUTOGENERATED_FILES)
	./builddep.py > make.incl

%.l: %.li
	cat $^ | ./preparse-flex.py > $@

%.cc: %.l
	flex $^

include make.incl


riscv-lut-compiler: $(OBJ)
	$(CXX) $^ -o$@ $(LFLAGS)


segdata.h: segdata-incl.h
	./mksegdata.py


clean:
	rm -f $(OBJ) $(TARGETS) $(AUTOGENERATED_FILES) make.incl

doc:
	doxygen lut-compiler.doxygen
