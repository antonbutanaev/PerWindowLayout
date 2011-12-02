#include <iostream>
#include <map>
#include <string>

#include <stdio.h>
#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>

using namespace std;

Display *display;
int xkbEventType;
Window root;

typedef int Layout;
const Layout Unknown = -1;

typedef map<Window, Layout> WindowLayouts;
WindowLayouts windowLayouts;

const char *prefix = "perWindowLayout:: ";

const static long rootEvents = StructureNotifyMask | SubstructureNotifyMask | FocusChangeMask | KeymapStateMask;

void init() {
  int xkbError, reason;
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  display = XkbOpenDisplay(0, &xkbEventType, &xkbError, &major, &minor, &reason);

  XkbSelectEventDetails(display, XkbUseCoreKbd, XkbStateNotify, XkbAllStateComponentsMask, XkbGroupStateMask);
  root = DefaultRootWindow(display);
  XSelectInput(display, root, rootEvents);

  cout << prefix << "init: major=" << major << " minor=" << minor << " root=" << root << "\n";
}

Layout getCurrentLayout() {
  XkbStateRec state;
  if (XkbGetState(display, XkbUseCoreKbd, &state) == Success)
    return state.locked_group;
  return Unknown;
}

bool wrongWindow(Window window) {
  return window == PointerRoot || window == None;
}

Window focusedWindow() {
  Window window;
  int param;
  XGetInputFocus(display, &window, &param);

  if (wrongWindow(window))
    return window;

  for (;;) {
    unsigned numChildren;
    Window *children, parent, root2;
    XQueryTree(display, window, &root2, &parent, &children, &numChildren);

    if (numChildren)
      XFree(children);
    if (parent == root)
      break;
    window = parent;
  }
  return window;
}

void proceedEvent(XEvent ev) {
  if (ev.type == ConfigureNotify) {

    Window window = focusedWindow();
    if (wrongWindow(window))
      return;

    Layout layout = getCurrentLayout();

    WindowLayouts::const_iterator i = windowLayouts.find(window);
    Layout saved = i != windowLayouts.end() ? i->second : 0;

    cout << prefix << "ConfigureNotify: window=" << window << " layout=" << layout << " saved=" << saved << "\n";

    if (layout != saved)
      XkbLockGroup(display, XkbUseCoreKbd, saved);

  } else if (ev.type == xkbEventType) {
    Window window = focusedWindow();
    Layout layout = getCurrentLayout();

    windowLayouts[window] = layout;

    cout << prefix << "xkbEventType: window=" << window << " layout=" << layout << "\n";
  }
}

void mainLoop() {
  for (;;) {
    XEvent ev = { 0 };
    XNextEvent(display, &ev);
    proceedEvent(ev);
  }
}

int main(int argc, char **argv) {
  string noDaemonArg = "-n", helpArg = "-h";
  if (argc > 2 || (argc == 2 && argv[1] != noDaemonArg) || (argc == 2 && argv[1] == helpArg)) {
    cout << "Keep per-window keyboard layout\n\nUsage: perWindowLayout [ -n ]\n\t-n\tdo not daemonize\n";
    return 1;
  }

  bool noDaemon = argc == 2 && argv[1] == noDaemonArg;
  if (!noDaemon && daemon(0, 0) != 0) {
    cerr << "Failed to daemonize\n";
    return 1;
  }

  init();
  mainLoop();
  return 0;
}
