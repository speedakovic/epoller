################################################################################
# User configuration
################################################################################
-include config.mk


CC          ?= gcc
CXX         ?= g++


$(info CC=$(CC))
$(info CXX=$(CXX))
$(info SYSROOT_DIR=$(SYSROOT_DIR))
$(info DEBUG=$(DEBUG))




################################################################################
# Construct variables
################################################################################
ifdef SYSROOT_DIR
	SYSROOT_CFLAGS   = --sysroot=$(SYSROOT_DIR)
	SYSROOT_CXXFLAGS = --sysroot=$(SYSROOT_DIR)
	SYSROOT_LDFLAGS  = --sysroot=$(SYSROOT_DIR)
else
	SYSROOT_CFLAGS   =
	SYSROOT_CXXFLAGS =
	SYSROOT_LDFLAGS  =
endif


ifdef DEBUG
	DEBUG_CFLAGS   = -O0 -ggdb -DDEBUG
	DEBUG_CXXFLAGS = -O0 -ggdb -DDEBUG
else
	DEBUG_CFLAGS   = -O2 -DNDEBUG
	DEBUG_CXXFLAGS = -O2 -DNDEBUG
endif




################################################################################
# Output file
################################################################################
TARGET = epoller.so




################################################################################
# Source files
################################################################################
SRC_DIR = src
SRC_DIR_EPOLLER = $(SRC_DIR)/epoller
SRC_DIR_LINBUFF = $(SRC_DIR)/linbuff


CSRC =                       \
$(SRC_DIR_LINBUFF)/linbuff.c


CXXSRC =                           \
$(SRC_DIR_EPOLLER)/epoller.cpp     \
$(SRC_DIR_EPOLLER)/fdepoller.cpp   \
$(SRC_DIR_EPOLLER)/sigepoller.cpp  \
$(SRC_DIR_EPOLLER)/sockepoller.cpp \
$(SRC_DIR_EPOLLER)/timepoller.cpp  \
$(SRC_DIR_EPOLLER)/ttyepoller.cpp




################################################################################
# Object files
################################################################################
COBJ   = $(CSRC:.c=.o)
CXXOBJ = $(CXXSRC:.cpp=.o)




################################################################################
# Preprocessor flags
################################################################################
CPPFLAGS +=




################################################################################
# Compiler flags
################################################################################
CFLAGS   += -Wall -std=gnu89 -fPIC $(DEBUG_CFLAGX) $(SYSROOT_CFLAGS)
CXXFLAGS += -Wall -std=c++98 -fPIC $(DEBUG_CXXFLAGS) $(SYSROOT_CXXFLAGS)




################################################################################
# Linker flags
################################################################################
LDFLAGS += -shared $(SYSROOT_LDFLAGS)




################################################################################
# Paths to header files
################################################################################
INCPATHS = -I$(SRC_DIR)




################################################################################
# Paths to libraries
################################################################################
LIBPATHS =




################################################################################
# Libraries
################################################################################
LIBS =




################################################################################
# Targets
################################################################################
.PHONY: all clean

all: $(TARGET)

clean:
	rm -f *~ *.o $(TARGET)
	rm -f `find $(SRC_DIR) -name "*.o"`
	rm -f `find $(SRC_DIR) -name "*~"`

help:
	@echo "possible targets:"
	@echo "all   - build all binaries (default)"
	@echo "clean - delete all files created during build phase"

$(TARGET): $(COBJ) $(CXXOBJ)
	$(CXX) $(LDFLAGS) $(LIBPATHS) -o $@ $^ $(LIBS)

$(COBJ): %.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCPATHS) -o $@ -c $<

$(CXXOBJ): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCPATHS) -o $@ -c $<
