#include <iostream>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>

using namespace std;

Display *display;
int xkbEventType;
Window root;

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

  cout << prefix << "init: major " << major << " minor: " << minor << " root window: " << root << "\n";
}

int getCurrentLayout() {
  XkbStateRec state;
  if (XkbGetState(display, XkbUseCoreKbd, &state) == Success)
    return state.locked_group;
  return -1;
}

Window focusedWindow() {
  Window window;
  int param;
  XGetInputFocus(display, &window, &param);

  for (;;) {
    unsigned nchildren;
    Window *children, parent;
    XQueryTree(display, window, &root, &parent, &children, &nchildren);
    XFree(children);
    if (parent == root)
      break;
    window = parent;
  }
  return window;
}

void proceedEvent(XEvent ev) {
  if (ev.type == ConfigureNotify)
    cout << prefix << "window: " << ev.xconfigure.window << " focus: " << focusedWindow() << " layout: "
        << getCurrentLayout() << "\n";
  else if (ev.type == xkbEventType)
    cout << prefix << "xkbEventType: layout: " << getCurrentLayout() << "\n";
}

void mainLoop() {
  for (;;) {
    XEvent ev = { 0 };
    XNextEvent(display, &ev);
    proceedEvent(ev);
  }
}

void cleanup() {
}

int main() {
  cout << prefix << "started\n";
  init();
  mainLoop();
  cleanup();
  cout << prefix << "finished\n";
  return 0;
}
