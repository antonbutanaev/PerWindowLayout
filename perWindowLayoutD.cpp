#include <iostream>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xmd.h>
#include <X11/keysym.h>

using namespace std;

Display *display;
int xkbEventType;

const char *prefix = "perWindowLayout:: ";

const static long rootEvents = StructureNotifyMask | SubstructureNotifyMask | FocusChangeMask | KeymapStateMask;

void init() {
  int xkbError, reason;
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  display = XkbOpenDisplay(0, &xkbEventType, &xkbError, &major, &minor, &reason);

  XkbSelectEventDetails(display, XkbUseCoreKbd, XkbStateNotify, XkbAllStateComponentsMask, XkbGroupStateMask);
  XSelectInput(display, RootWindow(display, DefaultScreen(display)), rootEvents);

  cout << prefix << "init: major " << major << " minor: " << minor << "\n";
}

void proceedEvent(XEvent ev) {
  if (ev.type == ConfigureNotify)
    cout << prefix << "window: " << ev.xconfigure.window << "\n";
  if (ev.type == xkbEventType)
    cout << prefix << "window: " << ev.xkey.window << ":" << ev.xkeymap.window << "\n";

}


void mainLoop() {
  for (;;) {
    XEvent ev = {0};
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
