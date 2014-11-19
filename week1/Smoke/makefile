# GNU (everywhere)
CPP = g++ -std=c++0x -g -Wall
LIBS        = -lglui -lglut -lGLU -lGL -lrfftw -lfftw -lm
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