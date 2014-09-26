#Project-Specific Items
LIB := 
INC :=
TARGET := agilepc
CFLAGS := `mysql_config --cflags`
LFLAGS := `mysql_config --libs`
	
# Standard Structure Stuff
CC := g++
SRCDIR := src
BUILDDIR := build
BINDIR := bin
INC := -I include $(INC)
LIBDIR := lib

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g $(CFLAGS)
LFLAGS := $(LFLAGS)
TARGET := $(BINDIR)/$(TARGET)

$(TARGET) : $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LFLAGS) -L $(LIBDIR) $(LIB)"; $(CC) $^ -o $(TARGET) $(LFLAGS) -L $(LIBDIR) $(LIB) 

$(BUILDDIR)/%.o : $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

# Tests
tester:
	$(CC) $(CFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

# Spikes
spike:
	$(CC) $(CFLAGS) spikes/spike.cpp $(INC) $(LIB) -o bin/ticket

.PHONY: clean