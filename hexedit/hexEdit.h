
// (c) 2003 - Matthias Melcher, Venice, CA 90291


#ifndef HEXEDIT_H
#define HEXEDIT_H

#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>

typedef unsigned int heIndex;

class Fl_Window;
class Fl_Group;
class Fl_Box;
class Fl_Scrollbar;
class Fl_Menu_Item;
class Fl_Menu_Bar;
class Fl_Tabs;
class Fl_Input;
class HeMenubar;
class HeToolbar;
class HeDocumentList;
class HeDocument;
class HeDocumentManager;
class HeStatusBar;
class HeColumnGroup;
class HeColumn;
class HeScrollbarColumn;
class HeInput;
class HeButton;
class HeCycleButton;

class HeApp {
  HeDocumentList *doclist;
  Fl_Window *window;
  HeMenubar *menubar;
  HeToolbar *toolbar;
  static void closeAppWindowCB(Fl_Widget*, void*);
public:
  HeApp(int argc, char **argv);
  ~HeApp();
  void quitApplication();
  void newDocument(const char *filename = 0);
  char closeDocument(HeDocument *doc = 0);
  HeDocument *document();
};

class HeMenubar : public Fl_Group {
  static HeApp *app;
  Fl_Menu_Bar *menu;
  static Fl_Menu_Item itemList[];
  static void newCB(Fl_Widget*, void*);
  static void openCB(Fl_Widget*, void*);
  static void saveCB(Fl_Widget*, void*);
  static void saveAsCB(Fl_Widget*, void*);
  static void closeCB(Fl_Widget*, void*);
  static void quitCB(Fl_Widget*, void*);
  static void cutCB(Fl_Widget*, void*);
  static void copyCB(Fl_Widget*, void*);
  static void pasteCB(Fl_Widget*, void*);
  static void insertModeCB(Fl_Widget*, void*);
  static void aboutCB(Fl_Widget*, void*);
public:
  HeMenubar(int x, int y, int w, int h, HeApp*);
};

class HeTool : public Fl_Group {
protected:
  static void parentDoCB(Fl_Widget*, void*);
public:
  HeTool(int x, int y, int w, int h);
  HeTool(int x, int y, int w, int h, const char *label, const char*const*xpm);
};

class HeToolSearch : public HeTool {
  static void convertInputCB(Fl_Widget*, void*);
  void convertInput();
  Fl_Input *input;
  unsigned short *buffer;
  int nBuffer, NBuffer;
public:
  HeToolSearch(int x, int y, int w, int h);
  ~HeToolSearch();
  const unsigned short *value() { return buffer; }
  int length() { return nBuffer; }
};

class HeToolbar : public Fl_Group {
  HeApp *app;
  static void newCB(Fl_Widget*, void*);
  static void openCB(Fl_Widget*, void*);
  static void saveCB(Fl_Widget*, void*);
  static void closeCB(Fl_Widget*, void*);
  static void searchCB(Fl_Widget*, void*);
  static void helpCB(Fl_Widget*, void*);
public:
  HeToolbar(int x, int y, int w, int h, HeApp*);
};

class HeDocumentList : public Fl_Tabs {
  HeApp *app;
public:
  HeDocumentList(int x, int y, int w, int h, HeApp*);
  void add(const char *filename=0);
};

class HeDocument : public Fl_Group {
  HeApp *app;
  HeDocumentManager *manager_;
  char *filename_;
  char *shortname;
  char *labelname;
  heIndex size_;
  unsigned char *buffer_;
  heIndex gap, gapSize;
  int file_;
  void moveGapTo(heIndex pos);
  void addToGap(heIndex n);
  char changed_;
  void clearChanged();
public:
  HeDocument(int x, int y, int w, int h, HeApp*);
  ~HeDocument();
  HeDocumentManager *manager() { return manager_; }
  void layout();
  void loadFile(const char *name);
  void saveFile(const char *name=0);
  char close();
  const char *filename() { return filename_; }
  void filename(const char *name);
  heIndex size();
  unsigned char byteAt(heIndex i);
  unsigned char *blockAt(heIndex start, heIndex n);
  void byteAt(heIndex i, unsigned char v);
  void deleteBytes(heIndex first, heIndex n);
  void insertBytes(heIndex first, heIndex n);
  void setChanged();
  char changed() { return changed_; }
};

// attribute flags
#define HE_CURSOR         0x0001
#define HE_SELECTED       0x0002
#define HE_OUT_OF_BOUNDS  0x0004

// search flags
#define HE_DONT_CARE      0x0100
#define HE_IGNORE_CASE    0x0200

class HeDocumentManager : public Fl_Group {
  HeDocument *doc;
  HeStatusBar *status;
  HeColumnGroup *column;
  heIndex cursor_;
  heIndex selection_;
  char insertMode_;
public:
  HeDocumentManager(int x, int y, int w, int h, HeDocument*);
  HeDocument *document() { return doc; }
  void layout();
  void update();
  void setFont();
  int fontHeight();
  int fontWidth();
  int spaceWidth();
  int fontAscent();
  int attributeAt(heIndex);
  void select(heIndex, heIndex, bool toggle);
  void extendSelection(heIndex, heIndex);
  void cursor(heIndex, bool extend = false);
  heIndex cursor() { return cursor_; }
  heIndex selection() { return selection_; }
  void insertMode(char m);
  char insertMode() { return insertMode_; }
  void insert(const char *text, heIndex len);
  void deleteSelection();
  void cutToClipboard();
  void copyToClipboard();
  void pasteFromClipboard();
  bool searchNext(const unsigned short*, int);
};

class HeStatusBar : public Fl_Group {
  HeDocumentManager *manager;
  HeDocument *doc;
  HeInput *crsr, *slct, *offs;
  HeCycleButton *adrb, *dtab, *blsb, *brwm, *bins;
  Fl_Group *hexGrp, *binGrp, *octGrp, *decGrp, *sdcGrp;
  HeInput *d1x, *d2x, *d4x;
  HeInput *d1b, *d2b;
  HeInput *d1o, *d2o, *d4o;
  HeInput *d1d, *d2d, *d4d;
  HeInput *d1c, *d4f, *d8f;
  int byteOrder_;
  static void cycleAddressBaseCB(Fl_Widget*, void*);
  static void cycleDataBaseCB(Fl_Widget*, void*);
  static void lsbMsbModeCB(Fl_Widget*, void*);
  static void readOnlyModeCB(Fl_Widget*, void*);
  static void insertModeCB(Fl_Widget*, void*);
public:
  HeStatusBar(int x, int y, int w, int h, HeDocumentManager*);
  void cursor(heIndex a, heIndex b);
  void updateData();
  void updateFlags();
  void update() { updateData(); updateFlags(); }
  void byteOrder(int n) { byteOrder_ = n; update(); }
};

class HeColumnGroup : public Fl_Group {
  HeDocument *doc;
  HeDocumentManager *mgr;
  HeScrollbarColumn *scroll;
  int rows_, rowsPerPage_;
  int bytesPerRow_;
  heIndex topByte_;
  heIndex topLeftByte_;
public:
  HeColumnGroup(int x, int y, int w, int h, HeDocumentManager*);
  void createStandardColumns();
  void resize(int x, int y, int w, int h);
  void layout();
  virtual int handle(int); 
  int bytesPerRow() { return bytesPerRow_; }
  int bytesPerPage() { return rowsPerPage_*bytesPerRow_; }
  int rows() { return rows_; }
  int rowsPerPage() { return rowsPerPage_; }
  heIndex topLeftByte() { return topLeftByte_; }
  heIndex topRow() { return topLeftByte_/bytesPerRow_; }
  heIndex topByte() { return topByte_; }
  void topRow(heIndex);
  void topByte(heIndex);
  void cursor(heIndex ix, bool extend = false) { mgr->cursor(ix, extend); }
  heIndex cursor() { return mgr->cursor(); }
};

class HeColumn : public Fl_Group {
protected:
  HeDocumentManager *manager;
  HeDocument *doc;
  int lines;
public:
  HeColumn(int x, int y, int w, int h, HeDocumentManager*);
  HeColumnGroup *column() { return (HeColumnGroup*)parent(); }
  virtual int handle(int); 
  virtual void layout();
  virtual void getWidth(int&, int&) = 0;
  void draw_bg();
  heIndex eventRow();
};

class HeScrollbarColumn : public HeColumn {
  Fl_Scrollbar *scroll;
  static void scrollCB(Fl_Widget*, void*);
public:
  HeScrollbarColumn(int x, int y, int w, int h, HeDocumentManager*);
  virtual void getWidth(int&, int&);
  virtual void layout();
  void value(heIndex);
};

class HeSeperatorColumn : public HeColumn {
public:
  HeSeperatorColumn(int x, int y, int w, int h, HeDocumentManager*);
  virtual void getWidth(int&, int&);
  virtual void draw();
};

class HeAddrColumn : public HeColumn {
public:
  HeAddrColumn(int x, int y, int w, int h, HeDocumentManager*);
  virtual void getWidth(int&, int&);
  virtual void draw();
  virtual int handle(int);
};

class HeHexColumn : public HeColumn {
  int subCrsr;
public:
  HeHexColumn(int x, int y, int w, int h, HeDocumentManager*);
  virtual void getWidth(int&, int&);
  virtual void draw();
  virtual int handle(int);
  heIndex eventAddr();
};

class HeTextColumn : public HeColumn {
public:
  HeTextColumn(int x, int y, int w, int h, HeDocumentManager*);
  virtual void getWidth(int&, int&);
  virtual void draw();
  virtual int handle(int);
  heIndex eventAddr();
};

class HeInput : public Fl_Input {
  int wdt;
  int base_;
  char fmt[10];
  heIndex value_;
  float value_f;
  double value_d;
public:
  HeInput(int x, int y, int w, int h, const char *label=0, int bb=10, int nn=4);
  void value(heIndex);
  void value(float);
  void value(double);
  void base(int);
};

class HeButton : public Fl_Button {
public:
  HeButton(int x, int y, int w, int h, const char *label=0);
};

class HeCycleButton : public HeButton {
  int choice, nChoice;
  int warn;
  char **labels;
public:
  HeCycleButton(int x, int y, int w, int h, int n, char *cc, ...);
  ~HeCycleButton();
  virtual int handle(int);
  void incr();
  void value(int);
  int value() { return choice; }
  void warnMask(int m) { warn=m; redraw(); }
};

class HePreferences {
  Fl_Preferences app;
public:
  HePreferences();
  ~HePreferences();
  int winflags, winx, winy, winw, winh;
  char *fixedfont, *propfont;
  int fixedsize, propsize;
};

#endif


