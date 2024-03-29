# makefile.inc (sonogrammer)
# This file contains all of the environment definitions
# common to each of the makefiles within the project. 
#

# Include directories that are not already on the path
# DO NOT include the -I prefix to these paths - it will
# be added automatically
INCDIRS_TEMP = \
	$(CURDIR)/src \

INCDIRS = $(addprefix -I,$(INCDIRS_TEMP))

# Library directories that are not already on the path
# DO NOT include the -L prefix to these paths - it will
# be added automatically
LIBDIRS_TEMP = \
	

LIBDIRS = $(addprefix -L,$(LIBDIRS_TEMP))

# Libraries to link against
# DO NOT include the -l prefix to these libraries - it
# will be added automatically
LIBS_TEMP = \
	

LIBS = $(addprefix -l,$(LIBS_TEMP))

# Static libraries to be build before the executable
# MUST be listed in order of dependence (i.e. first
# library must not be needed by other libraries and
# it must need information contained in the following
# libraries).
PSLIB = 

# Compiler to use
CC = g++

# Archiver to use
AR = ar rcs
RANLIB = ranlib

# Compiler flags
CFLAGS = -Wall -Wextra -Werror $(INCDIRS) `wx-config --cppflags` -std=c++14 `pkg-config --cflags libavformat libswresample libavcodec libavutil libswscale sdl2 x264 x265`
CFLAGS_DEBUG = $(CFLAGS) -g
CFLAGS_RELEASE = $(CFLAGS) -O2

# Linker flags
LDFLAGS = $(LIBDIRS) $(LIBS) `wx-config --libs` `pkg-config --libs libavformat libswresample libavcodec libavutil libswscale sdl2 x264 x265`

# Object file output directory
OBJDIR = $(PWD)/.obj/
OBJDIR_DEBUG = $(OBJDIR)debug/
OBJDIR_RELEASE = $(OBJDIR)release/

# Binary file output directory
BINDIR = $(PWD)/bin/

# Library output directory
LIBOUTDIR = $(PWD)/.lib/

# Method for creating directories
MKDIR = mkdir -p

# Method for removing files
RM = rm -f
