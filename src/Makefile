INCDIR=../include
CXX=g++
CFLAGS=-I$(INCDIR) `mysql_config --cflags`
LFLAGS=`mysql_config --libs`
OBJDIR=../obj
TARGET=../bin/agilepc

LIBS=-lstdc++ 

DEPS = application.h database.h datamanager.h

_OBJ = application.o database.o datamanager.o main.o 
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	gcc -o ../bin/$@ $^ $(LFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
