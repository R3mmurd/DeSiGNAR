CXX = clang++ -std=c++14

AR = ar

WARN = -Wall -Wextra -Wcast-align -Wno-sign-compare -Wno-write-strings \
       -Wno-parentheses 

DEBUG = -g -O0 -DDEBUG $(WARN)

OPT = -Ofast -DNDEBUG $(WARN)

# comment this for release mode
FLAGS = $(DEBUG)
# uncomment this for release mode
# FLAGS = $(OPT)

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
BIN         = $(SAMPLES:$(SAMPLESDIR)/%.C=$(BINDIR)/%)
LIBNAME     = Designar
LOCALLIB    = lib$(LIBNAME).a
INCLUDEPATH = -I$(INCLUDEDIR)
LIBLINK     = -L$(LIBDIR) -l$(LIBNAME)

library : $(OBJECTS) 
	$(RM) $(LIBDIR)/$(LOCALLIB)
	$(AR) -cvq $(LIBDIR)/$(LOCALLIB) $(OBJECTS)

$(OBJDIR)/%.o : $(SRCDIR)/%.C
	$(CXX) $(DEBUG) $(INCLUDEPATH) -c $< -o $@

$(BINDIR)/%: $(SAMPLESDIR)/%.C
	$(CXX) $(DEBUG) $(INCLUDEPATH) $< -o $@ $(LIBLINK)

samples: library $(BIN)

clean:
	$(RM) *~ $(INCLUDEDIR)/*~ $(SRCDIR)/*~ $(SAMPLESDIR)/*~ $(OBJECTS) $(BIN)
