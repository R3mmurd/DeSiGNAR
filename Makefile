CXX = clang++

FLAGS = -std=c++14 -fPIE

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
INCLUDES    = $(wildcard $(INCLUDEDIR)/*.hpp)
SOURCES     = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS     = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
SAMPLESRC   = $(wildcard $(SAMPLEDIR)/src/*.cpp)
SAMPLEBIN   = $(SAMPLESRC:$(SAMPLEDIR)/src/%.cpp=$(SAMPLEDIR)/bin/%)
TESTSRC     = $(wildcard $(TESTDIR)/src/*.cpp)
TESTBIN     = $(TESTSRC:$(TESTDIR)/src/%.cpp=$(TESTDIR)/bin/%)
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

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(FLAGS) $(OPT) $(INCLUDEPATH) -c $< -o $@

$(TESTDIR)/bin/%: $(TESTDIR)/src/%.cpp
	$(CXX) $(FLAGS) $(DEBUG) $(INCLUDEPATH) $< -o $@ $(LIBLINK)

$(SAMPLEDIR)/bin/%: $(SAMPLEDIR)/src/%.cpp
	$(CXX) $(FLAGS) $(DEBUG) $(INCLUDEPATH) $< -o $@ $(LIBLINK)

clean:
	$(RM) *~ $(INCLUDEDIR)/*~ $(SRCDIR)/*~ $(SAMPLEDIR)/src/*~ $(TESTDIR)/src/*~ $(OBJECTS) $(SAMPLEBIN) $(TESTBIN)
