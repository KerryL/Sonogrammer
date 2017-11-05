# makefile (Sonogrammer)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = sonogrammer

# Directories in which to search for source files
DIRS = \
	src

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
VERSION_FILE = src/gitHash.cpp

# Object files
TEMP_OBJS = $(addprefix $(OBJDIR),$(SRC:.cpp=.o))
VERSION_FILE_OBJ = $(OBJDIR)$(VERSION_FILE:.cpp=.o)
OBJS = $(filter-out $(VERSION_FILE_OBJ),$(TEMP_OBJS))
ALL_OBJS = $(OBJS) $(VERSION_FILE_OBJ)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) version
	$(MKDIR) $(BINDIR)
	$(CC) $(ALL_OBJS) $(LDFLAGS) -L$(LIBOUTDIR) $(addprefix -l,$(PSLIB)) -o $(BINDIR)$@

version:
	./getGitHash.sh
	$(MKDIR) $(dir $(VERSION_FILE_OBJ))
	$(CC) $(CFLAGS) -c $(VERSION_FILE) -o $(VERSION_FILE_OBJ)

$(OBJDIR)%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(BINDIR)$(TARGET)
	$(RM) $(VERSION_FILE)
