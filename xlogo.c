/*
 * $Xorg: xlogo.c,v 1.4 2001/02/09 02:05:54 xorgcvs Exp $
 *
Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 */

/* $XFree86: xc/programs/xlogo/xlogo.c,v 3.7 2001/07/25 15:05:26 dawes Exp $ */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "xlogo.h"
#include "Logo.h"
#include <X11/Xaw/Cardinals.h>
#ifdef XPRINT
#include "print.h"
#endif
#ifdef XKB
#include <X11/extensions/XKBbells.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* Global vars*/
const char *ProgramName;    /* program name (from argv[0]) */

static void quit(Widget w,  XEvent *event, String *params, Cardinal *num_params);
#ifdef XPRINT
static void print(Widget w, XEvent *event, String *params, Cardinal *num_params);
#endif

static XrmOptionDescRec options[] = {
{ "-shape", "*shapeWindow", XrmoptionNoArg, (XPointer) "on" },
#ifdef XRENDER
{"-render", "*render",XrmoptionNoArg, "TRUE"},
{"-sharp", "*sharp", XrmoptionNoArg, "TRUE"},
#endif
{"-v",         "Verbose",     XrmoptionNoArg,  "TRUE"},
#ifdef XPRINT
{"-print",     "Print",       XrmoptionNoArg,  "TRUE"},
{"-printer",   "printer",     XrmoptionSepArg, NULL},
{"-printfile", "printFile",   XrmoptionSepArg, NULL},
#endif
};

static XtActionsRec actions[] = {
    {"quit",	quit },
#ifdef XPRINT
    {"print",	print}
#endif
};

static Atom wm_delete_window;

/* See xlogo.h */
XLogoResourceData userOptions;

#define Offset(field) XtOffsetOf(XLogoResourceData, field)

XtResource resources[] = {
  {"verbose",   "Verbose",   XtRBoolean, sizeof(Boolean), Offset(verbose),      XtRImmediate, (XtPointer)False},
#ifdef XPRINT
  {"print",     "Print",     XtRBoolean, sizeof(Boolean), Offset(printAndExit), XtRImmediate, (XtPointer)False},
  {"printer",   "Printer",   XtRString,  sizeof(String),  Offset(printername),  XtRImmediate, (XtPointer)NULL},
  {"printFile", "PrintFile", XtRString,  sizeof(String),  Offset(printfile),    XtRImmediate, (XtPointer)NULL}
#endif
};


String fallback_resources[] = {
    "*iconPixmap:    xlogo32",
    "*iconMask:      xlogo32",
    "*baseTranslations: #override \\"
#ifdef XPRINT
                        "\t<Key>q: quit()\\n\\"
                        "\t<Key>p: print()",
#else
                        "\t<Key>q: quit()",
#endif
    NULL,
};

static void 
die(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtAppSetExitFlag(XtWidgetToApplicationContext(w));
}

static void 
save(Widget w, XtPointer client_data, XtPointer call_data)
{
    return;
}

/*
 * Report the syntax for calling xlogo.
 */

static void 
Syntax(Widget toplevel)
{
    Arg arg;
    SmcConn connection;
    String reasons[10];
    int i, n = 0;

    reasons[n++] = "Usage: ";
    reasons[n++] = (String)ProgramName;
    reasons[n++] = " [-fg <color>] [-bg <color>] [-rv] [-bw <pixels>] [-bd <color>]\n";
    reasons[n++] = "             [-d [<host>]:[<vs>]]\n";
    reasons[n++] = "             [-g [<width>][x<height>][<+-><xoff>[<+-><yoff>]]]\n";
#ifdef XPRINT
    reasons[n++] = "             [-print] [-printname <name>] [-printfile <file>]\n";
#endif
#ifdef XRENDER
    reasons[n++] = "             [-render] [-sharp]\n";
#endif /* XRENDER */
    reasons[n++] = "             [-shape]\n\n";

    XtSetArg(arg, XtNconnection, &connection);
    XtGetValues(toplevel, &arg, (Cardinal)1);
    if (connection) 
	SmcCloseConnection(connection, n, reasons);
    else {
	for (i=0; i < n; i++)
	    printf(reasons[i]);
    }
    exit(EXIT_FAILURE);
}

int 
main(int argc, char *argv[])
{
    Widget toplevel;
    XtAppContext app_con;

    ProgramName = argv[0];

    toplevel = XtOpenApplication(&app_con, "XLogo",
				 options, XtNumber(options), 
				 &argc, argv, fallback_resources,
				 sessionShellWidgetClass, NULL, ZERO);
    if (argc != 1)
	Syntax(toplevel);

    XtGetApplicationResources(toplevel, (XtPointer)&userOptions, resources, 
                              XtNumber(resources), NULL, 0);

    XtAppAddActions(app_con, actions, XtNumber(actions));

#ifdef XPRINT
    if (userOptions.printAndExit) {
        XtCallActionProc(toplevel, "print", NULL, NULL, 0);
    }
    else
#endif
    {
        XtAddCallback(toplevel, XtNsaveCallback, save, NULL);
        XtAddCallback(toplevel, XtNdieCallback,  die,  NULL);
        XtOverrideTranslations
          (toplevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));
        XtCreateManagedWidget("xlogo", logoWidgetClass, toplevel, NULL, ZERO);
        XtRealizeWidget(toplevel);
        wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
                                       False);
        (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                                &wm_delete_window, 1);
    }

    XtAppMainLoop(app_con);

    return EXIT_SUCCESS;
}

/*ARGSUSED*/
static void 
quit(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    Arg arg;
    
    if (event->type == ClientMessage && 
	(Atom)event->xclient.data.l[0] != wm_delete_window) {
#ifdef XKB
	XkbStdBell(XtDisplay(w), XtWindow(w), 0, XkbBI_BadValue);
#else
	XBell(XtDisplay(w), 0);
#endif
    } else {
	/* resign from the session */
	XtSetArg(arg, XtNjoinSession, False);
	XtSetValues(w, &arg, ONE);
	die(w, NULL, NULL);
    }
}

#ifdef XPRINT
/*ARGSUSED*/
static void 
print(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    DoPrint(w, userOptions.printername, userOptions.printfile);
}
#endif
