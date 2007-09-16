/*
 * itclWidgetBase.c --
 *
 * This file contains the C-implemeted part of a
 * Itcl implemenatation for package ItclWidget
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: itclWidgetBase.c,v 1.1.2.4 2007/09/16 17:16:30 wiede Exp $
 */

#include <stdlib.h>
#include "itclInt.h"
#include <tk.h>

extern struct ItclStubAPI itclStubAPI;

static int Initialize _ANSI_ARGS_((Tcl_Interp *interp));

/*
 * ------------------------------------------------------------------------
 *  Initialize()
 *
 *      that is the starting point when loading the library
 *      it initializes all internal stuff
 *
 * ------------------------------------------------------------------------
 */

static int
Initialize (
    Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    ItclObjectInfo *infoPtr;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }
    if (Tk_InitStubs(interp, "8.5", 0) == NULL) {
        return TCL_ERROR;
    }
    if (Itcl_InitStubs(interp, ITCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }

    infoPtr = (ItclObjectInfo *)Tcl_GetAssocData(interp,
            ITCL_INTERP_DATA, NULL);
    nsPtr = Tcl_CreateNamespace(interp, "::itclwidget", NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("Itcl: cannot create namespace: \"%s\" \n", "::itclwidget");
    }
    nsPtr = Tcl_CreateNamespace(interp, "::itclwidget::internal", NULL, NULL);
    if (nsPtr == NULL) {
        Tcl_Panic("Itcl: cannot create namespace: \"%s\" \n", "::itclwidget::internal");
    }

    Tcl_RenameCommand(interp, "::itcl::type", "::itcl::__type");
    Tcl_RenameCommand(interp, "::itcl::widget", "::itcl::__widget");
    Tcl_RenameCommand(interp, "::itcl::widgetadaptor",
            "::itcl::__widgetadaptor");
    infoPtr->windgetInfoPtr = (ItclWidgetInfo *)ckalloc(sizeof(ItclWidgetInfo));
    infoPtr->windgetInfoPtr->initObjectOpts = ItclInitObjectOptions;
    infoPtr->windgetInfoPtr->hullAndOptsInst = HullAndOptionsInstall;
    infoPtr->windgetInfoPtr->delegationInst = DelegationInstall;
    infoPtr->windgetInfoPtr->widgetConfigure = ItclWidgetConfigure;
    infoPtr->windgetInfoPtr->widgetCget = ItclWidgetCget;
    Itcl_WidgetParseInit(interp, infoPtr);

    /*
     *  Create "itcl::builtin" namespace for commands that
     *  are automatically built into class definitions.
     */
    if (Itcl_WidgetBiInit(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    /*
     *  Set up the variables containing version info.
     */

    Tcl_SetVar(interp, "::itclwidget::version", ITCL_VERSION, TCL_NAMESPACE_ONLY);
    Tcl_SetVar(interp, "::itclwidget::patchLevel", ITCL_PATCH_LEVEL,
            TCL_NAMESPACE_ONLY);


    /*
     *  Package is now loaded.
     */

    return Tcl_PkgProvideEx(interp, "ItclWidget", ITCL_VERSION, &itclStubAPI);
}

/*
 * ------------------------------------------------------------------------
 *  ItclWidget_Init()
 *
 *  Invoked whenever a new INTERPRETER is created to install the
 *  [incr Tcl] package.  Usually invoked within Tcl_AppInit() at
 *  the start of execution.
 *
 *  Creates the "::itcl" namespace and installs access commands for
 *  creating classes and querying info.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */

int
Itclwidget_Init (
    Tcl_Interp *interp)
{
    if (Initialize(interp) != TCL_OK) {
        return TCL_ERROR;
    }

    return  TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  ItclWidget_SafeInit()
 *
 *  Invoked whenever a new SAFE INTERPRETER is created to install
 *  the [incr Tcl] package.
 *
 *  Creates the "::itcl" namespace and installs access commands for
 *  creating classes and querying info.
 *
 *  Returns TCL_OK on success, or TCL_ERROR (along with an error
 *  message in the interpreter) if anything goes wrong.
 * ------------------------------------------------------------------------
 */

int
Itclwidget_SafeInit (
    Tcl_Interp *interp)
{
    if (Initialize(interp) != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}
