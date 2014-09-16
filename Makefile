INCDIR=include
CXX=g++
CFLAGS=-I$(INCDIR) `mysql_config --cflags`
LFLAGS=`mysql_config --libs`
OBJDIR=obj
SRCDIR=src
TARGET=bin/agilepc

LIBS=-lstdc++ 

_DEPS = application.h database.h datamanager.h
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = application.o database.o datamanager.o main.o 
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	gcc -o $@ $^ $(LFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
