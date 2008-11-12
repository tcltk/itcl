/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  [incr Tcl]
 *  DESCRIPTION:  Object-Oriented Extensions to Tcl
 *
 * Implementation of objects for package ItclWidget
 *
 * This implementation is based mostly on the ideas of snit
 * whose author is William Duquette.
 *
 * ========================================================================
 *  Author: Arnulf Wiedemann
 *
 *     RCS:  $Id: itclWidgetObject.c,v 1.1.2.5 2008/11/12 21:31:42 wiede Exp $
 * ========================================================================
 *           Copyright (c) 2007 Arnulf Wiedemann
 * ------------------------------------------------------------------------
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */
#include "itclWidgetInt.h"
#include <tk.h>


/*
 * ------------------------------------------------------------------------
 *  ItclWidgetInitObjectOptions()
 *
 *  Init all instance options 
 *  This is usually invoked automatically
 *  by Itcl_CreateObject(), when an object is created.
 * ------------------------------------------------------------------------
 */
int
ItclWidgetInitObjectOptions(
   Tcl_Interp *interp,
   ItclObject *ioPtr,
   ItclClass *iclsPtr,
   const char *name)
{
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch place;
    Tk_Window tkMainWin;
    Tk_Window tkWin;
    ItclOption *ioptPtr;
    const char *val;

    /* initialize the options array */
    tkMainWin = Tk_MainWindow(interp);
    const char *widgetName;
    widgetName = Tcl_GetString(ioPtr->hullWindowNamePtr);
    tkWin = Tk_NameToWindow(interp, widgetName, tkMainWin);
    if (tkWin == NULL) {
/* seems to be no widget */
Tcl_ResetResult(interp);
return TCL_OK;
    }
    if (tkWin == NULL) {
        Tcl_AppendResult(interp, "window for widget \"",
	        Tcl_GetString(ioPtr->namePtr), "\" not found", NULL);
	return TCL_ERROR;
    }
    hPtr = Tcl_FirstHashEntry(&iclsPtr->options, &place);
    while (hPtr) {
	ioptPtr = (ItclOption*)Tcl_GetHashValue(hPtr);
        val = Tk_GetOption(tkWin, Tcl_GetString(ioptPtr->resourceNamePtr),
	        Tcl_GetString(ioptPtr->classNamePtr));
	if (val != NULL) {
            val = ItclSetInstanceVar(interp, "itcl_options",
	            Tcl_GetString(ioptPtr->namePtr), val,
                    ioPtr, ioPtr->iclsPtr);
	} else {
	    if (ioptPtr->defaultValuePtr != NULL) {
                val = ItclSetInstanceVar(interp, "itcl_options",
	                Tcl_GetString(ioptPtr->namePtr),
		        Tcl_GetString(ioptPtr->defaultValuePtr),
                        ioPtr, ioPtr->iclsPtr);
	    }
	}
        hPtr = Tcl_NextHashEntry(&place);
    }
Tcl_ResetResult(interp);

    return TCL_OK;
}

/*
 * ------------------------------------------------------------------------
 *  HullAndOptionsInstall()
 * ------------------------------------------------------------------------
 */

int
HullAndOptionsInstall(
    Tcl_Interp *interp,
    ItclObject *ioPtr,
    ItclClass *iclsPtr,
    int objc,
    Tcl_Obj * const objv[],
    int *newObjc,
    Tcl_Obj **newObjv)
{
    Tcl_Obj *widgetClassPtr;
    Tcl_Obj **hullObjv;
    char *token;
    const char *hullType;
    int hullObjc;
    int foundWclass;
    int result;
    int i;

    ItclShowArgs(1, "HullAndOptionsInstall", objc, objv);
#ifdef NOTDEF
// options are initialized in Itcl_BiInstallHullCmd!!
    FOREACH_HASH_VALUE(ioptPtr, &iclsPtr->options) {
	if (ioptPtr->defaultValuePtr != NULL) {
	    ItclSetInstanceVar(interp, "itcl_options",
	            Tcl_GetString(ioptPtr->namePtr),
		    Tcl_GetString(ioptPtr->defaultValuePtr), ioPtr, iclsPtr);
	}
    }
#endif
    widgetClassPtr = iclsPtr->widgetClassPtr;
    foundWclass = 0;
    iclsPtr->infoPtr->buildingWidget = 1;
    iclsPtr->infoPtr->currIoPtr = ioPtr;
    for (i = 0; i < objc; i++) {
        token = Tcl_GetString(objv[i]);
	if ((*token == '-') && (strcmp(token, "-class") == 0)) {
	    /* check if it is in the -option position */
	    if (((i % 2) == 0) && (i + 1 <= objc)) {
		widgetClassPtr = objv[i+1];
		foundWclass = 1;
#ifdef NOTDEF 
// FIXME are these some options for the installhull command?
	        newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*(objc-1));
		*newObjc = objc - 2;
		memcpy(newObjv, objv, i * sizeof(Tcl_Obj *));
		if (objc-i-2 > 0) {
		    memcpy(newObjv+i, objv+i+2, (objc-i-2)*sizeof(Tcl_Obj *));
		}
#endif
	    }
	}
    }
    if (widgetClassPtr == NULL) {
	char buf[2];
	char *cp;
	cp = Tcl_GetString(iclsPtr->namePtr);
        widgetClassPtr = Tcl_NewStringObj("", -1);
	buf[0] = toupper(*cp);
	buf[1] = '\0';
	Tcl_AppendToObj(widgetClassPtr, buf, -1);
	Tcl_AppendToObj(widgetClassPtr, cp+1, -1);
	Tcl_IncrRefCount(widgetClassPtr);
    }
    hullObjc = 5;
    hullObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *)*hullObjc);
    hullObjv[0] = Tcl_NewStringObj("installhull", -1);
    Tcl_IncrRefCount(hullObjv[0]);
    hullObjv[1] = Tcl_NewStringObj("using", -1);
    Tcl_IncrRefCount(hullObjv[1]);
    if (iclsPtr->flags & ITCL_WIDGET_FRAME) {
        hullType = "frame";
    }
    if (iclsPtr->flags & ITCL_WIDGET_LABEL_FRAME) {
        hullType = "labelframe";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TOPLEVEL) {
        hullType = "toplevel";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TTK_FRAME) {
        hullType = "ttk::frame";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TTK_LABEL_FRAME) {
        hullType = "ttk::labelframe";
    }
    if (iclsPtr->flags & ITCL_WIDGET_TTK_TOPLEVEL) {
        hullType = "ttk::toplevel";
    }
    hullObjv[2] = Tcl_NewStringObj(hullType, -1);
    Tcl_IncrRefCount(hullObjv[2]);
    hullObjv[3] = Tcl_NewStringObj("-class", -1);
    Tcl_IncrRefCount(hullObjv[3]);
    hullObjv[4] = Tcl_NewStringObj(Tcl_GetString(widgetClassPtr), -1);
    Tcl_IncrRefCount(hullObjv[4]);
    ItclShowArgs(1, "installhull", hullObjc, hullObjv);
    result = Itcl_BiInstallHullCmd(iclsPtr->infoPtr, interp,
            hullObjc, hullObjv);
    Tcl_DecrRefCount(hullObjv[0]);
    Tcl_DecrRefCount(hullObjv[1]);
    Tcl_DecrRefCount(hullObjv[2]);
    Tcl_DecrRefCount(hullObjv[3]);
    Tcl_DecrRefCount(hullObjv[4]);
    iclsPtr->infoPtr->buildingWidget = 0;
    ItclShowArgs(1, "HullAndOptionsInstall END", objc, objv);
    return result;
}

/*
 * ------------------------------------------------------------------------
 *  InstallComponent()
 * ------------------------------------------------------------------------
 */

int
InstallComponent(
    Tcl_Interp *interp,
    ItclObject *ioPtr,
    ItclClass *iclsPtr,
    int objc,
    Tcl_Obj * const objv[])
{
    FOREACH_HASH_DECLS;
    Tk_Window tkMainWin;
    Tk_Window tkWin;
    Tcl_Obj ** newObjv;
    ItclDelegatedOption *idoPtr;
    const char *componentName;
    const char *componentValue;
    const char *usageStr;
    const char *widgetName;
    const char *val;
    int i;
    int j;
    int numOpts;
    int result;

    ItclShowArgs(1, "InstallComponent", objc, objv);
    usageStr = "usage: installcomponent <componentName> using <widgetType> <widgetPath> ?-option value ...?";
    if (objc < 4) {
        Tcl_AppendResult(interp, usageStr, NULL);
	return TCL_ERROR;
    }
    if (strcmp(Tcl_GetString(objv[2]), "using") != 0) {
        Tcl_AppendResult(interp, usageStr, NULL);
	return TCL_ERROR;
    }
    componentName = Tcl_GetString(objv[1]);
    val = ItclGetInstanceVar(interp, "itcl_hull", NULL, ioPtr, iclsPtr);
    if ((val != NULL) && (strlen(val) == 0)) {
	Tcl_AppendResult(interp, "cannot install \"", componentName,
	        " before \"itcl_hull\" exists", NULL);
        return TCL_ERROR;
    }
    /* check for delegated option and ask the option database for the values */
    /* first check for number of delegated options */
    numOpts = 0;
    FOREACH_HASH_VALUE(idoPtr, &ioPtr->objectDelegatedOptions) {
        numOpts++;
    }
    widgetName = Tcl_GetString(ioPtr->namePtr);
    tkMainWin = Tk_MainWindow(interp);
    tkWin = Tk_NameToWindow(interp, widgetName, tkMainWin);
    if (tkWin == NULL) {
        Tcl_AppendResult(interp, "InstallComponent: cannot get window",
                " info for \"", widgetName, "\"", NULL);
        return TCL_ERROR;
    }
    newObjv = (Tcl_Obj **)ckalloc(sizeof(Tcl_Obj *) *
           (objc - 3 + (numOpts * 2)));
    /* insert delegated options before any options on the command line */
    for (j = 3; j < objc; j++) {
	if (*Tcl_GetString(objv[j]) == '-') {
	   break;
	}
        newObjv[j - 3] = objv[j];
    }
    i = j - 3;
    FOREACH_HASH_VALUE(idoPtr, &ioPtr->objectDelegatedOptions) {
	val = Tk_GetOption(tkWin,
	        Tcl_GetString(idoPtr->resourceNamePtr),
	        Tcl_GetString(idoPtr->classNamePtr));
	if (val == NULL) {
	    Tcl_AppendResult(interp, "cannot get option \"",
	            Tcl_GetString(idoPtr->resourceNamePtr),
		    "\"", NULL);
	}
	if (idoPtr->asPtr != NULL) {
            newObjv[i] = idoPtr->asPtr;
	} else {
            newObjv[i] = idoPtr->namePtr;
	}
	Tcl_IncrRefCount(newObjv[i]);
	i++;
        newObjv[i] = Tcl_NewStringObj(val, -1);
	Tcl_IncrRefCount(newObjv[i]);
	i++;
        
    }
    for ( ;j < objc; j++) {
        newObjv[i] = objv[j];
        i++;
    }
    ItclShowArgs(0, "InstallComponent", objc - 3 + (numOpts * 2), newObjv);
    result = Tcl_EvalObjv(interp, objc - 3 + (numOpts * 2), newObjv, 0);
    if (result != TCL_OK) {
        return result;
    }
    componentValue = Tcl_GetStringResult(interp);
    Tcl_Obj *objPtr;
    objPtr = Tcl_NewStringObj(ITCL_VARIABLES_NAMESPACE, -1);
    Tcl_AppendToObj(objPtr, Tcl_GetString(iclsPtr->fullNamePtr), -1);
    Tcl_AppendToObj(objPtr, "::", -1);
    Tcl_AppendToObj(objPtr, componentName, -1);

    Tcl_SetVar2(interp, Tcl_GetString(objPtr), NULL, componentValue, 0);
//    ItclSetInstanceVar(interp, componentName, NULL, componentValue, ioPtr, iclsPtr);
    return TCL_OK;
}
