#!/usr/bin/make

# default configuration which may or may not work (tested on RedHat Linux 9.0):
FLTK_INCLUDE_PATH = /usr/local/include
FLTK_LIBRARY_PATH = /usr/local/lib
FLTK_LIBRARIES = -lfltk

X11_LIBRARY_PATH = /usr/X11R6/lib
X11_LIBRARIES = -lm  -lXext -lX11

CXXFLAGS = -I $(FLTK_INCLUDE_PATH) 
LDFLAGS = -L $(FLTK_LIBRARY_PATH) -L $(X11_LIBRARY_PATH) 
LIBS = $(FLTK_LIBRARIES) $(X11_LIBRARIES)

# now we can make modifications based on the operationg system and machine we run on
UNAME = `uname`
ifeq ($(UNAME),"Linux")
  # don't do anything. Default setup is for Linux
endif

ICONS = iconEmpty32.xpm iconSave24.xpm iconClose24.xpm iconNew24.xpm iconSpyglass.xpm \
	iconEmpty24.xpm iconOpen24.xpm

mickey: hexEdit.cxx $(ICONS)
	g++ $(CXXFLAGS) hexEdit.cxx -o mickey $(LDFLAGS) $(LIBS)

