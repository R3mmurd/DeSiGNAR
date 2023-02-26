CXX = clang++

FLAGS = -std=c++20 -fPIE

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
SAMPLEDIR   = ./samples
TESTDIR     = ./tests
INCLUDES    = $(wildcard $(INCLUDEDIR)/*.H)
SOURCES     = $(wildcard $(SRCDIR)/*.C)
OBJECTS     = $(SOURCES:$(SRCDIR)/%.C=$(OBJDIR)/%.o)
SAMPLESRC   = $(wildcard $(SAMPLEDIR)/src/*.C)
SAMPLEBIN   = $(SAMPLESRC:$(SAMPLEDIR)/src/%.C=$(SAMPLEDIR)/bin/%)
TESTSRC     = $(wildcard $(TESTDIR)/src/*.C)
TESTBIN     = $(TESTSRC:$(TESTDIR)/src/%.C=$(TESTDIR)/bin/%)
LIBNAME     = Designar
LOCALLIB    = lib$(LIBNAME).a
INCLUDEPATH = -I$(INCLUDEDIR)
LIBLINK     = -L$(LIBDIR) -l$(LIBNAME) -lpthread

all: library tests samples

tests: library $(TESTBIN)

samples: library $(SAMPLEBIN)

library : $(INCLUDES) $(OBJECTS) 
	$(RM) $(LIBDIR)/$(LOCALLIB)
	$(AR) -cvq $(LIBDIR)/$(LOCALLIB) $(OBJECTS)

$(OBJDIR)/%.o: $(SRCDIR)/%.C
	$(CXX) $(FLAGS) $(OPT) $(INCLUDEPATH) -c $< -o $@

$(TESTDIR)/bin/%: $(TESTDIR)/src/%.C
	$(CXX) $(FLAGS) $(DEBUG) $(INCLUDEPATH) $< -o $@ $(LIBLINK)

$(SAMPLEDIR)/bin/%: $(SAMPLEDIR)/src/%.C
	$(CXX) $(FLAGS) $(DEBUG) $(INCLUDEPATH) $< -o $@ $(LIBLINK)

clean:
	$(RM) *~ $(INCLUDEDIR)/*~ $(SRCDIR)/*~ $(SAMPLEDIR)/src/*~ $(TESTDIR)/src/*~ $(OBJECTS) $(SAMPLEBIN) $(TESTBIN)
