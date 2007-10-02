/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 *  [incr Tcl] provides object-oriented extensions to Tcl, much as
 *
 *  Procedures in this file support the new syntax for commands
 *  for class definitions of package ItclWidget:
 *
 *    itcl::type <className> {
 *    }
 *    itcl::widgetadaptor <className> {
 *    }
 *    itcl::widget <className> {
 *        inherit <base-class>...
 *
 *        delegate method/option to component as script
 *        delegate method/option to component using script
 *
 *        option {<nameSpec>} ?{value}? ?-cgetmethod {<name>}?
 *                ?-configuremethod {<name>}? ?-validatemethod {<name>}?
 *                ?-readonly {<boolean>}?
 *        component {<componentname>}
 *         
 *        constructor {<arglist>} ?{<init>}? {<body>}
 *        destructor {<body>}
 *
 *        method <name> {<arglist>} {<body>}
 *        proc <name> {<arglist>} {<body>}
 *        variable <name> ?<init>? ?<config>?
 *        common <name> ?<init>?
 *
 *        public <thing> ?<args>...?
 *        protected <thing> ?<args>...?
 *        private <thing> ?<args>...?
 *    }
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetParse.c,v 1.1.2.12 2007/10/02 22:43:31 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007  Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * ------------------------------------------------------------------------
 */

#include "itclInt.h"

Tcl_ObjCmdProc Itcl_ClassComponentInstallCmd;
Tcl_ObjCmdProc Itcl_ClassWidgetClassCmd;
Tcl_ObjCmdProc Itcl_ClassHullTypeCmd;
Tcl_ObjCmdProc Itcl_WidgetCmd;
Tcl_ObjCmdProc Itcl_WidgetAdaptorCmd;
Tcl_ObjCmdProc Itcl_TypeCmd;

static const struct {
    const char *name;
    Tcl_ObjCmdProc *objProc;
} parseCmds[] = {
    {"componentinstall", Itcl_ClassComponentInstallCmd},
    {"hulltype", Itcl_ClassHullTypeCmd},
    {"widgetclass", Itcl_ClassWidgetClassCmd},
    {NULL, NULL}
};


/*
 * ------------------------------------------------------------------------
 *  Itcl_WidgetParseInit()
 *
 *  Invoked by Itcl_Init() whenever a new interpeter is created to add
 *  [incr Tcl] facilities.  Adds the commands needed to parse class
 *  definitions.
 * ------------------------------------------------------------------------
 */
int
Itcl_WidgetParseInit(
    Tcl_Interp *interp,     /* interpreter to be updated */
    ItclObjectInfo *infoPtr) /* info regarding all known objects and classes */
{
    Tcl_DString buffer;
    int i;

    Itcl_PreserveData((ClientData)infoPtr);

    /*
     *  Add commands for parsing class definitions.
     */
    Tcl_DStringInit(&buffer);
    for (i=0 ; parseCmds[i].name ; i++) {
        Tcl_DStringAppend(&buffer, "::itcl::parser::", 16);
        Tcl_DStringAppend(&buffer, parseCmds[i].name, -1);
        Tcl_CreateObjCommand(interp, Tcl_DStringValue(&buffer),
                parseCmds[i].objProc, (ClientData) infoPtr, NULL);
        Tcl_DStringFree(&buffer);
    }

    Tcl_CreateObjCommand(interp, "::itcl::type", Itcl_TypeCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::widget", Itcl_WidgetCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);

    Tcl_CreateObjCommand(interp, "::itcl::widgetadaptor", Itcl_WidgetAdaptorCmd,
        (ClientData)infoPtr, (Tcl_CmdDeleteProc*)NULL);
    Itcl_PreserveData((ClientData)infoPtr);


    return TCL_OK;
}


/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassComponentInstallCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "componentinstall" command is invoked to define a componentinstall 
 *  Handles the following syntax:
 *
 *      componentinstall 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassComponentInstallCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
//    Tcl_Obj **newObjv;
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;
//    ItclComponent *icPtr;
//    const char *usage;
//    int inherit;
//    int newObjc;

    ItclShowArgs(0, "Itcl_ClassComponentInstallCmd", objc, objv);
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
iclsPtr = NULL;
    Tcl_AppendResult(interp, "componentinstall not yet implemented", NULL);
    return TCL_ERROR;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassHullTypeCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "hulltype" command is invoked to define a hulltype 
 *  Handles the following syntax:
 *
 *      hulltype 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassHullTypeCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    int correctArg;

    ItclShowArgs(1, "Itcl_ClassHullTypeCmd", objc, objv);
    ItclObjectInfo *infoPtr = (ItclObjectInfo*)clientData;
    ItclClass *iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (!(iclsPtr->flags & ITCL_WIDGET)) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget.", 
		" Only an ::itcl::widget can have a hulltype", NULL);
	return TCL_ERROR;
    }
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, " frame/toplevel");
	return TCL_ERROR;
    }
    correctArg = 0;
    if (strcmp(Tcl_GetString(objv[1]), "frame") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "labelframe") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_LABEL_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "toplevel") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_TOPLEVEL;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "ttk::frame") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_TTK_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "ttk::labelframe") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_TTK_LABEL_FRAME;
        correctArg = 1;
    }
    if (strcmp(Tcl_GetString(objv[1]), "ttk::toplevel") == 0) {
	iclsPtr->flags |= ITCL_WIDGET_TTK_TOPLEVEL;
        correctArg = 1;
    }
    if (!correctArg) {
        Tcl_AppendResult(interp,
	        "syntax: must be hulltype frame or toplevel",
	        NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  Itcl_ClassWidgetClassCmd()
 *
 *  Invoked by Tcl during the parsing of a class definition whenever
 *  the "widgetclass" command is invoked to define a widgetclass 
 *  Handles the following syntax:
 *
 *      widgetclass 
 *
 * ------------------------------------------------------------------------
 */
int
Itcl_ClassWidgetClassCmd(
    ClientData clientData,   /* info for all known objects */
    Tcl_Interp *interp,      /* current interpreter */
    int objc,                /* number of arguments */
    Tcl_Obj *CONST objv[])   /* argument objects */
{
    ItclObjectInfo *infoPtr;
    ItclClass *iclsPtr;

    ItclShowArgs(1, "Itcl_ClassWidgetClassCmd", objc, objv);
    infoPtr = (ItclObjectInfo*)clientData;
    iclsPtr = (ItclClass*)Itcl_PeekStack(&infoPtr->clsStack);
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "<widgetclass>");
        return TCL_ERROR;
    }
    if (!(iclsPtr->flags & ITCL_WIDGET)) {
        Tcl_AppendResult(interp, "\"", Tcl_GetString(iclsPtr->namePtr),
	        " is no ::itcl::widget.", 
		" Only an ::itcl::widget can have a widgetclass", NULL);
	return TCL_ERROR;
    }
    iclsPtr->widgetClassPtr = objv[1];
    Tcl_IncrRefCount(iclsPtr->widgetClassPtr);
    /* FIX ME !! */
    /* maybe have to check for valid widget class name ?? */
    Tcl_AppendResult(interp, "not yet completely implemented", NULL);
    return TCL_OK;
}
