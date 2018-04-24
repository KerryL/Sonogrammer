# makefile (sonogrammer)
#
# Include the common definitions
include makefile.inc

# Name of the executable to compile and link
TARGET = sonogrammer
TARGET_D = sonogrammerd

# Directories in which to search for source files
DIRS = \
	src

# Source files
SRC = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
VERSION_FILE = src/gitHash.cpp

# Object files
TEMP_OBJS_D = $(addprefix $(OBJDIR_DEBUG),$(SRC:.cpp=.o))
VERSION_FILE_OBJ_D = $(OBJDIR_DEBUG)$(VERSION_FILE:.cpp=.o)
OBJS_D = $(filter-out $(VERSION_FILE_OBJ_D),$(TEMP_OBJS_D))
ALL_OBJS_D = $(OBJS_D) $(VERSION_FILE_OBJ_D)
TEMP_OBJS = $(addprefix $(OBJDIR_RELEASE),$(SRC:.cpp=.o))
VERSION_FILE_OBJ = $(OBJDIR_RELEASE)$(VERSION_FILE:.cpp=.o)
OBJS = $(filter-out $(VERSION_FILE_OBJ),$(TEMP_OBJS))
ALL_OBJS = $(OBJS) $(VERSION_FILE_OBJ)

.PHONY: all clean debug version versiond

all: $(TARGET)

debug: $(TARGET_D)

$(TARGET): $(OBJS) version
	$(MKDIR) $(BINDIR)
	$(CC) $(ALL_OBJS) $(LDFLAGS) -L$(LIBOUTDIR) $(addprefix -l,$(PSLIB)) -o $(BINDIR)$@

version:
	./getGitHash.sh
	$(MKDIR) $(dir $(VERSION_FILE_OBJ))
	$(CC) $(CFLAGS_RELEASE) -c $(VERSION_FILE) -o $(VERSION_FILE_OBJ)

$(OBJDIR_RELEASE)%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

$(TARGET_D): $(OBJS_D) versiond
	$(MKDIR) $(BINDIR)
	$(CC) $(ALL_OBJS_D) $(LDFLAGS) -L$(LIBOUTDIR) $(addprefix -l,$(PSLIB)) -o $(BINDIR)$@

versiond:
	./getGitHash.sh
	$(MKDIR) $(dir $(VERSION_FILE_OBJ_D))
	$(CC) $(CFLAGS_DEBUG) -c $(VERSION_FILE) -o $(VERSION_FILE_OBJ_D)

$(OBJDIR_DEBUG)%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(BINDIR)$(TARGET)
	$(RM) $(VERSION_FILE)
