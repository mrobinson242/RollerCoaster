# Makefile 
# CSCI 420
# Assignment 2 - RollerCoaster

# we assume the pic directory locates one level above,
# change PIC_PATH if this is not the case
PIC_PATH = $(abspath $(CURDIR)/../pic)
HEADER_PATH = $(abspath $(CURDIR)/headers)

INCLUDE = -I$(PIC_PATH)
HEADER = -I$(HEADER_PATH)
LIBRARIES = -L$(PIC_PATH) -framework OpenGL -framework GLUT -lpicio -ljpeg -lm

COMPILER = g++
COMPILERFLAGS = -O3 $(INCLUDE) $(HEADER)

PROGRAM = RollerCoaster
SOURCE = RollerCoaster.cpp
OBJECT = RollerCoaster.o

.cpp.o: 
	$(COMPILER) -c $(COMPILERFLAGS) $<

all: $(PROGRAM)

$(PROGRAM): $(OBJECT) $(DEPS)
	$(COMPILER) $(COMPILERFLAGS) -o $(PROGRAM) $(OBJECT) $(LIBRARIES)

clean:
	-rm -rf core *.o *~ "#"*"#" $(PROGRAM)