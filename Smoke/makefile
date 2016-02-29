# GNU (everywhere)
# Debug
# CPP = g++ -std=c++11 -g -Wall
# Optimizing
CPP = g++ -std=c++11 -O3 -ffast-math -g -Wall
# Clang optimizing
# CPP = clang++ -std=c++11 -O3 -ffast-math -g -Wall
LIBS        = -lglui -lglut -lGLU -lGL -lsrfftw -lsfftw  -lm
EXECUTABLE = smoke

OBJS = fluids.o model.o visualization.o

### TARGETS

$(EXECUTABLE): $(OBJS)
	$(CPP) $(OBJS) $(LIBS) -o $@

depend: make.dep

clean:
	- /bin/rm -f  *.bak *~ $(OBJS) $(EXECUTABLE)
	
make.dep:
	g++ -MM $(OBJS:.o=.cpp) > make.dep

### RULES

.SUFFIXES: .cpp .o

.cpp.o:
	$(CPP) -c -o $@ $<

### DEPENDENCIES

include make.dep
