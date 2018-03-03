CXX = clang++ -std=c++14

AR = ar

WARN = -Wall -Wextra -Wcast-align -Wno-sign-compare -Wno-write-strings \
       -Wno-parentheses -Wfloat-equal -pedantic

DEBUG = -g -O0 -DDEBUG $(WARN)

RELEASE = -Ofast -DNDEBUG $(WARN)

# comment next line to compile library in release mode
#OPT = $(DEBUG)
# uncomment next line to compile library in release mode
OPT = $(RELEASE)

INCLUDEDIR  = ./include
SRCDIR      = ./src
OBJDIR      = ./obj
LIBDIR      = ./lib
SAMPLESDIR  = ./samples
BINDIR      = ./bin
INCLUDES    = $(wildcard $(INCLUDEDIR)/*.H)
SOURCES     = $(wildcard $(SRCDIR)/*.C)
SAMPLES     = $(wildcard $(SAMPLESDIR)/*.C)
OBJECTS     = $(SOURCES:$(SRCDIR)/%.C=$(OBJDIR)/%.o)
BIN         = $(SAMPLES:$(SAMPLESDIR)/%.C=$(BINDIR)/%.out)
LIBNAME     = Designar
LOCALLIB    = lib$(LIBNAME).a
INCLUDEPATH = -I$(INCLUDEDIR)
LIBLINK     = -L$(LIBDIR) -l$(LIBNAME)

library : $(OBJECTS) 
	$(RM) $(LIBDIR)/$(LOCALLIB)
	$(AR) -cvq $(LIBDIR)/$(LOCALLIB) $(OBJECTS)

$(OBJDIR)/%.o : $(SRCDIR)/%.C
	$(CXX) $(OPT) $(INCLUDEPATH) -c $< -o $@

$(BINDIR)/%: $(SAMPLESDIR)/%.C
	$(CXX) $(DEBUG) $(INCLUDEPATH) $< -o $@ $(LIBLINK)

samples: library $(BIN)

clean:
	$(RM) *~ $(INCLUDEDIR)/*~ $(SRCDIR)/*~ $(SAMPLESDIR)/*~ $(OBJECTS) $(BIN)
