/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  These procedures handle the "info" method for package ItclWidget
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetInfo.c,v 1.1.2.3 2008/11/11 11:37:36 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007 Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclWidgetInt.h"

Tcl_ObjCmdProc ItclBiInfoComponentsCmd;
Tcl_ObjCmdProc ItclBiInfoComponentCmd;
Tcl_ObjCmdProc ItclBiInfoOptionsCmd;
Tcl_ObjCmdProc ItclBiInfoOptionCmd;
Tcl_ObjCmdProc ItclBiInfoDelegateCmd;
Tcl_ObjCmdProc ItclBiInfoDelegateMethodCmd;
Tcl_ObjCmdProc ItclBiInfoDelegateOptionCmd;
Tcl_ObjCmdProc ItclBiInfoTypesCmd;
Tcl_ObjCmdProc ItclBiInfoWidgetsCmd;
Tcl_ObjCmdProc ItclBiInfoWidgetAdaptorsCmd;

#ifdef NOTDEF
typedef struct InfoMethod {
    char* name;              /* method name */
    char* usage;             /* string describing usage */
    Tcl_ObjCmdProc *proc;    /* implementation C proc */
} InfoMethod;

static InfoMethod InfoMethodList[] = {
    { "components", "", ItclBiInfoComponentsCmd },
    { "component", "componentname", ItclBiInfoComponentCmd },
    { "options", "", ItclBiInfoOptionsCmd },
    { "option", "?", ItclBiInfoOptionCmd },
    { "types", "", ItclBiInfoTypesCmd },
    { "widgets", "", ItclBiInfoWidgetsCmd },
    { "widgetadaptors", "", ItclBiInfoWidgetAdaptorsCmd },
    { NULL, NULL, NULL }
};
#endif

struct NameProcMap { const char *name; Tcl_ObjCmdProc *proc; };

/*
 * List of commands that are used to implement the [info object] subcommands.
 */

static const struct NameProcMap infoCmds2[] = {
    { "::itcl::builtin::Info::components", ItclBiInfoComponentsCmd },
    { "::itcl::builtin::Info::component", ItclBiInfoComponentCmd },
    { "::itcl::builtin::Info::options", ItclBiInfoOptionsCmd },
    { "::itcl::builtin::Info::option", ItclBiInfoOptionCmd },
    { "::itcl::builtin::Info::types", ItclBiInfoTypesCmd },
    { "::itcl::builtin::Info::widgets", ItclBiInfoWidgetsCmd },
    { "::itcl::builtin::Info::widgetadapters", ItclBiInfoWidgetAdaptorsCmd },
    { NULL, NULL }
};


/*
 * ------------------------------------------------------------------------
 *  ItclWidgetInfoInit()
 *
 *  Creates a namespace full of built-in methods/procs for [incr Tcl]
 *  classes.  This includes things like the "info"
 *  for querying class info.  Usually invoked by Itcl_Init() when
 *  [incr Tcl] is first installed into an interpreter.
 *
 *  Returns TCL_OK/TCL_ERROR to indicate success/failure.
 * ------------------------------------------------------------------------
 */
int
ItclWidgetInfoInit(
    Tcl_Interp *interp,      /* current interpreter */
    ItclObjectInfo *infoPtr)
{
    Tcl_Namespace *nsPtr;
    Tcl_Command cmd;
    int i;

    for (i=0 ; infoCmds2[i].name!=NULL ; i++) {
        Tcl_CreateObjCommand(interp, infoCmds2[i].name,
                infoCmds2[i].proc, infoPtr, NULL);
    }
    nsPtr = Tcl_CreateNamespace(interp, "::itcl::builtin::Info::delegate", NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("ITCL: error in creating namespace: ::itcl::builtin::Info::delegate\n");
    }
    cmd = Tcl_CreateEnsemble(interp, nsPtr->fullName, nsPtr,
        TCL_ENSEMBLE_PREFIX);
    Tcl_Export(interp, nsPtr, "[a-z]*", 1);
    Tcl_CreateObjCommand(interp, "::itcl::builtin::Info::delegate::method",
                ItclBiInfoDelegateMethodCmd, infoPtr, NULL);
    Tcl_CreateObjCommand(interp, "::itcl::builtin::Info::delegate::option",
                ItclBiInfoDelegateOptionCmd, infoPtr, NULL);

    return TCL_OK;
}

int
ItclBiInfoComponentsCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoComponentsCmd", objc, objv);
    
    return TCL_OK;
}
    
int
ItclBiInfoComponentCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoComponentCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoOptionsCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoOptionsCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoOptionCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoOptionCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoDelegateCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoDelegateCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoDelegateMethodCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoDelegateMethodCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoDelegateOptionCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoDelegateOptionCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoTypesCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoTypesCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoWidgetsCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoWidgetsCmd", objc, objv);
    
    return TCL_OK;
}
int
ItclBiInfoWidgetAdaptorsCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclShowArgs(0, "ItclBiInfoTypesCmd", objc, objv);
    
    return TCL_OK;
}
