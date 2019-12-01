SHELL=cmd
PATH=c:\msys64\mingw64\bin
FILENAMES=1.exe 2.exe
FILENAME=1.exe
SRCDIR=.
OBJDIR=.obj
ARGS=

PCH=pch
pch_NAME=precompiled.h
pch_OBJECT=$(OBJDIR)/$(pch_NAME).gch
pch_CPPFLAGS=-include $(OBJDIR)/$(pch_NAME)

CC=gcc
CXX=g++
CPPFLAGS=-D_UNICODE
CFLAGS=-mwindows -municode -march=native -mtune=native -mfpmath=sse -fno-ident -Wall -Werror -Werror=trampolines -pedantic-errors -O2 -ggdb
CXXFLAGS=-std=c++2a
LDFLAGS=
LDLIBS=

all: $(FILENAMES)
test: $(FILENAME)
	$(FILENAME) $(ARGS)
debug: $(OBJDIR)/$(FILENAME)
	gdb --quiet --batch -ex run -ex "bt full" -ex quit --args $(OBJDIR)/$(FILENAME) $(ARGS)
clean:
	-rd /s /q $(OBJDIR)
	-del $(FILENAMES)

.PHONY: all test debug clean

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $($(PCH)_OBJECT) | $(OBJDIR)
	$(CXX) -MMD $(CPPFLAGS) $($(PCH)_CPPFLAGS) $(CFLAGS) $(CXXFLAGS) $< -c -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $($(PCH)_OBJECT) | $(OBJDIR)
	$(CXX) -MMD $(CPPFLAGS) $($(PCH)_CPPFLAGS) $(CFLAGS) $(CXXFLAGS) -x c++ $< -c -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.rc | $(OBJDIR)
	$(CXX) -x c++ $< $(CPPFLAGS) -MM -MT $@ -MF $(@:%.o=%.d)
	windres $(CPPFLAGS) $(RCFLAGS) $< $@

$(OBJDIR)/%.gch: $(SRCDIR)/% | $(OBJDIR)
	$(CXX) -MMD $(CPPFLAGS) $(CFLAGS) $(CXXFLAGS) -x c++-header $< -c -o $@

$(OBJDIR)/%.exe: $(OBJDIR)/%.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.exe: $(OBJDIR)/%.exe
	strip $< -o $@

$(OBJDIR): ; -@md $(OBJDIR)

.SECONDARY: $(($(pch)_OBJECT)

-include $(OBJECTS:%.o=$(OBJDIR)/%.d) $($(PCH)_OBJECT:%.gch=%.d)
