// Copyright © 2003-2004 Matthias Melcher
// Copyright © 2019-2020 Neil McNeight

// ACTIVE:

// TODO:
// - ERROR: OS X Shift+Cmd menu shortcut doesn't work
// - statusbar
//    o show unicode, utf8, 64bit hex
//    o HeInput does not support 64bithex or signed decimal
//    o allow user input and replace bytes in document/jump to address
//    o show affected bytes for above input in hex display
//    o enter key should stay in same field, but advance cursor in doc
//    o allow clicking for relative and absolute cursor positioning
//    o LSB/MSB selector
//    o write protection indicator/button/dialog
// - make selection work like text editor (more than 1 selected shows no cursor)
// - triple choice on close dialog: save, don't save, cancel
// - don't allow 'backspace' and 'delete' in overwrite mode (or set 00)
// - find and replace dialog
// - Cycle Buttons should have shortcut
// - find syntax interpreter
// - wildcard find
// - error message on big and huge files
// - undo/redo!
// - handling of selection
//    o drag 'n drop for selected text
//    o double-click selects multiple bytes/block/struct
// - Preferences
//    o user settings
//    o previously opened file list
// - Ctrl-up/down should also do page up/down
// - builtin calculator
// - manage LSB and MSB files
// - settings for end-of-line character
// - command line arguments (file names, folders, patches, scripts)
// - support for big files (>1MB)
// - support for huge files (>physical RAM, GBytes)
// - partial redraw instead of full page redraw
// - ask if user is sure to overwrite a file
// - make previous/next line visible when scrolling
// - file and directory drag 'n drop
// - user marks (see VisualC F2)
// - user annotations with separate text column
// - annotated areas, structs, classes
// - file format description, scripting
// - tab key to change between hex and text editing
// - meaning of 'search' field could change between ASCII and HEX depending
//   on the active editing window... .
// - file type recognition
// - file analysing script
// - file analysing plugins
// - create and apply patch files
// - visual file diff
// - make editor into a widget/plugin
// - internationalisation
// - menu graying
// - toolbar
//    o insert/overwrite indicator/toggle
//    o help
// - if a second instance is opened, don't open it right on top!
// - toolbar editor

// DONE:
// - basic structure
// - basic UI
// - address, hex and ascii column
// - handle insertion with moving gap (small files)
// - basic selection handling
// - handle 'changed' flag (* indicator, ask before close)
// - font settings, resizing
// - save on application quit
// - error messages on file load and file save
// - PgDown over bound should set cursor to end
// - CRASH: handle zero byte files, handle cursor at 'size'
// - insert/overwrite mode
// - ERROR: can't change first byte in hex editor
// - handling of selection
//    o shift-move should extend selection
//    o delete should remove selection
//    o copy/paste selection
// - Preferences
//    o previous settings
// - toolbar
//    o new
//    o open
//    o save
//    o close
//    o search
// - compile on Linux, Windows, Max OS X
// - make cycle button automatically increment and cycle
// - statusbar
//    o show address, selection, selection size
//    o show byte, word, dword, float, double, ASCII

#ifdef __APPLE__
#define MM_OS "OS X"
#define MM_MENUSTYLE 0, FL_HELVETICA, MM_PROP_SIZE
#define MM_PROP_FONT " Osaka"
#define MM_PROP_SIZE 12
#define MM_PROP_SIZE_MED 11
#define MM_FIXED_FONT " Monaco"
#define MM_FIXED_SIZE 10
#define MM_CMD FL_META
#define UL
#elif (defined WIN32)
#define MM_OS "Win32"
#define MM_MENUSTYLE 0, FL_HELVETICA, MM_PROP_SIZE
#define MM_PROP_FONT " Arial"
#define MM_PROP_SIZE 12
#define MM_PROP_SIZE_MED 11
#define MM_FIXED_FONT " Fixedsys"
#define MM_FIXED_SIZE 10
#define MM_CMD FL_META
#define UL
#else
#define MM_OS "X11"
#define MM_MENUSTYLE 0, FL_HELVETICA, MM_PROP_SIZE
#define MM_PROP_FONT "-*-helvetica-medium-r-normal--*"
#define MM_PROP_SIZE 13
#define MM_PROP_SIZE_MED 12
#define MM_FIXED_FONT "-*-lucidatypewriter-medium-r-normal-sans-*"
#define MM_FIXED_SIZE 12
#define MM_CMD FL_CTRL
#define UL "&"
#endif

#define MM_VERSION UL"v0.8 for " MM_OS
#define MM_COPYRIGHT UL"Copyright © 2003-2004 Matthias Melcher\nCopyright © 2019-2020 Neil McNeight"
#define MM_APPNAME UL"mickey hex editor"
#define MM_WEB UL"http://www.github.com/McNeight/mickey/"

#include "hexEdit.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_draw.H>
#include <FL/Fl_message.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/filename.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_utf8.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <corecrt_io.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>

#include "iconEmpty24.xpm"
#include "iconNew24.xpm"
#include "iconOpen24.xpm"
#include "iconSave24.xpm"
#include "iconClose24.xpm"
#include "iconSpyglass.xpm"

HePreferences prefs;

int fixedFontHeight = MM_FIXED_SIZE;
int fixedFontAscent = 8;
int fixedFontWidth = 6;

char appname[] = MM_APPNAME;

//---- HeApp -------------------------------------------------------------------

HeApp::HeApp(int argc, char **argv) {
  if (prefs.winflags&1)
    window = new Fl_Double_Window(prefs.winx, prefs.winy,
                                  prefs.winw, prefs.winh, appname);
  else
    window = new Fl_Double_Window(prefs.winw, prefs.winh, appname);
  menubar = new HeMenubar(0, 0, window->w(), 10+prefs.propsize, this);
  toolbar = new HeToolbar(0, menubar->h(), window->w(), 30+prefs.propsize, this);
  doclist = new HeDocumentList
    ( 0, menubar->h()+toolbar->h(), window->w(),
      window->h()-menubar->h()-toolbar->h(), this);
  window->end();
  window->callback(closeAppWindowCB, this);
  window->resizable(doclist);
  //++ for testing only:
  //++ doclist->add("demo.o");
  for (int i=1;i<argc;) {
    if (Fl::arg(argc, argv, i)==0 && i<argc)
      doclist->add(argv[i++]);
  }
  window->show(argc, argv);
}

HeApp::~HeApp() {
  delete window; window = 0;
}

void HeApp::quitApplication() {
  while (document()) {
    if (!closeDocument())
      return;
  }
  if (window) {
    prefs.winflags = 1;
    prefs.winx = window->x();
    prefs.winy = window->y();
    prefs.winw = window->w();
    prefs.winh = window->h();
  }
  delete window; window = 0;
  //++ close all existing windows which will make FLTK leave the run() loop
}

void HeApp::closeAppWindowCB(Fl_Widget*, void *userdata) {
  HeApp *app = (HeApp*)userdata;
  app->quitApplication();
}

HeDocument *HeApp::document() {
  return (HeDocument*)doclist->value();
}

void HeApp::newDocument(const char *filename) {
  doclist->add(filename);
  doclist->value(doclist->child(doclist->children()-1));
  doclist->redraw();
}

char HeApp::closeDocument(HeDocument *doc) {
  if (!doc) doc = document();
  if (!doc) return true;
  if (doc->close()) {
    doclist->remove(doc);
    window->redraw();
    delete doc;
    return true;
  }
  return false;
}

//---- HeMenubar ---------------------------------------------------------------

HeApp *HeMenubar::app = 0;

Fl_Menu_Item HeMenubar::itemList[] = {
  { UL"File", 0, 0, 0, FL_SUBMENU, MM_MENUSTYLE },
  {   UL"New", MM_CMD+'n', newCB, 0, 0, MM_MENUSTYLE },
  {   UL"Open...", MM_CMD+'o', openCB, 0, FL_MENU_DIVIDER, MM_MENUSTYLE },
  {   UL"Save", MM_CMD+'s', saveCB, 0, 0, MM_MENUSTYLE },
  {   UL"Save &As...", FL_SHIFT+MM_CMD+'s', saveAsCB, 0, 0, MM_MENUSTYLE },
  {   UL"Close", MM_CMD+'w', closeCB, 0, FL_MENU_DIVIDER, MM_MENUSTYLE },
  {   UL"E&xit mickey", MM_CMD+'q', quitCB, 0, 0, MM_MENUSTYLE },
  {   0 },
  { UL"Edit", 0, 0, 0, FL_SUBMENU, MM_MENUSTYLE },
  {   UL"Undo", MM_CMD+'z', 0, 0, FL_MENU_INACTIVE, MM_MENUSTYLE },
  {   UL"Redo", FL_SHIFT+MM_CMD+'z', 0, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER,
    MM_MENUSTYLE },
  {   UL"C&ut", MM_CMD+'x', cutCB, 0, 0, MM_MENUSTYLE },
  {   UL"Copy", MM_CMD+'c', copyCB, 0, 0, MM_MENUSTYLE },
  {   UL"Paste", MM_CMD+'v', pasteCB, 0, 0, MM_MENUSTYLE },
  {   UL"Delete", 0, 0, 0, FL_MENU_INACTIVE, MM_MENUSTYLE },
  {   UL"Select &All", MM_CMD+'a', 0, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER,
    MM_MENUSTYLE },
  {   UL"Insert", MM_CMD+'i', insertModeCB, (void*)1, 0, MM_MENUSTYLE },
  {   UL"Overwrite", FL_SHIFT+MM_CMD+'i', insertModeCB, 0, 0, MM_MENUSTYLE },
  {   0 },
  { UL"Find", 0, 0, 0, FL_SUBMENU, MM_MENUSTYLE },
  {   UL"Find", MM_CMD+'f', 0, 0, FL_MENU_INACTIVE, MM_MENUSTYLE },
  {   UL"Find && &Replace", MM_CMD+'h', 0, 0, FL_MENU_INACTIVE, MM_MENUSTYLE },
  {   UL"Find &Next", MM_CMD+'g', 0, 0, FL_MENU_INACTIVE, MM_MENUSTYLE },
  {   0 },
  { UL"Help", 0, 0, 0, FL_SUBMENU, MM_MENUSTYLE },
  {   UL"About mickey...", 0, aboutCB, 0, 0, MM_MENUSTYLE },
  {   0 },
  { 0 }
};

HeMenubar::HeMenubar(int x, int y, int w, int h, HeApp *a)
: Fl_Group(x, y, w, h)
{
  app = a;
  menu = new Fl_Menu_Bar(x, y, w, h);
  menu->menu(itemList);
  end();
}

/// 'new' creates a new unnamed file with zero bytes in them
void HeMenubar::newCB(Fl_Widget*, void*) {
  app->newDocument();
}

void HeMenubar::openCB(Fl_Widget*, void*) {
  const char *defaultFilename = 0;
  if (app->document())
    defaultFilename = app->document()->filename();
  const char *filename = fl_file_chooser
    ( "Open File", 0, defaultFilename);
  if (filename)
    app->newDocument(filename);
}

void HeMenubar::saveCB(Fl_Widget*, void*) {
  if (!app->document()) return;
  app->document()->saveFile();
}

void HeMenubar::saveAsCB(Fl_Widget*, void*) {
  if (!app->document()) return;
  const char *filename = fl_file_chooser
  ( "Save File As", 0, app->document()->filename());
  if (filename)
    app->document()->saveFile(filename);
}

void HeMenubar::closeCB(Fl_Widget*, void*) {
  if (!app->document()) return;
  app->closeDocument();
}

void HeMenubar::quitCB(Fl_Widget*, void*) {
  app->quitApplication();
}

void HeMenubar::cutCB(Fl_Widget*, void*) {
  if (!app->document()) return;
  app->document()->manager()->cutToClipboard();
}

void HeMenubar::copyCB(Fl_Widget*, void*) {
  if (!app->document()) return;
  app->document()->manager()->copyToClipboard();
}

void HeMenubar::pasteCB(Fl_Widget*, void*) {
  if (!app->document()) return;
  app->document()->manager()->pasteFromClipboard();
}

void HeMenubar::insertModeCB(Fl_Widget*, void *userdata) {
  //++ should the insert mode by per application or per document?
  if (!app->document()) return;
  int i = (int)userdata;
  app->document()->manager()->insertMode(i);
}

void HeMenubar::aboutCB(Fl_Widget*, void*) {
  fl_message(UL"mickey " MM_VERSION"\n" MM_COPYRIGHT"\n\n"
             "a free cross platform hex editor\n\n"
             MM_WEB);
}

//---- HeTool ------------------------------------------------------------------

HeTool::HeTool(int x, int y, int w, int h)
: Fl_Group(x, y, w, h) { }

HeTool::HeTool(int x, int y, int w, int h, const char *label,
               const char *const*xpm)
: Fl_Group(x, y, w, h)
{
  Fl_Button *w1 = new Fl_Button(x, y, w, h-prefs.propsize-2);
  w1->image(new Fl_Pixmap(xpm));
  w1->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
  w1->box(FL_FLAT_BOX);
  w1->callback(parentDoCB, this);
  Fl_Button *w2 = new Fl_Button
    (x, y+h-prefs.propsize-2, w, prefs.propsize+2, label);
  w2->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
  w2->labelsize(prefs.propsize-1);
  w2->box(FL_FLAT_BOX);
  w2->callback(parentDoCB, this);
  end();
}

void HeTool::parentDoCB(Fl_Widget *w, void *user_data) {
  HeTool *t = (HeTool*)user_data;
  if (t->callback())
    t->do_callback();
}

//---- HeToolSearch ------------------------------------------------------------

HeToolSearch::HeToolSearch(int x, int y, int w, int h)
: HeTool(x, y, w, h) {
  buffer = 0;
  nBuffer = NBuffer = 0;
  // Create a group that will fram the spyglass and the text insert field
  Fl_Group *frame = new Fl_Group
    (x, y+3, w, prefs.fixedsize+11);
  frame->box(FL_PLASTIC_DOWN_BOX);
  frame->color(0xe0e0e000);
  // enter search text here
  Fl_Input *in = input = new Fl_Input
    (frame->x()+frame->h()-3, frame->y()+3, frame->w()-frame->h(), frame->h()-6);
  in->textsize(prefs.fixedsize);
  in->textfont(FL_COURIER);
  in->color(0xf0f0f000);
  in->box(FL_FLAT_BOX);
  in->callback(convertInputCB, this);
  in->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
  // spyglass
  Fl_Button *bt = new Fl_Button
    (frame->x()+3, frame->y()+3, frame->h()-6, frame->h()-6);
  bt->color(0xf0f0f000);
  bt->box(FL_FLAT_BOX);
  bt->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
  bt->image(new Fl_Pixmap(iconSpyglass_xpm));
  bt->callback(convertInputCB, this);
  frame->resizable(in);
  frame->end();
  // label this as the 'search' tool
  Fl_Button *w2 = new Fl_Button
    (x, y+h-prefs.propsize-2, w, prefs.propsize+2, "Search");
  w2->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
  w2->labelsize(prefs.propsize-1);
  w2->box(FL_FLAT_BOX);
  w2->callback(convertInputCB, this);
  end();
}

HeToolSearch::~HeToolSearch() {
  if (buffer) free(buffer);
}

void HeToolSearch::convertInput() {
  int n = strlen(input->value());
  if (n==0) {
    nBuffer = 0;
    return;
  }
  if (NBuffer<=n)
    buffer = (unsigned short*)realloc(buffer, (n+1)*sizeof(short));
  //++ simplified version just copies text over
  //++ we must write a little interpreter instead!
  const char *src = input->value();
  for (int i=0; i<=n; i++) {
    buffer[i] = src[i];
  }
  nBuffer = n;
}

void HeToolSearch::convertInputCB(Fl_Widget *w, void *user_data) {
  HeToolSearch *ts = (HeToolSearch*)user_data;
  ts->convertInput();
  ts->parentDoCB(w, user_data);
}

//---- HeToolbar ---------------------------------------------------------------

HeToolbar::HeToolbar(int x, int y, int w, int h, HeApp *a)
: Fl_Group(x, y, w, h)
{
  int xl = x+8, xr = w-8;
  app = a;
  HeTool *t;
  t = new HeTool(xl, y, 33, h, "New", iconNew24_xpm); xl += 36;
  t->callback(newCB, this);
  t = new HeTool(xl, y, 33, h, "Open", iconOpen24_xpm); xl += 36;
  t->callback(openCB, this);
  t = new HeTool(xl, y, 33, h, "Save", iconSave24_xpm); xl += 36;
  t->callback(saveCB, this);
  t = new HeTool(xl, y, 33, h, "Close", iconClose24_xpm); xl += 36;
  t->callback(closeCB, this);
  xr -= 36; t = new HeTool(xr+3, y, 33, h, "Help", iconEmpty24_xpm);
  t->callback(helpCB, this);
  t = new HeToolSearch(xl, y, xr-xl, h);
  t->callback(searchCB, this);
  resizable(t);
  end();
}

void HeToolbar::newCB(Fl_Widget*, void *t) {
  ((HeToolbar*)t)->app->newDocument();
}

void HeToolbar::openCB(Fl_Widget*, void *tu) {
  HeToolbar *t = (HeToolbar*)tu;
  const char *defaultFilename = 0;
  if (t->app->document())
    defaultFilename = t->app->document()->filename();
  const char *filename = fl_file_chooser
    ( "Open File", 0, defaultFilename);
  if (filename)
    t->app->newDocument(filename);
}

void HeToolbar::saveCB(Fl_Widget*, void *tu) {
  HeToolbar *t = (HeToolbar*)tu;
  if (!t->app->document()) return;
  t->app->document()->saveFile();
}

void HeToolbar::closeCB(Fl_Widget*, void *tu) {
  HeToolbar *t = (HeToolbar*)tu;
  if (!t->app->document()) return;
  t->app->closeDocument();
}

void HeToolbar::searchCB(Fl_Widget *ws, void *tu) {
  HeToolbar *t = (HeToolbar*)tu;
  if (!t->app->document()) return;
  HeToolSearch *ts = (HeToolSearch*)ws;
  t->app->document()->manager()->searchNext(ts->value(), ts->length());
}

void HeToolbar::helpCB(Fl_Widget*, void *t) {
  fl_message("No help available yet.");
}

//---- HeDocumentList ----------------------------------------------------------

HeDocumentList::HeDocumentList(int x, int y, int w, int h, HeApp *a)
: Fl_Tabs(x, y+5, w, h-5)
{
  app = a;
}

void HeDocumentList::add(const char *filename) {
  begin();
  HeDocument *doc = new HeDocument(x(), y()+18, w(), h()-18, app);
  resizable(doc);
  end();
  if (filename)
    doc->loadFile(filename);
  doc->layout();
  redraw();
}

//---- HeDocument --------------------------------------------------------------

HeDocument::HeDocument(int x, int y, int w, int h, HeApp *a)
: Fl_Group(x, y, w, h, "unnamed")
{
  labelfont(FL_HELVETICA);
  labelsize(MM_PROP_SIZE);
  app = a;
  filename_ = 0;
  shortname = 0;
  labelname = 0;
  size_ = 0;
  gap = 0;
  gapSize = 2048;
  buffer_ = (unsigned char*)malloc(size_+gapSize+2);
  file_ = -1;
  manager_ = new HeDocumentManager(x, y, w, h, this);
  end();
  resizable(manager_);
}

HeDocument::~HeDocument() {
  if (filename_)
    free(filename_);
  if (shortname)
    free(shortname);
  if (labelname)
    free(labelname);
  if (buffer_)
    free(buffer_);
  if (file_!=-1)
    ::_close(file_);
}

void HeDocument::filename(const char *name) {
  char buffer[2048];
  if (!name) return;
  fl_filename_absolute(buffer, 2047, name);
  if (filename_) free(filename_);
  filename_ = _strdup(buffer);
  if (shortname) free(shortname);
  shortname = _strdup(fl_filename_name(name));
  int n = strlen(shortname);
  if (labelname) free(labelname);
  labelname = (char*)malloc(n+3);
  strcpy(labelname, shortname);
  strcat(labelname, " *");
  if (!changed())
    labelname[n] = 0;
  label(labelname);
  tooltip(filename_);
}

void HeDocument::loadFile(const char *name) {
  if (!name) return;
  size_t n = 0;
  filename(name);
  size_ = 0;
  file_ = _open(filename(), O_RDONLY, 0644);
  if (file_==-1) {
    fl_alert("Can't open file \n\"%s\"\nfor reading.\n%s.",
             filename(), strerror(errno));
    goto cleanReturn;
  }
  struct stat st;
  if (fstat(file_, &st)==-1) {
    fl_alert("Can't find size of file \n\"%s\".\n%s.\n"
             "Assuming empty file.",
             filename(), strerror(errno));
    goto cleanReturn;
  }
  size_ = st.st_size;
  if (size_==0) return;
  if (buffer_)
    free(buffer_);
  gap = size_; gapSize = 2048;
  buffer_ = (unsigned char*)malloc(size_+gapSize+2);
  n = _read(file_, buffer_, size_);
  if (n==(size_t)-1) {
    fl_alert("Can't read contents of file \n\"%s\".\n%s.\n"
             "Assuming empty file.",
             filename(), strerror(errno));
    size_ = 0;
    free(buffer_); buffer_ = 0;
    goto cleanReturn;
  } else if (n<(size_t)size_) {
    fl_alert("File \n\"%s\"\ntruncated while reading."
             "Editing file is not recommended.",
             filename());
    size_ = n;
  }
cleanReturn:
  if (file_!=-1)
    ::_close(file_);
  clearChanged();
  manager()->update();
}

void HeDocument::saveFile(const char *name) {
  if (name)
    filename(name);
  else
    name = filename();
  int out = _open(filename(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
  if (out==-1) {
    fl_alert("Can't open file \n\"%s\"\nfor writing.\n%s.",
             filename(), strerror(errno));
    return;
  }
  size_t n1=0, n2=0;
  n1 = _write(out, buffer_, gap);
  if (n1!=(size_t)-1) n2 = _write(out, buffer_+gap+gapSize, size_-gap);
  if (n1==(size_t)-1 || n2==(size_t)-1) {
    fl_alert("Can't write document to file \n\"%s\".\n%s.\n"
             "File will be truncated.",
             filename(), strerror(errno));
  } else if (n1+n2<(size_t)size_) {
    fl_alert("File \"%s\"\ntruncated while writing!",
             filename());
  }
  ::_close(out);
  clearChanged();
}

char HeDocument::close() {
  if (!changed())
    return true;
  if (fl_ask("Your document \"%s\" was changed, but not saved.\n"
             "Do you want to close this document and discard all changes?",
             shortname))
    return true;
  return false;
}

void HeDocument::layout() {
  manager()->layout();
}

heIndex HeDocument::size() {
  return size_;
}

unsigned char HeDocument::byteAt(heIndex i) {
  if (i>size_ || !buffer_) return 0x00;
  if (i>=gap) i+=gapSize;
  return buffer_[i];
}

void HeDocument::byteAt(heIndex i, unsigned char c) {
  if (i>size_ || !buffer_) return;
  if (i>=gap) i+=gapSize;
  buffer_[i] = c;
  if (!changed_) setChanged();
}

unsigned char *HeDocument::blockAt(heIndex start, heIndex n) {
  if (gap<=start) return buffer_+gapSize+start;
  if (gap>=start+n) return buffer_+start;
  moveGapTo(start);
  return buffer_+gapSize+start;
}

void HeDocument::moveGapTo(heIndex pos) {
  if (gap==pos) return;
  if (gapSize) {
    if (pos<gap) {
      heIndex n = gap-pos;
      memmove(buffer_+gap+gapSize-n, buffer_+pos, n);
    } else {
      heIndex n = pos-gap;
      memmove(buffer_+gap, buffer_+gap+gapSize, n);
    }
  }
  gap = pos;
}

void HeDocument::addToGap(heIndex n) {
  unsigned char *b2 = (unsigned char*)malloc(size_+gapSize+n);
  memcpy(b2, buffer_, gap);
  memcpy(b2+gap+gapSize+n, buffer_+gap+gapSize, size_-gap);
  free(buffer_);
  buffer_ = b2;
  gapSize += n;
}

void HeDocument::deleteBytes(heIndex first, heIndex n) {
  if (first+n>size_)
    n = size_-first;
  moveGapTo(first);
  gapSize+=n;
  size_-=n;
  if (!changed_) setChanged();
  redraw();
}

void HeDocument::insertBytes(heIndex first, heIndex n) {
  moveGapTo(first);
  if (gapSize<n)
    addToGap(n+16384);
  gap+=n;
  gapSize-=n;
  size_+=n;
  if (!changed_) setChanged();
  redraw();
}

void HeDocument::setChanged() {
  if (changed_) return;
  changed_ = true;
  int n = strlen(shortname);
  labelname[n] = ' ';
  label(labelname);
  redraw();
}

void HeDocument::clearChanged() {
  if (!changed_) return;
  changed_ = false;
  int n = strlen(shortname);
  labelname[n] = 0;
  label(labelname);
  redraw();
}

//---- HeDocumentManager ---------------------------------------------------------

HeDocumentManager::HeDocumentManager(int x, int y, int w, int h, HeDocument *d)
: Fl_Group(x, y, w, h)
{
  doc = d;
  cursor_ = 1;
  selection_ = 1;
  insertMode_ = 0;
  int sbh = 3*fontHeight()+12;
  status = new HeStatusBar(x+2, y+2, w-4, sbh, this);
  column = new HeColumnGroup(x+2, y+sbh, w-4, h-sbh, this);
  resizable(column);
  end();
  cursor(0);
}

void HeDocumentManager::layout() {
  column->layout();
}

void HeDocumentManager::update() {
  column->layout();
  status->update();
  redraw();
}

void HeDocumentManager::setFont() {
  fl_font(FL_COURIER, MM_FIXED_SIZE);
}

int HeDocumentManager::fontHeight() {
  return fixedFontHeight;
}

int HeDocumentManager::fontAscent() {
  return fixedFontAscent;
}

int HeDocumentManager::fontWidth() {
  return fixedFontWidth;
}

int HeDocumentManager::spaceWidth() {
  return fontWidth()/3;
}

int HeDocumentManager::attributeAt(heIndex ix) {
  int ret = 0;
  if (ix==cursor_)
    ret |= HE_CURSOR;
  if (selection_>cursor_) {
    if (ix>=cursor_ && ix<=selection_) ret |= HE_SELECTED;
  } else if (selection_<cursor_) {
    if (ix<=cursor_ && ix>=selection_) ret |= HE_SELECTED;
  }
  if (ix<0 || ix>=doc->size())
    ret |= HE_OUT_OF_BOUNDS;
  return ret;
}

void HeDocumentManager::select(heIndex a, heIndex b, bool toggle) {
  //++ untested
  //++ toggle support missing
  selection_ = a;
  cursor(b, true);
  /*
  if (toggle && a==selection_ && b==cursor_) {
    a = cursor_;
  }
  if (a>=doc->size()) a = doc->size();
  if (b>doc->size()) b = doc->size();
  if (selection_ != a || cursor_ != b) {
    selection_ = a;
    cursor(b, true);
    redraw();
  }
  */
}

void HeDocumentManager::insertMode(char m) {
  insertMode_ = m;
  status->updateFlags();
  redraw();
}

void HeDocumentManager::extendSelection(heIndex a, heIndex b) {
  /* //++
  if (a>selectionFirst_) a = selectionFirst_;
  if (a>=doc->size()) a = doc->size();
  if (b<selectionEnd_) b = selectionEnd_;
  if (b>=doc->size()) b = doc->size();
  if (selectionFirst_ != a || selectionEnd_ != b) {
    selectionFirst_ = a;
    selectionEnd_ = b;
    redraw();
  }
  */
}

void HeDocumentManager::cursor(heIndex c, bool extend) {
  if (c==cursor_ && (c==selection_ || !extend))
    return;
  if (c>doc->size()) c = doc->size();
  //++ we should make the previous line visible, too
  //++ make the stuff below a function of the ColumnGroup (cursorVisible)
  if (c<column->topLeftByte())
    column->topByte(c);
  //++ we should make the next line visible, too
  if (c>column->topLeftByte()+column->bytesPerPage()-column->bytesPerRow())
    column->topByte(c-column->bytesPerPage()+column->bytesPerRow());
  if (c==cursor_ && (c==selection_ || !extend))
    return;
  cursor_ = c;
  if (!extend) selection_ = c;
  status->cursor(selection_, cursor_);
  redraw();
}

void HeDocumentManager::deleteSelection() {
  if (selection_>cursor_) {
    doc->deleteBytes(cursor_, selection_-cursor_+1);
    cursor(cursor_);
  } else if (selection_<cursor_) {
    doc->deleteBytes(selection_, cursor_-selection_+1);
    cursor(selection_);
  }
}

void HeDocumentManager::insert(const char *text, heIndex len) {
  bool ins = insertMode() || cursor_==doc->size();
  if (selection_!=cursor_) {
    deleteSelection();
    ins = true;
  }
  heIndex dst = cursor_;
  if (ins)
    doc->insertBytes(dst, len);
  for (heIndex i=0; i<len; i++) {
    doc->byteAt(dst++, (unsigned char)text[i]);
  }
  cursor(dst);
}

void HeDocumentManager::cutToClipboard() {
  copyToClipboard();
  deleteSelection();
}

void HeDocumentManager::copyToClipboard() {
  if (selection_>cursor_) {
    int n = selection_-cursor_+1;
    char *src = (char*)doc->blockAt(cursor_, n);
    Fl::copy(src, n, 1);
  } else {
    int n = cursor_-selection_+1;
    char *src = (char*)doc->blockAt(selection_, n);
    Fl::copy(src, n, 1);
  }
}

void HeDocumentManager::pasteFromClipboard() {
  Fl::paste(*this, 1);
}

bool HeDocumentManager::searchNext(const unsigned short *txt, int len) {
  //++ simple search
  heIndex pos = selection_ + 1, size = doc->size();
  int i;
  for (;pos<size;pos++) {
    for (i=0; i<len; i++) {
      unsigned char b = doc->byteAt(pos+i);
      if (txt[i]!=b) break; //++ handle modifier flags in txt
    }
    if (i==len) {
      select(pos, pos+len-1, false);
      return true;
    }
  }
  //++ continue search at beginning of file
  return false;
}

//---- HeStatusBar -------------------------------------------------------------

HeStatusBar::HeStatusBar(int x, int y, int w, int h, HeDocumentManager *m)
: Fl_Group(x, y, w, h)
{
  byteOrder_ = 0;
  manager = m;
  doc = manager->document();
  int fw = m->fontWidth(), fh = m->fontHeight();
  int ow = 3;
  int wlb = 40, wsb = 20, w10 = 10*fw+ow;
  //-- top line: address and selection
  adrb = new HeCycleButton(x+2, y+1, 27, fh+1, 3, "hex", "oct", "dec");
  adrb->callback(cycleAddressBaseCB, this);
  crsr = new HeInput(        x+wlb+30,  y+2, w10, fh, "cursor:", 16, 10);
  slct = new HeInput(crsr->x()+wlb+w10, y+2, w10, fh, "select:", 16, 10);
  offs = new HeInput(slct->x()+wlb+w10, y+2, w10, fh, "offset:", 16, 10);
  //-- second line: integer data
  dtab = new HeCycleButton(x+2, y+fh+3, 27, fh+1,
                           4, "hex", "bin", "oct", "dec");
  dtab->callback(cycleDataBaseCB, this);
  int gx = x+30, gy = y+fh+4, gw = w-61;
  hexGrp = new Fl_Group(gx, gy, gw, fh);
  {
    d1x = new HeInput(gx+wlb, gy, 2*fw+ow, fh, "data 1:", 16, 2);
    d2x = new HeInput(d1x->x()+d1x->w()+wsb, gy, 4*fw+ow, fh, "2:", 16, 4);
    d4x = new HeInput(d2x->x()+d2x->w()+wsb, gy, 8*fw+ow, fh, "4:", 16, 8);
  }
  end();
  binGrp = new Fl_Group(gx, gy, gw, fh);
  {
    d1b = new HeInput(gx+wlb, gy, 8*fw+ow, fh, "data 1:", 2, 8);
    d2b = new HeInput(d1b->x()+d1b->w()+wsb, gy, 16*fw+ow, fh, "2:", 2, 16);
  }
  binGrp->end();
  binGrp->hide();
  octGrp = new Fl_Group(gx, gy, gw, fh);
  {
    d1o = new HeInput(gx+wlb, gy, 4*fw+ow, fh, "data 1:", 8);
    d2o = new HeInput(d1o->x()+d1o->w()+wsb, gy, 7*fw+ow, fh, "2:", 8);
    d4o = new HeInput(d2o->x()+d2o->w()+wsb, gy, 12*fw+ow, fh, "4:", 8);
  }
  octGrp->end();
  octGrp->hide();
  decGrp = new Fl_Group(gx, gy, gw, fh);
  {
    d1d = new HeInput(gx+wlb, gy, 3*fw+ow, fh, "data 1:", 10);
    d2d = new HeInput(d1d->x()+d1d->w()+wsb, gy, 5*fw+ow, fh, "2:", 10);
    d4d = new HeInput(d2d->x()+d2d->w()+wsb, gy, 10*fw+ow, fh, "4:", 10);
  }
  decGrp->end();
  decGrp->hide();
  //-- third line: text and fp data
  // char, utf8, unicode, float, double
  gy =  y+2*fh+6;
  d1c = new HeInput(gx+wlb, gy, 3*fw+ow, fh, "char:", 0);
  //++ half float: d2f = new HeInput(x+wlb+30, y+fh*2+6, fw+ow, fh, "text: c");
  d4f = new HeInput(d1c->x()+d1c->w()+wlb, gy, 13*fw+ow, fh, "float:", 100);
  d8f = new HeInput(d4f->x()+d4f->w()+wlb, gy, 13*fw+ow, fh, "double:", 101);

  // flags: (RO,R/W) (INS/OVR) (LSB/MSB)
  blsb = new HeCycleButton(x+w-30, y+1, 27, fh+1, 2, "LSB", "MSB");
  blsb->callback(lsbMsbModeCB, this);
  brwm = new HeCycleButton(x+w-30, y+fh+3, 27, fh+1, 2, "R/W", "RO");
  brwm->warnMask(2);
  brwm->callback(readOnlyModeCB, this);
  brwm->deactivate();
  bins = new HeCycleButton(x+w-30, y+2*fh+5, 27, fh+1, 2, "OVR", "INS");
  bins->warnMask(2);
  bins->callback(insertModeCB, this);

  box(FL_FLAT_BOX);
  end();
  Fl_Widget *rw = new Fl_Box(x+w-31, y, 1, h);
  rw->hide();
  resizable(rw);
}

void HeStatusBar::cursor(heIndex a, heIndex b) {
  crsr->value(b);
  slct->value(a);
  if (a>b) offs->value(a-b);
  else offs->value(b-a);
  updateData();
}

void HeStatusBar::updateData() {
  heIndex ix = manager->cursor();
  //++ fill these more elegant using a 'union'
  int i;
  union { unsigned char u[8]; float f; double d; } ff;
  for (i=0; i<8; i++) ff.u[i] = doc->byteAt(ix+i);
  heIndex v0 = doc->byteAt(ix);
  heIndex v1 = doc->byteAt(ix+1);
  heIndex v2 = doc->byteAt(ix+2);
  heIndex v3 = doc->byteAt(ix+3);
  if (hexGrp->visible()) {
    d1x->value(v0);
    if (byteOrder_) {
      d2x->value(v1+(v0<<8));
      d4x->value(v3+(v2<<8)+(v1<<16)+(v0<<24));
    } else {
      d2x->value(v0+(v1<<8));
      d4x->value(v0+(v1<<8)+(v2<<16)+(v3<<24));
    }
  } else if (binGrp->visible()) {
    d1b->value(v0);
    if (byteOrder_) {
      d2b->value(v1+(v0<<8));
    } else {
      d2b->value(v0+(v1<<8));
    }
  } else if (octGrp->visible()) {
    d1o->value(v0);
    if (byteOrder_) {
      d2o->value(v1+(v0<<8));
      d4o->value(v3+(v2<<8)+(v1<<16)+(v0<<24));
    } else {
      d2o->value(v0+(v1<<8));
      d4o->value(v0+(v1<<8)+(v2<<16)+(v3<<24));
    }
  } else if (decGrp->visible()) {
    d1d->value(v0);
    if (byteOrder_) {
      d2d->value(v1+(v0<<8));
      d4d->value(v3+(v2<<8)+(v1<<16)+(v0<<24));
    } else {
      d2d->value(v0+(v1<<8));
      d4d->value(v0+(v1<<8)+(v2<<16)+(v3<<24));
    }
  }
  d1c->value(v0);
  d4f->value(ff.f);
  d8f->value(ff.d);
}

void HeStatusBar::updateFlags() {
  bins->value(manager->insertMode());
}

void HeStatusBar::cycleAddressBaseCB(Fl_Widget*, void *user_data) {
  static int lu[] = {16, 8, -10};
  HeStatusBar *sb = (HeStatusBar*)user_data;
  sb->adrb->incr();
  int bb = lu[sb->adrb->value()];
  sb->crsr->base(bb);
  sb->slct->base(bb);
  sb->offs->base(bb);
}

void HeStatusBar::cycleDataBaseCB(Fl_Widget*, void *user_data) {
  HeStatusBar *sb = (HeStatusBar*)user_data;
  sb->dtab->incr();
  switch (sb->dtab->value()) {
    case 0: sb->decGrp->hide(); sb->hexGrp->show(); break;
    case 1: sb->hexGrp->hide(); sb->binGrp->show(); break;
    case 2: sb->binGrp->hide(); sb->octGrp->show(); break;
    case 3: sb->octGrp->hide(); sb->decGrp->show(); break;
  }
  sb->updateData();
}

void HeStatusBar::lsbMsbModeCB(Fl_Widget*, void *user_data) {
  HeStatusBar *sb = (HeStatusBar*)user_data;
  sb->blsb->incr();
  sb->byteOrder(sb->blsb->value());
}

void HeStatusBar::readOnlyModeCB(Fl_Widget*, void *user_data) {
  HeStatusBar *sb = (HeStatusBar*)user_data;
  //++ display read/write mode. May offer to change ReadOnly mode...
  sb->brwm->incr();
}

void HeStatusBar::insertModeCB(Fl_Widget*, void *user_data) {
  HeStatusBar *sb = (HeStatusBar*)user_data;
  sb->bins->incr();
  sb->manager->insertMode(sb->bins->value());
}

//---- HeColumnGroup -----------------------------------------------------------

HeColumnGroup::HeColumnGroup(int x, int y, int w, int h, HeDocumentManager *m)
: Fl_Group(x, y, w, h)
{
  end();
  doc = m->document();
  mgr = m;
  scroll = 0;
  bytesPerRow_ = 8;
  rows_ = 20;
  rowsPerPage_ = 10;
  topByte_ = 0;
  createStandardColumns();
}


void HeColumnGroup::createStandardColumns() {
  begin();
  new HeAddrColumn(x(), y(), 95, h(), mgr);
  new HeSeperatorColumn(x()+95, y(), 5, h(), mgr);
  new HeHexColumn(x()+100, y(), 195, h(), mgr);
  new HeSeperatorColumn(x()+295, y(), 5, h(), mgr);
  new HeTextColumn(x()+300, y(), 160, h(), mgr);
  scroll = new HeScrollbarColumn(x()+w()-14, y(), 14, h(), mgr);
  end();
  resize(x(), y(), w(), h());
}

void HeColumnGroup::layout() {
  resize(x(), y(), w(), h());
}

void HeColumnGroup::resize(int wx, int wy, int ww, int wh) {
  int i, wfixed = 0, wflex = 0;
  for (i=0; i<children(); i++) {
    HeColumn *ci = (HeColumn*)child(i);
    ci->getWidth(wfixed, wflex);
  }
  bytesPerRow_ = (w()-wfixed)/wflex;
  rows_ = doc->size()/bytesPerRow_ + 1;
  rowsPerPage_ = wh / mgr->fontHeight();
  topByte(topByte_);
  Fl_Widget::resize(wx, wy, ww, wh);
  for (i=0; i<children(); i++) {
    wfixed = wflex = 0;
    HeColumn *ci = (HeColumn*)child(i);
    ci->getWidth(wfixed, wflex);
    int wdt = wfixed+bytesPerRow_*wflex;
    if (i==children()-1)
      wx = w()-wdt;
    ci->resize(wx, wy, wdt, wh);
    wx += wdt;
    ci->layout();
  }
}

void HeColumnGroup::topByte(heIndex v) {
  topByte_ = v;
  topLeftByte_ = v - (v % bytesPerRow());
  scroll->value(topRow());
  redraw();
}

void HeColumnGroup::topRow(heIndex v) {
  topLeftByte_ = topByte_ = v*bytesPerRow();
  scroll->value(topRow());
  redraw();
}

int HeColumnGroup::handle(int event) {
  switch (event) {
    case FL_KEYBOARD: {
      bool xt = ((Fl::event_state()&FL_SHIFT)!=0);
      switch (Fl::event_key()) {
        case FL_Up:
          if (cursor()>=(heIndex)bytesPerRow_)
            cursor(cursor()-bytesPerRow_, xt);
          return 1;
        case FL_Down:
          if (cursor()<=doc->size()-bytesPerRow_)
            cursor(cursor()+bytesPerRow_, xt);
          return 1;
        case FL_Left:
          if (cursor()>0) cursor(cursor()-1, xt);
          return 1;
        case FL_Right:
          if (cursor()<doc->size()) cursor(cursor()+1, xt);
          return 1;
        case FL_Page_Up:
          if (cursor()>=(heIndex)bytesPerPage())
            cursor(cursor()-bytesPerPage(), xt);
          else
            cursor(cursor()%bytesPerRow(), xt);
          return 1;
        case FL_Page_Down:
          if (cursor()<=doc->size()-bytesPerPage())
            cursor(cursor()+bytesPerPage(), xt);
          else {
            heIndex c = cursor()%bytesPerRow();
            heIndex d = doc->size()%bytesPerRow();
            cursor(doc->size()-d+c-(c>d?bytesPerRow():0), xt); }
            return 1;
        case FL_Home:
          if (Fl::event_state()&MM_CMD)
            cursor(0, xt);
          else
            cursor(cursor()-cursor()%bytesPerRow_, xt);
          return 1;
        case FL_End:
          if (Fl::event_state()&MM_CMD)
            cursor(doc->size(), xt);
          else
            cursor(cursor()-cursor()%bytesPerRow_+bytesPerRow_-1, xt);
          return 1;
        case FL_BackSpace:
          if (mgr->selection()!=cursor()) {
            mgr->deleteSelection();
          } else if (cursor()>0) {
            doc->deleteBytes(cursor()-1, 1);
            cursor(cursor()-1);
          }
          return 1;
      }
      break; }
    case FL_PASTE:
      mgr->insert(Fl::event_text(), Fl::event_length());
      return 1;
  }
  return Fl_Group::handle(event);
}

//---- HeColumn ----------------------------------------------------------------

HeColumn::HeColumn(int x, int y, int w, int h, HeDocumentManager *m)
: Fl_Group(x, y, w, h)
{
  end();
  box(FL_FLAT_BOX);
  manager = m;
  doc = m->document();
  layout();
}

int HeColumn::handle(int event) {
  switch (event) {
    case FL_KEYBOARD:
      break;
  }
  return Fl_Group::handle(event);
}

void HeColumn::layout() {
  lines = h() / manager->fontHeight();
  redraw();
}

void HeColumn::draw_bg() {
  int i, ch = manager->fontHeight();
  draw_box();
  fl_color(0xc8c8c800);
  for (i=column()->topRow()&1; i<lines; i+=2) {
    int yp = i*ch + y();
    fl_rectf(x(), yp, w(), ch);
  }
}

heIndex HeColumn::eventRow() {
  int py = Fl::event_y() - y() - 2; //++ why -2 ???
  py /= manager->fontHeight();
  return column()->topRow()+py;
}

//---- HeSeperatorColumn -------------------------------------------------------

HeScrollbarColumn::HeScrollbarColumn(int x, int y, int w, int h,
                                     HeDocumentManager *cm)
: HeColumn(x, y, w, h, cm)
{
  begin();
  scroll = new Fl_Scrollbar(x, y, w, h-14);
  scroll->type(FL_VERTICAL);
  scroll->callback(scrollCB, this);
  end();
  resizable(scroll);
}

void HeScrollbarColumn::getWidth(int &fixed, int &perByte) {
  fixed += 14;
  perByte += 0;
}

void HeScrollbarColumn::layout() {
  HeColumn::layout();
  value(column()->topRow());
}

void HeScrollbarColumn::value(heIndex ix) {
  scroll->value(ix, column()->rowsPerPage(), 0, column()->rows());
}

void HeScrollbarColumn::scrollCB(Fl_Widget*, void *userdata) {
  HeScrollbarColumn *This = (HeScrollbarColumn*)userdata;
  This->column()->topRow(This->scroll->value());
}

//---- HeSeperatorColumn -------------------------------------------------------

HeSeperatorColumn::HeSeperatorColumn(int x, int y, int w, int h,
                                     HeDocumentManager *cm)
: HeColumn(x, y, w, h, cm)
{
}

void HeSeperatorColumn::getWidth(int &fixed, int &perByte) {
  fixed += 5;
  perByte += 0;
}

void HeSeperatorColumn::draw() {
  int xp = x()+w()/2;
  draw_bg();
  fl_color(FL_BLUE);
  fl_line(xp, y(), xp, y()+h());
  draw_label();
}

//---- HeAddrColumn ------------------------------------------------------------

HeAddrColumn::HeAddrColumn(int x, int y, int w, int h, HeDocumentManager *cm)
: HeColumn(x, y, w, h, cm)
{
}

void HeAddrColumn::getWidth(int &fixed, int &perByte) {
  int cw = manager->fontWidth(), cs = manager->spaceWidth();
  fixed += 10*cw + 4*cs;
  perByte += 0;
}

void HeAddrColumn::draw() {
  int i, cw = manager->fontWidth(), ch = manager->fontHeight();
  int cs = manager->spaceWidth(), ca = manager->fontAscent();
  int first = column()->topLeftByte(), bpr = column()->bytesPerRow();
  char buf[20];
  draw_bg();
  manager->setFont();
  fl_color(FL_BLACK);
  for (i=0; i<lines; i++) {
    int xp = x()+cs, yp = i*ch + y() + ca;
    unsigned int ix = first+i*bpr;
    if (ix<=doc->size()) {
      sprintf(buf, "%010x", ix);
      fl_draw(buf,   2, xp,           yp);
      fl_draw(buf+2, 4, xp+cw*2+  cs, yp);
      fl_draw(buf+6, 4, xp+cw*6+2*cs, yp);
    }
  }
  draw_label();
}

int HeAddrColumn::handle(int event) {
  switch (event) {
    case FL_PUSH:
    case FL_DRAG: {
      int y = eventRow();
      int bpr = column()->bytesPerRow();
      if (Fl::event_shift() || event==FL_DRAG)
        manager->extendSelection(y*bpr, y*bpr+bpr);
      else
        manager->select(y*bpr, y*bpr+bpr, true);
      return 1; }
  }
  return HeColumn::handle(event);
}

//---- HeHexColumn -------------------------------------------------------------

HeHexColumn::HeHexColumn(int x, int y, int w, int h, HeDocumentManager *cm)
: HeColumn(x, y, w, h, cm)
{
  subCrsr = 0;
}

void HeHexColumn::getWidth(int &fixed, int &perByte) {
  int cw = manager->fontWidth(), cs = manager->spaceWidth();
  fixed += 2*cs;
  perByte += 2*cw+cs;
}

void HeHexColumn::draw() {
  int i, j;
  int cw = manager->fontWidth(), ch = manager->fontHeight();
  int cs = manager->spaceWidth(), ca = manager->fontAscent(), cd = 2*cw+cs;
  int first = column()->topLeftByte(), bpr = column()->bytesPerRow();
  char buf[4];
  draw_bg();
  manager->setFont();
  fl_color(FL_BLACK);
  for (i=0; i<lines; i++) {
    int xp = x()+cs, yp = i*ch + y() + ca;
    for (j=0; j<bpr; j++) {
      heIndex ix = first+i*bpr+j;
      if (ix<=doc->size()) {
        int a = manager->attributeAt(ix);
        if (a & HE_SELECTED) { // draw a red background cursor
          fl_rectf(xp+j*cd-2, yp-ca, 2*cw+4, ch, 180, 200, 255);
          fl_color(FL_BLACK);
        }
        if (a & HE_CURSOR) { // draw a red background cursor
          fl_rectf(xp+j*cd-2, yp-ca, 2*cw+4, ch, 255, 180, 180);
          fl_color(FL_RED);
          fl_rect(xp+j*cd-2, yp-ca, 2*cw+4, ch);
          if (this == Fl::focus()) {
            if (subCrsr) {
              if (manager->insertMode())
                fl_rectf(xp+j*cd+cw, yp-ca+ch/2, cw+2, ch/2, 255, 48, 48);
              else
                fl_rectf(xp+j*cd+cw, yp-ca, cw+2, ch, 255, 48, 48);
            } else {
              if (manager->insertMode())
                fl_rectf(xp+j*cd-2, yp-ca+ch/2, cw+2, ch/2, 255, 48, 48);
              else
                fl_rectf(xp+j*cd-2, yp-ca, cw+2, ch, 255, 48, 48);
            }
          }
          fl_color(FL_BLACK);
        }
        if (!(a & HE_OUT_OF_BOUNDS)) {
          sprintf(buf, "%02x", doc->byteAt(first+i*bpr+j));
          fl_draw(buf, 2, xp+j*cd, yp);
        }
      }
    }
  }
  draw_label();
}

heIndex HeHexColumn::eventAddr() {
  int col = (Fl::event_x()-x()-manager->spaceWidth()-1)
    / ( 2*manager->fontWidth()+manager->spaceWidth());
  return eventRow() * column()->bytesPerRow() + col;
}

int HeHexColumn::handle(int event) {
  switch (event) {
    case FL_FOCUS:
      subCrsr = 0;
      manager->redraw(); //++ make sure that only the cursor is redrawn!
      return 1;
    case FL_UNFOCUS:
      subCrsr = 0;
      manager->redraw(); //++ make sure that only the cursor is redrawn!
      return 1;
    case FL_PUSH:
      if (Fl::focus() != this) {
        Fl::focus(this);
        handle(FL_FOCUS);
      }
    case FL_DRAG:
      manager->cursor(eventAddr(), (event==FL_DRAG)||(Fl::event_shift()));
      subCrsr = 0;
      return 1;
    case FL_KEYBOARD:
      if (Fl::event_length()) {
        int v = -1;
        char c = Fl::event_text()[0];
        if (c>='0' && c<='9') v = c-'0';
        else if (c>='a' && c<='f') v = c-'a'+10;
        else if (c>='A' && c<='F') v = c-'A'+10;
        if (v==-1) break;
        heIndex crsr = manager->cursor();
        if (subCrsr==0) {
          if (manager->insertMode() || crsr==doc->size()) {
            doc->insertBytes(crsr, 1);
            doc->byteAt(crsr, v<<4);
          } else {
            doc->byteAt(crsr, (doc->byteAt(crsr)&0x0f)|(v<<4));
          }
          subCrsr = 1;
          redraw();
        } else {
          doc->byteAt(crsr, (doc->byteAt(crsr)&0xf0)|v);
          manager->cursor(crsr+1);
          subCrsr = 0;
        }
        return 1;
      }
      subCrsr = 0;
      break;
  }
  return HeColumn::handle(event);
}

//---- HeTextColumn ------------------------------------------------------------

HeTextColumn::HeTextColumn(int x, int y, int w, int h, HeDocumentManager *cm)
: HeColumn(x, y, w, h, cm)
{
}

void HeTextColumn::getWidth(int &fixed, int &perByte) {
  int cw = manager->fontWidth(), cs = manager->spaceWidth();
  fixed += 2*cs;
  perByte += cw;
}

void HeTextColumn::draw() {
  int i, j;
  int cw = manager->fontWidth(), ch = manager->fontHeight();
  int cs = manager->spaceWidth(), ca = manager->fontAscent();
  int first = column()->topLeftByte(), bpr = column()->bytesPerRow();
  draw_bg();
  manager->setFont();
  fl_color(FL_BLACK);
  for (i=0; i<lines; i++) {
    int xp = x()+cs, yp = i*ch + y() + ca;
    for (j=0; j<bpr; j++) {
      heIndex ix = first+i*bpr+j;
      if (ix<=doc->size()) {
        unsigned char c = doc->byteAt(ix);
        int a = manager->attributeAt(ix);
        if (a & HE_SELECTED) {
          fl_rectf(xp+j*cw, yp-ca, cw, ch, 180, 200, 255);
          fl_color(FL_BLACK);
        }
        if (a & HE_CURSOR) {
          if (this == Fl::focus()) {
            if (manager->insertMode())
              fl_rectf(xp+j*cw, yp-ca+ch/2, cw, ch/2, 255, 48, 48);
            else
              fl_rectf(xp+j*cw, yp-ca, cw, ch, 255, 48, 48);
          } else {
            fl_rectf(xp+j*cw, yp-ca, cw, ch, 255, 180, 180);
            fl_color(FL_RED);
            fl_rect(xp+j*cw, yp-ca, cw, ch);
          }
          fl_color(FL_BLACK);
        }
        if (c<32||c==127) c = '.';
        if (!(a & HE_OUT_OF_BOUNDS))
          fl_draw((char*)&c, 1, xp+j*cw, yp);
      }
    }
  }
  draw_label();
}

heIndex HeTextColumn::eventAddr() {
  int col = (Fl::event_x()-x()-manager->spaceWidth()) / manager->fontWidth();
  return eventRow() * column()->bytesPerRow() + col;
}

int HeTextColumn::handle(int event) {
  switch (event) {
    case FL_FOCUS:
      manager->redraw(); //++ make sure that only the cursor is redrawn!
      return 1;
    case FL_UNFOCUS:
      manager->redraw(); //++ make sure that only the cursor is redrawn!
      return 1;
    case FL_PUSH:
      if (Fl::focus() != this) {
        Fl::focus(this);
        handle(FL_FOCUS);
      }
    case FL_DRAG:
      manager->cursor(eventAddr(), (event==FL_DRAG)||(Fl::event_shift()));
      return 1;
    case FL_KEYBOARD:
      if (Fl::event_state()&(MM_CMD|FL_META)) break;
      if (Fl::event_length()) {
        char c = Fl::event_text()[0];
        if ( c<' ' && c!=0x0d && c!=0x0a ) break;
        manager->insert(Fl::event_text(), Fl::event_length());
        return 1;
      }
      break;
  }
  return HeColumn::handle(event);
}

//---- HeInput -----------------------------------------------------------------

HeInput::HeInput(int x, int y, int w, int h, const char *label, int bb, int nn)
: Fl_Input(x, y, w, h, label) {
  wdt = nn;
  labelfont(FL_HELVETICA);
  labelsize(prefs.propsize-3);
  textfont(FL_COURIER);
  textsize(prefs.fixedsize);
  box(FL_FLAT_BOX);
  base(bb);
}

void HeInput::value(heIndex v) {
  static char *lu[] = {
    "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
    "BS" , "HT" , "NL" , "VT" , "NP" , "CR" , "SO" , "SI",
    "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
    "CAN", "EM" , "SUB", "ESC", "FS" , "GS" , "RS" , "US" };
  static char buf[65];
  value_ = v;
  switch (base_) {
    case 0:
      if (v==127) strcpy(buf, "DEL");
      else if (v<32) strcpy(buf, lu[v]);
      else { buf[0] = ' '; buf[1]=v; buf[2]=0; }
      break;
    case 2: {
      char *dst = buf;
      for (int i=wdt-1; i>=0; i--)
        *dst++ = v&(1<<i)?'1':'0';
      *dst = 0;
      break; }
    default:
      sprintf(buf, fmt, v);
      break;
  }
  Fl_Input::value(buf);
}

void HeInput::value(float f) {
  value_f = f;
  static char buf[65];
  sprintf(buf, "%g", f);
  Fl_Input::value(buf);
}

void HeInput::value(double d) {
  value_d = d;
  static char buf[65];
  sprintf(buf, "%lg", d);
  Fl_Input::value(buf);
}

void HeInput::base(int bb) {
  base_ = bb;
  switch (bb) {
    case 0: strcpy(fmt, "%c"); break;
    case 8: strcpy(fmt, "%#o"); break;
    case 10: strcpy(fmt, "%u"); break;
    case -10: strcpy(fmt, "%d"); break;
    case 16: sprintf(fmt, "%%0%dx", wdt); break;
    case 100: strcpy(fmt, "%g"); break;
    case 101: strcpy(fmt, "%lg"); break;
  }
  value(value_);
}

//----- HeButton ---------------------------------------------------------------

HeButton::HeButton(int x, int y, int w, int h, const char *t)
: Fl_Button(x, y, w, h, t) {
  labelsize(prefs.propsize-3);
  box(FL_PLASTIC_UP_BOX);
}

//----- HeButton ---------------------------------------------------------------

HeCycleButton::HeCycleButton(int x, int y, int w, int h,
                             int n, char *t, ...)
: HeButton(x, y, w, h, t) {
  labelsize(prefs.propsize-3);
  box(FL_PLASTIC_UP_BOX);
  choice = 0; nChoice = n;
  warn = 0;
  labels = new char*[n];
  labels[0] = t;
  va_list va;
  va_start(va, t);
  for (int i=1; i<n; i++) {
    labels[i] = va_arg(va, char*);
  }
  va_end(va);
}

HeCycleButton::~HeCycleButton() {
  delete[] labels;
}

int HeCycleButton::handle(int event) {
  switch (event) {
    case FL_FOCUS: return 0;
  }
  return HeButton::handle(event);
}

void HeCycleButton::value(int n) {
  choice = n;
  if (warn & (1<<n))
    labelcolor(FL_RED);
  else
    labelcolor(FL_BLACK);
  label(labels[n]);
}

void HeCycleButton::incr() {
  int n = choice+1;
  if (n>=nChoice)
    n = 0;
  value(n);
}

//---- Preferences -------------------------------------------------------------

HePreferences::HePreferences()
: app(Fl_Preferences::USER, "matthiasm.com", "mickeyHexEditor") {
  app.get("propfont", propfont, MM_PROP_FONT);
  app.get("propsize", propsize, MM_PROP_SIZE);
  app.get("fixedfont", fixedfont, MM_FIXED_FONT);
  app.get("fixedsize", fixedsize, MM_FIXED_SIZE);
  Fl_Preferences win(app, "win");
  win.get("flags", winflags, 0);
  win.get("x", winx, 55);
  win.get("y", winy, 60);
  win.get("w", winw, 420);
  win.get("h", winh, 650);
}

HePreferences::~HePreferences() {
  app.set("propfont", propfont);
  app.set("propsize", propsize);
  app.set("fixedfont", fixedfont);
  app.set("fixedsize", fixedsize);
  Fl_Preferences win(app, "win");
  win.set("flags", winflags);
  win.set("x", winx);
  win.set("y", winy);
  win.set("w", winw);
  win.set("h", winh);
  if (propfont) free(propfont);
  if (fixedfont) free(fixedfont);
}

//---- main --------------------------------------------------------------------

int main(int argc, char **argv) {
  Fl::set_font(FL_HELVETICA, prefs.propfont);
  Fl::set_font(FL_COURIER, prefs.fixedfont);
  fl_font(FL_COURIER, prefs.fixedsize);
  fixedFontHeight = fl_height();
  fixedFontAscent = fl_height()-fl_descent();
  fixedFontWidth = (int)(fl_width("W")+.7);
  fl_message_font(FL_HELVETICA, MM_PROP_SIZE_MED);

  HeApp app(argc, argv);
  return Fl::run();
}

