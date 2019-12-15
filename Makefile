SHELL=cmd
PATH=c:\msys64\mingw64\bin
FILENAMES=1.exe 2.exe 3.exe 4.exe 5.exe 6.exe 7.exe 8.exe 9.exe 10.exe 11.exe 12.exe 13.exe 14.exe
FILENAME=1.exe
SRCDIR=.
OBJDIR=.obj
OBJECTS=intcode.o d2d.o d2d_stuff.o
ARGS=

PCH=pch
pch_NAME=precompiled.h
pch_OBJECT=$(OBJDIR)/$(pch_NAME).gch
pch_CPPFLAGS=-include $(OBJDIR)/$(pch_NAME)

CC=gcc
CXX=g++
CPPFLAGS=-D_UNICODE
CFLAGS=-mwindows -municode -march=native -mtune=native -mfpmath=sse -fno-ident -Wall -Werror -Werror=trampolines -pedantic-errors -O0 -ggdb
CXXFLAGS=-std=c++2a
LDFLAGS=
LDLIBS=-lwindowscodecs -ldwrite -ld2d1 -lole32

all: $(FILENAMES)
test: $(FILENAME)
	$(FILENAME) $(ARGS)
debug: $(OBJDIR)/$(FILENAME)
	gdb --quiet --batch -ex run -ex "bt" -ex quit --args $(OBJDIR)/$(FILENAME) $(ARGS)
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

$(OBJDIR)/%.exe: $(OBJDIR)/%.o $(OBJECTS:%=$(OBJDIR)/%)
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.exe: $(OBJDIR)/%.exe
	strip $< -o $@

%.webm: %.exe
	-del ".obj\frame-*.png"
	$< $(ARGS)
	ffmpeg -nostats -hide_banner -loglevel warning -thread_queue_size 1024 -i ".obj/frame-%04d.png" -c:v libvpx-vp9 -deadline good -crf 31 -b:v 0 -vf format=yuv420p -an -y "$@"

$(OBJDIR): ; -@md $(OBJDIR)

.SECONDARY: $(($(pch)_OBJECT)

-include $(wildcard .obj/*.d) $($(PCH)_OBJECT:%.gch=%.d)
