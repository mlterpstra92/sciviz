# GNU (everywhere)
CPP = g++ -g -Wall

LIBS        = -lglut -lrfftw -lfftw -lGL -lGLU -lm

EXECUTABLE = smoke

OBJS = fluids.o simulation.o visualization.o

### TARGETS

$(EXECUTABLE): $(OBJS)
	$(CPP) $(OBJS) $(LIBS) -o $@

depend: make.dep

clean:
	- /bin/rm -f  *.bak *~ $(OBJS) $(EXECUTABLE)
	
make.dep:
	gcc -MM $(OBJS:.o=.cpp) > make.dep

### RULES

.SUFFIXES: .cpp .o

.cpp.o:
	$(CPP) -c -o $@ $<

### DEPENDENCIES

include make.dep
