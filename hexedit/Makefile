#!/usr/bin/make

# default configuration which may or may not work (tested on RedHat Linux 9.0):
FLTK_INCLUDE_PATH = -I/usr/local/include
FLTK_LIBRARY_PATH = -L/usr/local/lib
FLTK_LIBRARIES    = -lfltk
MY_CXXFLAGS = -O2 -Wall -Wunused -fno-exceptions
EXE =
POSTBUILD = echo

# now we can make modifications based on the operationg system and machine
# we run on
UNAME := $(shell uname) 
ifneq (,$(findstring SunOS,$(UNAME)))
  MY_CXXFLAGS     += -Wno-unknown-pragmas
  SYS_LIBRARY_PATH = -L/usr/openwin/lib
  SYS_LIBRARIES    = -lm -lXext -lX11 -lsupc++
endif
ifneq (,$(findstring Linux,$(UNAME)))
  SYS_LIBRARY_PATH = -L/usr/X11R6/lib
  SYS_LIBRARIES    = -lm -lXext -lX11 -lsupc++
endif
ifneq (,$(findstring CYGWIN,$(UNAME)))
  MY_CXXFLAGS     += -mwindows -DWIN32 -mno-cygwin
  SYS_LIBRARY_PATH = 
  SYS_LIBRARIES    = -lole32 -luuid -lcomctl32 -lwsock32 -lsupc++
  EXE              = .exe
endif
ifneq (,$(findstring Darwin,$(UNAME)))
  SYS_LIBRARY_PATH =
  SYS_LIBRARIES    = -framework Carbon -framework ApplicationServices -lsupc++
  POSTBUILD        = /Developer/Tools/Rez -t APPL -o $@ mac.r
endif

CXXFLAGS = $(FLTK_INCLUDE_PATH) $(MY_CXXFLAGS)
LDFLAGS  = $(FLTK_LIBRARY_PATH) $(SYS_LIBRARY_PATH)
LIBS     = $(FLTK_LIBRARIES) $(SYS_LIBRARIES)


ICONS = iconEmpty32.xpm iconSave24.xpm iconClose24.xpm iconNew24.xpm \
        iconSpyglass.xpm iconEmpty24.xpm iconOpen24.xpm

mickey$(EXE): hexEdit.cxx $(ICONS)
	echo $(TEST)
	g++ $(CXXFLAGS) hexEdit.cxx $(LDFLAGS) $(LIBS) -o $@
	$(POSTBUILD) 


